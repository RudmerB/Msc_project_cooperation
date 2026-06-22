#pragma once 

#include<array>
#include<string>
using namespace std;

//--- Parameters ---
constexpr int       nrIndividuals           = 200;              // Nr of individuals in one population
constexpr int       nrStrategies            = 12;               // Total number of social strategies (Leading eight, image-scoring etc.)
constexpr int       nrPermutationsMutant    = 132;              // Number of population permutations (number of unique strategy combinations)
constexpr int       nrPermutationsUpdating  = 66;
constexpr int       nrReplicas              = 1;

constexpr double    error1                  = 0.01;
constexpr double    error2                  = 0.05;
constexpr double    cost                    = 1.0;
constexpr char      startingReputation      = 'G';
constexpr double    startingPayOff          = 0.0;
constexpr double    baseFitness             = 100.0;
constexpr int       totalRounds             = 1000;

const array<string, 12> strategyNames = {"L1","L2","L3","L4","L5","L6","L7","L8","SH","I-S","ALLC","ALLD"};