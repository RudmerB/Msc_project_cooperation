#pragma once

#include <random>
#include <chrono>

#include "Parameters.h"
using namespace std;

/*----------------         Create InputFile functions (seperate cpp file)          -----------*/
void CreateInputFileRareMutant();
void CreateInputFileStrategyUpdating();

/*---------------------------         Seeding function             ---------------------------*/
void GetSeedRNG(mt19937_64 &rng);

/*--------------------------         Shuffling function             --------------------------*/
void shuffle(array<int, nrIndividuals> &array, mt19937_64 &rng);

/*---------------         Check global parameters with input file             ----------------*/
void checkGlobalParameters(const string inputFile,
                           const int nrPermutations);

/*---------------         Reading strategy frequencies from file            ------------------*/
array<int, nrStrategies> ReadStrategyFrequencies(ifstream& csvFile);

/*-----------------------         Create results directory             -----------------------*/
string CreateResultsDirectory(const string simulationType,
                              const double cost, 
                              const double benefit,
                              const double error1,
                              const double error2,
                              const int totalRounds);

/*-----------------------       Terminal output Progress               -----------------------*/
void PrintProgress(const int simulation,
                   const int nrThreads,
                   const int nrPermutations,
                   const chrono::high_resolution_clock::time_point start,
                   const double benefit,
                   const double cost);
