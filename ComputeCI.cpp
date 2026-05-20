#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>

#include "ComputeCI.h"
using namespace std;


int main (){
    constexpr int nrPermutations    = 132;              // Number of population permutations (number of unique strategy combinations)
    constexpr int nrReplicas        = 1000;

    system(("mkdir summaries"));
    
    string resultsFolder;
    cout << "Enter the name of results folder : ";
    cin >> resultsFolder;

    string summaryName = "summaries/summary" + resultsFolder.substr(7) + ".csv";
    
    ofstream summaryFile(summaryName);
    if(!summaryFile.is_open()){                                                  // Open output file
        cerr << "Error: unable to open output file\n";
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nrPermutations; ++i){

        ifstream resultsFile(resultsFolder + "/Output_" + to_string(i) + ".csv");
        if(!resultsFile.is_open()){                                                  // Open input file
            cerr << "Error: unable to open output file\n";
            exit(EXIT_FAILURE);
        }

        int rowCount = 0;
        array <double, nrReplicas> residentPayOff;
        array <double, nrReplicas> mutantPayOff;
        array <double, nrReplicas> payOffDifference;

        for (int j = 0; j < nrReplicas; ++j){
            ++rowCount;
            string line;
            getline(resultsFile, line);
            stringstream ss(line);

            string resVal, mutVal;
            getline(ss, resVal, ',');
            getline(ss, mutVal, ',');

            residentPayOff[j] = stod(resVal);
            mutantPayOff[j] = stod(mutVal);
            payOffDifference[j] = mutantPayOff[j] - residentPayOff[j];
        }

        if (rowCount != nrReplicas) {                           // If the number of rows does not equal global parameter...
            cerr << "\nError: file has " << rowCount                // ...throw error message
                 << " rows, expected " << nrReplicas 
                 << "\n\n";
            exit(EXIT_FAILURE);                                     // Exit program
        }
        ConfidenceInterval results;
        results = CalculateCI(payOffDifference);
        
        cout << "lower = " << results.lower << ", mean = " << results.mean << ", upper = " << results.upper << endl;
        //cout << "n = " << results.n << "\n";
        //cout << "std = " << results.std << "\n";
        //cout << "mean = " << results.mean << "\n";
        //cout << "lower = " << results.lower << "\n";
        //cout << "upper = " << results.upper << "\n";
    }

    return 0;
}