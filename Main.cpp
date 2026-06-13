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
----------------------------------         Main             --------------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
int main()
{
    //--- parameters ---
    const double  error1              = 0.01;
    const double  error2              = 0.1;
    const double  cost                = 1.0;
    //const double  benefit             = 1.05;
    const char    startingReputation  = 'G';
    const double  startingPayOff      = 0.0;
    const int     totalRounds         = 10;

    int nrThreads = thread::hardware_concurrency();                     // Read the number of threads available on current device
    if(nrThreads == 0){                                                 // If the above fails...
        nrThreads = 4;                                                  // ...set the number of threads to 4
    }
    cout << "Running with " << nrThreads << " number of threads\n";     // Write number of threads to terminal


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
    
    //--- read strategy permutation ---
    array<array<int, nrStrategies>, nrPermutations> allPopulations;     // Create an array of arrays
    for (int i = 0; i < nrPermutations; ++i){                           // Loop over the strategy permutations
        allPopulations[i] = ReadStrategyFrequencies(inputFile);         // Store the permutations in the array
    }
    inputFile.close();                                                  // Close the input file


    system("mkdir results_mutant");

    //NEW
    for (double benefit = 2.0; benefit <= 10.0; benefit += 1.0) {
        SimulationRareMutant(benefit, allPopulations, nrThreads, socialNorms, actionRules);
    }
    return 0;
}
