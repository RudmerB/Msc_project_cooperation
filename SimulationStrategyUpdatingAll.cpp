// Libraries
#include <fstream>
#include <thread>
#include <iomanip>

// Header Files
#include "Parameters.h"
#include "BasePopulation.h"
#include "Utilities.h"
#include "SimulationStrategyUpdatingAll.h"
using namespace std;


/*==========================================================================================
============================================================================================
=====================         Population - Class Constructor         =======================
============================================================================================
==========================================================================================*/
PopulationClassStrategyUpdatingAllStrats::PopulationClassStrategyUpdatingAllStrats(const strategiesSocialNorms& socialNorms,
                                                                 const strategiesActionRules& actionRules,
                                                                 const array<int, nrStrategies>& nrIndividualsPerStrategy,
                                                                 const char startingReputation,
                                                                 const double startingPayOff)
{
    int populationIndex = 0;                        // Keeps tab on how many individuals have been assigned
    
    for(int i = 0; i < nrStrategies; ++i) {
        for(int count = 0; count < nrIndividualsPerStrategy[i]; ++count) {
            if (populationIndex >= nrIndividuals) {
                throw logic_error("The index counts a larger population "
                                "than the real population size.\n");
            }

            population[populationIndex] = BaseIndividual(startingReputation, startingPayOff);
            population[populationIndex].socialNorm  = socialNorms.socialNorm[i];
            population[populationIndex].actionRule  = actionRules.actionRule[i];
            population[populationIndex].strategyLabel = 'A' + i;

            ++StrategiesStats.nrIndPerStrategy[i];
            ++populationIndex;
        }
    }

    if (populationIndex != nrIndividuals) {                                             // Safety check: 'Did we assign the correct number of individuals?'
        throw logic_error("The number of assigned individuals does not "
                          "equal the population size.\n");
    }
    
    int totalAssigned = 0;
    for (int i = 0; i < nrStrategies; ++i) {
        totalAssigned += StrategiesStats.nrIndPerStrategy[i];
    }

    if (totalAssigned != nrIndividuals) {                                    // Safety check: Is the number assigned equal to the total?
        throw logic_error("The number of assigned individuals does not "
                          "equal the population size.\n");
    }
}

/*==========================================================================================
============================================================================================
=====================        Payoff       =======================
============================================================================================
==========================================================================================*/


void PopulationClassStrategyUpdatingAllStrats::DetermineStrategyPayOff()
{
    vector<double> vStrategiesSum(nrStrategies, 0.0);      // also: this should be double, not int

    for (int i = 0; i < nrIndividuals; ++i) {                          // Loop over all individuals
        int strategyIdx = population[i].strategyLabel - 'A';          // Convert label back to strategy index ??????? I dont undestand //FIXME

        if (strategyIdx < 0 || strategyIdx >= nrStrategies) {         // Check if individual has a valid label
            throw logic_error("DetermineStrategyPayOff() sees individuals without labels");
        }

        vStrategiesSum[strategyIdx] += population[i].PayOff;          // Add their payoff to the sum for their strategy
    }

    for (int i = 0; i < nrStrategies; ++i) {
        if (StrategiesStats.nrIndPerStrategy[i] == 0) {
            StrategiesStats.avgPayOffPerStrategy[i] = 0.0;
        } else {
            StrategiesStats.avgPayOffPerStrategy[i] = vStrategiesSum[i] / StrategiesStats.nrIndPerStrategy[i];
        }
    }
}

/*==========================================================================================
============================================================================================
=============================       Replicator Dynamics       ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdatingAllStrats::RunReplicatorDynamics()
{
    vector<double> vFractionPerStrategy(nrStrategies, 0.0);
    for (int i = 0; i < nrStrategies; ++i) {
        vFractionPerStrategy[i] = StrategiesStats.nrIndPerStrategy[i] / static_cast<double>(nrIndividuals);
    }

    vector<double> vMeanFitnessPerStrategy(nrStrategies, 0.0);
    for (int i = 0; i < nrStrategies; ++i) {
        vMeanFitnessPerStrategy[i] = baseFitness + StrategiesStats.avgPayOffPerStrategy[i];
    }

    vector<double> vProductPerStrategy(nrStrategies, 0.0);
    for (int i = 0; i < nrStrategies; ++i) {
        vProductPerStrategy[i] = vFractionPerStrategy[i] * vMeanFitnessPerStrategy[i];
    }

    double meanFitnessTotal = 0.0;
    for (int i = 0; i < nrStrategies; ++i) {
        meanFitnessTotal += vProductPerStrategy[i];
    }

    vector<double> vNewFractionPerStrategy(nrStrategies, 0.0);
    for (int i = 0; i < nrStrategies; ++i) {
        if (StrategiesStats.nrIndPerStrategy[i] == 0) {                          // Extinct strategies stay extinct
            vNewFractionPerStrategy[i] = 0.0;
            continue;
        }
        
        vNewFractionPerStrategy[i] = vFractionPerStrategy[i] * (vMeanFitnessPerStrategy[i] / meanFitnessTotal);
        if (vNewFractionPerStrategy[i] < 0.0 || vNewFractionPerStrategy[i] > 1.0) {
            cerr << "\nvNewFractionPerStrategy took on an impossible value in RunReplicatorDynamics()\n";
            exit(EXIT_FAILURE);
        }
    }

    vector<int> newCounts(nrStrategies, 0);
    vector<double> rawCounts(nrStrategies, 0.0);
    int assignedTotal = 0;

    for (int i = 0; i < nrStrategies; ++i) {
        rawCounts[i] = vNewFractionPerStrategy[i] * nrIndividuals;
        newCounts[i] = static_cast<int>(floor(rawCounts[i]));   // round down first
        assignedTotal += newCounts[i];
    }

    int remainder = nrIndividuals - assignedTotal;               // individuals still to assign

    // Give the leftover individuals to the strategies with the largest fractional remainders
    vector<int> order(nrStrategies);
    for (int i = 0; i < nrStrategies; ++i) {
        order[i] = i;
    }

    sort(order.begin(), order.end(), [&](int a, int b) {
        return (rawCounts[a] - newCounts[a]) > (rawCounts[b] - newCounts[b]);
    });

    int cappedRemainder = min(remainder, nrStrategies);
    for (int k = 0; k < cappedRemainder; ++k) {
        ++newCounts[order[k]];
    }

    int checkSum = 0;
    for (int i = 0; i < nrStrategies; ++i) checkSum += newCounts[i];
    if (checkSum != nrIndividuals) {
        cerr << "WARNING: newCounts sums to " << checkSum << ", expected " << nrIndividuals << "\n";
    }

    StrategiesStats.nrIndPerStrategy = newCounts;
}

/*==========================================================================================
============================================================================================
=============================        Strategy Updating        ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdatingAllStrats::ConstructNewGeneration(const strategiesSocialNorms& socialNorms,
                                                             const strategiesActionRules& actionRules,
                                                             const double startingPayOff,
                                                             const char startingReputation)
{

    vector<int> targetNrIndividuals (12,0);                 // Store traget nr of individuals in new generation
    vector<int> currentNrIndividuals (12,0);                // Store current nr of individuals that have been assigned either A or B
    for (int i = 0; i < nrStrategies; ++i) {
        targetNrIndividuals[i] = StrategiesStats.nrIndPerStrategy[i];
    }

    for (int i = 0; i < nrIndividuals; ++i) {                                   // Loop over individuals
        int strategyIdx = population[i].strategyLabel - 'A';                    // Convert this individual's label to a strategy index

        if (strategyIdx < 0 || strategyIdx >= nrStrategies) {
            throw logic_error("ConstructNewGeneration() sees individual without valid label");
        }

        if (currentNrIndividuals[strategyIdx] < targetNrIndividuals[strategyIdx]) {   // Their current strategy still has room
            ++currentNrIndividuals[strategyIdx];
            if (population[i].socialNorm != socialNorms.socialNorm[strategyIdx] ||
                population[i].actionRule != actionRules.actionRule[strategyIdx]) {
                throw logic_error("A labelled individual with mismatching socialnorm/actionrule has been found\n");
            }
        }

        else {                                                                 // Their current strategy is full; find one that isn't
            int newStrategyIdx = -1;
            for (int j = 0; j < nrStrategies; ++j) {
                if (currentNrIndividuals[j] < targetNrIndividuals[j]) {
                    newStrategyIdx = j;
                    break;
                }
            }
            
            if (newStrategyIdx == -1) {
                throw logic_error("ConstructNewGeneration() could not find a strategy with remaining capacity\n");
            }

            population[i].socialNorm    = socialNorms.socialNorm[newStrategyIdx];
            population[i].actionRule    = actionRules.actionRule[newStrategyIdx];
            population[i].strategyLabel = 'A' + newStrategyIdx;
            ++currentNrIndividuals[newStrategyIdx];
        }
    }

    for (int i = 0; i < nrStrategies; ++i) {                                    // Safety check: did we hit every target exactly?
        if (currentNrIndividuals[i] != targetNrIndividuals[i]) {
            throw logic_error("ConstructNewGeneration() failed to correctly reconstruct the population frequencies.\n");
        }
    }

    ResetIndividuals(startingPayOff, startingReputation);
}


/*==========================================================================================
============================================================================================
=============================       Simulation Replicas       ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdatingAllStrats::RunReplicas(
                                  mt19937_64 &rng,
                                  const int nrThreads,
                                  const int baseSeed, 
                                  array <int, nrStrategies> nrIndividualsPerStrategy,
                                  const double benefit,
                                  const strategiesSocialNorms& socialNorms,
                                  const strategiesActionRules& actionRules,
                                  ofstream &outputFile)
{   
    //--- Create results matrix ---
    int nrUpdateEvents = totalRoundsUpdating/nrRoundsToUpdateEvent;
    vector<vector<vector<int>>> results(
        nrReplicasUpdating,
        vector<vector<int>>(nrUpdateEvents + 1, vector<int>(nrStrategies, 0))       // added dimension compared to other simulation strategy
    );
    // results[replica][eventIndex][strategyIndex]

    auto start = chrono::high_resolution_clock::now();   

    //--- Run replicats ---
    for(int replica = 0; replica < nrReplicasUpdating; replica += nrThreads){               // Replica for-loop, jumps with nrThreads increments
        vector<thread> threads;                                                             // Store thread objects (all functions the thread needs)
        threads.reserve(nrThreads);                                                         // Reserve space for the vector based on nr of threads

        //--- Run simultaneous threads ---
        for(int t = 0; t < nrThreads && replica + t < nrReplicasUpdating; ++t) {            // Loop over threads in current batch
        //             ^                ^
        //             |                don't exceed total replicates (last batch may have fewer replicates than nrThreads)
        //             don't exceed usable threads

            int rep = replica + t;                                                          // Determine true replica index (replica=8 & t=2, means thread handles replica 10)
            threads.emplace_back([=, &results](){                                           // Construct thread obejct in vector
            //                    ^  ^
            //                    |  pass results by Reference (different threads need to right to same matrix)
            //                    pass local variables given by RunReplicas() by value
                try {
                    mt19937_64 localRng(baseSeed + rep);                                        // Create local random number generators

                    PopulationClassStrategyUpdatingAllStrats localPopulation(socialNorms,                // Create a population for each thread/replicate
                                                                    actionRules,
                                                                    nrIndividualsPerStrategy,
                                                                    startingReputation,
                                                                    startingPayOff);
                    int eventIndex = 0;                                                         // Initiate updating-event counter
                    results[rep][eventIndex++] =                                                // Write initial frequency to matrix 
                        localPopulation.StrategiesStats.nrIndPerStrategy;

                    //--- Running replicate ---
                    for(int round = 1; round <= totalRoundsUpdating; ++round){                  // Run each population for set amount fo rounds

                        localPopulation.PlayInteractionRound(localRng, error1, error2,          // Everyone has one interaction (either as donor or recipient)
                                                            cost, benefit);

                        //--- Strategy Updating ---
                        if(round % nrRoundsToUpdateEvent == 0){                                 // Check if round has an update event
                            localPopulation.DetermineStrategyPayOff();                          // Determine the payoff of both strategies
                            localPopulation.RunReplicatorDynamics();                            // Add payoff to fitness and determine new generation frequencies
                            localPopulation.ConstructNewGeneration(socialNorms, actionRules,    // Create the new generation (resets individuals within function)
                                                                startingPayOff, 
                                                                startingReputation);
                            results[rep][eventIndex++] =                                        // Write the new strategy frequency to file
                                localPopulation.StrategiesStats.nrIndPerStrategy;
                        }
                        
                        int nrSurviving = 0;                        // count nr of suvriving strategies
                        for (int s = 0; s < nrStrategies; ++s) {    // run over strategies
                            if (localPopulation.StrategiesStats.nrIndPerStrategy[s] > 0) {  // check if there are 1 or more individuals of said strategy
                                ++nrSurviving;  // add to counter
                            }
                        }

                        bool nearFixation = false;  
                        if (nrSurviving <= 2) {     // if 2 or less strategies remaining
                            for (int s = 0; s < nrStrategies; ++s) {        // run over strategies
                                int count = localPopulation.StrategiesStats.nrIndPerStrategy[s];    // count number of individuals of remaining strategy
                                if (count > 0 && count <= minNrInd) {       // if more than zero but below or equal to minimum number
                                    nearFixation = true;                    // mark the end of simulation
                                    break;
                                }
                            }
                        }

                        if (nearFixation || nrSurviving == 1) {     // if end of simulation is close or only one strategy remains 
                            vector<int> finalCounts = localPopulation.StrategiesStats.nrIndPerStrategy; // store the final nr of individuals per strategy
                            while (eventIndex < results[rep].size()) {  // ?
                                results[rep][eventIndex++] = finalCounts; // store the final counts
                            }
                            break;
                        }
                    }
                }
                catch (const exception& e) {
                        cerr << "Exception in replica " << rep << ": " << e.what() << "\n";
                        exit(EXIT_FAILURE);   // or some other controlled shutdown
                }
            });
        }

        for(auto& t : threads) t.join();                                                    // Wait till all threads are finished before starting the next batch of replicates

        PrintProgress(replica, nrThreads, nrReplicasUpdating, start, benefit, cost); 
    }    

    for(int rep = 0; rep < nrReplicasUpdating; ++rep){      // loop over replicas
        for(int e = 0; e < results[rep].size(); ++e){       // loop over generations
            for (int s = 0; s < nrStrategies; ++s) {        // loop over strategies
                outputFile << results[rep][e][s];           // write results of replicate/generation/strategy to output file
                if (s < nrStrategies - 1) outputFile << ",";    // write , between strategy counts
            }
            if (e < results[rep].size() - 1) outputFile << ";"; // write ; between generations
        }
        outputFile << "\n";
    }
}

/*==========================================================================================
============================================================================================
=============================       Complete simulation       ==============================
============================================================================================
==========================================================================================*/
void SimulationStrategyUpdatingAllStrats(const string simulationType,
                                const double benefit,
                                const int totalRoundsUpdating,
                                array <int, nrStrategies> nrIndividualsPerStrategy,
                                const int nrThreads,
                                const strategiesSocialNorms& socialNorms,
                                const strategiesActionRules& actionRules)
{
    cout << "\nRunning b/c = " << benefit/cost << "\n";                                     // Terminal output
    
    //--- random nr generator ---
    mt19937_64 rng;                                                                         // Initialize rng
    GetSeedRNG(rng);                                                                        // Seed the rng with current time
    unsigned int baseSeed = rng();                                                          // Extract base seed

    //auto start = chrono::high_resolution_clock::now();                                      // Keep track of time (for terminal progress)

    //--- setup ---
    string resultsFolder = CreateResultsDirectory(simulationType, cost, benefit,            // Create results directory and stores the name prefix for result files
                                                  error1, error2,
                                                  totalRoundsUpdating);


    ofstream parameterFile(resultsFolder + "/model_parameters.csv");                        // Create parameter file to save in results directory
        if(!parameterFile.is_open()){
            cerr << "Error: unable to open output file\n";
            exit(EXIT_FAILURE);
        }

    parameterFile << "cost = " << cost                                                  // Write parameters to file
                << "\nbenefit = " << benefit
                        << "\nerror1 = " << error1
                        << "\nerror2 = " << error2
                        << "\nnrIndividuals = " << nrIndividuals
                        << "\nnrTotalRounds = " << totalRoundsUpdating
                        << "\nnrRoundsToUpdateEvent = " << nrRoundsToUpdateEvent
                        << "\nnrReplicates = " << nrReplicasUpdatingTotalAllStrats;
    parameterFile.close();                                                              // Close parameter file

    //--- simulation loop ---    
    //cout <<  "Progress(%): "; //HACK
        
    PopulationClassStrategyUpdatingAllStrats tempPopulation(socialNorms, actionRules,            // Create a temporary population to intialize the result directories
                                                        nrIndividualsPerStrategy,   
                                                        startingReputation, 
                                                        startingPayOff);
                       
    string outputFileName = resultsFolder + "/Output.csv";

    // Check if already complete
    ifstream checkFile(outputFileName);
    int existingLines = 0;

    if (checkFile.is_open()) {
            existingLines = count(istreambuf_iterator<char>(checkFile),
                                  istreambuf_iterator<char>(), '\n');
    }

    checkFile.close();
    //cout << "existingLines = " << existingLines << endl;
    //cout << "nrReplicasUpdatingTotalAllStrats = " << nrReplicasUpdatingTotalAllStrats << endl;

    if (existingLines >= nrReplicasUpdatingTotalAllStrats + 1) {
        cout << "Skipping - already complete\n";
        return;
    }

    // Open for writing
    ofstream outputFile(outputFileName, ios::app);
    if(!outputFile.is_open()){
        cerr << "Error: unable to open output file\n";
        exit(EXIT_FAILURE);
    }

    // Write header only if file is new (empty)
    if (existingLines == 0) {
        outputFile << "L1,L2,L3,L4,L5,L6,L7,L8,SH,I-S,ALLC,ALLD" << "\n";
    }                                               

    tempPopulation.RunReplicas(rng, nrThreads, baseSeed,                                // Run replicates for the current permutation
                                nrIndividualsPerStrategy, benefit, 
                                socialNorms, actionRules, outputFile);
    outputFile.close();                                                                 // Close output file
}
