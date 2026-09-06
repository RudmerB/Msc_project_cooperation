#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <filesystem>
#include <numeric>
#include <cmath>

#include "Parameters.h"
#include "ComputeCI.h"

using namespace std;


int main (){

    system("mkdir summaries_updating_allstrats");

    //---------------------------------------------
    // *****     Find correct directories     *****
    for (const auto& entry : filesystem::directory_iterator(".")){                      // Loop over current directory
        if(!entry.is_directory()) continue;                                             // If item is not a directory ignore
        if (entry.path().filename().string() != "results_updating_allstrats") continue;           // If foldername doesn't start with results updating ignore


        for (const auto& subEntry : filesystem::directory_iterator(entry.path())){      // Loop over items in results_updating
            
            if (!subEntry.is_directory()) continue;                                     // If item is not a directory ignore
            string subFolderName = subEntry.path().filename().string();                 // Create subfoldername variable
            if (subFolderName.substr(0, 8) != "results_") continue;                     // If subfoldername doesn't start with results_ ignore

            cout << "Processing: " << subFolderName << endl;                            // Terminal output
        

            //---------------------------------------
            // *****     Create output file     *****
            string summaryName = "summaries_updating_allstrats/summary"                           // Create name for summaryfile
                                 + subFolderName.substr(7) + ".csv";    
        
            ofstream summaryFile(summaryName);                                          // Create and open summaryfile
            if(!summaryFile.is_open()){
                cerr << "Error: unable to open output file\n";
                exit(EXIT_FAILURE);
            }

            summaryFile << "Generation,L1,lowerL1,upperL1,L2,lowerL2,upperL2,"
                        << "L3,lowerL3,upperL3,L4,lowerL4,upperL4,"
                        << "L5,lowerL5,upperL5,L6,lowerL6,upperL6,"
                        << "L7,lowerL7,upperL7,L8,lowerL8,upperL8,"
                        << "Sh,lowerSh,upperSh,I-S,lowerI-S,upperI-S,"
                        << "ALLC,lowerALLC,upperALLC,ALLD,lowerALLD,upperALLD";
                                
            filesystem::path dataFilePath = subEntry.path() / "Output.csv";
            ifstream dataFile(dataFilePath);
            if (!dataFile.is_open()) {
                std::cerr << "Failed to open file: " << dataFilePath << "\n";
            }


            int nrGenerations = totalRoundsUpdating/nrRoundsToUpdateEvent;          // Calculate the total number of generations //FIXME needs change if introducing more rounds
            int nrPointsPerReplicate = nrGenerations + 1;

            // data[generation][strategy] = vector of values across replicates
            vector<vector<vector<double>>> data(
                nrPointsPerReplicate,
                vector<vector<double>>(nrStrategies)
            );

            string line;
            getline(dataFile, line); //skip header

            while (getline(dataFile, line)) {                                   // One line per replicate
                istringstream lineStream(line);
                string eventBlock;
                int e = 0;

                while (getline(lineStream, eventBlock, ';')) {                  // Split into generation blocks
                    if (e >= nrPointsPerReplicate) break;                       // Safety: don't overrun expected size

                    istringstream eventStream(eventBlock);
                    string value;
                    int s = 0;
                    while (getline(eventStream, value, ',')) {                  // Split each block into 12 strategy counts
                        if (s >= nrStrategies) break;
                        data[e][s].push_back(stod(value));
                        ++s;
                    }
                    ++e;
                }
            }
            dataFile.close();
            for (int e = 0; e < nrPointsPerReplicate; ++e) {
                for (int s = 0; s < nrStrategies; ++s) {
                    if (data[e][s].size() != static_cast<size_t>(nrReplicasUpdating)) {
                        cerr << "MISMATCH at generation " << e << ", strategy " << s
                            << ": got " << data[e][s].size() << " values, expected "
                            << nrReplicasUpdating << "\n";
                            exit(EXIT_FAILURE);
                    }
                }
            }
            for (int i = 0; i < nrPointsPerReplicate; ++i){
                summaryFile << "\n" << i;

                for (int j = 0; j < nrStrategies; ++j){
                    ConfidenceInterval ci = CalculateCI(data[i][j]);
                    summaryFile << "," << ci.mean << "," << ci.lower << "," << ci.upper;
                }
            }
            summaryFile.close();
        }
    }
    return 0;
}