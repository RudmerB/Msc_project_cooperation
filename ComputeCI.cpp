#include <iostream>
#include <fstream>
#include <sstream>

#include "ComputeCI.h"
using namespace std;



/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------------------         Global parameters            ---------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
constexpr int nrIndividuals     = 100;              // Nr of individuals in one population
constexpr int nrStrategies      = 12;               // Total number of social strategies (Leading eight, image-scoring etc.)
constexpr int nrPermutations    = 132;              // Number of population permutations (number of unique strategy combinations)


int main (){
    
    double  error1              = 0.01;
    double  error2              = 0.05;
    double  cost                = 1;
    double  benefit             = 2;
    
    string resultsFolder;
    cout << "Enter the name of results folder : ";
    cin >> resultsFolder;

    string summaryName = "summary" + resultsFolder.substr(7) + ".csv";
    
    ofstream summaryFile(summaryName);
    

    return 0;
}

