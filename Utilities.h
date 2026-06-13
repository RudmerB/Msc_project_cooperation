#pragma once

#include <random>
#include <chrono>
using namespace std;

/*---------------------------         Seeding function             ---------------------------*/
void GetSeedRNG(mt19937_64 &rng);

/*--------------------------         Shuffling function             --------------------------*/
void shuffle(array<int, nrIndividuals> &array, mt19937_64 &rng);

/*---------------         Check global parameters with input file             ----------------*/
void checkGlobalParameters();

/*---------------         Reading strategy frequencies from file            ------------------*/
array<int, nrStrategies> ReadStrategyFrequencies(ifstream& csvFile);

/*-----------------------         Create results directory             -----------------------*/
string CreateResultsDirectory(const double cost, 
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