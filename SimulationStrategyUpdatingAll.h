#pragma once

// Header Files
#include "Parameters.h"
#include "BasePopulation.h"
#include "Strategies.h"
using namespace std;


/*----------------------------         Results (struct)            ---------------------------*/
struct StrategiesAll{
    vector<int> strategiesIndex;
    vector<int> nrIndPerStrategy;
    vector<double> avgPayOffPerStrategy;

    StrategiesAll()
        : strategiesIndex{0,1,2,3,4,5,6,7,8,9,10,11},
          nrIndPerStrategy(12, 0),
          avgPayOffPerStrategy(12, 0.0)
    {}
};

/*==========================          Population (class)          ============================*/
class PopulationClassStrategyUpdatingAllStrats : public BasePopulationClass{
public:
    PopulationClassStrategyUpdatingAllStrats(const strategiesSocialNorms& socialNorms,                       // Population class constructor
                    const strategiesActionRules& actionRules,
                    const array<int, nrStrategies>& nrIndividualsPerStrategy,
                    const char startingReputation,
                    const double startingPayOff);

    StrategiesAll StrategiesStats;

    void DetermineStrategyPayOff();

    void RunReplicatorDynamics();

    void ConstructNewGeneration(const strategiesSocialNorms& socialNorms,
                                const strategiesActionRules& actionRules,
                                const double startingPayOff,
                                const char startingReputation);

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
void SimulationStrategyUpdatingAllStrats(const string simulationType,
                                const double benefit,
                                const int totalRoundsUpdating,
                               const array<int, nrStrategies> nrIndividualsPerStrategy,
                               const int nrThreads,
                               const strategiesSocialNorms& socialNorms,
                               const strategiesActionRules& actionRules);