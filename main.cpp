#include "syphilis.hpp"

int main() {

    // Creating an empty model
    ModelSyphilis model{};

    // Adding agents from small-world network
    model.agents_smallworld(100'000, 10, 0.1);

    // Runing a single simulation for two years
    model.run(365 * 2, 22);

    // We can also run this multiple times
    auto saver = make_save_run(
        "results/%03lu-episimulation.csv",
        true // Total history
        // We could save lots of other things,
        // but are skipping for simplicity
    );

    model.run_multiple(365 * 2, 100, 22, saver, true, true, 4);

    // Showing what we have so far
    model.print();

    return 0;
}