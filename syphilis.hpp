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
    static void _update_i_under_treatment(Agent<> * agent, Model<> * model);

    // Bool indicating agents that will get treatment
    std::vector< bool > _will_get_treatment;
    std::vector< int > _treatment_start_day;

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
    static const int I_LATENT_TERMINAL = 5;
    static const int I_UNDER_TREATMENT = 6;

    /**
     * @brief Construct a new Model Syphilis object
      * 
      * @param prob_infection Probability of infection per contact
      * @param incubation_period Average incubation period in days
      * @param duration_primary Average duration of primary stage in days
      * @param duration_secondary Average duration of secondary stage in days
      * @param duration_latent Average duration of latent stage in days
      * @param duration_of_treatment Fixed number of days for treatment.
      * @param days_to_detect Average number of days to detect an infection (if 
      * included) (geometrically distributed).
      * @param proportion_will_get_treatment Proportion of infected individuals 
      * that will receive treatment.
      * Note: The tertiary stage is not included in this model for simplicity,
      * as it occurs years after the initial infection and is less relevant 
      * for short-term epidemic modeling
     */
    ModelSyphilis(
        epiworld_double prob_infection = 0.001,
        epiworld_double incubation_period = 21.0,
        epiworld_double duration_primary = 52.2,
        epiworld_double duration_secondary = 105.0,
        epiworld_double duration_latent = 365.0,
        epiworld_double duration_of_treatment = 21.0,
        epiworld_double days_to_detect = 14.0,
        epiworld_double proportion_will_get_treatment = 2.0/3.0
    );

    void reset() override;

    std::unique_ptr<Model<>> clone_ptr() override;

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
UPDATE_SYPHILIS(i_secondary, "Duration of Secondary Stage", I_LATENT);
UPDATE_SYPHILIS(i_latent, "Duration of Latent Stage", I_LATENT_TERMINAL);

inline void ModelSyphilis::_update_i_primary(Agent<> * agent, Model<> * model) {

    auto * model_s = model_cast<ModelSyphilis,int>(model);

    // Logic to move the agent to the treatment stage
    if (model_s->_will_get_treatment[agent->get_id()]) 
    {
        double p_detect = 1.0 / model->par("Days to Detect Infection");
        if (model->runif() < p_detect)
        {
            model_s->_treatment_start_day[agent->get_id()] = model->today();
            agent->change_state(*model, I_UNDER_TREATMENT);
            return;
        }
    }

    // In case the agent is not detected, we proceed with the normal 
    // progression of the disease
    auto prob_transition = 1.0 / model->get_param("Duration of Primary Stage");
    if (model->runif() < prob_transition) {
        agent->change_state(*model, I_SECONDARY);
    }

}

inline void ModelSyphilis::_update_i_under_treatment(
    Agent<> * agent,
    Model<> * model
) {

    // Just to recover the model
    auto* model_s = model_cast<ModelSyphilis,int>(model);

    // Computing how many days have passed since the treatment started
    int days_since_treatment = static_cast<int>(model->today()) -
        model_s->_treatment_start_day[agent->get_id()];

    // If the treatment duration has passed, we assume the agent is
    // cured and goes back to susceptible
    if (days_since_treatment >= model->get_param("Duration of Treatment")) {
        agent->rm_virus(*model, SUSCEPTIBLE);
    }
}


inline ModelSyphilis::ModelSyphilis(
    epiworld_double prob_infection,
    epiworld_double incubation_period,
    epiworld_double duration_primary,
    epiworld_double duration_secondary,
    epiworld_double duration_latent,
    epiworld_double duration_of_treatment,
    epiworld_double days_to_detect,
    epiworld_double proportion_will_get_treatment
) {
    set_name("Syphilis Model");

    // Model states
    add_state("Susceptible", default_update_susceptible<>);
    add_state("Exposed", _update_exposed);
    add_state("I. Primary", _update_i_primary);
    add_state("I. Secondary", _update_i_secondary);
    add_state("I. Latent", _update_i_latent);
    add_state("I. Latent Terminal");
    add_state("I. Under Treatment", _update_i_under_treatment);

    // Creating parameters
    add_param(prob_infection, "Prob. Infection");
    add_param(incubation_period, "Incubation Period");
    add_param(duration_primary, "Duration of Primary Stage");
    add_param(duration_secondary, "Duration of Secondary Stage");
    add_param(duration_latent, "Duration of Latent Stage");
    add_param(duration_of_treatment, "Duration of Treatment");
    add_param(days_to_detect, "Days to Detect Infection");
    add_param(proportion_will_get_treatment, "Proportion Will Get Treatment");

    // Creating the virus
    Virus<> syphilis("Syphilis");
    syphilis.set_prob_infecting("Prob. Infection"); // Based on the model param
    syphilis.set_state(EXPOSED, I_LATENT_TERMINAL);

    // The virus will be distributed randomly at the start of the
    // simulation, with 1 infected agent and the rest susceptible
    syphilis.set_distribution(distribute_virus_randomly(1, false));

    add_virus(syphilis);

}

inline void ModelSyphilis::reset() {

    Model<>::reset();

    // Randomizing who will get treatment among those infected (this is done
    // at the start of each simulation)
    _will_get_treatment.assign(size(), false);
    for (size_t i = 0; i < size(); ++i) {
        if (runif() < get_param("Proportion Will Get Treatment"))
            _will_get_treatment[i] = true;
    }

    if (_will_get_treatment.size() != size())
        throw std::logic_error("Will get treatment vector size doesn't match population size.");

    _treatment_start_day.assign(size(), -1);

}

inline std::unique_ptr<Model<>> ModelSyphilis::clone_ptr() {
    return std::make_unique<ModelSyphilis>(*this);
}
