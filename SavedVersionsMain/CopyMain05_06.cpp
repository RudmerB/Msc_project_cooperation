#include <iostream>
#include <array>
#include <random>
#include <chrono>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm> // find largest element
using namespace std;


/*==========================================================================================
============================================================================================
==========================          Global parameters           ============================
============================================================================================
==========================================================================================*/
constexpr int nrIndividuals = 100;              // constexpr initiates int during compling rather than run time (usefull for making arrays)
constexpr int nrStrategies = 12;

int countNrSimulations()
{
    ifstream csvFile("InputFile.csv");
    if(!csvFile.is_open()){
    cerr<<"error:unable to open data file\n";
    exit(EXIT_FAILURE);
    }
    
    // ----------Suggestion by AI----------
    string line;                                            // Create empty string
    getline(csvFile, line);                                 // Fill string with contents of first line
    
    istringstream ss(line);                                 // Make the string a sort of file (so that functions for files can be used)
    string val;                                             // Create another empty string
    vector<int> firstRow;                                   // And create a vector(!) of the first row 
    while(getline(ss, val, ',')) {                          // For each value in the line (seperated by commas) 
        firstRow.push_back(stoi(val));                      // transform value to integer and add to back of the vector
    }
    
    int sum = 0;                                            // create sum
    for (int i = 0; i < firstRow.size(); ++i) {             // Run over the vector
        sum += firstRow[i];                                 // Add each value to the sum
    }
    if (sum != nrIndividuals) {                             // If sum is not equal to global paramters shoot error message
    cerr << "\nError: the file population " << sum << " doesn't match global parameter " 
         << nrIndividuals << "\nSubstitute global parameter at the top of"
         << " the program with the file population " << sum << "\n\n";
    exit(EXIT_FAILURE);
    }
    //----------------------------------------

    csvFile.clear();                                        // clear file (so getline can find the first line again)
    csvFile.seekg(0);                                       // start at top of the file

    //https://stackoverflow.com/questions/50085440/what-is-the-fastest-way-to-get-the-number-of-rows-in-a-csv-file
    string s;
    int rowCount = 0;
    
    while(!csvFile.eof()) {                                 // As long as there are lines in the file
        getline(csvFile, s);                                // Temporarily store the line (so we move over the lines)
        rowCount ++;                                        // Add 1 to rowcount
    }
    csvFile.close();                                        // Close file
    return rowCount;                                        // Give back the number of rows (simulations)
}


/*==========================================================================================
============================================================================================
==========================       Social norms (struct)          ============================
============================================================================================
==========================================================================================*/
struct strategiesSocialNorms{
    // 8 letter codes how leading eight strategies update reputation of donors
    // G/B = Good/Bad reputation, C/D = Cooperation/Defection
    // 1st = donor, 2nd = recipient, 3rd = action
    // * = same between strategies

    array<array<char, 8>, nrStrategies> socialNorm = {{
    //    *     *           *     *     * 
    //   GGC - GGD - GBC - GBD - BGC - BGD - BBC - BBD
        {'G' , 'B' , 'G' , 'G' , 'G' , 'B' , 'G' , 'B'}, // L1
        {'G' , 'B' , 'B' , 'G' , 'G' , 'B' , 'G' , 'B'}, // L2 = consistent standing
        {'G' , 'B' , 'G' , 'G' , 'G' , 'B' , 'G' , 'G'}, // L3 = simple standing (2nd order) (file:///C:/Users/rudme/Downloads/s41598-022-14171-4.pdf)
        {'G' , 'B' , 'G' , 'G' , 'G' , 'B' , 'B' , 'G'}, // L4 
        {'G' , 'B' , 'B' , 'G' , 'G' , 'B' , 'G' , 'G'}, // L5
        {'G' , 'B' , 'B' , 'G' , 'G' , 'B' , 'B' , 'G'}, // L6 = stern judging (2nd order) (file:///C:/Users/rudme/Downloads/s41598-022-14171-4.pdf)
        {'G' , 'B' , 'G' , 'G' , 'G' , 'B' , 'B' , 'B'}, // L7 = staying
        {'G' , 'B' , 'B' , 'G' , 'G' , 'B' , 'B' , 'B'}, // L8 = judging
        {'G' , 'B' , 'B' , 'B' , 'G' , 'B' , 'B' , 'B'}, // Shunning
        {'G' , 'B' , 'G' , 'B' , 'G' , 'B' , 'G' , 'B'}, // Image-scoring
        {'G' , 'G' , 'G' , 'G' , 'G' , 'G' , 'G' , 'G'}, // ALLC (will be ignored)
        {'B' , 'B' , 'B' , 'B' , 'B' , 'B' , 'B' , 'B'}, // ALLD (will be ignored)
    }};
};


/*==========================================================================================
============================================================================================
==========================       Action rules (struct)          ============================
============================================================================================
==========================================================================================*/
struct strategiesActionRules{
    // 4 letter codes on how leading eight strategies behave as donor
    // there are only 2 different options
    // G/B = Good/Bad reputation, C/D = Cooperation/Defection
    // 1st = donor, 2nd = recipient

    array<array<char, 4>, nrStrategies> actionRule = {{
    //    *     *     *
    //    GG  - GB  - BG  - BB
        {'C' , 'D' , 'C' , 'C'}, // L1 (OR) help if recipient is good or you are bad
        {'C' , 'D' , 'C' , 'C'}, // L2 (OR)
        {'C' , 'D' , 'C' , 'D'}, // L3 (CO) pays only attention to recipients reputation (has to be good)
        {'C' , 'D' , 'C' , 'D'}, // L4  ''
        {'C' , 'D' , 'C' , 'D'}, // L5
        {'C' , 'D' , 'C' , 'D'}, // L6
        {'C' , 'D' , 'C' , 'D'}, // L7
        {'C' , 'D' , 'C' , 'D'}, // L8
        {'C' , 'D' , 'C' , 'D'}, // Shunning
        {'C' , 'D' , 'C' , 'D'}, // Image-scoring
        {'C' , 'C' , 'C' , 'C'}, // ALLC
        {'D' , 'D' , 'D' , 'D'}, // ALLD
    }};
};


/*==========================================================================================
============================================================================================
==========================          Seeding Function            ============================
============================================================================================
==========================================================================================*/
void GetSeedRNG(mt19937_64 &rng)
{
    unsigned seed =
    static_cast<unsigned>(chrono::high_resolution_clock::now().time_since_epoch().count());
    clog << "random seed : " << seed << "\n";
    rng.seed(seed);
}


/*==========================================================================================
============================================================================================
==========================          Shuffling function          ============================
============================================================================================
==========================================================================================*/
void shuffle(array<int, nrIndividuals> &array,                      // Shuffle from c++ course, exercise 14.2
             mt19937_64 &rng)
{
    for(int i = array.size()- 1; i > 0; --i) {
    std::uniform_int_distribution<int> randomElement(0, i);
    int j = randomElement(rng);
    int tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
    }
}


/*==========================================================================================
============================================================================================
==========================         Individual (struct)          ============================
============================================================================================
==========================================================================================*/
struct Individual{
    array<char, 8> socialNorm;                                      // Each individual has a social norm (how he updates reputations when he is observer)
    array<char, 4> actionRule;                                      // Each individual has action rules (what to do when he himself is actor)
    array<char, nrIndividuals> privateReputations;                  // Each individual can store the reputation of each other individual 
    double PayOff;                                                  // Each individual has a pay-off
    bool isResident = false;

    Individual() = default;                                         // Basic constructor (used to initialize population, I think)
    Individual(const char& startingReputation,                      // Constructor
               const double& startingPayOff);                                                 
};


/*==========================================================================================
============================================================================================
==========================          Struct constructor          ============================
============================================================================================
==========================================================================================*/
Individual::Individual(const char& startingReputation,
                       const double& startingPayOff)
{
    privateReputations.fill(startingReputation);                    // Initiliaze everyone's reputation
    PayOff = startingPayOff;                                        // Initialize pay off 
}


/*==========================================================================================
============================================================================================
==========================            Results Struct            ============================
============================================================================================
==========================================================================================*/
struct StrategyPayOff{
    double mutantPayOff;
    double avgResidentPayOff;
};


/*==========================================================================================
============================================================================================
==========================          Population (class)          ============================
============================================================================================
==========================================================================================*/
class classPopulation{
public:
    classPopulation(const strategiesSocialNorms& socialNorms,
                    const strategiesActionRules& actionRules,
                    const array<int, nrStrategies>& nrIndividualsPerStrategy,
                    const char& startingReputation,
                    const double& startingPayOff);

    void printtest();

    bool DetermineAction(const int& donor,
                         const int& recipient,
                         uniform_real_distribution<double>& uniform,
                         mt19937_64& rng,
                         const double& error1);

    void UpdatePayOff(const bool& action,
                      const int& donor,
                      const int& recipient,
                      const double& cost,
                      const double& benefit);

    void UpdatePrivateReputations(const bool& action,
                                  const int& donor,
                                  const int& recipient,
                                  uniform_real_distribution<double>& uniform,
                                  mt19937_64& rng,
                                  const double& error2);

    void PlayInteractionRound(mt19937_64 &rng,
                              double &error1,
                              double &error2,
                              double &cost,
                              double &benefit);
    
    StrategyPayOff DetermineStrategyPayOff();

private:
    array <Individual, nrIndividuals> population;
};


/*==========================================================================================
============================================================================================
=====================         Population - Class Constructor         =======================
============================================================================================
==========================================================================================*/
classPopulation::classPopulation(const strategiesSocialNorms& socialNorms,
                                 const strategiesActionRules& actionRules,
                                 const array<int, nrStrategies>& nrIndividualsPerStrategy,
                                 const char& startingReputation,
                                 const double& startingPayOff
                                 )
{
    int maxValue = *max_element(nrIndividualsPerStrategy.begin(), nrIndividualsPerStrategy.end());
    int populationIndex = 0;                                                        // Keeps tab on how many individuals have been assigned

    for(int i = 0; i < nrIndividualsPerStrategy.size(); ++i) {                      // Loop over the different strategies
        for(int count = 0; count < nrIndividualsPerStrategy[i]; ++count) {          // Loop over the number of individuals per strategy 
            if (populationIndex >= population.size()) {                             // Safety check: 'Are we counting more individuals than possible?'
                throw logic_error("The index counts a larger population than the real population size.\n");
            }
            population[populationIndex] = Individual(startingReputation, startingPayOff);
            population[populationIndex].socialNorm = socialNorms.socialNorm[i];     // Assign the respective social norm to the individual of that strategy
            population[populationIndex].actionRule = actionRules.actionRule[i];     // Do the same for the action rule

            if (maxValue == nrIndividualsPerStrategy[i]) {
                population[populationIndex].isResident = true;
            }

            ++populationIndex;                                                      // Add an individual to the counter
        }
    }

    if (populationIndex != population.size()) {                                     // Safety check: 'Did we assign all individuals?'
        throw logic_error("The number of assigned individuals does not equal the population size.\n");
    }
}


/*==========================================================================================
============================================================================================
=====================          Population - terminal tester          =======================
============================================================================================
==========================================================================================*/
void classPopulation::printtest()
{
    for(int i = 0; i < population.size(); ++i){
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
        for (int k =0; k < population.size();++k) {
            cout << population[i].privateReputations[k] << " ";
        }
        cout << endl;
        cout << "Their payoff equals : \n";
        cout << population[i].PayOff << endl;

        cout << "The individual is a resident : " << population[i].isResident << endl;
    }
}


/*==========================================================================================
============================================================================================
=====================         Population - Determine Action          =======================
============================================================================================
==========================================================================================*/
bool classPopulation::DetermineAction(const int& donor,
                                      const int& recipient,
                                      uniform_real_distribution<double>& uniform,
                                      mt19937_64& rng,
                                      const double& error1)
{
    bool bDonorRep = (population[donor].privateReputations[donor] == 'G') ? 0 : 1;                      // What reputation has the donor from their own perspective?
    bool bRecipientRep = (population[donor].privateReputations[recipient] == 'G') ? 0 : 1;              // What reputation has the recipient from the donor's persepctive?

    bool bAction = (population[donor].actionRule[2 * bDonorRep + bRecipientRep] == 'C') ? 0 : 1;        // The donor performs an action based on their action rule (Formula by AI)
  
    if (uniform(rng) <= error1) {                                                                       // Take a random number from the distribution and compare to error rate
        bAction = !bAction;                                                                             // Convert the choosen action to opposite action
        //cout << "A type 1 error has been made" << endl;
        }

    return bAction;
}


/*==========================================================================================
============================================================================================
=====================           Population - Update Payoff           =======================
============================================================================================
==========================================================================================*/
void classPopulation::UpdatePayOff(const bool& action,
                                   const int& donor,
                                   const int& recipient,
                                   const double& cost,
                                   const double& benefit)
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
void classPopulation::UpdatePrivateReputations(const bool& action,
                                               const int& donor,
                                               const int& recipient,
                                               uniform_real_distribution<double>& uniform,
                                               mt19937_64& rng,
                                               const double& error2)
{
    for (int observer = 0; observer < nrIndividuals; ++observer){                                       // Loop over the population
        if (observer == recipient){                                                                     // If the recipient is the observer...
            continue;                                                                                   // ...Move to the next individual
        }

        bool bDonorRepObserver      = (population[observer].privateReputations[donor]                   // What reputation has the donor from the observer's perspective?
                                        == 'G') ? 0 : 1;    
        bool bRecipientRepObserver  = (population[observer].privateReputations[recipient]               // What reputation has the recipient from the observer's perspective?
                                        == 'G') ? 0 : 1;

        bool bNewReputation = (population[observer].socialNorm[4 * bDonorRepObserver +                  // The observer gives a new reputation to donor based on their social rule
                                                                2 * bRecipientRepObserver +             // (Formula by AI)
                                                                action] == 'G') ? 0 : 1;
        if (uniform(rng) <= error2) {                                                                   // Take a random number from the distribution and compare to error rate
            bNewReputation = !bNewReputation;                                                           // Convert the assigned reputation to opposite reputation
            //cout << "A type 2 error has been made" << endl;
        }
        //cout << "The observer " << observer << " saw the donor " << 
        //        donor << " do (0=c, 1=d)" << action << " with recipient " << recipient << endl;
        //cout << "The observer saw the donor as " << 
        //        population[observer].privateReputations[donor] << endl;

        population[observer].privateReputations[donor] = (bNewReputation == 0) ? 'G' : 'B';             // Save the new reputation in the memory of the observer
        
        //cout << "The reputation that " << observer << " has given " << donor << 
        //        " is " << population[observer].privateReputations[donor] << endl;
        //cout << "\n\n";

        // !!!!!!!!!!!!!!!!!!!!!!ALL cout statements can be removed but is handy for testing!!!!!!!!!!!!!!!!!!!!!!!!
    }
}


/*==========================================================================================
============================================================================================
====================             Population - Interaction            =======================
============================================================================================
==========================================================================================*/
void classPopulation::PlayInteractionRound(mt19937_64 &rng,
                                           double &error1,
                                           double &error2,
                                           double &cost,
                                           double &benefit)
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
=====================           Read strategies from File            =======================
============================================================================================
==========================================================================================*/
array<int, nrStrategies> ReadStrategyFrequencies(ifstream& csvFile)
{
    string line;                                            // Initiate a string
    array<int, nrStrategies> nrIndividualsPerStrategy;      // create/empty array

    getline(csvFile, line);                                 // read next line in input file
    stringstream ss(line);                                  // Creates a copy (stream object) of the line 

    for(int j = 0; j < nrStrategies; ++j) {                 // loop over strategies
        string value;                                       // Inititae a string
        getline(ss, value, ',');                            // Read the value of the current postion in stream object 
        nrIndividualsPerStrategy[j] = stoi(value);          // set the nrIndividuals in array to that value
    }


    return nrIndividualsPerStrategy;
}


/*==========================================================================================
============================================================================================
=====================       Mutant and  Average Resident Payoff       =======================
============================================================================================
==========================================================================================*/
StrategyPayOff classPopulation::DetermineStrategyPayOff()
{
    StrategyPayOff Results;                                     // Create a structure for results
    double dSumPayOff = 0.0;                                    // Initiate sum of pay offs
    int iResidentsNr = 0;                                       // Initiate resident counter

    for (int i = 0; i < nrIndividuals; ++i) {                   // Loop over all individuals
        if (population[i].isResident) {                         // Check if the individual is a resident
            dSumPayOff += population[i].PayOff;                 // Add their payoff to the sum
            ++ iResidentsNr;                                    // Increase resident counter with 1
        }
        else {                                                  // If an indvidual is not a resident
            Results.mutantPayOff = population[i].PayOff;        // Store the mutant payoff (only works if there's one mutant)
        }
    }

    Results.avgResidentPayOff = dSumPayOff/iResidentsNr;        // Store the average resident payoff in the struct
    return Results;                                             // Return the struct
}


/*==========================================================================================
============================================================================================
=====================        Average Pay Off between Replicas        =======================
============================================================================================
==========================================================================================*/
//StrategyPayOff classPopulation::

/*==========================================================================================
============================================================================================
====================                       Main                      =======================
============================================================================================
==========================================================================================*/
int main()
{
    //**********   parameters   **********
    double  error1              = 0.01;
    double  error2              = 0.05;
    double  cost                = 1;
    double  benefit             = 2;
    char    startingReputation  = 'G';
    double  startingPayOff      = 0.0;
    int     totalRounds         = 1000;
    //************************************


    //******   random nr generator   *****
    mt19937_64 rng;
    GetSeedRNG(rng);
    //************************************


    //******   initialize structure   *****
    int nrSimulations = countNrSimulations();
    strategiesSocialNorms socialNorms;
    strategiesActionRules strategies;
    //************************************
    

    //***********  open files   **********
    ifstream inputFile("InputFile.csv");
    if(!inputFile.is_open()){
        cerr<<"error:unable to open input file\n";
        exit(EXIT_FAILURE);
    }

    ofstream outputFile("OutputFile.csv");
    if(!outputFile.is_open()){
        cerr<<"error:unavle to open output file\n";
        exit(EXIT_FAILURE);
    }
    //************************************


    for(int i = 0; i < nrSimulations; ++i){
        array<int, nrStrategies> nrIndividualsPerStrategy = ReadStrategyFrequencies(inputFile);

        // Used to check the outcome of one poulation! can be removed later
        //if (i != 126) { 
        //    continue;
        //}
        cout << "Population " << i << "\n";
        // Used to check the string of frequencies
        for (auto x : nrIndividualsPerStrategy) {
            cout << x << ", ";
        }
        cout << endl;
        

        classPopulation myPopulation(socialNorms,
                                     strategies,
                                     nrIndividualsPerStrategy,
                                     startingReputation,
                                     startingPayOff);
        
        //myPopulation.printtest();
        
        for (int round = 0; round < totalRounds; ++round) {
            //cout << "\n\n Start round " << round + 1 << " !!!\n\n";
            myPopulation.PlayInteractionRound(rng, error1, error2, cost, benefit);
            //myPopulation.printtest();
        }
        
        StrategyPayOff Results;
        Results = myPopulation.DetermineStrategyPayOff();
        cout << "The average resident payoff = " << Results.avgResidentPayOff <<
                " and the mutant payoff = " << Results.mutantPayOff << endl;
        outputFile << Results.avgResidentPayOff << "," << Results.mutantPayOff << "\n";
    }
    return 0;
}