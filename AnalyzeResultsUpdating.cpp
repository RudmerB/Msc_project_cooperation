#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <filesystem>
#include <cmath>

#include "Parameters.h"
#include "ComputeCI.h"

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

            summaryFile << "simulation,strategy1,nrWins1,"                              // Write header to file
                        << "strategy2,nrWins2,genOfFixation,"
                        << "avgPayoff1,lowerPayoff1,upperPayoff1,"
                        << "avgPayoff2,lowerPayoff2,upperPayoff2,"
                        << "avgFinalInd1_coexist,avgFinalInd2_coexist,"
                        << "nrCoexistReplicates,nrPayoff1,nrPayoff2\n";

            //-------------------------------------------------
            // *****     Find the correct data files     ******
            for (int i = 0; i < nrPermutationsUpdating; ++i){                           // Loop to search for 66 files (number of permutations)
                bool foundPayoffFile = false;
                string prefix = "Output_" + to_string(i) + "_";                         // Define the respective prefix for the desired data file 
                string payoffPrefix = "OutputPayOff_" + to_string(i) + "_";             // Prefix for the matching payoff file

                for (const auto& fileEntry : filesystem::directory_iterator(subEntry.path())){  // Run over all the files
                    if (!fileEntry.is_regular_file()) continue;                                 // Make sure it's a normal file

                    string filename = fileEntry.path().filename().string();                     // Store filename as string
                    if (filename.rfind(prefix, 0) != 0) continue;                                     // Check if filename start with correct prefix

                        //cout << "Trying to open: " << fileEntry.path().string() << endl;      // Usefull to check for errors per file, but floods the terminal
                    filesystem::path payoffPath;
                    for (const auto& payoffFileEntry : filesystem::directory_iterator(subEntry.path())){
                        if (!payoffFileEntry.is_regular_file()) continue;
                        string payoffFilename = payoffFileEntry.path().filename().string();
                        if (payoffFilename.rfind(payoffPrefix, 0) != 0) continue;               // Skip files that don't match "OutputPayOff_i_"
                        payoffPath = payoffFileEntry.path();
                        foundPayoffFile = true;
                        break;
                    }

                    if (!foundPayoffFile) {
                        cerr << "\nError: no payoff file found matching prefix " << payoffPrefix << "\n\n";
                        exit(EXIT_FAILURE);
                    }

                    ifstream dataFile(fileEntry.path());                                    // Open the data file
                    if (!dataFile.is_open()) {
                        std::cerr << "Failed to open file!\n";
                        exit(EXIT_FAILURE);
                    }

                    ifstream payoffFile(payoffPath);
                    if (!payoffFile.is_open()) {
                        cerr << "Failed to open file: " << payoffPath << "\n";
                        exit(EXIT_FAILURE);
                    }


                    //------------------------------------------
                    // *****      Intialize variables     ******
                    int nrGenerations = totalRoundsUpdating/nrRoundsToUpdateEvent;          // Calculate the total number of generations
                    
                    int rowCount = 0;                                                       // Keeps track of replicates
                    int nrWinsStrategy1 = 0;                                                // Counts "wins" (number of fixation times)
                    int nrWinsStrategy2 = 0;

                    vector<double> payoffValues1;                                          // all recorded payoffs of strategy 1 (one per replicate)
                    vector<double> payoffValues2;                                          // all recorded payoffs of strategy 2
                        
                    double sumFinalInd1Coexist = 0.0;                                      // Accumulates final-generation strategy1 count, coexistence replicates only
                    int nrCoexistReplicates = 0;     

                    string headerLine;
                    getline(dataFile, headerLine);                                         // discard frequency file header

                    string payoffHeader;
                    getline(payoffFile, payoffHeader);      

                    vector<double> timeOfFixation(nrReplicasUpdatingTotal, 0);              // Create vector to store the time of fixation for each replicate


                    //--------------------------------------------------
                    // *****     Start data analysis per line     ******
                    for (int j = 0; j < nrReplicasUpdatingTotal; ++j){                          // Create loop to run over replicates in one file
                        
                        ++rowCount;
                        
                        string line;
                        getline(dataFile, line);                                                // Grab one replicate (one line, e.g. 100,101,102,102,103...)
                        stringstream ss(line);                                                  // Transform into stringstream object

                        string payoffLine;
                        getline(payoffFile, payoffLine);                                   // MATCHING payoff row for this same replicate
                        stringstream pss(payoffLine);
                        string cell;
                        getline(pss, cell, ',');                                           // Replicate index — discard
                        string outcomeStr;
                        getline(pss, outcomeStr, ',');                                     // Outcome: "fixation" or "coexistence"
                        getline(pss, cell, ',');                                           // Winner — discard
                        string payoffCell1, payoffCell2;
                        getline(pss, payoffCell1, ',');                                    // AvgPayoff for strategy 1
                        getline(pss, payoffCell2, ',');                                    // AvgPayoff for strategy 2

                        if (!payoffCell1.empty()) {
                            payoffValues1.push_back(stod(payoffCell1));
                        }
                        if (!payoffCell2.empty()) {
                            payoffValues2.push_back(stod(payoffCell2));
                        }

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
                        // At this point, `frequency` holds the LAST value read for this replicate —
                        // either the value that triggered fixation, or (if the loop ran to completion
                        // without breaking) the genuine final-generation composition.

                        if (outcomeStr == "coexistence") {                                 // NEW: accumulate only for true coexistence replicates
                            sumFinalInd1Coexist += frequency;
                            ++nrCoexistReplicates;
                        }
                    }

                    dataFile.close();
                    payoffFile.close();


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
                    averageFixationTime = (countFixationNr > 0) ? sumFixation/countFixationNr : 0.0;          // Calculate average fixation time 


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


                    //--------------------------------------
                    // *****     Calculate payoffs + CI     *****
                    double avgPayoff1 = 0.0, lowerPayoff1 = 0.0, upperPayoff1 = 0.0;
                    double avgPayoff2 = 0.0, lowerPayoff2 = 0.0, upperPayoff2 = 0.0;

                    if (payoffValues1.size() >= 2) {
                        ConfidenceInterval ci1 = CalculateCI(payoffValues1);               // default z = 1.96 (95%)
                        avgPayoff1 = ci1.mean;
                        lowerPayoff1 = ci1.lower;
                        upperPayoff1 = ci1.upper;
                    } else if (payoffValues1.size() == 1) {
                        avgPayoff1 = lowerPayoff1 = upperPayoff1 = payoffValues1[0];
                    }

                    if (payoffValues2.size() >= 2) {
                        ConfidenceInterval ci2 = CalculateCI(payoffValues2);
                        avgPayoff2 = ci2.mean;
                        lowerPayoff2 = ci2.lower;
                        upperPayoff2 = ci2.upper;
                    } else if (payoffValues2.size() == 1) {
                        avgPayoff2 = lowerPayoff2 = upperPayoff2 = payoffValues2[0];
                    }

                    double avgFinalInd1Coexist = (nrCoexistReplicates > 0) ? sumFinalInd1Coexist / nrCoexistReplicates : 0.0;
                    double avgFinalInd2Coexist = (nrCoexistReplicates > 0) ? nrIndividuals - avgFinalInd1Coexist : 0.0;

                    //---------------------------------------------------
                    // *****     Write analysis to summary file     *****
                    summaryFile << i << ","
                                << name1 << "," << nrWinsStrategy1 << ","
                                << name2 << "," << nrWinsStrategy2 << ","
                                << averageFixationTime << ","
                                << avgPayoff1 << "," << lowerPayoff1 << "," << upperPayoff1 << ","
                                << avgPayoff2 << "," << lowerPayoff2 << "," << upperPayoff2 << ","
                                << avgFinalInd1Coexist << "," << avgFinalInd2Coexist << ","
                                << nrCoexistReplicates << ","
                                << payoffValues1.size() << "," << payoffValues2.size() << "\n";

                    break;                                                                  // frequency file for this i found and processed, move to next i
                }
            }
        }
    }
    return 0;
}