#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <random>
#include <string>

#include "Parameters.h"
#include "Utilities.h"
#include "Strategies.h"
#include "BasePopulation.h"
#include "SimulationStrategyUpdating.h"     // for PopulationClassStrategyUpdating, GetSeedRNG, etc.

using namespace std;

// ---- Diagnostic-specific overrides (kept local, do not touch Parameters.h) ----
const int diagNrReplicas = 30;
const int diagTotalRounds = 1000000;         // 10x your usual totalRoundsUpdating, adjust as needed

struct DiagPair {
    string name;
    int idxA;
    int idxB;
    int countA;
    int countB;
};

void RunDiagnostic(const DiagPair& pairDef,
                    const strategiesSocialNorms& socialNorms,
                    const strategiesActionRules& actionRules,
                    mt19937_64& rng,
                    unsigned int baseSeed)
{
    cout << "Running diagnostic: " << pairDef.name << "\n";

    int nrUpdateEvents = diagTotalRounds / nrRoundsToUpdateEvent;

    array<int, nrStrategies> startPop{};                       // zero-init
    startPop[pairDef.idxA] = pairDef.countA;
    startPop[pairDef.idxB] = pairDef.countB;

    ofstream freqFile("Diagnostic_" + pairDef.name + "_Freq.csv");
    ofstream payoffFile("Diagnostic_" + pairDef.name + "_Payoff.csv");
    freqFile << "Replicate";
    payoffFile << "Replicate";
    for (int e = 0; e < nrUpdateEvents; ++e) {
        freqFile << ",gen" << e;
        payoffFile << ",gen" << e << "_A,gen" << e << "_B";
    }
    freqFile << "\n";
    payoffFile << "\n";

    for (int rep = 0; rep < diagNrReplicas; ++rep) {
        mt19937_64 localRng(baseSeed + rep);

        PopulationClassStrategyUpdating localPopulation(socialNorms,
                                                        actionRules,
                                                        startPop,
                                                        startingReputation,
                                                        startingPayOff);

        vector<int> freqTrack(nrUpdateEvents, -1);              // -1 = not reached (fixation ended early)
        vector<pair<double,double>> payoffTrack(nrUpdateEvents, {-1.0, -1.0});

        int eventIndex = 0;
        bool stoppedEarly = false;
        double benefit = 1.05;

        for (int round = 1; round <= diagTotalRounds; ++round) {

            localPopulation.PlayInteractionRound(localRng, error1, error2, cost, benefit);

            if (round % nrRoundsToUpdateEvent == 0) {
                localPopulation.DetermineStrategyPayOff();

                freqTrack[eventIndex] = localPopulation.StrategiesStats.nrIndStrategyA;
                payoffTrack[eventIndex] = {
                    localPopulation.StrategiesStats.avgStrategyAPayOff,
                    localPopulation.StrategiesStats.avgStrategyBPayOff
                };

                if (localPopulation.StrategiesStats.nrIndStrategyA <= minNrInd ||
                        localPopulation.StrategiesStats.nrIndStrategyB <= minNrInd) {
                    stoppedEarly = true;
                    ++eventIndex;
                    break;                                       // stop; remaining slots stay -1
                }

                ++eventIndex;

                localPopulation.RunReplicatorDynamics();
                localPopulation.ConstructNewGeneration(socialNorms, actionRules,
                                                       startingPayOff, startingReputation);
            }
        }

        freqFile << rep;
        for (int e = 0; e < nrUpdateEvents; ++e) freqFile << "," << freqTrack[e];
        freqFile << "\n";

        payoffFile << rep;
        for (int e = 0; e < nrUpdateEvents; ++e) {
            payoffFile << "," << payoffTrack[e].first << "," << payoffTrack[e].second;
        }
        payoffFile << "\n";
    }

    freqFile.close();
    payoffFile.close();
}

int main()
{
    mt19937_64 rng;
    GetSeedRNG(rng);
    unsigned int baseSeed = rng();

    strategiesSocialNorms socialNorms;
    strategiesActionRules actionRules;

    // Fill in the correct indices for your strategy ordering (0-based, matching strategyNames)
    vector<DiagPair> pairs = {
        {"L8_vs_IS", 7, 9, 100, 100},     // adjust indices/counts as needed
        {"L6_vs_L7", 5, 6, 100, 100},
        {"SH_vs_IS", 8, 9, 100, 100}
    };

    for (const auto& p : pairs) {
        RunDiagnostic(p, socialNorms, actionRules, rng, baseSeed);
    }

    return 0;
}