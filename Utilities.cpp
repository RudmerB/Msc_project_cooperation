// Libraries
#include <chrono>
#include <iostream>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>

// Header Files
#include "Parameters.h"
#include "Utilities.h"
using namespace std;
/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------------------         Seeding function             ---------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void GetSeedRNG(mt19937_64 &rng)                                    // RNG seeding function from c++ course
{
    unsigned seed =
    static_cast<unsigned>(chrono::high_resolution_clock::now().time_since_epoch().count());
    clog << "random seed : " << seed << "\n";
    rng.seed(seed);
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
--------------------------         Shuffling function             --------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void shuffle(array<int, nrIndividuals> &array,                      // Shuffle from c++ course, exercise 14.2
             mt19937_64 &rng)
{
    for(int i = array.size()- 1; i > 0; --i) {
    std::uniform_int_distribution<int> randomElement(0, i);
    int j = randomElement(rng);
    int tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
    }
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------         Check global parameters with input file             ----------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void checkGlobalParameters(const string inputFile,
                           const int nrPermutations)
{
    ifstream csvFile(inputFile);
    if(!csvFile.is_open()){
        cerr<<"error:unable to open data file\n";
        exit(EXIT_FAILURE);
    }
    
    int rowCount = 0;                                           // Create row counter
    string line;                                                // Create empty string for lines in input file
    while (getline(csvFile, line)) {                            // For each line in the input file...
        
        ++rowCount;                                             // Increase row counter
        vector<int> row;                                        // Initialize a vector
        istringstream lineStream(line);                         // Transform current line to stream object
        string val;                                             // Create empty string for values in line
        
        while(getline(lineStream, val, ',')){                   // For each value in the current line (comma-separated)...
            row.push_back(stoi(val));                           // Add the value (string-to-integer) to the row vector
        }

        if (static_cast<int>(row.size()) != nrStrategies) {     // If the row length does not equal global paramater...
            cerr << "\nError: row " << rowCount << " has "      // ...throw error message
                 << row.size() << " columns, expected "
                 << nrStrategies << "\n\n";
            exit(EXIT_FAILURE);                                 // Exit program
        }

        int sum = 0;                                            // Initialize individual counter
        for (int v : row) {                                     // Run over the current line 
            sum += v;                                           // Add the current value to the sum
        }

        if (sum != nrIndividuals) {                             // If the sum does not equal global parameter...
            cerr << "\nError: row " << rowCount                 // ...throw error message
                 << " sums to " << sum << ", expected " 
                 << nrIndividuals << "\n\n";
            exit(EXIT_FAILURE);                                 // Exit program
        }
    }
    csvFile.close();                                            // Close File

    if (rowCount != nrPermutations) {                           // If the number of rows does not equal global parameter...
        cerr << "\nError: file has " << rowCount                // ...throw error message
             << " rows, expected " << nrPermutations 
             << "\n\n";
        exit(EXIT_FAILURE);                                     // Exit program
    }
}
//https://stackoverflow.com/questions/50085440/what-is-the-fastest-way-to-get-the-number-of-rows-in-a-csv-file


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------         Reading strategy frequencies from file            ------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
array<int, nrStrategies> ReadStrategyFrequencies(ifstream& csvFile)
{
    string line;                                            // Initiate a string
    array<int, nrStrategies> nrIndividualsPerStrategy;      // create/empty array

    getline(csvFile, line);                                 // read next line in input file
    stringstream ss(line);                                  // Creates a copy (stream object) of the line 

    for(int j = 0; j < nrStrategies; ++j) {                 // loop over strategies
        string value;                                       // Inititae a string
        getline(ss, value, ',');                            // Read the value of the current postion in stream object 
        nrIndividualsPerStrategy[j] = stoi(value);          // set the nrIndividuals in array to that value
    }


    return nrIndividualsPerStrategy;
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
-----------------------         Create results directory             -----------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
string CreateResultsDirectory(const string simulationType,
                              const double cost, 
                              const double benefit,
                              const double error1,
                              const double error2,
                              const int totalRounds)
{
    ostringstream oss_cost;
    ostringstream oss_benefit;
    ostringstream oss_e1;
    ostringstream oss_e2;

    oss_cost << defaultfloat << (double)cost;
    oss_benefit << defaultfloat << (double)benefit;
    oss_e1 << defaultfloat << (double)error1;
    oss_e2 << defaultfloat << (double)error2;

    string resultsFolder = "results_" + simulationType + "\\results_ind" + to_string(nrIndividuals) +
                        "_rounds" + to_string(totalRounds) +
                        "_c" + oss_cost.str() +
                        "_b" + oss_benefit.str() +
                        "_e1-" + oss_e1.str() +
                        "_e2-" + oss_e2.str();

    system(("mkdir " + resultsFolder).c_str());
    // resultsFolder uses \\ for mkdir
    // but ofstream uses / so replace:
    replace(resultsFolder.begin(), resultsFolder.end(), '\\', '/');
    
    return resultsFolder;
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
-----------------------       Terminal output Progress               -----------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void PrintProgress(const int simulation,
                   const int nrThreads,
                   const int nrPermutations,
                   const chrono::high_resolution_clock::time_point start,
                   const double benefit,
                   const double cost)
{
    int completed = simulation + nrThreads;                                 // Number of completed simulations
    int percentage = (completed * 100) / nrPermutations;                    // Calculate completed percentage 
    
    int prevPercentage = ((simulation) * 100) / nrPermutations;             // Calculate completed percentage of previous batch
    bool crossedThreshold = (percentage / 20) > (prevPercentage / 20);      // If current division > previous devision, the run has crossed 20,40,60,80,or 100%
    if (!crossedThreshold) return;                                          // Stop function if the threshold is not crossed

    auto batchEnd = chrono::high_resolution_clock::now();                   // Start timer
    chrono::duration<double> elapsed = batchEnd - start;                    // Calculate time passing

    double remaining = ((elapsed.count() / completed) *                     // Calculate remaining time
                        (nrPermutations - completed)) / 60;
    
    cout << "b/c = " << setw(2) << benefit/cost
         << " | " << setw(3) << min(percentage, 100) << "% done"
         << " | Time elapsed: " << fixed << setprecision(3) << elapsed.count() / 60 << "m"
         << " | Estimated remaining: " << fixed << setprecision(3) << max(0.0, remaining) << "m"
         << defaultfloat    // Future numbers don't have 3 decimals
         << "\n";
}