#pragma once 

#include<array>
#include<string>
using namespace std;

//---Strategy Names---
const array<string, 12> strategyNames = {"L1","L2","L3","L4","L5","L6","L7","L8","SH","I-S","ALLC","ALLD"};

//--- Global Parameters ---
constexpr int       nrIndividuals           = 200;              // Nr of individuals in one population
constexpr int       nrStrategies            = 12;               // Total number of social strategies (Leading eight, image-scoring etc.)
constexpr double    error1                  = 0.01;             // Errors in actions
constexpr double    error2                  = 0.05;             // Errors in reputations updating
constexpr double    cost                    = 1.0;              // Cost of cooperation to donor
constexpr char      startingReputation      = 'G';              // Starting private reputation of others
constexpr double    startingPayOff          = 0.0;              // Starting payoff

//---Specific to Rare Mutant Simulation---
constexpr int       nrPermutationsMutant    = 132;              // Number of population permutations (132)
constexpr int       nrReplicasMutant        = 1000;             // Number of replicates for specific population and parameters (1000)
constexpr int       totalRoundsMutant       = 200;              // Number of rounds to complete replicate (200)

//---Specific to Strategy Updating Simulation---
constexpr int       nrPermutationsUpdating  = 66;               // Number of population permutations (66)
constexpr int       nrReplicasUpdating      = 100;              // Number of replicates for specific population and parameters (?)
constexpr int       nrReplicasUpdatingTotal = 400;              // Test
constexpr double    baseFitness             = 100.0;            // Base fitness of each individual (used for replicator dynamics, influenced by payoff) (100)
constexpr int       totalRoundsUpdating     = 100000;           // Number of rounds to complete replicate (100000)
constexpr int       nrRoundsToUpdateEvent   = 100;              // Number of rounds between strategy updating events / constructing new generation (100)
constexpr int       minNrInd                = 5;