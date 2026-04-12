#include "syphilis.hpp"
#include <cstdlib>

int main(int argc, char* argv[]) {

    // Parse parameters (with defaults)
    float prob_infection     = argc > 1 ? std::atof(argv[1]) : 0.001;
    float incubation_period  = argc > 2 ? std::atof(argv[2]) : 21.0;
    float duration_primary   = argc > 3 ? std::atof(argv[3]) : 52.2;
    float duration_secondary = argc > 4 ? std::atof(argv[4]) : 105.0;
    float duration_latent    = argc > 5 ? std::atof(argv[5]) : 365.0;
    int n_runs               = argc > 6 ? std::atoi(argv[6]) : 100;

    // Creating the model with parameters
    ModelSyphilis model{
        prob_infection,
        incubation_period,
        duration_primary,
        duration_secondary,
        duration_latent
    };

    // Adding agents from small-world network
    model.agents_smallworld(100'000, 10, 0.1);

    // Runing a single simulation for two years
    model.run(365 * 2, 22);

    // We can also run this multiple times
    auto saver = make_save_run(
        "results/%03lu-episimulation.csv",
        true // Total history
    );

    model.run_multiple(365 * 2, n_runs, 22, saver, true, true, 4);

    // Showing what we have so far
    model.print();

    return 0;
}
