#include <vector>
#include <numeric>
#include <thread>
#include <fstream>

#include "Parameters.h"
#include "Utilities.h"
#include "Strategies.h"
#include "BasePopulation.h"
#include "SimulationRareMutant.h"
using namespace std;




/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
-----------------------       Terminal output Progress               -----------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void PrintProgress(const int simulation,
                   const int nrThreads,
                   const int nrPermutations,
                   const chrono::high_resolution_clock::time_point start)
{
    auto batchEnd = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = batchEnd - start;
    cout << "Completed simulations " << simulation << " to " 
         << min(simulation + nrThreads - 1, nrPermutations - 1) 
         << " | Time elapsed: " << elapsed.count() << "s"
         << " (" << elapsed.count() / 60 << "m)"
         << " | Estimated remaining: " 
         << (elapsed.count() / (simulation + nrThreads)) * (nrPermutations - simulation - nrThreads) 
         << "s"
         << " (" << ((elapsed.count() / (simulation + nrThreads)) * (nrPermutations - simulation - nrThreads)) / 60 << "m)"
         << "\n";
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
----------------------------------         Main             --------------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
int main()
{
    //--- parameters ---
    const double  error1              = 0.01;
    const double  error2              = 0.1;
    const double  cost                = 1.0;
    const double  benefit             = 1.05;
    const char    startingReputation  = 'G';
    const double  startingPayOff      = 0.0;
    const int     totalRounds         = 5;
    //const int     replicasPerPop      = 1000;

    //Moved RNG inside the loop

    //--- timer for terminal output ---
    auto start = chrono::high_resolution_clock::now();                  // For terminal output


    //--- initialize structures ---
    strategiesSocialNorms socialNorms;      // Create socialnorm struct
    strategiesActionRules actionRules;      // Create actionrule struct
    

    //--- open input file ---
    ifstream inputFile("InputFile.csv");
    if(!inputFile.is_open()){
        cerr<<"error:unable to open input file\n";
        exit(EXIT_FAILURE);
    }

    checkGlobalParameters();                   // Function that checks whether constexpr at top = file input
    system("mkdir results_mutant");

    //--- read strategy permutation ---
    array<array<int, nrStrategies>, nrPermutations> allPopulations;     // Create an array of arrays
    for (int i = 0; i < nrPermutations; ++i){                           // Loop over the strategy permutations
        allPopulations[i] = ReadStrategyFrequencies(inputFile);         // Store the permutations in the array
    }
    inputFile.close();                                                  // Close the input file

    //NEW
    //for (double benefit = 2.0; benefit <= 10.0; benefit += 1.0) {

        //cout << "Running b/c = " << benefit/cost << "\n";
        
        //--- random nr generator ---
        mt19937_64 rng;                         // Initialize rng
        GetSeedRNG(rng);                        // Seed the rng with current time
        unsigned int baseSeed = rng();          // Extract base seed


        //--- setup ---
        string resultsFolder = CreateResultsDirectory(cost, benefit,        // Create results directory and stores the name prefix for result files
                                                    error1, error2,
                                                    totalRounds);

        
        int nrThreads = thread::hardware_concurrency();                     // Read the number of threads available on current device
        if(nrThreads == 0){                                                 // If the above fails...
            nrThreads = 4;                                                  // ...set the number of threads to 4
        }
        cout << "Running with " << nrThreads << " number of threads\n";     // Write number of threads to terminal

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
        for(int simulation = 0; simulation < nrPermutations; simulation += nrThreads){          // Runs over simulations in groups based on nrThreads
        vector<thread> threads;                                                                 // Create vector
        threads.reserve(nrThreads);                                                             // Reserve space for vector

            for(int t = 0; t < nrThreads && simulation + t < nrPermutations; ++t){              // Gives one simulation to each thread, and prevents out of bounds
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
                    
                    myPopulation.RunReplicas(nrReplicas, startingPayOff,                    // Run replicas of the population
                                            startingReputation, totalRounds, 
                                            rng, error1, error2, cost, 
                                            benefit, outputFile);
                    //if (sim == 21) {
                    //    myPopulation.printtest();
                    //    for (auto x : nrIndividualsPerStrategy) {
                    //    cout << x << ", ";
                    //    }
                    //}
                    outputFile.close();                                                         // Close output file
                }));
            }

            for(auto& t : threads){                                                             // Check all threads                
                t.join();                                                                       // Wait for all threads to be done
            }

            PrintProgress(simulation, nrThreads, nrPermutations, start);                        // Print to terminal
        }
    //}
    return 0;
}
