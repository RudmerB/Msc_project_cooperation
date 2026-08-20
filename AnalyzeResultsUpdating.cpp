#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <filesystem>

#include "Parameters.h"

using namespace std;


int main (){

    system("mkdir summaries_updating");
    
    //*******************************************************************************
    // Outer-loop runs over the directories
    //*******************************************************************************
    for (const auto& entry : filesystem::directory_iterator(".")){              // Loop over current directory
        if(!entry.is_directory()) continue;                                     // If item is not a directory ignore
        string folderName = entry.path().filename().string();                   // Create foldername variable
        if (folderName != "results_updating") continue;                         // If foldername doesn't start with results updating ignore

        for (const auto& subEntry : filesystem::directory_iterator(folderName)){    // Loop over items in results updating
            if (!subEntry.is_directory()) continue;                                 // If item is not a directory ignore
            string subFolderName = subEntry.path().filename().string();             // Create subfoldername variable
            if (subFolderName.substr(0, 8) != "results_") continue;                 // If subfoldername doesn't start with results_ ignore

            cout << "Processing: " << subFolderName << endl;                        // terminal output
            string summaryName = "summaries/summary" + subFolderName.substr(7) + ".csv";    // Create name for outputfile
        
            ofstream summaryFile(summaryName);                                      // Create and open outputfile
            if(!summaryFile.is_open()){                                             // Check if Open output file
                cerr << "Error: unable to open output file\n";
                exit(EXIT_FAILURE);
            }

            summaryFile << "simulation,strategy1,nrWins1,strategy2,nrWins2\n";      // Write header to summaryfile #TODO add speed

            //*******************************************************************************
            // Middle-loop runs over the permutations(different populations, same parameters)
            //*******************************************************************************
            for (int i = 0; i < nrPermutationsUpdating; ++i){
                
                for (const auto& subsubEntry : filesystem::directory_iterator(subEntry)){
                    if (!subsubEntry.is_regular_file()) continue;

                    string filename = subsubEntry.path().filename().string();
                    
                    string prefix = "Output_" + to_string(i) + "_";

                    if (filename.rfind(prefix, 0) == 0) { //?
                        string rest = filename.substr(prefix.size());
                        string nameOnly = rest.substr(0, rest.size() - 4); //-4 removes .csv

                        string sep = "_&_";
                        size_t sepPos = nameOnly.find(sep); //position of the seperator

                        string name1 = nameOnly.substr(0, sepPos);          // "L1"
                        string name2 = nameOnly.substr(sepPos + sep.size()); // "L3"

                        ifstream dataFile(entry.path()); // open the actual file
                        
                        /*
                        Here come the operations that are done within a file*/
                        //cout << "i=" << i << " name1=" << name1 << " name2=" << name2 << "\n";
                        
                        
                        int rowCount = 0; //keep track of replicas
                        int nrWinsStrategy1 = 0;
                        int nrWinsStrategy2 = 0;

                        for (int j = 0; j < nrReplicasUpdatingTotal; ++j){                          // goes over one line 
                            ++rowCount;
                            string line;
                            getline(dataFile, line);                                                // get line in memory
                            stringstream ss(line);          //100,101,102,102,103
                    
                            int nrGenerations = totalRoundsUpdating/nrRoundsToUpdateEvent;
                            vector<double> strategy1Frequency;
                            strategy1Frequency.reserve(nrGenerations);
                            string frequencyString;
                            double frequency = 0.0;

                            for (int k = 0; k < nrGenerations; ++k){;                               // Run over the line
                                getline(ss,frequencyString, ',');                                      // get each value
                                frequency = stod(frequencyString);

                                if (frequency <= minNrInd){                                // check if value is 5 or under
                                    ++nrWinsStrategy2;
                                    continue;
                                } 
                        
                                else if (frequency >= nrIndividuals - minNrInd){           // check if value is 195 or above
                                    ++nrWinsStrategy1;
                                    continue;
                                }
                                //else{}
                            }
                        }
                        
                        if (rowCount != nrReplicasUpdatingTotal) {                           // If the number of rows does not equal global parameter...
                            cerr << "\nError: file has " << rowCount                // ...throw error message
                                << " rows, expected " << nrReplicasUpdatingTotal 
                                << "\n\n";
                            exit(EXIT_FAILURE);                                     // Exit program
                        }

                        summaryFile << i << ","
                            << name1 << "," << nrWinsStrategy1 << ","
                            << name2 << "," << nrWinsStrategy2 << "\n";

                        break; // found the file for this i, stop searching the folder
                    }
                }
            }
        }
    }
    return 0;
}



/*
                ifstream dataFile(folderName + "/" + subFolderName + "/Output_" + to_string(i) + "_" )

                ifstream dataFile(folderName + "/" + subFolderName + "/Output_" + to_string(i) + (here goes extra stuff) ".csv"); //TODO wont open because names are different 
                if(!dataFile.is_open()){                                                  // Open input file
                    cerr << "Error: unable to open output file\n";
                    exit(EXIT_FAILURE);
                }

                int rowCount = 0; //keep track of replicas
                int nrWinsStrategy1 = 0;
                int nrWinsStrategy2 = 0;

                *******************************************************************************
                // Inner-loop runs over the replicas of the same permutation
                *******************************************************************************
                for (int j = 0; j < nrReplicasUpdatingTotal; ++j){                          // goes over one line 
                    ++rowCount;
                    string line;
                    getline(dataFile, line);                                                // get line in memory
                    stringstream ss(line);          //100,101,102,102,103
                    
                    int nrGenerations = totalRoundsUpdating/nrRoundsToUpdateEvent;
                    vector<double> strategy1Frequency;
                    strategy1Frequency.reserve(nrGenerations);
                    string frequencyVal;

                    for (int k = 0; k < nrGenerations; ++k){;                               // Run over the line
                        getline(ss,frequencyVal, ',');                                      // get each value
                        
                        if (stod(frequencyVal) <= minNrInd){                                // check if value is 5 or under
                            ++nrWinsStrategy2;
                            continue;
                        } 
                        
                        else if (stod(frequencyVal) >= nrIndividuals - minNrInd){           // check if value is 195 or above
                            ++nrWinsStrategy1;
                            continue;
                        }

                        //else{}
                    }
                }

                if (rowCount != nrReplicasUpdatingTotal) {                           // If the number of rows does not equal global parameter...
                    cerr << "\nError: file has " << rowCount                // ...throw error message
                         << " rows, expected " << nrReplicasUpdatingTotal 
                         << "\n\n";
                    exit(EXIT_FAILURE);                                     // Exit program
                }

            

                summaryFile << i << ","
                            << name of strategy 1 << "," << nrWinsStrategy1 << ","
                            << name of strategy 2 << "," << nrWinsStrategy2 << "\n";
            }
        }
    }
    return 0;
}*/