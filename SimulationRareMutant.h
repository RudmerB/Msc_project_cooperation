#pragma once

// Header Files
#include "Parameters.h"
#include "BasePopulation.h"
#include "Strategies.h"
using namespace std;

/*----------------------------         Results (struct)            ---------------------------*/
struct StrategyPayOff{
    double mutantPayOff = 0.0;
    double avgResidentPayOff = 0.0;
};


/*==========================          Population (class)          ============================*/
class PopulationClassRareMutant : public BasePopulationClass{
public:
    PopulationClassRareMutant(const strategiesSocialNorms& socialNorms,                       // Population class constructor
                    const strategiesActionRules& actionRules,
                    const array<int, nrStrategies>& nrIndividualsPerStrategy,
                    const char startingReputation,
                    const double startingPayOff);
    
    StrategyPayOff DetermineStrategyPayOff();                                       // Give both the pay offs of the mutant and the average resident pay off

    void RunReplicas(const int replicasPerPop,
                     const double startingPayOff,
                     const char startingReputations,
                     const int totalRounds,
                     mt19937_64 &rng,
                     const double error1,
                     const double error2,
                     const double cost,
                     const double benefit,
                     ofstream &outputFile);
};

/*=============================       Complete simulation       ==============================*/
void SimulationRareMutant(const string simulationType,
                          const double benefit,
                          const array<array<int, nrStrategies>, nrPermutationsMutant>& allPopulations,
                          const int nrThreads,
                          const strategiesSocialNorms& socialNorms,
                          const strategiesActionRules& actionRules);