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
    
    //---------------------------------------------
    // *****     Find correct directories     *****
    for (const auto& entry : filesystem::directory_iterator(".")){                      // Loop over current directory
        if(!entry.is_directory()) continue;                                             // If item is not a directory ignore
        if (entry.path().filename().string() != "results_updating") continue;           // If foldername doesn't start with results updating ignore


        for (const auto& subEntry : filesystem::directory_iterator(entry.path())){      // Loop over items in results_updating
            
            if (!subEntry.is_directory()) continue;                                     // If item is not a directory ignore
            string subFolderName = subEntry.path().filename().string();                 // Create subfoldername variable
            if (subFolderName.substr(0, 8) != "results_") continue;                     // If subfoldername doesn't start with results_ ignore

            cout << "Processing: " << subFolderName << endl;                            // Terminal output
            

            //---------------------------------------
            // *****     Create output file     *****
            string summaryName = "summaries_updating/summary"                           // Create name for summaryfile
                                 + subFolderName.substr(7) + ".csv";    
        
            ofstream summaryFile(summaryName);                                          // Create and open summaryfile
            if(!summaryFile.is_open()){
                cerr << "Error: unable to open output file\n";
                exit(EXIT_FAILURE);
            }

            summaryFile << "simulation,strategy1,nrWins1,"                              // Write header to summaryfile 
                        << "strategy2,nrWins2,genOfFixation\n";      


            //-------------------------------------------------
            // *****     Find the correct data files     ******
            for (int i = 0; i < nrPermutationsUpdating; ++i){                           // Loop to search for 66 files (number of permutations)
                string prefix = "Output_" + to_string(i) + "_";                         // Define the respective prefix for the desired data file 

                for (const auto& fileEntry : filesystem::directory_iterator(subEntry.path())){  // Run over all the files
                    
                    if (!fileEntry.is_regular_file()) continue;                                 // Make sure it's a normal file

                    string filename = fileEntry.path().filename().string();                     // Store filename as string
                    if (filename.rfind(prefix, 0) == 0) {                                       // Check if filename start with correct prefix

                        //cout << "Trying to open: " << fileEntry.path().string() << endl;      // Usefull to check for errors per file, but floods the terminal

                        ifstream dataFile(fileEntry.path());                                    // Open the data file
                        if (!dataFile.is_open()) {
                            std::cerr << "Failed to open file!\n";
                        }


                        //------------------------------------------
                        // *****      Intialize variables     ******
                        int nrGenerations = totalRoundsUpdating/nrRoundsToUpdateEvent;          // Calculate the total number of generations
                        
                        int rowCount = 0;                                                       // Keeps track of replicates
                        int nrWinsStrategy1 = 0;                                                // Counts "wins" (number of fixation times)
                        int nrWinsStrategy2 = 0;
                        
                        string headerLine;
                        getline(dataFile, headerLine);                                          // Reads and discards the first line (the header)
                        
                        vector<double> timeOfFixation(nrReplicasUpdatingTotal, 0);              // Create vector to store the time of fixation for each replicate

                        //--------------------------------------------------
                        // *****     Start data analysis per line     ******
                        for (int j = 0; j < nrReplicasUpdatingTotal; ++j){                          // Create loop to run over replicates in one file
                            
                            ++rowCount;
                            string line;
                            getline(dataFile, line);                                                // Grab one replicate (one line, e.g. 100,101,102,102,103...)
                            stringstream ss(line);                                                  // Transform into stringstream object

                            string frequencyString;                                                 // Temporarily stores a data point as a string
                            double frequency = 0.0;


                            //-------------------------------------------
                            // *****     Extract data from file     *****
                            for (int k = 0; k < nrGenerations; ++k){;                               // Create loop to run over one population replicate

                                //----------------------------------
                                // ******      Read values     *****
                                if (!getline(ss, frequencyString, ',')) {                           // Grab the first data point of the replicate, including safety check
                                    cerr << "Row " << rowCount << " in " << filename
                                         << " ran out of values at k=" << k 
                                         << " (expected " << nrGenerations << ")\n";
                                    break;                                                          // Skip row to not crash programm
                                }                                  

                                try {
                                    frequency = stod(frequencyString);                              // Transform value from string to double
                                } catch (const exception& e) {                                      // Safety Check
                                    cerr << "stod failed on \"" << frequencyString 
                                         << "\" in " << filename
                                         << " (row " << rowCount 
                                         << ", k=" << k << "): " << e.what() << "\n";
                                    break;                                                          // Skip row to not crash programm
                                }
                                
                                
                                //--------------------------------------------
                                // *****     Find population fixation    *****
                                if (frequency <= minNrInd){                                         // Check if strategy 1 has 5 or less individuals
                                    ++nrWinsStrategy2;                                              // Award a "win" (fixation of) strategy 2
                                    timeOfFixation[j] = k;                                          // Store the generation of fixation
                                    break;                                                          // Skip over remaining data (frequency updating is halted after fixation, therefore useless)
                                } 
                        
                                else if (frequency >= nrIndividuals - minNrInd){                    // Check if strategy 1 has 195 or more individuals
                                    ++nrWinsStrategy1;                                              // Award a "win" to strategy 1
                                    timeOfFixation[j] = k;                                          // Store the generation of fixation
                                    break;                                                          // Skip over remaining data of this replicate
                                }
                            }
                        }


                        //--------------------------------------------
                        // *****     Calculate fixation time     *****
                        double sumFixation = 0.0;
                        double averageFixationTime = 0.0;
                        int countFixationNr = 0;

                        for (int x = 0; x < nrReplicasUpdatingTotal; ++x ) {        // Create a loop to run over timeOffFixation (vector)
                            if (timeOfFixation[x] > 0){                             // Vector is initialized at zero, so only if there is a generation of fixation found...
                                sumFixation += timeOfFixation[x];                   // ...add the generation number to the sum
                                ++countFixationNr;                                  // Add one to the fixation counter
                            }
                        }
                        averageFixationTime = sumFixation/countFixationNr;          // Calculate average fixation time 


                        //---------------------------------
                        // *****     Safety check     *****
                        if (rowCount != nrReplicasUpdatingTotal) {                  // If the number of rows in datafile does not equal global parameter...
                            cerr << "\nError: file has " << rowCount                // ...throw error message
                                << " rows, expected " << nrReplicasUpdatingTotal 
                                << "\n\n";
                            exit(EXIT_FAILURE);                                     // Exit program
                        }
                

                        //------------------------------------------------------
                        // *****     Extract names from inputfile name     *****
                        string rest = filename.substr(prefix.size());               // Remove prefix "Output_0_" from filename
                        string nameOnly = rest.substr(0, rest.size() - 4);          // Remove file format ".csv" from filename

                        string sep = "_&_";                                         // Define the name seperator
                        size_t sepPos = nameOnly.find(sep);                         // Find the position of the seperator in the file

                        string name1 = nameOnly.substr(0, sepPos);                  // Extract the first name (beginning to seperator)
                        string name2 = nameOnly.substr(sepPos + sep.size());        // Extract the second name (seperator to end)


                        //---------------------------------------------------
                        // *****     Write analysis to summary file     *****
                        summaryFile << i << ","                                     // Write the number of the simulation, 
                            << name1 << "," << nrWinsStrategy1 << ","               // The first strategy name and the number of times it fixated,
                            << name2 << "," << nrWinsStrategy2 << ","               // The second strategy name and the number of times it fixated,
                            << averageFixationTime << "\n";                         // And the average number of generations before fixation


                        break;                                                      // The correct file has been found, so the search loop is halted
                    }   
                }
            }
        }
    }
    return 0;
}