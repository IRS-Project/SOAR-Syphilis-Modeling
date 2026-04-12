"""Run syphilis simulation and summarize results into quantiles."""

import argparse
import csv
import glob
import os
import shutil
import subprocess
import sys
from collections import defaultdict

QUANTILES = [round(q / 100, 2) for q in range(5, 100, 5)]  # 0.05, 0.10, ..., 0.95


def run_simulation(args):
    """Run main.o with the given parameters."""
    cmd = [
        "./main.o",
        str(args.prob_infection),
        str(args.incubation_period),
        str(args.duration_primary),
        str(args.duration_secondary),
        str(args.duration_latent),
        str(args.n_runs),
    ]
    print(f"Running: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)


def parse_results(results_dir):
    """Parse all result CSVs into {(day, state): [counts across runs]}."""
    data = defaultdict(list)
    files = sorted(glob.glob(os.path.join(results_dir, "*_total_hist.csv")))
    if not files:
        print(f"No result files found in {results_dir}", file=sys.stderr)
        sys.exit(1)

    for f in files:
        with open(f) as fh:
            reader = csv.reader(fh, delimiter=" ")
            next(reader)  # skip header
            for row in reader:
                day, _, state, count = row
                data[(int(day), state)].append(int(count))

    return data


def compute_quantiles(values, quantiles):
    """Compute quantiles using linear interpolation."""
    s = sorted(values)
    n = len(s)
    result = []
    for q in quantiles:
        pos = q * (n - 1)
        lo = int(pos)
        hi = min(lo + 1, n - 1)
        frac = pos - lo
        result.append(s[lo] + frac * (s[hi] - s[lo]))
    return result


def clean_results(results_dir):
    """Remove old simulation CSVs (not summary)."""
    for f in glob.glob(os.path.join(results_dir, "*_total_hist.csv")):
        os.remove(f)


def main():
    parser = argparse.ArgumentParser(description="Run syphilis model and summarize results")
    parser.add_argument("--prob-infection", type=float, default=0.001)
    parser.add_argument("--incubation-period", type=float, default=21.0)
    parser.add_argument("--duration-primary", type=float, default=52.2)
    parser.add_argument("--duration-secondary", type=float, default=105.0)
    parser.add_argument("--duration-latent", type=float, default=365.0)
    parser.add_argument("--n-runs", type=int, default=100)
    parser.add_argument("--output", type=str, default="results/summary.csv")
    args = parser.parse_args()

    # Ensure results directory exists
    os.makedirs(os.path.dirname(args.output), exist_ok=True)

    # Clean old run files
    clean_results("results")

    # Run simulation
    run_simulation(args)

    # Parse and summarize
    data = parse_results("results")

    header = ["date", "state", "quantile", "value"]

    rows = []
    for (day, state) in sorted(data.keys()):
        counts = data[(day, state)]
        qs = compute_quantiles(counts, QUANTILES)
        for q, v in zip(QUANTILES, qs):
            rows.append([day, state, q, v])

    with open(args.output, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        for row in rows:
            writer.writerow(row)

    # Clean up per-run files, keep only summary
    clean_results("results")

    print(f"Wrote {len(rows)} rows to {args.output}")


if __name__ == "__main__":
    main()
