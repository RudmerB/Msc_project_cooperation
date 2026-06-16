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
    
    int userInput = 0;
    cout << "What program do you want to run?\n"
         << "   - Rare mutant strategy simulation (1)\n"
         << "   - Strategy updating simulation (2)\n"
         << "Type in 1 or 2\n";
    cin >> userInput;

    if (userInput == 1) {

        const string simulationType = "mutant";
        const int nrPermutations = nrPermutationsMutant; // neceesary?  or probably make it a function in utilities and give it parameters like filename, nrpermuations 

        //This is specific to mutant
        if (!filesystem::exists("InputFileRareMutant.csv")) {
            CreateInputFileRareMutant();
        }

        //--- open input file ---
        ifstream inputFile("InputFileRareMutant.csv");
        if(!inputFile.is_open()){
            cerr<<"error:unable to open input file\n";
            exit(EXIT_FAILURE);
        }

        checkGlobalParameters("InputFileRareMutant.csv", nrPermutationsMutant);                   // Function that checks whether constexpr at top = file input
        
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
    return 0;
}
