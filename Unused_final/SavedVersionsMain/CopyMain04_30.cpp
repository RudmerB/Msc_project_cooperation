#include <iostream>
#include <array>
#include <random>
#include <chrono>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
using namespace std;


/*==========================================================================================
============================================================================================
==========================          Global parameters           ============================
============================================================================================
==========================================================================================*/
constexpr int nrIndividuals = 4;              // constexpr initiates int during compling rather than run time (usefull for making arrays)
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
==========================         Individual (struct)          ============================
============================================================================================
==========================================================================================*/
struct Individual{
    array<char, 8> socialNorm;                                      // Each individual has a social norm (how he updates reputations when he is observer)
    array<char, 4> actionRule;                                      // Each individual has action rules (what to do when he himself is actor)
    array<char, nrIndividuals> privateReputations;                  // Each individual can store the reputation of each other individual 
    double PayOff;                                                  // Each individual has a pay-off

    Individual() = default;
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
==========================          Population (class)          ============================
============================================================================================
==========================================================================================*/
class classPopulation{
public:
    classPopulation(const char& startingReputation,
                    const double& startingPayOff,const strategiesSocialNorms&,
                    const strategiesActionRules&,
                    const array<int, nrStrategies>&);
    void printtest();
    void Interaction(Individual &donor,
                     Individual &recipient);
    void Draw2RandomIndividuals(mt19937_64 &rng,
                                double &error1,
                                double &error2,
                                double &cost,
                                double &benefit);
    
    void UpdatePrivateReputations();
    void CalculatePayOff();
    void UpdateStrategies();
private:
    array <Individual, nrIndividuals> population;
    int iDonorID;
    int iRecipientID;
};


/*==========================================================================================
============================================================================================
==========================           Class Constructor          ============================
============================================================================================
==========================================================================================*/
classPopulation::classPopulation(const char& startingReputation,
                                 const double& startingPayOff,
                                 const strategiesSocialNorms& socialNorms,
                                 const strategiesActionRules& actionRules,
                                 const array<int, nrStrategies>& nrIndividualsPerStrategy
                                 )
{
    int populationIndex = 0;                                                        // Keeps tab on how many individuals have been assigned

    for(int i = 0; i < nrIndividualsPerStrategy.size(); ++i) {                      // Loop over the different strategies
        for(int count = 0; count < nrIndividualsPerStrategy[i]; ++count) {          // Loop over the number of individuals per strategy 
            if (populationIndex >= population.size()) {                             // Safety check: 'Are we counting more individuals than possible?'
                throw logic_error("The index counts a larger population than the real population size.\n");
            }
            population[populationIndex] = Individual(startingReputation, startingPayOff);
            population[populationIndex].socialNorm = socialNorms.socialNorm[i];     // Assign the respective social norm to the individual of that strategy
            population[populationIndex].actionRule = actionRules.actionRule[i];     // Do the same for the action rule
            ++populationIndex;                                                      // Add an individual to the counter
        }
    }

    if (populationIndex != population.size()) {                                     // Safety check: 'Did we assign all individuals?'
        throw logic_error("The number of assigned individuals does not equal the population size.\n");
    }
}

void classPopulation::printtest()
{
    for(int i = 0; i < population.size(); ++i){
        cout << "\nThe social norms of individual " << i << " is :\n";
        for (int j = 0; j < 8; ++j){
            cout << population[i].socialNorm[j] << " ";
        }
        cout << endl;
        cout << "Their reputations of everyone is as follows: \n";
        for (int k =0; k < population.size();++k) {
            cout << population[i].privateReputations[k] << " ";
        }
        cout << endl;
        cout << "Their payoff equals : \n";
        cout << population[i].PayOff << endl;
    }
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


void classPopulation::Interaction(Individual &donor,
                                  Individual &recipient)
{

}


/*==========================================================================================
============================================================================================
==========================             Interaction              ============================
============================================================================================
==========================================================================================*/
void classPopulation::Draw2RandomIndividuals(mt19937_64 &rng,
                                             double &error1,
                                             double &error2,
                                             double &cost,
                                             double &benefit)
{
    array <int, nrIndividuals> Indexes;                     // Create an array of indexes
    iota(begin(Indexes), end(Indexes), 0);                  // Fill the array with 0, 1, 2, etc
    shuffle(Indexes, rng);                                  // Shuffle the contents of the array 

    uniform_real_distribution<double> uniform(0.0, 1.0);    // Create a uniform distribution from 0 to 1     
    for (int i = 0; i + 1 < nrIndividuals; i += 2)          // Loop over the population (i + 1 is necessary to handle uneven nr of individuals)
    {
        int iDonorID = Indexes[i];                          // Create a donor (randomly chosen with the shuffled indexes)
        int iRecipientID = Indexes[i + 1];                  // Create a recipient (randomly chosen with the shuffled indexes)

        // Choose action of donor
        bool bDonorRep = (population[iDonorID].privateReputations[iDonorID] == 'G') ? 0 : 1;                // What reputation has the donor from their own perspective?
        bool bRecipientRep = (population[iDonorID].privateReputations[iRecipientID] == 'G') ? 0 : 1;        // What reputation has the recipient from the donor's persepctive?

        bool bAction = (population[iDonorID].actionRule[2 * bDonorRep + bRecipientRep] == 'C') ? 0 : 1;     // The donor performs an action based on their action rule (Formula by AI)
        
        if (uniform(rng) <= error1) {                       // Take a random number from the distribution and compare to error rate
            bAction = !bAction;                             // Convert the choosen action to opposite action
            cout << "A type 1 error has been made" << endl;
        }

        if (bAction == 0) {                                 // 0 means cooperation happened
            population[iDonorID].PayOff -= cost;            // Donor pays cost
            population[iRecipientID].PayOff += benefit;     // Recipient gets benefit
        }

        // Update the reputation of the donor (everyone except the recipient)
        for (int observer = 0; observer < nrIndividuals; ++observer){                                       // Loop over the population
            if (observer == iRecipientID){                                                                  // If the recipient is the observer
                continue;                                                                                   // Move to the next individual
            }

            bool bDonorRepObserver      = (population[observer].privateReputations[iDonorID]                // What reputation has the donor from the observer's perspective?
                                           == 'G') ? 0 : 1;    
            bool bRecipientRepObserver  = (population[observer].privateReputations[iRecipientID]            // What reputation has the recipient from the observer's perspective?
                                           == 'G') ? 0 : 1;

            bool bNewReputation = (population[observer].socialNorm[4 * bDonorRepObserver +                  // The observer gives a new reputation to donor based on their social rule
                                                                   2 * bRecipientRepObserver +              // (Formula by AI)
                                                                   bAction] == 'G') ? 0 : 1;
            if (uniform(rng) <= error2) {                                                                   // Take a random number from the distribution and compare to error rate
                bNewReputation = !bNewReputation;                                                           // Convert the assigned reputation to opposite reputation
                cout << " switch error2" << endl;
            }
            cout << "The observer " << observer << " saw the donor " << iDonorID << " do (0=c, 1=d)" << bAction << " with recipient " << iRecipientID << endl;
            cout << "The observer saw the donor as " << population[observer].privateReputations[iDonorID] << endl;
            population[observer].privateReputations[iDonorID] = (bNewReputation == 0) ? 'G' : 'B';          // Save the new reputation in the memory of the observer
            cout << "The reputation that " << observer << " has given " << iDonorID << " is " << population[observer].privateReputations[iDonorID] << endl;
            cout << "\n\n";
        }
    }
}


void classPopulation::UpdatePrivateReputations()
{
    // After a donor has executed its action everyone else (? or does not everyone update?)
    // updates their view on the donor
}

void classPopulation::CalculatePayOff()
{
    // Calculate the accumulated pay off in last generation
    // so subtract cost from pay off when i was donor and defected
    // and add benefit each time i was recipient and the donor cooperated
}

void classPopulation::UpdateStrategies()
{
    // Determine the probability for whether i chooses to swap its strategy with j
    // How do we select j?
    // Is it possible that j is the same strategy as i?
    // Pr(i -> j) = 1 / [1 + exp(-s(Pj - Pi))]
}

/*==========================================================================================
============================================================================================
==========================                 Main                 ============================
============================================================================================
==========================================================================================*/
int main()
{
    //parameters
    double error1 = 0.5;
    double error2 = 0.05;
    double cost = 1;
    double benefit = 2;
    char startingReputation = 'G';
    double startingPayOff = 0.0;

    mt19937_64 rng;
    GetSeedRNG(rng);

    int nrSimulations = countNrSimulations();
    strategiesSocialNorms socialNorms;
    strategiesActionRules strategies;

    ifstream csvFile("InputFile.csv");
    if(!csvFile.is_open()){
    cerr<<"error:unable to open data file\n";
    exit(EXIT_FAILURE);
    }


    for(int i = 0; i < nrSimulations; ++i){
        //cout << "Simulation: \n" << i+1 << endl;
        
        string line;                                                // Initiate a string
        array<int, nrStrategies> nrIndividualsPerStrategy;         // create/empty array
        getline(csvFile, line);                             // read next line in input file
        stringstream ss(line);                              // Creates a copy (stream object) of the line 


        if (i != 126) { //********************************************************************************************************
            continue;
        }


        for(int j = 0; j < nrStrategies; ++j) {             // loop over strategies
            string value;                                   // Inititae a string
            getline(ss, value, ',');                        // Read the value of the current postion in stream object 
            nrIndividualsPerStrategy[j] = stoi(value);             // set the nrIndividuals in array to that value
        }
        
        for (auto x : nrIndividualsPerStrategy) {                  // Check to see what previous code does, it works, but is it good?
            cout << x << ", ";
        }
        cout << endl;
        
        classPopulation myPopulation(startingReputation,
                                     startingPayOff,
                                     socialNorms,
                                     strategies,
                                     nrIndividualsPerStrategy);
        
        myPopulation.printtest();
        for (int k = 0; k < 3; ++k) {
            cout << "\n\n Start round " << k + 1 << " !!!\n\n";
            myPopulation.Draw2RandomIndividuals(rng, error1, error2, cost, benefit);
            myPopulation.printtest();
        }
    }
    return 0;
}