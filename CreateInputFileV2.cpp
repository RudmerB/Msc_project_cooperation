#include <iostream>
#include <fstream>
#include <array>
#include <bits/stdc++.h>
using namespace std;


int main()
{
    const int populationSize    = 150;                                        // Set population size
    constexpr int nrStrategies  = 12;                                        // Set number of strategies (at compile time)
    const int nrMutants         = 1;

    ofstream ofs("InputFile.csv");
    if (!ofs.is_open()){                                                    // If opening the output file fails...
        cerr << "Error, cannot open file: instructions.csv" << endl;        // ...print a warning message
        exit(EXIT_FAILURE);    
    }

    //https://www.geeksforgeeks.org/cpp/stdnext_permutation-prev_permutation-c/
    array<int, nrStrategies> row = {populationSize - nrMutants, nrMutants,};                                 // Create array of 0's

    bool firstRow = true;                                                   // Set bool to true (AI)

    do {
        if(!firstRow) {                                                     // If not first row (AI)
            ofs << "\n";                                                    // Create new row (AI)
        }
        firstRow = false;                                                   // Set bool to false (AI)

        for (int i = 0; i < nrStrategies; ++i) {                            // For each strategy
            ofs << row[i];                                                  // Write 'val' nr of individuals if 1 and 0 individuals if 0
            if (i < nrStrategies - 1) ofs << ',';                           // Write a comma between strategies
        } 
    } while (prev_permutation(row.begin(), row.end()));           // Find the next unique permutation of the array

    cout << "\nPermutations written to InputFile.csv\n\n";
    return 0;
}