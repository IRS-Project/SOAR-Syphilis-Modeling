#pragma once

#include "epiworld.hpp"

using namespace epiworld;

class ModelSyphilis: public Model<> {
private:

    // Agent update functions
    static void _update_susceptible(Agent<> * agent, Model<> * model);
    static void _update_exposed(Agent<> * agent, Model<> * model);
    static void _update_i_primary(Agent<> * agent, Model<> * model);
    static void _update_i_secondary(Agent<> * agent, Model<> * model);
    static void _update_i_latent(Agent<> * agent, Model<> * model);
    static void _update_i_tertiary(Agent<> * agent, Model<> * model);

public:

    // Disease states
    static const int SUSCEPTIBLE = 0;
    static const int EXPOSED = 1;
    static const int I_PRIMARY = 2;
    static const int I_SECONDARY = 3;
    static const int I_LATENT = 4;
    // The tertiary stage happens years later, so we will not
    // include it in the model for simplicity
    // static const int I_TERTIARY = 5;
    static const int RECOVERED = 5;

    /**
     * @brief Construct a new Model Syphilis object
      * 
      * @param prob_infection Probability of infection per contact
      * @param incubation_period Average incubation period in days
      * @param duration_primary Average duration of primary stage in days
      * @param duration_secondary Average duration of secondary stage in days
      * @param duration_latent Average duration of latent stage in days
      * Note: The tertiary stage is not included in this model for simplicity,
      * as it occurs years after the initial infection and is less relevant 
      * for short-term epidemic modeling
     */
    ModelSyphilis(
        epiworld_double prob_infection = 0.001,
        epiworld_double incubation_period = 21.0,
        epiworld_double duration_primary = 52.2,
        epiworld_double duration_secondary = 105.0,
        epiworld_double duration_latent = 365.0
    );

};

// Using a factory macro since these updates are very similar
// Individual functions can be defined if more complex dynamics
// are needed.
#define UPDATE_SYPHILIS(funname, param, nextstate) \
    inline void ModelSyphilis::_update_##funname( \
        Agent<> * agent, Model<> * model \
    ) { \
        if (agent->get_virus() == nullptr) { \
            throw std::runtime_error("Agent has no virus!"); \
        } \
        auto prob_transition = 1.0 / model->get_param(param); \
        if (model->runif() < prob_transition) { \
            agent->change_state(*model, nextstate); \
        } \
    }

UPDATE_SYPHILIS(exposed, "Incubation Period", I_PRIMARY);
UPDATE_SYPHILIS(i_primary, "Duration of Primary Stage", I_SECONDARY);
UPDATE_SYPHILIS(i_secondary, "Duration of Secondary Stage", I_LATENT);

inline void ModelSyphilis::_update_i_latent(Agent<> * agent, Model<> * model) {
    auto prob_transition = 1.0 / model->get_param("Duration of Latent Stage");
    if (model->runif() < prob_transition) {
        agent->rm_virus(*model, RECOVERED);
    }
}

inline ModelSyphilis::ModelSyphilis(
    epiworld_double prob_infection,
    epiworld_double incubation_period,
    epiworld_double duration_primary,
    epiworld_double duration_secondary,
    epiworld_double duration_latent
) {
    set_name("Syphilis Model");

    // Model states
    add_state("Susceptible", default_update_susceptible<>);
    add_state("Exposed", _update_exposed);
    add_state("I. Primary", _update_i_primary);
    add_state("I. Secondary", _update_i_secondary);
    add_state("I. Latent", _update_i_latent);
    add_state("Recovered");

    // Creating parameters
    add_param(prob_infection, "Prob. Infection");
    add_param(incubation_period, "Incubation Period");
    add_param(duration_primary, "Duration of Primary Stage");
    add_param(duration_secondary, "Duration of Secondary Stage");
    add_param(duration_latent, "Duration of Latent Stage");

    // Creating the virus
    Virus<> syphilis("Syphilis");
    syphilis.set_prob_infecting("Prob. Infection"); // Based on the model param
    syphilis.set_state(EXPOSED, RECOVERED);

    // The virus will be distributed randomly at the start of the
    // simulation, with 1 infected agent and the rest susceptible
    syphilis.set_distribution(distribute_virus_randomly(1, false));

    add_virus(syphilis);

}