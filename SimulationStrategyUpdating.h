#pragma once

// Header Files
#include "Parameters.h"
#include "BasePopulation.h"
#include "Strategies.h"
using namespace std;


/*----------------------------         Results (struct)            ---------------------------*/
struct Strategies{
    int strategyAIndex = -1;                        // Used to identify unique strategies, values in nrIndividualsPerStrategy cannot ...
    int strategyBIndex = -1;                        // ...be below zero, therefore Index = -1 (becomes clear in StrategyUpdating.cpp)
    int nrIndStrategyA = 0;
    int nrIndStrategyB = 0;
    double avgStrategyAPayOff = 0.0;
    double avgStrategyBPayOff = 0.0;
};


/*==========================          Population (class)          ============================*/
class PopulationClassStrategyUpdating : public BasePopulationClass{
public:
    PopulationClassStrategyUpdating(const strategiesSocialNorms& socialNorms,                       // Population class constructor
                    const strategiesActionRules& actionRules,
                    const array<int, nrStrategies>& nrIndividualsPerStrategy,
                    const char startingReputation,
                    const double startingPayOff);

    Strategies StrategiesStats;

    void DetermineStrategyPayOff();

    void RunReplicatorDynamics();

    void ConstructNewGeneration(const strategiesSocialNorms& socialNorms,
                                const strategiesActionRules& actionRules,
                                const double startingPayOff,
                                const char startingReputation,
                                const int minNrInd);

    void RunReplicas(
                     mt19937_64 &rng,
                     const int nrThreads,
                     const int baseSeed,
                     array <int, nrStrategies> nrIndividualPerStrategy,
                     const double benefit,
                     const strategiesSocialNorms& socialNorms,
                     const strategiesActionRules& actionRules,
                     ofstream &outputFile);
};


/*=============================       Complete simulation       ==============================*/
void SimulationStrategyUpdating(const string simulationType,
                                const double benefit,
                                const int totalRoundsUpdating,
                               const array<array<int, nrStrategies>, nrPermutationsUpdating>& allPopulations,
                               const int nrThreads,
                               const strategiesSocialNorms& socialNorms,
                               const strategiesActionRules& actionRules);