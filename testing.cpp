#include <iostream>
#include <array>
#include <random>
#include <chrono>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <thread>
using namespace std;

void GetSeedRNG(mt19937_64 &rng)
{
    unsigned seed =
    static_cast<unsigned>(chrono::high_resolution_clock::now().time_since_epoch().count());
    clog << "random seed : " << seed << "\n";
    rng.seed(seed);
}

void shuffle(array<int, 10> &array,
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

int main()
{
    /*
    mt19937_64 rng;
    GetSeedRNG(rng);
    array <int, 10> array1 {1,2,3,4,5,6,7,8,9,10};
    shuffle(array1, rng);
    for (int i = 0; i < array1.size(); ++i){
        cout << array1[i] << ' ';
    }

    array <int, 10> array2 {1,2,3,4,5,6,7,8,9,10};
    shuffle(array2, rng);
    for (int i = 0; i < array2.size(); ++i){
        cout << array2[i];
    }
    */

    int nrThreads = thread::hardware_concurrency();
    cout << "Running with " << nrThreads << " threds" << endl;

   bool statement = false;
   if (statement) {
    cout << "hello";
   }
    return 0;
    

}