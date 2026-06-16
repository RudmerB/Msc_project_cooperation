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
            
            population[populationIndex] = BaseIndividual(startingReputation,                // Construct an individual at this location with reputation and payoff given in main()
                                                     startingPayOff);
            population[populationIndex].socialNorm = socialNorms.socialNorm[i];         // Assign the respective social norm to the individual of that strategy
            population[populationIndex].actionRule = actionRules.actionRule[i];         // Do the same for the action rule

            if (i == StrategiesStats.strategyAIndex) {
                population[populationIndex].strategyLabel = 'A';
            }
            else if (i == StrategiesStats.strategyBIndex) {
                population[populationIndex].strategyLabel = 'B';
            }
            ++populationIndex;                                                          // Increase the counter to initiate unassigned individuals
        }
    }

    if (populationIndex != nrIndividuals) {                                             // Safety check: 'Did we assign the correct number of individuals?'
        throw logic_error("The number of assigned individuals does not "
                          "equal the population size.\n");
    }
}


/*==========================================================================================
============================================================================================
=====================       Mutant and  Average Resident Payoff       =======================
============================================================================================
==========================================================================================*/
Strategies PopulationClassStrategyUpdating::DetermineStrategyPayOff()
{
    double dStrategyASum = 0.0;                                  // Initiate sum of pay offs                                     // Initiate resident counter
    double dStrategyBSum = 0.0;

    for (int i = 0; i < nrIndividuals; ++i) {                   // Loop over all individuals
        if (population[i].strategyLabel == 'A') {                         // Check if the individual is a resident
            dStrategyASum += population[i].PayOff;               // Add their payoff to the sum
            ++ StrategiesStats.nrIndStrategyA;                                    // Increase resident counter with 1
        }
        else if (population[i].strategyLabel == 'B') {                                               // If an indvidual is not a resident
            dStrategyBSum += population[i].PayOff;
            ++ StrategiesStats.nrIndStrategyB;
        }
        else throw logic_error("DetermineStrategyPayOff() sees individuals without labels");
    }
    
    if (StrategiesStats.nrIndStrategyA == 0 || StrategiesStats.nrIndStrategyB == 0) {                 // Safety check: 'Did we assign the correct number of individuals?'
        throw logic_error("There are no individuals of A and/or B found.\n");
    }

    StrategiesStats.avgStrategyAPayOff = dStrategyASum/StrategiesStats.nrIndStrategyA;      // Store the average resident payoff in the struct
    StrategiesStats.avgStrategyBPayOff = dStrategyBSum/StrategiesStats.nrIndStrategyB;

    return StrategiesStats;                                             // Return the struct
}


/*==========================================================================================
============================================================================================
=============================       Replicator Dynamics       ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdating::RunReplicatorDynamics()
{
    double pA = StrategiesStats.nrIndStrategyA / nrIndividuals;
    double pB = StrategiesStats.nrIndStrategyB / nrIndividuals;

    double meanPayOff = (pA * StrategiesStats.avgStrategyAPayOff) +
                        (pB * StrategiesStats.avgStrategyBPayOff);

    double pA_new = pA * (StrategiesStats.avgStrategyAPayOff / meanPayOff);

    StrategiesStats.nrIndStrategyA = round(pA_new * nrIndividuals);
    StrategiesStats.nrIndStrategyB = nrIndividuals - StrategiesStats.nrIndStrategyA;
}


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
    int StrategyACounter = 0;
    int StrategyBCounter = 0;

    for (int i = 0; i < nrIndividuals; ++i) {
        if (population[i].strategyLabel == 'A' && StrategyACounter < StrategiesStats.nrIndStrategyA) {
            ++StrategyACounter;
        }
        else if (population[i].strategyLabel == 'A' && StrategyACounter > StrategiesStats.nrIndStrategyA) {
            population[i].socialNorm = socialNorms.socialNorm[StrategiesStats.strategyBIndex];
            population[i].actionRule = actionRules.actionRule[StrategiesStats.strategyBIndex];
            population[i].strategyLabel = 'B';
            ++ StrategyBCounter;
        }
        else if (population[i].strategyLabel == 'B') {
            ++ StrategyBCounter;
        }
        else throw logic_error("ConstructNewGeneration() sees individuals without labels");
    }

    if(StrategyACounter != StrategiesStats.nrIndStrategyA || StrategyBCounter != StrategiesStats.nrIndStrategyB) {
        throw logic_error("ConstrcutNewGeneration() failed to correctly reconstruct the population frequencies.\n");
    }

    ResetIndividuals(startingPayOff, startingReputation);
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
            
            if(round % 50) {
                DetermineStrategyPayOff();
                RunReplicatorDynamics();
                ConstructNewGeneration(socialNorms, actionRules, startingPayOff, startingReputation);
            }
        }
        outputFile << StrategiesStats.nrIndStrategyA << "," << StrategiesStats.nrIndStrategyB << "\n";    // Write payoffs to output file
    }    
}


/*==========================================================================================
============================================================================================
=============================       Complete simulation       ==============================
============================================================================================
==========================================================================================*/
void SimulationStrategyUpdating(const string simulationType,
                                const double benefit,
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