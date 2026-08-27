#include <vector>
#include <numeric>
#include <thread>
#include <fstream>
#include <filesystem>
#include <algorithm>

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
    char answer = ' ';
    cout << "Don't forget to put the simulation in release/debug version (Ctrl + Shift + P), do you want to continue(y/n)?" << endl;
    cin >> answer;
    if (answer != 'y') {
        cout << "Understood, till next time.\n";
        exit(EXIT_FAILURE);
    }

    int nrThreads = thread::hardware_concurrency();                     // Read the number of threads available on current device
    if(nrThreads == 0){                                                 // If the above fails...
        nrThreads = 4;                                                  // ...set the number of threads to 4
    }
    cout << "Running with " << nrThreads << " number of threads\n";     // Write number of threads to terminal


    //--- initialize structures ---
    strategiesSocialNorms socialNorms;      // Create socialnorm struct
    strategiesActionRules actionRules;      // Create actionrule struct

    string simulationType = " ";
    string permutationsFile = " ";
    int nrPermutations = 0;
    array<double, 11> benefitArray = {1.05, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};

    int userInput = 0;
    cout << "What program do you want to run?\n"
         << "   - Rare mutant strategy simulation (1)\n"
         << "   - Strategy updating simulation (2)\n"
         << "Type in 1 or 2\n";
    cin >> userInput;

    if (userInput == 1) {

        simulationType = "mutant";
        if (!filesystem::exists("InputFileRareMutant.csv")) {
            CreateInputFileRareMutant();
        }
        permutationsFile = "InputFileRareMutant.csv";
        nrPermutations = nrPermutationsMutant; // neceesary?  or probably make it a function in utilities and give it parameters like filename, nrpermuations 
    }

    else if (userInput == 2) {
  
        simulationType = "updating";
        if (!filesystem::exists("InputFileStrategyUpdating.csv")) {
            CreateInputFileStrategyUpdating();
        }
        permutationsFile = "InputFileStrategyUpdating.csv";
        nrPermutations = nrPermutationsUpdating;
    }

    //--- open input file ---
    ifstream inputFile(permutationsFile);
    if(!inputFile.is_open()){
        cerr<<"error:unable to open input file\n";
        exit(EXIT_FAILURE);
    }

    checkGlobalParameters(permutationsFile, nrPermutations);                   // Function that checks whether constexpr at top = file input


    if (userInput == 1) { 
        //--- read strategy permutation ---
        array<array<int, nrStrategies>, nrPermutationsMutant> allPopulations;     // Create an array of arrays
        for (int i = 0; i < nrPermutationsMutant; ++i){                           // Loop over the strategy permutations
            allPopulations[i] = ReadStrategyFrequencies(inputFile);         // Store the permutations in the array
        }
        inputFile.close();                                                  // Close the input file

        if (!filesystem::exists("results_mutant")) {
            system("mkdir results_mutant");
        }

        //NEW
        for (double benefit = 2.0; benefit <= 10.0; benefit += 1.0) {
            SimulationRareMutant(simulationType, benefit, allPopulations, nrThreads, socialNorms, actionRules);
        }
    }

    else if (userInput == 2) {
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
    }
    return 0;
}
