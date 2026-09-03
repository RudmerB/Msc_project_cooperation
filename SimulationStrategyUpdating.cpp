// Libraries
#include <fstream>
#include <thread>
#include <iomanip>

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
    if (StrategiesStats.nrIndStrategyA + StrategiesStats.nrIndStrategyB != nrIndividuals) { // Safety check: Is the number of A and B equal to the total?
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
                if(population[i].socialNorm != socialNorms.socialNorm[StrategiesStats.strategyAIndex] ||
                   population[i].actionRule != actionRules.actionRule[StrategiesStats.strategyAIndex]) {
                    throw logic_error("An 'A'-Labelled individual with mismatching socialnorm/actionrule has been found\n");
                   }
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
                if(population[i].socialNorm != socialNorms.socialNorm[StrategiesStats.strategyBIndex] ||
                   population[i].actionRule != actionRules.actionRule[StrategiesStats.strategyBIndex]) {
                    throw logic_error("A 'B'-Labelled individual with mismatching socialnorm/actionrule has been found\n");
                   }
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

    ResetIndividuals(startingPayOff, startingReputation);   // Reset the payoff and private reputations of all individuals
}


/*==========================================================================================
============================================================================================
=============================       Simulation Replicas       ==============================
============================================================================================
==========================================================================================*/
void PopulationClassStrategyUpdating::RunReplicas(
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
    int nrUpdateEvents = totalRoundsUpdating/nrRoundsToUpdateEvent;                         // Calculate the nr of update events 
    vector<vector<int>> results(nrReplicasUpdating, vector<int>(nrUpdateEvents + 1));       // Create a matrix for results
    // ^Define Matrix   ^Name   ^Nr rows            ^Nr values per replicate     ^Initial frequencies

    int nextThreshold = 25; //HACK 

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

                mt19937_64 localRng(baseSeed + rep);                                        // Create local random number generators

                PopulationClassStrategyUpdating localPopulation(socialNorms,                // Create a population for each thread/replicate
                                                                actionRules,
                                                                nrIndividualsPerStrategy,
                                                                startingReputation,
                                                                startingPayOff);
                int eventIndex = 0;                                                         // Initiate updating-event counter
                results[rep][eventIndex++] =                                                // Write initial frequency to matrix 
                    localPopulation.StrategiesStats.nrIndStrategyA;                         // (default = 100 ind)

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
                            localPopulation.StrategiesStats.nrIndStrategyA;
                    }
                    
                    if (localPopulation.StrategiesStats.nrIndStrategyA <= minNrInd ||       //TODO comment update, looks to see if frequencies are above 
                            localPopulation.StrategiesStats.nrIndStrategyB <= minNrInd){
                            int value = localPopulation.StrategiesStats.nrIndStrategyA;
                            while (eventIndex < results[rep].size()) {
                                results[rep][eventIndex++] = value;
                            }

                            break;
                    }
                }  
            });
        }
        for(auto& t : threads) t.join();                                                    // Wait till all threads are finished before starting the next batch of replicates

        int completedReplicas = min(replica + nrThreads, nrReplicasUpdating); //TODO add comment
        int percentage = (completedReplicas * 100) / nrReplicasUpdating;

        //cerr << "DEBUG: replica=" << replica << " completed=" << completedReplicas << " percentage=" << percentage << " nextThreshold=" << nextThreshold << "\n";
        while (percentage >= nextThreshold && nextThreshold <= 100) { //HACK
            cout << "---" << nextThreshold;
            nextThreshold += 25;
        }
    }    

    //outputFile << "NrIndStrategy_"                                                          //TODO Write first strategy name to file---- moved to main function
    //          << strategyNames[StrategiesStats.strategyAIndex] << "\n";

    for(int rep = 0; rep < nrReplicasUpdating; ++rep){                                      // Loop over replicates
        for(int i = 0; i < results[rep].size(); ++i){                                       // Loop over strategy frequencies of replicate
            outputFile << results[rep][i];                                                  // Write frequencies to output file
            if(i < results[rep].size() - 1) outputFile << ",";                              // comma-seperated
        }
        outputFile << "\n";                                                                 // New line for each replicate
    }
}

/*==========================================================================================
============================================================================================
=============================       Complete simulation       ==============================
============================================================================================
==========================================================================================*/
void SimulationStrategyUpdating(const string simulationType,
                                const double benefit,
                                const int totalRoundsUpdating,
                                const array<array<int, nrStrategies>, nrPermutationsUpdating>& allPopulations,
                                const int nrThreads,
                                const strategiesSocialNorms& socialNorms,
                                const strategiesActionRules& actionRules)
{
    cout << "\nRunning b/c = " << benefit/cost << "\n";                                     // Terminal output
    
    //--- random nr generator ---
    mt19937_64 rng;                                                                         // Initialize rng
    GetSeedRNG(rng);                                                                        // Seed the rng with current time
    unsigned int baseSeed = rng();                                                          // Extract base seed

    auto start = chrono::high_resolution_clock::now();                                      // Keep track of time (for terminal progress)

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
                        << "\nnrReplicates = " << nrReplicasUpdating;
        parameterFile.close();                                                              // Close parameter file

    //--- simulation loop ---
    for(int simulation = 0; simulation < nrPermutationsUpdating; ++simulation){             // Loop over population permutations
            
        //if (simulation != 42) { //FIXME once test completed remove   65 = allc/alld
        //    continue;
        //}
        cout << simulation + 1 << "/" << nrPermutationsUpdating << " | Progress(%): "; //HACK

        array<int, nrStrategies> nrIndividualsPerStrategy = allPopulations[simulation];     // Store strategy distribution of population (e.g. 100,100,0,0,0,0,0,0,0,0,0,0)
        
        PopulationClassStrategyUpdating tempPopulation(socialNorms, actionRules,            // Create a temporary population to intialize the result directories
                                                        nrIndividualsPerStrategy,   
                                                        startingReputation, 
                                                        startingPayOff);
                       
        string outputFileName = resultsFolder + "/Output_" +
                        to_string(simulation) + "_" + 
                        strategyNames[tempPopulation.StrategiesStats.strategyAIndex] +
                        "_&_" +
                        strategyNames[tempPopulation.StrategiesStats.strategyBIndex] +
                        ".csv";

        // Check if already complete
        ifstream checkFile(outputFileName);
        int existingLines = 0;

        if (checkFile.is_open()) {
            existingLines = count(istreambuf_iterator<char>(checkFile),
                                  istreambuf_iterator<char>(), '\n');
}
        checkFile.close();
        //cout << "existingLines = " << existingLines << endl;
        //cout << "nrReplicasUpdatingTotal = " << nrReplicasUpdatingTotal << endl;

        if (existingLines >= nrReplicasUpdatingTotal + 1) {
            cout << "Skipping - already complete\n";
            continue;
        }

        // Open for writing
        ofstream outputFile(outputFileName, ios::app);
        if(!outputFile.is_open()){
            cerr << "Error: unable to open output file\n";
            exit(EXIT_FAILURE);
        }

        // Write header only if file is new (empty)
        if (existingLines == 0) {
            outputFile << "NrIndStrategy_"
                    << strategyNames[tempPopulation.StrategiesStats.strategyAIndex] << "\n";
        }                                               

        tempPopulation.RunReplicas(rng, nrThreads, baseSeed,                                // Run replicates for the current permutation
                                   nrIndividualsPerStrategy, benefit, 
                                   socialNorms, actionRules, outputFile);
        outputFile.close();                                                                 // Close output file

        auto end = chrono::high_resolution_clock::now(); //HACK
        chrono::duration<double> elapsed = end - start;
        double remaining = ((elapsed.count() / (simulation + 1)) * 
                        (nrPermutationsUpdating - simulation - 1)) / 60;

        cout << " | " << "Elapsed: " << setw(3) << fixed << setprecision(2)                 // Show runtime and //TODO
             << elapsed.count() / 60 << "m"    
             << " | Remaining: " << setw(3) << max(0.0, remaining) << "m"
             << defaultfloat << "\n";
    }
}
