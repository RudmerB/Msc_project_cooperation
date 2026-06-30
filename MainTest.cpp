#include <vector>
#include <numeric>
#include <thread>
#include <fstream>
#include <filesystem>

#include "Parameters.h"
#include "Utilities.h"
#include "Strategies.h"
#include "BasePopulation.h"
#include "SimulationRareMutant.h"
#include "SimulationStrategyUpdating.h"
using namespace std;


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
----------------------------------         Main             --------------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
int main()
{
    //--- initialize structures ---
    strategiesSocialNorms socialNorms;      // Create socialnorm struct
    strategiesActionRules actionRules;      // Create actionrule struct

    int nrThreads = thread::hardware_concurrency();                     // Read the number of threads available on current device
    if(nrThreads == 0){                                                 // If the above fails...
        nrThreads = 4;                                                  // ...set the number of threads to 4
    }
    cout << "Running with " << nrThreads << " number of threads\n";     // Write number of threads to terminal

    string simulationType = " ";
    string permutationsFile = " ";
    int nrPermutations = 0;
  
    simulationType = "updating";
    if (!filesystem::exists("InputFileStrategyUpdating.csv")) {
        CreateInputFileStrategyUpdating();
    }
    permutationsFile = "InputFileStrategyUpdating.csv";
    nrPermutations = nrPermutationsUpdating;


    //--- open input file ---
    ifstream inputFile(permutationsFile);
    if(!inputFile.is_open()){
        cerr<<"error:unable to open input file\n";
        exit(EXIT_FAILURE);
    }

    checkGlobalParameters(permutationsFile, nrPermutations);                   // Function that checks whether constexpr at top = file input
    

    //--- read strategy permutation ---
    array<array<int, nrStrategies>, nrPermutationsUpdating> allPopulations;     // Create an array of arrays
    for (int i = 0; i < nrPermutationsUpdating; ++i){                           // Loop over the strategy permutations
        allPopulations[i] = ReadStrategyFrequencies(inputFile);         // Store the permutations in the array
    }
    inputFile.close(); 
    
    if (!filesystem::exists("results_updating")) {
    system("mkdir results_updating");
    }

    const double benefit = 2.0;
    SimulationStrategyUpdating(simulationType, benefit, totalRoundsUpdating, allPopulations, nrThreads, socialNorms, actionRules);
    return 0;
}
