#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
using namespace std;


/*=========================================
===           Seeding Function          ===
=========================================*/
void GetSeedrng(mt19937_64 &rng)
{
    unsigned seed =
    static_cast<unsigned>(chrono::high_resolution_clock::now().time_since_epoch().count());
    clog << "random seed : " << seed << "\n";
    rng.seed(seed);
}


/*=========================================
===             Class object            ===
=========================================*/
class filament{
public:
    filament(){}
    double deltaX = 0.0;                                        // The distance from filament to barrier
    vector<double> filamentDimers {};                           // All dimers in the filament
private:
};


/*=========================================
===             Class object            ===
=========================================*/
class classMicrotubuli{
public:
    classMicrotubuli(double loadForce);
    void CalculateBarrierDistance();
    void simulationLoop(mt19937_64 &rng);
    void exportData(ofstream &dataFile);
    //void print();
private:
    const int nrFilaments = 13;                                 // Number of filaments in microtbulus
    const double dimerLength = 8;                               // Dimer length in nm
    const double tEnd = 1000;                                   // Max number of time steps

    const double kb = 1.3806485e-2;                             // Conversion by AI
                                                                // Everything that has to do with unit conversion is by AI
    const int T = 298;                                          // Temperature in Kelvin
    //const int F = 2;                                          // Force in pN
    const int kOn = 200;                                        // Rate of adding dimers in min^-1
    const int kOff = 50;                                        // Rate of removing dimers in min^1
    const double dtSav = 1.0;                                   // Time interval for calculating elongation rate
    double F;                                                   // Force in pN (now set in main())
    const int maxTubulesLength = 50000;                         // Maximum length a tubules can take on in nm

    //double barrierPosition = dimerLength;                     // Unused -> This would need to be added to get the real result
    double farthestDimer;                                       // The dimer that touches the barrier
    double prevFarthestDimer = dimerLength;                     // The previous farthest dimer, initiated at 8 nm 
    vector<filament> microtubuli;                               // The microtubli vector with filaments 
    double AvgElongationRate;                                   // Average elongation rate in μm/min
};

/*=========================================
===           Class Constructor         ===
=========================================*/
classMicrotubuli::classMicrotubuli(double loadForce):                           // Pass F to contructor
microtubuli(nrFilaments), F(loadForce)                                          // Set number of neighbouring filaments and the load force
{
    for (int i = 0; i < nrFilaments; ++i) {                                     // Loop over filaments
        microtubuli[i].filamentDimers.push_back(
            dimerLength - i * dimerLength/nrFilaments);                         // The first dimer's heights are 8/13 nm relative to each other
    }
}


/*=============================================                                 // Stupid function
===  Member Function: Calculating Distance  ===                                 
=============================================*/
void classMicrotubuli::CalculateBarrierDistance()                               
{
    vector<double> terminalDimers(nrFilaments);                                 // Create vector to get the terminal subunits
    for(int i = 0; i <nrFilaments; ++i){                                        // Loop over filaments
        terminalDimers[i] = microtubuli[i].filamentDimers.back();               // Get the last unit from each filament
    }
    
    farthestDimer = *max_element(terminalDimers.begin(),                        // From all terminal subunits, find the largest one
                                 terminalDimers.end());

    for (int i = 0; i <nrFilaments; ++i) {                                      // Loop over filaments 
        microtubuli[i].deltaX = farthestDimer -                                 // DeltaX is distance between farthest terminal subunit and 
                                microtubuli[i].filamentDimers.back();           // terminal subunit of the iterated filament (if same, deltaX = 0)

    }
}


/*
void classMicrotubuli::print()
{
    for (int i = 0; i <nrFilaments; ++i) 
        cout << microtubuli[i].filamentDimers[0] << " to go "
             << microtubuli[i].deltaX << endl;
}*/


/*=========================================
===     Member Function: Simulation     ===
=========================================*/
void classMicrotubuli::simulationLoop(mt19937_64 &rng)                          // Pass rng by reference
{
    CalculateBarrierDistance();                                                 // Calculate the distance between each filament to barrier
    
    double sumElongation = 0.0;                                                 // Initiate variables for average elongation
    int nSamples = 0;

    for (double t = 0.0, tsav = 0.0; t < tEnd &&                                // Start simulation, simulation end is determined by both time
         farthestDimer < maxTubulesLength;) {                                   // and the length of the microtubules
        
        vector<double> vecEvents(nrFilaments*2);                                // Create events (double the number of filaments)

        for (int i = 0; i < vecEvents.size(); ++i) {                            // Loop over events
            if (i < nrFilaments) {                                              // If in the first half of vector
                vecEvents[i] = kOn * exp(-(F * microtubuli[i].deltaX) /         // Compute the growth probability
                                        (kb * T));
            }
            else {                                                              // If in second half of vector
                vecEvents[i] = kOff;                                            // Compute shrink probability
            }
        }

        double sum = 0.0;                                                       // Initiate sum
        for(int i = 0; i < vecEvents.size(); ++i) {                             // Loop over vecEvents
            sum += vecEvents[i];                                                // Each event is added to sum
        }
        if(sum <= 0.0) {
            throw runtime_error("Unable to daw waiting time.\n");
        }
        exponential_distribution<double> waitingTime(sum);                      // Draw waiting time
        const double dt = waitingTime(rng);                                     // Random waiting time

        discrete_distribution<int> drawEvent (vecEvents.begin(),                // Draw event
                                              vecEvents.end());
        const int event = drawEvent(rng);                                       // Random event

        t += dt;                                                                // Time += waiting time

        if (event < nrFilaments) {                                              // If event is one of the first halves in vecEvents
            int growingFilament = event;                                        // The filament index is same as event
            microtubuli[growingFilament].filamentDimers.push_back(              // Add a new dimer at the back of the filament
                microtubuli[growingFilament].filamentDimers.back() + 
                dimerLength);
        }
        else {
            int shrinkingFilament = event -nrFilaments;                         // The filament index is event - 13
            if (microtubuli[shrinkingFilament].filamentDimers.size() > 1) {     // Prevent emptying the filament completely
                microtubuli[shrinkingFilament].filamentDimers.pop_back();       // Remove the last dimer from the filament
            }
        }

        CalculateBarrierDistance();                                             // Recalculate the distance to barrier for each filament
        

        if (t > tsav) {                                                         // For certain save states
            if (tsav > 0.0) {                                                   // Skip first (maunally initialized)
                double elongationRate = (farthestDimer - prevFarthestDimer) /   // Calculate elongationRate
                                        (dtSav * 1000.0);                       // Scaling (*1000.0) is by AI
                
                sumElongation += elongationRate;                                // Add to sum
                ++nSamples;
            }
            prevFarthestDimer = farthestDimer;                                  // Update the previous fartherst dimer
            tsav += dtSav;                                                      // Progress time save
        }
    }
    AvgElongationRate = sumElongation/nSamples;                                 // Average elongationRate
}


/*=========================================
===   Member Function: Exporting Data   ===
=========================================*/
void classMicrotubuli::exportData(ofstream &dataFile)                           // Pass file by reference
{
    dataFile << F << "," << AvgElongationRate << "\n";                          // Give F and Rate to file
}


/*=========================================
===                Main                 ===
=========================================*/
int main()
{
    mt19937_64 rng;
    GetSeedrng(rng);
    
    ofstream dataFile("data.csv");
    if(!dataFile.is_open()) {
        throw runtime_error("unable to open datafile.\n");
        }

    for (double F = 0; F < 40; F += 2) {                                        // Give various F values for simulation
        classMicrotubuli myMicrotubuli(F);                                      // Construct object with different F values
        myMicrotubuli.simulationLoop(rng);                                      // Run simulation
        myMicrotubuli.exportData(dataFile);                                     // Export output
        cout << "Simulation with F = " << F 
             << " completed and printed to file\n";
        //myMicrotubuli.print();
    }
    return 0;
}