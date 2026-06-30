#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <filesystem>

#include "Parameters.h"
#include "ComputeCI.h"
using namespace std;


int main (){

    system("mkdir summaries");
    
    //*******************************************************************************
    // Outer-loop runs over the directories
    //*******************************************************************************
    for (const auto& entry : filesystem::directory_iterator(".")){              // current directory
        if(!entry.is_directory()) continue;
        string folderName = entry.path().filename().string();
        if (folderName != "results_mutant") continue;

        for (const auto& subEntry : filesystem::directory_iterator(folderName)){
            if (!subEntry.is_directory()) continue;
            string subFolderName = subEntry.path().filename().string();
            if (subFolderName.substr(0, 8) != "results_") continue;

            cout << "Processing: " << subFolderName << endl;
            string summaryName = "summaries/summary" + subFolderName.substr(7) + ".csv";
        
            ofstream summaryFile(summaryName);
            if(!summaryFile.is_open()){                                                  // Open output file
                cerr << "Error: unable to open output file\n";
                exit(EXIT_FAILURE);
            }

            summaryFile << "simulation,mean,lowerCI,upperCI\n";                         // Write header to summaryfile

            //*******************************************************************************
            // Middle-loop runs over the permutations(different populations, same parameters)
            //*******************************************************************************
            for (int i = 0; i < nrPermutationsMutant; ++i){

                ifstream dataFile(folderName + "/" + subFolderName + "/Output_" + to_string(i) + ".csv");
                if(!dataFile.is_open()){                                                  // Open input file
                    cerr << "Error: unable to open output file\n";
                    exit(EXIT_FAILURE);
                }

                int rowCount = 0;
                array <double, nrReplicasMutant> residentPayOff;
                array <double, nrReplicasMutant> mutantPayOff;
                array <double, nrReplicasMutant> payOffDifference;

                //*******************************************************************************
                // Inner-loop runs over the replicas of the same permutation
                //*******************************************************************************
                for (int j = 0; j < nrReplicasMutant; ++j){
                    ++rowCount;
                    string line;
                    getline(dataFile, line);
                    stringstream ss(line);

                    string resVal, mutVal;
                    getline(ss, resVal, ',');
                    getline(ss, mutVal, ',');

                    residentPayOff[j] = stod(resVal);
                    mutantPayOff[j] = stod(mutVal);
                    payOffDifference[j] = mutantPayOff[j] - residentPayOff[j];
                }

                if (rowCount != nrReplicasMutant) {                           // If the number of rows does not equal global parameter...
                    cerr << "\nError: file has " << rowCount                // ...throw error message
                         << " rows, expected " << nrReplicasMutant 
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
    }
    return 0;
}