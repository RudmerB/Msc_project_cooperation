// Libraries
#include <iostream>

// Header files
#include "BasePopulation.h"
#include "Utilities.h"
using namespace std;

/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
-------------------------         Individual constructor            ------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
BaseIndividual::BaseIndividual(const char startingReputation,
                               const double startingPayOff)
{
    privateReputations.fill(startingReputation);                    // Initiliaze everyone's reputation
    PayOff = startingPayOff;                                        // Initialize pay off   
}


/*==========================================================================================
============================================================================================
=====================          Population - terminal tester (unused) =======================
============================================================================================
==========================================================================================*/
void BasePopulationClass::printtest()
{
    for(int i = 0; i < nrIndividuals; ++i){
        cout << "\nThe social norms of individual " << i << " is :\n";
        for (int j = 0; j < 8; ++j){
            cout << population[i].socialNorm[j] << " ";
        }
        cout << "\nThe action rule of individual " << i << " is : \n";
        for (int j = 0; j < 4; ++j){
            cout << population[i].actionRule[j] << " ";
        }
        cout << endl;
        cout << "Their reputations of everyone is as follows: \n";
        for (int k =0; k < nrIndividuals; ++k) {
            cout << population[i].privateReputations[k] << " ";
        }
        cout << endl;
        cout << "Their payoff equals : \n";
        cout << population[i].PayOff << endl;

        //cout << "The individual is a resident : " << population[i].isResident << endl;
    }
}


/*==========================================================================================
============================================================================================
=====================         Population - Determine Action          =======================
============================================================================================
==========================================================================================*/
bool BasePopulationClass::DetermineAction(const int donor,
                                      const int recipient,
                                      uniform_real_distribution<double>& uniform,
                                      mt19937_64& rng,
                                      const double error1)
{
    bool bDonorRep = (population[donor].privateReputations[donor] == 'G') ? 0 : 1;                      // What reputation has the donor from their own perspective?
    bool bRecipientRep = (population[donor].privateReputations[recipient] == 'G') ? 0 : 1;              // What reputation has the recipient from the donor's persepctive?

    bool bAction = (population[donor].actionRule[2 * bDonorRep + bRecipientRep] == 'C') ? 0 : 1;        // The donor performs an action based on their action rule 
  
    if (uniform(rng) < error1) {                      
                                                          // Take a random number from the distribution and compare to error rate
        bAction = !bAction;                                                                             // Convert the choosen action to opposite action
        }

    return bAction;
}


/*==========================================================================================
============================================================================================
=====================           Population - Update Payoff           =======================
============================================================================================
==========================================================================================*/
void BasePopulationClass::UpdatePayOff(const bool action,
                                   const int donor,
                                   const int recipient,
                                   const double cost,
                                   const double benefit)
{
        if (action == 0) {                                  // 0 means cooperation happened
            population[donor].PayOff -= cost;               // Donor pays cost
            population[recipient].PayOff += benefit;        // Recipient gets benefit
        }
}


/*==========================================================================================
============================================================================================
=====================         Population - Update reputations        =======================
============================================================================================
==========================================================================================*/
void BasePopulationClass::UpdatePrivateReputations(const bool action,
                                               const int donor,
                                               const int recipient,
                                               uniform_real_distribution<double>& uniform,
                                               mt19937_64& rng,
                                               const double error2)
{
    for (int observer = 0; observer < nrIndividuals; ++observer){                                       // Loop over the population
        if (observer == recipient){                                                                     // If the recipient is the observer...
            continue;                                                                                   // ...move to next individual (recipient judging their donor would be direct reciprocity)
        }

        bool bDonorRepObserver      = (population[observer].privateReputations[donor]                   // What reputation has the donor from the observer's perspective?
                                        == 'G') ? 0 : 1;    
        bool bRecipientRepObserver  = (population[observer].privateReputations[recipient]               // What reputation has the recipient from the observer's perspective?
                                        == 'G') ? 0 : 1;

        bool bNewReputation = (population[observer].socialNorm[4 * bDonorRepObserver +                  // The observer gives a new reputation to donor based on their social rule
                                                                2 * bRecipientRepObserver +             
                                                                action] == 'G') ? 0 : 1;
        if (uniform(rng) < error2) {                                                                    // Take a random number from the distribution and compare to error rate
            bNewReputation = !bNewReputation;                                                           // Convert the assigned reputation to opposite reputation
        }

        population[observer].privateReputations[donor] = (bNewReputation == 0) ? 'G' : 'B';             // Save the new reputation in the memory of the observer
    }
}


/*==========================================================================================
============================================================================================
====================             Population - Interaction            =======================
============================================================================================
==========================================================================================*/
void BasePopulationClass::PlayInteractionRound(mt19937_64 &rng,
                                           const double error1,
                                           const double error2,
                                           const double cost,
                                           const double benefit)
{
    array <int, nrIndividuals> Indexes;                             // Create an array of indexes
    iota(begin(Indexes), end(Indexes), 0);                          // Fill the array with 0, 1, 2, etc
    shuffle(Indexes, rng);                                          // Shuffle the contents of the array 

    uniform_real_distribution<double> uniform(0.0, 1.0);            // Create a uniform distribution from 0 to 1     
    for (int i = 0; i + 1 < nrIndividuals; i += 2)                  // Loop over the population (i + 1 is necessary to handle uneven nr of individuals)
    {
        int iDonorID = Indexes[i];                                  // Create a donor (randomly chosen with the shuffled indexes)
        int iRecipientID = Indexes[i + 1];                          // Create a recipient (randomly chosen with the shuffled indexes)

        bool bAction = DetermineAction(iDonorID, iRecipientID, 
                                       uniform, rng, error1);

        UpdatePayOff(bAction, iDonorID, iRecipientID, 
                     cost, benefit);

        UpdatePrivateReputations(bAction, iDonorID, iRecipientID, 
                                 uniform, rng, error2);
    }
}


/*==========================================================================================
============================================================================================
======================       Reset Payoff and Reset Reputations     ========================
============================================================================================
==========================================================================================*/
void BasePopulationClass::ResetIndividuals(const double startingPayOff,
                                       const char startingReputation)
{
    for (auto& individual : population) {
        individual.PayOff = startingPayOff;
        individual.privateReputations.fill(startingReputation);
    }
}
