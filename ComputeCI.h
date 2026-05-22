#pragma once 

#include <iostream>
#include <array>
#include <cmath>

using namespace std;

struct ConfidenceInterval{
    double mean;
    double lower;
    double upper;
    double std;
    int n;   
};

// --- Confidence Interval Calculator ---
// zScore: 1.645 = 90%, 1.96 = 95%, 2.576 = 99%
template <typename Container>
ConfidenceInterval CalculateCI(const Container& data, double zScore = 1.96)
{
    int n = data.size();

    if(n < 2){
        throw logic_error("Need at least 2 data points to calculate CI\n");
    }

    double sum = 0.0;
    for (double x : data){
        sum += x;
    }
    double mean = sum/n;


    double SumOfSquares = 0.0;
    for (double x : data) {
        SumOfSquares += (x - mean) * (x - mean);
    }

    double variance = SumOfSquares / (n - 1);
    double std = sqrt(variance);
    double CI = zScore * (std / sqrt(n));

    return {mean, mean - CI, mean + CI, std, n};
}