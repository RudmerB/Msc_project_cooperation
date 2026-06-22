// Libraries
#include <fstream>
#include <thread>

// Header Files
#include "Parameters.h"
#include "BasePopulation.h"
#include "Utilities.h"
#include "SimulationStrategyUpdating.h"
using namespace std;


/*==========================================================================================
============================================================================================
=====================         Population - Class Constructor         =======================
============================================================================================
==========================================================================================*/
PopulationClassStrategyUpdating::PopulationClassStrategyUpdating(const strategiesSocialNorms& socialNorms,
                                                                 const strategiesActionRules& actionRules,
                                                                 const array<int, nrStrategies>& nrIndividualsPerStrategy,
                                                                 const char startingReputation,
                                                                 const double startingPayOff)
{
    int populationIndex = 0;                        // Keeps tab on how many individuals have been assigned

    for(int i = 0; i < nrStrategies; ++i) {         // Loop over the different strategies
        if (nrIndividualsPerStrategy[i] > 0) {      // If a value >0 has been found 
            if (StrategiesStats.strategyAIndex == -1) {             // Check if strategy A is assigned (-1 = unnassigned)
                StrategiesStats.strategyAIndex = i;                 // Replace the index with the value at nrIndividualsPerStrategy[i]
            }
            else if (StrategiesStats.strategyBIndex == -1) {        // If A is assigned check if strategy B is assigned (-1 = unnassigned)
                StrategiesStats.strategyBIndex = i;                 // Replace the index witht the value at nrIndividualsPerStrategy[i]
            }
        }
    }
    
    for(int i = 0; i < nrStrategies; ++i) {
        for(int count = 0; count < nrIndividualsPerStrategy[i]; ++count) {              // Loop over the number of individuals per strategy 
            if (populationIndex >= nrIndividuals) {                                     // Safety check: 'Are we counting more individuals than possible?'
                throw logic_error("The index counts a larger population " 
                                  "than the real population size.\n");
            }
            
            population[populationIndex] = BaseIndividual(startingReputation,            // Construct an individual at this location with reputation and payoff given in main()
                                                     startingPayOff);
            population[populationIndex].socialNorm = socialNorms.socialNorm[i];         // Assign the respective social norm to the individual of that strategy
            population[populationIndex].actionRule = actionRules.actionRule[i];         // Do the same for the action rule

            if (i == StrategiesStats.strategyAIndex) {                                  // If the iterator matches the index of strategy A...
                population[populationIndex].strategyLabel = 'A';                        // ...Give the corresponding individual the label 'A' and...
                ++StrategiesStats.nrIndStrategyA;                                       // ...add one to the counter of strategy A individual
            }
            else if (i == StrategiesStats.strategyBIndex) {                             // If the iterator matches the index of strategy B...
                population[populationIndex].strategyLabel = 'B';                        // ...Give the corresponding individual the label 'B' and...
                ++StrategiesStats.nrIndStrategyB;                                       // ...add one to the counter of strategy B individual
            }
            ++populationIndex;                                                          // Increase the counter for the total population number
        }
    }

    if (populationIndex != nrIndividuals) {                                             // Safety check: 'Did we assign the correct number of individuals?'
        throw logic_error("The number of assigned individuals does not "
                          "equal the population size.\n");
    }
    if (StrategiesStats.nrIndStrategyA + StrategiesStats.nrIndStrategyA != nrIndividuals) { // Safety check: Is the number of A and B equal to the total?
        throw logic_error("The number of assigned individuals does not "
                          "equal the population size.\n");
    }
}


/*==========================================================================================
============================================================================================
=====================        Payoff       =======================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdating::DetermineStrategyPayOff()
{
    double dStrategyASum = 0.0;                                 // Initiate pay off sum for A 
    double dStrategyBSum = 0.0;                                 // Initiate pay off sum for B

    for (int i = 0; i < nrIndividuals; ++i) {                   // Loop over all individuals
        if (population[i].strategyLabel == 'A') {               // Check if individual has label 'A'
            dStrategyASum += population[i].PayOff;              // Add their payoff to sum of A
        }
        else if (population[i].strategyLabel == 'B') {          // Check if individual has label 'B'
            dStrategyBSum += population[i].PayOff;              // Add their payoff to sum of B
        }
        else throw logic_error("DetermineStrategyPayOff() sees individuals without labels");
    }

    StrategiesStats.avgStrategyAPayOff = dStrategyASum/StrategiesStats.nrIndStrategyA;      // Calculate average payoff for both strategies 
    StrategiesStats.avgStrategyBPayOff = dStrategyBSum/StrategiesStats.nrIndStrategyB;
    //cout << "PayOff " << strategyNames[StrategiesStats.strategyAIndex] << "/" 
    //     << strategyNames[StrategiesStats.strategyBIndex] << " = "
    //     << StrategiesStats.avgStrategyAPayOff << "/" << StrategiesStats.avgStrategyBPayOff; //TODO Remove after testing
}


/*==========================================================================================
============================================================================================
=============================       Replicator Dynamics       ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdating::RunReplicatorDynamics()
{
    double pA = StrategiesStats.nrIndStrategyA / static_cast<double>(nrIndividuals);        // Calculate the fraction of 'A' and 'B' individuals, static_cast
    double pB = StrategiesStats.nrIndStrategyB / static_cast<double>(nrIndividuals);        // allows to obtain a double from dividing two intergers

    double meanFitnessA = baseFitness + StrategiesStats.avgStrategyAPayOff;
    double meanFitnessB = baseFitness + StrategiesStats.avgStrategyBPayOff;

    double meanFitnessTotal = (pA * meanFitnessA) +                                              // Calculate the mean payoff (fraction A * mean payoff A + fraction B + mean payoff B))
                              (pB * meanFitnessB);

    double pA_new = pA * (meanFitnessA / meanFitnessTotal);                 // Fraction of A in new generation is fraction A * (mean payoff A / mean payoff total)
    if (pA_new < 0.0 || pA_new > 1.0) {
        cerr << "\npA_new took on an impossible value in RunReplicatorDynamics()\n";
        exit(EXIT_FAILURE);
    }
    StrategiesStats.nrIndStrategyA = round(pA_new * nrIndividuals);                         // Number of A in new generation is new fraction A * population size
    StrategiesStats.nrIndStrategyB = nrIndividuals - StrategiesStats.nrIndStrategyA;        // Number of B in new generation is population size - new number of A
    //cout << "    \t\tFitness " << strategyNames[StrategiesStats.strategyAIndex] << "/" 
    //     << strategyNames[StrategiesStats.strategyBIndex] << " = "
    //     << meanFitnessA << "/" << meanFitnessB; //TODO Remove after testing
    //cout << "    \t\tnrInd post update " << strategyNames[StrategiesStats.strategyAIndex] << "/" 
    //     << strategyNames[StrategiesStats.strategyBIndex] << " = " 
    //     << StrategiesStats.nrIndStrategyA << "/" << StrategiesStats.nrIndStrategyB << endl; //TODO remove after testing
}
//BUG Payoff can become negative, which in turn may lead to negative nr of individuals

/*==========================================================================================
============================================================================================
=============================        Strategy Updating        ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdating::ConstructNewGeneration(const strategiesSocialNorms& socialNorms,
                                                             const strategiesActionRules& actionRules,
                                                             const double startingPayOff,
                                                             const char startingReputation)
{
    int targetNrA = StrategiesStats.nrIndStrategyA;         // Store traget nr of individuals in new generation
    int targetNrB = StrategiesStats.nrIndStrategyB;
    int currentNrA = 0;                                     // Store current nr of individuals that have been assigned either A or B
    int currentNrB = 0;

    for (int i = 0; i < nrIndividuals; ++i) {               // Loop over individuals
        if (population[i].strategyLabel == 'A') {           // If the individual is currently 'A'...
            if (currentNrA < targetNrA) {                   // ...And if the counted nr of 'A' is smaller than the target...
                ++currentNrA;                               // ...Add this individual (still 'A') to the current counter A
            }
            else {                                          // If the counted nr of individuals is not smaller than the target...
                population[i].socialNorm = socialNorms.socialNorm[StrategiesStats.strategyBIndex];      // Switch social norm to 'B'
                population[i].actionRule = actionRules.actionRule[StrategiesStats.strategyBIndex];      // Switch action rule to 'B'
                population[i].strategyLabel = 'B';          // Swithc strategy label to 'B'
                ++currentNrB;                               // Add the indvidual (now 'B') to the current counter B
            }
        }
        else if (population[i].strategyLabel == 'B') {      // Else if the individual is currently 'B'...
            if (currentNrB < targetNrB) {                   // ...And if the counted nr of 'B' is smaller than the target...
                ++currentNrB;                               // ...Add this individual (still 'B') to the current counter B
            }
            else {                                          // If the counted nr of individuals is not smaller than the target
                population[i].socialNorm = socialNorms.socialNorm[StrategiesStats.strategyAIndex];      // Switch social norm to 'A'
                population[i].actionRule = actionRules.actionRule[StrategiesStats.strategyAIndex];      // Switch action rule to 'A'
                population[i].strategyLabel = 'A';          // Switch the strategy label to 'A'
                ++currentNrA;                               // Add the individual (now 'A') to the current counter A
            }
        }
    }

    if(currentNrA != StrategiesStats.nrIndStrategyA || currentNrB != StrategiesStats.nrIndStrategyB) {      // Safety check: did we switch the correct nr of individuals
        throw logic_error("ConstrcutNewGeneration() failed to correctly reconstruct the population frequencies.\n");
    }

    if(StrategiesStats.nrIndStrategyA == 0 || StrategiesStats.nrIndStrategyB == 0) {
        cout << "One of the strategies has reached fixation, the simulation has stopped\n";
        exit(EXIT_SUCCESS);
    }
    ResetIndividuals(startingPayOff, startingReputation);   // Reset the payoff and private reputations of all individuals
}


/*==========================================================================================
============================================================================================
=============================       Simulation Replicas       ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdating::RunReplicas(const int replicasPerPop,
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
                                  ofstream &outputFile)
{   
    outputFile << "NrIndStrategyA, NrIndStrategyB" << endl;                                                            
    for(int replica = 0; replica < replicasPerPop; ++replica){

        ResetIndividuals(startingPayOff, startingReputation);
        for(int round = 0; round < totalRounds; ++round){

            PlayInteractionRound(rng, error1, error2, cost, benefit);                      // Play interactions for certain number of rounds
            if (round == 0){
                outputFile << StrategiesStats.nrIndStrategyA << "," << StrategiesStats.nrIndStrategyB << "\n";
            }
            if(round > 0 && round % 100 == 0){ //TODO determine if 50 is good enough
                DetermineStrategyPayOff(); 
                RunReplicatorDynamics();
                ConstructNewGeneration(socialNorms, actionRules, startingPayOff, startingReputation);
                outputFile << StrategiesStats.nrIndStrategyA << "," << StrategiesStats.nrIndStrategyB << "\n";
            }
        }  
    }    
}


/*==========================================================================================
============================================================================================
=============================       Complete simulation       ==============================
============================================================================================
==========================================================================================*/
void SimulationStrategyUpdating(const string simulationType,
                                const double benefit,
                                const int totalRounds,
                                const array<array<int, nrStrategies>, nrPermutationsUpdating>& allPopulations,
                                const int nrThreads,
                                const strategiesSocialNorms& socialNorms,
                                const strategiesActionRules& actionRules)
{
    cout << "\nRunning b/c = " << benefit/cost << "\n";
        
    //--- random nr generator ---
    mt19937_64 rng;                         // Initialize rng
    GetSeedRNG(rng);                        // Seed the rng with current time
    unsigned int baseSeed = rng();          // Extract base seed

    auto startCurrentTimer = chrono::high_resolution_clock::now();                  // For terminal output

    //--- setup ---
    string resultsFolder = CreateResultsDirectory(simulationType, cost, benefit,        // Create results directory and stores the name prefix for result files
                                                  error1, error2,
                                                  totalRounds);


    ofstream parameterFile(resultsFolder + "/model_parameters.csv");
        if(!parameterFile.is_open()){
            cerr << "Error: unable to open output file\n";
            exit(EXIT_FAILURE);
        }

        parameterFile << "cost = " << cost
                        << "\nbenefit = " << benefit
                        << "\nerror1 = " << error1
                        << "\nerror2 = " << error2
                        << "\nnrIndividuals = " << nrIndividuals
                        << "\nnrRounds = " << totalRounds
                        << "\nnrReplicates = " << nrReplicas;
        parameterFile.close();

    //--- simulation loop ---
    for(int simulation = 0; simulation < nrPermutationsUpdating; simulation += nrThreads){          // Runs over simulations in groups based on nrThreads
    vector<thread> threads;                                                                 // Create vector
    threads.reserve(nrThreads);                                                             // Reserve space for vector

        for(int t = 0; t < nrThreads && simulation + t < nrPermutationsUpdating; ++t){              // Gives one simulation to each thread, and prevents out of bounds
            int sim = simulation + t;

            if (sim != 42) { //FIXME once test completed remove   65 = allc/alld
                continue;
            }
            cout << "running\n";
            threads.push_back(thread([=](){                                                 // Create lambda function (function with no name)
                mt19937_64 rng(baseSeed + sim);                                             // Create a rng with time and simulation nr
                array<int, nrStrategies> nrIndividualsPerStrategy = allPopulations[sim];    // Store strategy distribution of population 
                
                PopulationClassStrategyUpdating myPopulation(socialNorms,                                   // Create population
                                                             actionRules,
                                                             nrIndividualsPerStrategy,
                                                             startingReputation,
                                                             startingPayOff);
                
                ofstream outputFile(resultsFolder + "/Output_" +                            // Create output file for population
                                    to_string(sim) + ".csv");
                if(!outputFile.is_open()){                                                  // Open output file
                    cerr << "Error: unable to open output file\n";
                    return;
                }
                
                myPopulation.RunReplicas(nrReplicas, startingPayOff,                    // Run replicas of the population
                                        startingReputation, totalRounds, 
                                        rng, error1, error2, cost, 
                                        benefit, socialNorms, actionRules, 
                                        outputFile);
                outputFile.close();                                                         // Close output file
            }));
        }
        for(auto& t : threads){                                                             // Check all threads                
            t.join();                                                                       // Wait for all threads to be done
        }
        PrintProgress(simulation, nrThreads, nrPermutationsUpdating, startCurrentTimer, benefit, cost);                        // Print to terminal
    }
}