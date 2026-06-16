#pragma once 

//--- Parameters ---
constexpr int       nrIndividuals           = 200;              // Nr of individuals in one population
constexpr int       nrStrategies            = 12;               // Total number of social strategies (Leading eight, image-scoring etc.)
constexpr int       nrPermutationsMutant    = 132;              // Number of population permutations (number of unique strategy combinations)
constexpr int       nrPermutationsUpdating  = 66;
constexpr int       nrReplicas              = 4;

constexpr double    error1                  = 0.01;
constexpr double    error2                  = 0.1;
constexpr double    cost                    = 1.0;
constexpr char      startingReputation      = 'G';
constexpr double    startingPayOff          = 0.0;
constexpr int       totalRounds             = 10;