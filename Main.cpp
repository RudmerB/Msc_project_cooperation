#include <iostream>
#include <array>
#include <random>
#include <chrono>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <numeric>
#include <algorithm>
#include <thread>
using namespace std;
/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------------------         Global parameters            ---------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
constexpr int nrIndividuals     = 100;              // Nr of individuals in one population
constexpr int nrStrategies      = 12;               // Total number of social strategies (Leading eight, image-scoring etc.)
constexpr int nrPermutations    = 132;              // Number of population permutations (number of unique strategy combinations)


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------         Check global parameters with input file             ----------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void checkGlobalParameters()
{
    ifstream csvFile("InputFile.csv");
    if(!csvFile.is_open()){
        cerr<<"error:unable to open data file\n";
        exit(EXIT_FAILURE);
    }
    
    int rowCount = 0;                                           // Create row counter
    string line;                                                // Create empty string for lines in input file
    while (getline(csvFile, line)) {                            // For each line in the input file...
        
        ++rowCount;                                             // Increase row counter
        vector<int> row;                                        // Initialize a vector
        istringstream lineStream(line);                         // Transform current line to stream object
        string val;                                             // Create empty string for values in line
        
        while(getline(lineStream, val, ',')){                   // For each value in the current line (comma-separated)...
            row.push_back(stoi(val));                           // Add the value (string-to-integer) to the row vector
        }

        if (static_cast<int>(row.size()) != nrStrategies) {     // If the row length does not equal global paramater...
            cerr << "\nError: row " << rowCount << " has "      // ...throw error message
                 << row.size() << " columns, expected "
                 << nrStrategies << "\n\n";
            exit(EXIT_FAILURE);                                 // Exit program
        }

        int sum = 0;                                            // Initialize individual counter
        for (int v : row) {                                     // Run over the current line 
            sum += v;                                           // Add the current value to the sum
        }

        if (sum != nrIndividuals) {                             // If the sum does not equal global parameter...
            cerr << "\nError: row " << rowCount                 // ...throw error message
                 << " sums to " << sum << ", expected " 
                 << nrIndividuals << "\n\n";
            exit(EXIT_FAILURE);                                 // Exit program
        }
    }
    csvFile.close();                                            // Close File

    if (rowCount != nrPermutations) {                           // If the number of rows does not equal global parameter...
        cerr << "\nError: file has " << rowCount                // ...throw error message
             << " rows, expected " << nrPermutations 
             << "\n\n";
        exit(EXIT_FAILURE);                                     // Exit program
    }
}
//https://stackoverflow.com/questions/50085440/what-is-the-fastest-way-to-get-the-number-of-rows-in-a-csv-file


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------         Reading strategy frequencies from file            ------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
------------------------         Social Norms (struct)             -------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
struct strategiesSocialNorms{
    // 8 letter codes how leading eight strategies update reputation of donors
    // G/B = Good/Bad reputation, C/D = Cooperation/Defection
    // 1st = donor, 2nd = recipient, 3rd = action
    // * = same between strategies

    array<array<char, 8>, nrStrategies> socialNorm = {{
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


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
------------------------         Action rules (struct)             -------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
struct strategiesActionRules{
    // 4 letter codes on how leading eight strategies behave as donor
    // there are only 2 different options
    // G/B = Good/Bad reputation, C/D = Cooperation/Defection
    // 1st = donor, 2nd = recipient

    array<array<char, 4>, nrStrategies> actionRule = {{
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


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
---------------------------         Seeding function             ---------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void GetSeedRNG(mt19937_64 &rng)
{
    unsigned seed =
    static_cast<unsigned>(chrono::high_resolution_clock::now().time_since_epoch().count());
    clog << "random seed : " << seed << "\n";
    rng.seed(seed);
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
--------------------------         Shuffling function             --------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
-----------------------         Create results directory             -----------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
string CreateResultsDirectory(const double cost, 
                              const double benefit,
                              const double error1,
                              const double error2)
{
    ostringstream oss_e1;
    ostringstream oss_e2;
    oss_e1 << defaultfloat << (double)error1;
    oss_e2 << defaultfloat << (double)error2;
    
    //"_e2-" + oss.str();

    string resultsFolder = "results_c" + to_string((int)cost) + 
                           "_b" + to_string((int)benefit) +
                           "_e1-" + oss_e1.str() + 
                           "_e2-" + oss_e2.str();
    system(("mkdir " + resultsFolder).c_str());                         // Create subfolder for results
    return resultsFolder;
}

/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
--------------------------         Individual (struct)            --------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
struct Individual{
    array<char, 8> socialNorm;                                      // Each individual has a social norm (how he updates reputations when he is observer)
    array<char, 4> actionRule;                                      // Each individual has action rules (what to do when he himself is actor)
    array<char, nrIndividuals> privateReputations;                  // Each individual can store the reputation of each other individual 
    double PayOff;                                                  // Each individual has a pay-off
    bool isResident = false;

    Individual() = default;                                         // Basic constructor 
    Individual(const char& startingReputation,                      // Constructor
               const double& startingPayOff);                                                 
};


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
-------------------------         Individual constructor            ------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
Individual::Individual(const char& startingReputation,
                       const double& startingPayOff)
{
    privateReputations.fill(startingReputation);                    // Initiliaze everyone's reputation
    PayOff = startingPayOff;                                        // Initialize pay off 
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
----------------------------         Results (struct)            ---------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
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
    classPopulation(const strategiesSocialNorms& socialNorms,                       // Population class constructor
                    const strategiesActionRules& actionRules,
                    const array<int, nrStrategies>& nrIndividualsPerStrategy,
                    const char startingReputation,
                    const double startingPayOff);

    void printtest();                                                               // Print test (not used in current code 13/05)

    bool DetermineAction(const int donor,                                           // Function to determine action based on action rule
                         const int recipient,
                         uniform_real_distribution<double>& uniform,
                         mt19937_64& rng,
                         const double error1);

    void UpdatePayOff(const bool action,                                           // Update pay off based on the donor's action
                      const int donor,
                      const int recipient,
                      const double cost,
                      const double benefit);

    void UpdatePrivateReputations(const bool action,                               // Update pay offs based on social norms 
                                  const int donor,
                                  const int recipient,
                                  uniform_real_distribution<double>& uniform,
                                  mt19937_64& rng,
                                  const double error2);

    void PlayInteractionRound(mt19937_64 &rng,                                      // Play one round of interactions = 
                              const double error1,                                  // Everyone is in an interaction once, either as donor or recipient
                              const double error2,                                  // Meaning nrIndividuals / 2 interactions are played 
                              const double cost,
                              const double benefit);
    
    StrategyPayOff DetermineStrategyPayOff();                                       // Give both the pay offs of the mutant and the average resident pay off

    void ResetIndividuals(const double startingPayOff,
                          const char stratingReputation);

    void RunReplicas(const int replicasPerPop,
                     const double startingPayOff,
                     const char startingReputations,
                     const int totalRounds,
                     mt19937_64 &rng,
                     const double error1,
                     const double error2,
                     const double cost,
                     const double benefit,
                     ofstream &outputFile);

private:
    array <Individual, nrIndividuals> population;                                    // Store the individuals in an array
};


/*==========================================================================================
============================================================================================
=====================         Population - Class Constructor         =======================
============================================================================================
==========================================================================================*/
classPopulation::classPopulation(const strategiesSocialNorms& socialNorms,
                                 const strategiesActionRules& actionRules,
                                 const array<int, nrStrategies>& nrIndividualsPerStrategy,
                                 const char startingReputation,
                                 const double startingPayOff
                                 )
{
    int maxValue = *max_element(nrIndividualsPerStrategy.begin(),                       // Find the highest value of the current permutation
                                nrIndividualsPerStrategy.end());
    int populationIndex = 0;                                                            // Keeps tab on how many individuals have been assigned

    for(int i = 0; i < nrStrategies; ++i) {                                             // Loop over the different strategies
        bool isResident = (nrIndividualsPerStrategy[i] == maxValue);                    // Check if the current strategy has the most individuals, coin this the resident strategy
        
        for(int count = 0; count < nrIndividualsPerStrategy[i]; ++count) {              // Loop over the number of individuals per strategy 
            if (populationIndex >= nrIndividuals) {                                     // Safety check: 'Are we counting more individuals than possible?'
                throw logic_error("The index counts a larger population " 
                                  "than the real population size.\n");
            }
            
            population[populationIndex] = Individual(startingReputation,                // Construct an individual at this location with reputation and payoff given in main()
                                                     startingPayOff);
            population[populationIndex].socialNorm = socialNorms.socialNorm[i];         // Assign the respective social norm to the individual of that strategy
            population[populationIndex].actionRule = actionRules.actionRule[i];         // Do the same for the action rule

            if (isResident) {                                                           // Check if the current strategy is the resident strategy
                population[populationIndex].isResident = true;                          // Store the individual as a resident (used for average payoff later)
            }
            ++populationIndex;                                                          // Increase the counter to initiate unassigned individuals
        }
    }

    if (populationIndex != nrIndividuals) {                                             // Safety check: 'Did we assign the correct number of individuals?'
        throw logic_error("The number of assigned individuals does not "
                          "equal the population size.\n");
    }
}


/*==========================================================================================
============================================================================================
=====================          Population - terminal tester (unused) =======================
============================================================================================
==========================================================================================*/
void classPopulation::printtest()
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

        cout << "The individual is a resident : " << population[i].isResident << endl;
    }
}


/*==========================================================================================
============================================================================================
=====================         Population - Determine Action          =======================
============================================================================================
==========================================================================================*/
bool classPopulation::DetermineAction(const int donor,
                                      const int recipient,
                                      uniform_real_distribution<double>& uniform,
                                      mt19937_64& rng,
                                      const double error1)
{
    bool bDonorRep = (population[donor].privateReputations[donor] == 'G') ? 0 : 1;                      // What reputation has the donor from their own perspective?
    bool bRecipientRep = (population[donor].privateReputations[recipient] == 'G') ? 0 : 1;              // What reputation has the recipient from the donor's persepctive?

    bool bAction = (population[donor].actionRule[2 * bDonorRep + bRecipientRep] == 'C') ? 0 : 1;        // The donor performs an action based on their action rule 
  
    if (uniform(rng) < error1) {                                                                        // Take a random number from the distribution and compare to error rate
        bAction = !bAction;                                                                             // Convert the choosen action to opposite action
        }

    return bAction;
}


/*==========================================================================================
============================================================================================
=====================           Population - Update Payoff           =======================
============================================================================================
==========================================================================================*/
void classPopulation::UpdatePayOff(const bool action,
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
void classPopulation::UpdatePrivateReputations(const bool action,
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
void classPopulation::PlayInteractionRound(mt19937_64 &rng,
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
=====================       Mutant and  Average Resident Payoff       =======================
============================================================================================
==========================================================================================*/
StrategyPayOff classPopulation::DetermineStrategyPayOff()
{
    StrategyPayOff Results;                                     // Create a structure for results
    double dResidentSum = 0.0;                                  // Initiate sum of pay offs
    int iResidentsNr = 0;                                       // Initiate resident counter
    double dMutantSum = 0.0;
    int iMutantsNr = 0;

    for (int i = 0; i < nrIndividuals; ++i) {                   // Loop over all individuals
        if (population[i].isResident) {                         // Check if the individual is a resident
            dResidentSum += population[i].PayOff;               // Add their payoff to the sum
            ++ iResidentsNr;                                    // Increase resident counter with 1
        }
        else {                                                  // If an indvidual is not a resident
            dMutantSum += population[i].PayOff;
            ++ iMutantsNr;
        }
    }
    
    if (iResidentsNr == 0 || iMutantsNr == 0) {                 // Safety check: 'Did we assign the correct number of individuals?'
        throw logic_error("There are no residents or mutants found.\n");
    }

    Results.avgResidentPayOff = dResidentSum/iResidentsNr;      // Store the average resident payoff in the struct
    Results.mutantPayOff = dMutantSum/iMutantsNr;

    return Results;                                             // Return the struct
}


/*==========================================================================================
============================================================================================
======================       Reset Payoff and Reset Reputations     ========================
============================================================================================
==========================================================================================*/
void classPopulation::ResetIndividuals(const double startingPayOff,
                                       const char startingReputation)
{
    for (auto& individual : population) {
        individual.PayOff = startingPayOff;
        individual.privateReputations.fill(startingReputation);
    }
}


/*==========================================================================================
============================================================================================
=============================       Simulation Replicas       ==============================
============================================================================================
==========================================================================================*/
void classPopulation::RunReplicas(const int replicasPerPop,
                                  const double startingPayOff,
                                  const char startingReputation,
                                  const int totalRounds,
                                  mt19937_64 &rng,
                                  const double error1,
                                  const double error2,
                                  const double cost,
                                  const double benefit,
                                  ofstream &outputFile)
{
    StrategyPayOff Results;                                                                // Create a struct for results
    for(int replica = 0; replica < replicasPerPop; ++replica){
        ResetIndividuals(startingPayOff, startingReputation);
        for(int round = 0; round < totalRounds; ++round){
            PlayInteractionRound(rng, error1, error2, cost, benefit);                      // Play interactions for certain number of rounds
        }
        Results = DetermineStrategyPayOff();                                               // Store payoffs
        outputFile << Results.avgResidentPayOff << "," << Results.mutantPayOff << "\n";    // Write payoffs to output file
    }    
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
-----------------------       Terminal output Progress               -----------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
void PrintProgress(const int simulation,
                   const int nrThreads,
                   const int nrPermutations,
                   const chrono::high_resolution_clock::time_point start)
{
    auto batchEnd = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = batchEnd - start;
    cout << "Completed simulations " << simulation << " to " 
         << min(simulation + nrThreads - 1, nrPermutations - 1) 
         << " | Time elapsed: " << elapsed.count() << "s"
         << " (" << elapsed.count() / 60 << "m)"
         << " | Estimated remaining: " 
         << (elapsed.count() / (simulation + nrThreads)) * (nrPermutations - simulation - nrThreads) 
         << "s"
         << " (" << ((elapsed.count() / (simulation + nrThreads)) * (nrPermutations - simulation - nrThreads)) / 60 << "m)"
         << "\n";
}


/*------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------
----------------------------------         Main             --------------------------------
--------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------*/
int main()
{
    //--- parameters ---
    double  error1              = 0.01;
    double  error2              = 0.05;
    double  cost                = 1;
    double  benefit             = 2;
    char    startingReputation  = 'G';
    double  startingPayOff      = 0.0;
    int     totalRounds         = 100;
    int     replicasPerPop      = 10;


    //--- random nr generator ---
    mt19937_64 rng;                         // Initialize rng
    GetSeedRNG(rng);                        // Seed the rng with current time
    unsigned int baseSeed = rng();          // Extract base seed


    //--- timer for terminal output ---
    auto start = chrono::high_resolution_clock::now();                  // For terminal output


    //--- initialize structures ---
    strategiesSocialNorms socialNorms;      // Create socialnorm struct
    strategiesActionRules actionRules;      // Create actionrule struct
    

    //--- open input file ---
    ifstream inputFile("InputFile.csv");
    if(!inputFile.is_open()){
        cerr<<"error:unable to open input file\n";
        exit(EXIT_FAILURE);
    }

    checkGlobalParameters();                   // Function that checks whether constexpr at top = file input

    //--- read strategy permutation ---
    array<array<int, nrStrategies>, nrPermutations> allPopulations;     // Create an array of arrays
    for (int i = 0; i < nrPermutations; ++i){                           // Loop over the strategy permutations
        allPopulations[i] = ReadStrategyFrequencies(inputFile);         // Store the permutations in the array
    }
    inputFile.close();                                                  // Close the input file


    //--- setup ---
    string resultsFolder = CreateResultsDirectory(cost, benefit,        // Create results directory and stores the name prefix for result files
                                                  error1, error2);

    
    int nrThreads = thread::hardware_concurrency();                     // Read the number of threads available on current device
    if(nrThreads == 0){                                                 // If the above fails...
        nrThreads = 4;                                                  // ...set the number of threads to 4
    }
    cout << "Running with " << nrThreads << " number of threads\n";     // Write number of threads to terminal


    //--- simulation loop ---
    for(int simulation = 0; simulation < nrPermutations; simulation += nrThreads){          // Runs over simulations in groups based on nrThreads
    vector<thread> threads;                                                                 // Create vector
    threads.reserve(nrThreads);                                                             // Reserve space for vector

        for(int t = 0; t < nrThreads && simulation + t < nrPermutations; ++t){              // Gives one simulation to each thread, and prevents out of bounds
            int sim = simulation + t;
            
            threads.push_back(thread([=](){                                                 // Create lambda function (function with no name)
                mt19937_64 rng(baseSeed + sim);                                             // Create a rng with time and simulation nr
                array<int, nrStrategies> nrIndividualsPerStrategy = allPopulations[sim];    // Store strategy distribution of population 
                
                classPopulation myPopulation(socialNorms,                                   // Create population
                                             actionRules,
                                             nrIndividualsPerStrategy,
                                             startingReputation,
                                             startingPayOff);
                
                ofstream outputFile(resultsFolder + "/Output_" +                            // Create output file for population
                                    to_string(sim) + ".csv");
                if(!outputFile.is_open()){                                                  // Open output file
                    cerr << "Error: unable to open output file\n";
                    return;
                }
                
                myPopulation.RunReplicas(replicasPerPop, startingPayOff,                    // Run replicas of the population
                                         startingReputation, totalRounds, 
                                         rng, error1, error2, cost, 
                                         benefit, outputFile);
                //if (sim == 21) {
                //    myPopulation.printtest();
                //    for (auto x : nrIndividualsPerStrategy) {
                //    cout << x << ", ";
                //    }
                //}
                outputFile.close();                                                         // Close output file
            }));
        }

        for(auto& t : threads){                                                             // Check all threads                
            t.join();                                                                       // Wait for all threads to be done
        }

        PrintProgress(simulation, nrThreads, nrPermutations, start);                        // Print to terminal
    }
    return 0;
}
