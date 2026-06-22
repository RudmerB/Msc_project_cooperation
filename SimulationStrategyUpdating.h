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
                                const char startingReputation);

    void RunReplicas(const int replicasPerPop,
                     const double startingPayOff,
                     const char startingReputation,
                     const int totalRounds,
                     mt19937_64 &rng,
                     const double error1,
                     const double error2,
                     const double cost,
                     const double benefit,
                     const strategiesSocialNorms& socialNorms,
                     const strategiesActionRules& actionRules,
                     ofstream &outputFile);
private:
    //int nrIndStrategyAPostUpdate = 0;
    //int nrIndStrategyBPostUpdate = 0;
};


/*=============================       Complete simulation       ==============================*/
void SimulationStrategyUpdating(const string simulationType,
                                const double benefit,
                                const int totalRounds,
                               const array<array<int, nrStrategies>, nrPermutationsUpdating>& allPopulations,
                               const int nrThreads,
                               const strategiesSocialNorms& socialNorms,
                               const strategiesActionRules& actionRules);