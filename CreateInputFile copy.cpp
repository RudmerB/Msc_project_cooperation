#include <iostream>
#include <fstream>
#include <array>
#include <bits/stdc++.h>
using namespace std;

//https://www.geeksforgeeks.org/dsa/binomial-coefficient-dp-9/
// Returns value of Binomial Coefficient C(n, k)
int binomialCoeff(int n, int k) {
  	
    // k can not be grater then k so we return 0 here
    if (k > n)
        return 0;
  
  	// base condition when k and n are equal or k = 0
    if (k == 0 || k == n)
        return 1;

    // Recurvie add the value 
    return binomialCoeff(n - 1, k - 1)
           + binomialCoeff(n - 1, k);
}


int main()
{
    const int populationSize    = 100;
    constexpr int nrStrategies  = 8;
    const int k                 = 2;
    int val                     = populationSize / k;

    ofstream ofs("InputFile.csv");
    if (!ofs.is_open()){                                                    // If opening the output file fails...
        cerr << "Error, cannot open file: instructions.csv" << endl;        // ...print a warning message
        exit(EXIT_FAILURE);    
    }
    ofs << binomialCoeff(nrStrategies,k) << endl;

    //https://www.geeksforgeeks.org/cpp/stdnext_permutation-prev_permutation-c/
    array<int, nrStrategies> selector = {};
    fill(selector.begin(), selector.begin() + k, 1);

    do {
        for (int i = 0; i < nrStrategies; ++i) {
            ofs << (selector[i] ? val : 0);
            if (i < nrStrategies - 1) ofs << ',';
        } 
        ofs << endl;
    } while (prev_permutation(selector.begin(), selector.end()));

    return 0;
}




