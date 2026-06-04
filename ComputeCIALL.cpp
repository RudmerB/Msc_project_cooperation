#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <filesystem>

#include "ComputeCI.h"
using namespace std;


int main (){
    constexpr int nrPermutations    = 132;              // Number of population permutations (number of unique strategy combinations)
    constexpr int nrReplicas        = 1000;

    system("mkdir summaries");
    
    //*******************************************************************************
    // Outer-loop runs over the directories
    //*******************************************************************************
    for (const auto& entry : filesystem::directory_iterator(".")){              // current directory
        string folderName = entry.path().filename().string();
    
        if (!entry.is_directory()) continue;                                    // skip anything (like files) but folders
        if (folderName.substr(0,8) != "results_") continue;                     // skip anything that doesn't start with results_

        cout << "Processing: " << folderName << endl;

        
        string summaryName = "summaries/summary" + folderName.substr(7) + ".csv";
        
        ofstream summaryFile(summaryName);
        if(!summaryFile.is_open()){                                                  // Open output file
            cerr << "Error: unable to open output file\n";
            exit(EXIT_FAILURE);
        }

        summaryFile << "simulation,mean,lowerCI,upperCI\n";                         // Write header to summaryfile

        //*******************************************************************************
        // Middle-loop runs over the permutations(different populations, same parameters)
        //*******************************************************************************
        for (int i = 0; i < nrPermutations; ++i){

            ifstream resultsFile(folderName + "/Output_" + to_string(i) + ".csv");
            if(!resultsFile.is_open()){                                                  // Open input file
                cerr << "Error: unable to open output file\n";
                exit(EXIT_FAILURE);
            }

            int rowCount = 0;
            array <double, nrReplicas> residentPayOff;
            array <double, nrReplicas> mutantPayOff;
            array <double, nrReplicas> payOffDifference;

            //*******************************************************************************
            // Inner-loop runs over the replicas of the same permutation
            //*******************************************************************************
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


            summaryFile << i << "," 
                << results.mean  << "," 
                << results.lower << "," 
                << results.upper << "\n";
        }
    }
    return 0;
}