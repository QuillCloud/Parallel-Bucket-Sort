#include "BucketSort.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <iostream>
#include <thread>
#include <ctime>
using namespace std;

using std::cout;
using std::endl;
bool aLessB(const unsigned int& x, const unsigned int& y, unsigned int pow) {

    if (x == y) return false; // if the two numbers are the same then one is not less than the other

    unsigned int a = x;
    unsigned int b = y;

    // work out the digit we are currently comparing on.
    if (pow == 0) {
        while (a / 10 > 0) {
            a = a / 10;
        }
        while (b / 10 > 0) {
            b = b / 10;
        }
    } else {
        while (a / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
            a = a / 10;
        }
        while (b / 10 >= (unsigned int) std::round(std::pow(10,pow))) {
            b = b / 10;
        }
    }

    if (a == b)
        return aLessB(x,y,pow + 1);  // recurse if this digit is the same
    else
        return a < b;
}

void BucketSort::sort(unsigned int numCores) {
    std::map<unsigned int, std::vector<unsigned>> buckets;
    while (!numbersToSort.empty()) {
        auto num = numbersToSort.back();
        numbersToSort.pop_back();
        auto first_digit = num;
        while (first_digit > 10) {
            first_digit /= 10;
        }
        if ( buckets.find(first_digit) == buckets.end() ) {
            buckets.insert(std::pair<unsigned int, std::vector<unsigned int>>(first_digit, std::vector<unsigned int>()));
        }
        buckets.at(first_digit).push_back(num);
    }
    auto sortFunc = [&buckets](std::vector<unsigned int> bucket_list) {
        std::for_each(bucket_list.begin(), bucket_list.end(), [&buckets] (auto& i) {
            std::sort(buckets[i].begin(), buckets[i].end(), [](const unsigned int& x, const unsigned int& y){
                return aLessB(x,y,0);
            } );
        });
    };
    auto base_num = buckets.size() / numCores;
    auto extra_num = buckets.size() % numCores;
    unsigned int counter = 0;

    std::vector<unsigned int> l;
    std::vector<std::thread> containerOfThreads;
    for (auto it = buckets.begin(); it != buckets.end(); ++it) {
        l.push_back(it->first);
        ++counter;
        if (base_num == counter && extra_num == 0) {
            containerOfThreads.push_back(std::thread(sortFunc, l));
            l.clear();
            counter = 0;
        } else if (base_num < counter){
            containerOfThreads.push_back(std::thread(sortFunc, l));
            l.clear();
            counter = 0;
            --extra_num;
        }
    }
    clock_t begin = clock();
    for (auto& t : containerOfThreads) {
        t.join();
    }
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << "time : " << elapsed_secs << endl;
    for (auto it = buckets.begin(); it != buckets.end(); ++it) {
        for (auto& i : it->second) {
            numbersToSort.push_back(i);
        }
    }
}