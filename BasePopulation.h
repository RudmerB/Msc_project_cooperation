#pragma once

// Libraries
#include <array>
#include <algorithm>
#include <random>

// Header Files
#include "Parameters.h"
using namespace std;

/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
--------------------------         Individual (struct)            --------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
struct BaseIndividual{
    array<char, 8> socialNorm;                                      // Each individual has a social norm (how he updates reputations when he is observer)
    array<char, 4> actionRule;                                      // Each individual has action rules (what to do when he himself is actor)
    array<char, nrIndividuals> privateReputations;                  // Each individual can store the reputation of each other individual 
    double PayOff;                                                  // Each individual has a pay-off
    bool isResident = false;   

    BaseIndividual() = default;                                         // Basic constructor 
    BaseIndividual(const char& startingReputation,                      // Constructor
                   const double& startingPayOff);                                                 
};


/*==========================================================================================
============================================================================================
==========================          Population (class)          ============================
============================================================================================
==========================================================================================*/
class BasePopulationClass{
public:

    void printtest();                                                               // Print test (not used in current code 13/05)

    bool DetermineAction(const int donor,                                           // Function to determine action based on action rule
                         const int recipient,
                         uniform_real_distribution<double>& uniform,
                         mt19937_64& rng,
                         const double error1);

    void UpdatePayOff(const bool action,                                           // Update pay off based on the donor's action
                      const int donor,
                      const int recipient,
                      const double cost,
                      const double benefit);

    void UpdatePrivateReputations(const bool action,                               // Update pay offs based on social norms 
                                  const int donor,
                                  const int recipient,
                                  uniform_real_distribution<double>& uniform,
                                  mt19937_64& rng,
                                  const double error2);

    void PlayInteractionRound(mt19937_64 &rng,                                      // Play one round of interactions = 
                              const double error1,                                  // Everyone is in an interaction once, either as donor or recipient
                              const double error2,                                  // Meaning nrIndividuals / 2 interactions are played 
                              const double cost,
                              const double benefit);
    
    void ResetIndividuals(const double startingPayOff,
                          const char stratingReputation);

protected:
    array <BaseIndividual, nrIndividuals> population;                                    // Store the individuals in an array
};


