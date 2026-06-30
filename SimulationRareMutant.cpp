// Libraries
#include <fstream>
#include <thread>
#include <filesystem>

// Header Files
#include "Parameters.h"
#include "BasePopulation.h"
#include "Utilities.h"
#include "SimulationRareMutant.h"
using namespace std;


/*==========================================================================================
============================================================================================
=====================         Population - Class Constructor         =======================
============================================================================================
==========================================================================================*/
PopulationClassRareMutant::PopulationClassRareMutant(const strategiesSocialNorms& socialNorms,
                                 const strategiesActionRules& actionRules,
                                 const array<int, nrStrategies>& nrIndividualsPerStrategy,
                                 const char startingReputation,
                                 const double startingPayOff
                                 )
{
    int maxValue = *max_element(nrIndividualsPerStrategy.begin(),                       // Find the highest value of the current permutation
                                nrIndividualsPerStrategy.end());
    int populationIndex = 0;                                                            // Keeps tab on how many individuals have been assigned

    for(int i = 0; i < nrStrategies; ++i) {                                             // Loop over the different strategies
        bool isResident = (nrIndividualsPerStrategy[i] == maxValue);                    // Check if the current strategy has the most individuals, coin this the resident strategy
        
        for(int count = 0; count < nrIndividualsPerStrategy[i]; ++count) {              // Loop over the number of individuals per strategy 
            if (populationIndex >= nrIndividuals) {                                     // Safety check: 'Are we counting more individuals than possible?'
                throw logic_error("The index counts a larger population " 
                                  "than the real population size.\n");
            }
            
            population[populationIndex] = BaseIndividual(startingReputation,                // Construct an individual at this location with reputation and payoff given in main()
                                                     startingPayOff);
            population[populationIndex].socialNorm = socialNorms.socialNorm[i];         // Assign the respective social norm to the individual of that strategy
            population[populationIndex].actionRule = actionRules.actionRule[i];         // Do the same for the action rule

            if (isResident) {                                                           // Check if the current strategy is the resident strategy
                population[populationIndex].isResident = true;                          // Store the individual as a resident (used for average payoff later)
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
StrategyPayOff PopulationClassRareMutant::DetermineStrategyPayOff()
{
    StrategyPayOff Results;                                     // Create a structure for results
    double dResidentSum = 0.0;                                  // Initiate sum of pay offs
    int iResidentsNr = 0;                                       // Initiate resident counter
    double dMutantSum = 0.0;
    int iMutantsNr = 0;

    for (int i = 0; i < nrIndividuals; ++i) {                   // Loop over all individuals
        if (population[i].isResident) {                         // Check if the individual is a resident
            dResidentSum += population[i].PayOff;               // Add their payoff to the sum
            ++ iResidentsNr;                                    // Increase resident counter with 1
        }
        else {                                                  // If an indvidual is not a resident
            dMutantSum += population[i].PayOff;
            ++ iMutantsNr;
        }
    }
    
    if (iResidentsNr == 0 || iMutantsNr == 0) {                 // Safety check: 'Did we assign the correct number of individuals?'
        throw logic_error("There are no residents or mutants found.\n");
    }

    Results.avgResidentPayOff = dResidentSum/iResidentsNr;      // Store the average resident payoff in the struct
    Results.mutantPayOff = dMutantSum/iMutantsNr;

    return Results;                                             // Return the struct
}


/*==========================================================================================
============================================================================================
=============================       Simulation Replicas       ==============================
============================================================================================
==========================================================================================*/
void PopulationClassRareMutant::RunReplicas(const int nrReplicasMutant,
                                  const double startingPayOff,
                                  const char startingReputation,
                                  const int totalRoundsMutant,
                                  mt19937_64 &rng,
                                  const double error1,
                                  const double error2,
                                  const double cost,
                                  const double benefit,
                                  ofstream &outputFile)
{
    StrategyPayOff Results;                                                                // Create a struct for results
    for(int replica = 0; replica < nrReplicasMutant; ++replica){
        ResetIndividuals(startingPayOff, startingReputation);
        for(int round = 0; round < totalRoundsMutant; ++round){
            PlayInteractionRound(rng, error1, error2, cost, benefit);                      // Play interactions for certain number of rounds
        }
        Results = DetermineStrategyPayOff();                                               // Store payoffs
        outputFile << Results.avgResidentPayOff << "," << Results.mutantPayOff << "\n";    // Write payoffs to output file
    }    
}


/*==========================================================================================
============================================================================================
=============================       Complete simulation       ==============================
============================================================================================
==========================================================================================*/
void SimulationRareMutant(const string simulationType,
                          const double benefit,
                          const array<array<int, nrStrategies>, nrPermutationsMutant>& allPopulations,
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
                                                  totalRoundsMutant);


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
                        << "\nnrRounds = " << totalRoundsMutant
                        << "\nnrReplicates = " << nrReplicasMutant;
        parameterFile.close();

    //--- simulation loop ---
    for(int simulation = 0; simulation < nrPermutationsMutant; simulation += nrThreads){          // Runs over simulations in groups based on nrThreads
    vector<thread> threads;                                                                 // Create vector
    threads.reserve(nrThreads);                                                             // Reserve space for vector

        for(int t = 0; t < nrThreads && simulation + t < nrPermutationsMutant; ++t){              // Gives one simulation to each thread, and prevents out of bounds
            int sim = simulation + t;


            threads.push_back(thread([=](){                                                 // Create lambda function (function with no name)
                mt19937_64 rng(baseSeed + sim);                                             // Create a rng with time and simulation nr
                array<int, nrStrategies> nrIndividualsPerStrategy = allPopulations[sim];    // Store strategy distribution of population 
                
                PopulationClassRareMutant myPopulation(socialNorms,                                   // Create population
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
                
                myPopulation.RunReplicas(nrReplicasMutant, startingPayOff,                    // Run replicas of the population
                                        startingReputation, totalRoundsMutant, 
                                        rng, error1, error2, cost, 
                                        benefit, outputFile);
                outputFile.close();                                                         // Close output file
            }));
        }
        for(auto& t : threads){                                                             // Check all threads                
            t.join();                                                                       // Wait for all threads to be done
        }
        PrintProgress(simulation, nrThreads, nrPermutationsMutant, startCurrentTimer, benefit, cost);                        // Print to terminal
    }
}