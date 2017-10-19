#include "BucketSort.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <iostream>
#include <thread>

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
    // use a map as buckets
    std::map<unsigned int, std::vector<unsigned>> buckets;
    // distribute numbers into buckets
    while (!numbersToSort.empty()) {
        // get number
        auto num = numbersToSort.back();
        numbersToSort.pop_back();
        auto first_digit = num;
        // get most significant digit
        while (first_digit >= 10) {
            first_digit /= 10;
        }
        // create bucket if not exists, and put the number into bucket
        if ( buckets.find(first_digit) == buckets.end() ) {
            buckets.insert(std::pair<unsigned int, std::vector<unsigned int>>(first_digit, std::vector<unsigned int>()));
        }
        // put the number into bucket
        buckets.at(first_digit).push_back(num);
    }
    // lambda function for a single thread, sort each bucket for this thread
    auto sortFunc = [&buckets] (std::vector<unsigned int> bucket_list) {
        std::for_each(bucket_list.begin(), bucket_list.end(), [&buckets] (unsigned int& i) {
            std::sort(buckets[i].begin(), buckets[i].end(), [] (const unsigned int& x, const unsigned int& y){
                return aLessB(x,y,0);
            } );
        });
    };
    // store the location of buckets for each thread
    std::vector<std::vector<unsigned int>> l;
    // count total buckets' sizes for each thread
    std::vector<unsigned long> l_size;
    // initialize the 'l'(start with 0 bucket for each thread) and 'l_size' (start with 0 for each thread)
    for (unsigned int i = 0; i < numCores; ++i) {
        l.push_back(std::vector<unsigned int>());
        l_size.push_back(0);
    }
    // store each thread
    std::vector<std::thread> containerOfThreads;
    // distribute buckets for each thread, each time add current bucket to the thread with smallest size
    std::for_each(buckets.begin(), buckets.end(), [&l, &l_size] (std::pair<unsigned int, std::vector<unsigned>> i) {
        auto min_index = std::min_element(l_size.begin(),l_size.end()) - l_size.begin();
        l[min_index].push_back(i.first);
        l_size[min_index] += i.second.size();
    });
    // start each thread
    std::for_each(l.begin(), l.end(), [&containerOfThreads, &sortFunc] (std::vector<unsigned int>& i) {
        containerOfThreads.push_back(std::thread(sortFunc, i));
    });
    // join each thread
    std::for_each(containerOfThreads.begin(), containerOfThreads.end(), [] (std::thread& i) {
        i.join();
    });
    // combine the sorted buckets
    std::for_each(buckets.begin(), buckets.end(), [&] (std::pair<unsigned int, std::vector<unsigned>>& i) {
        numbersToSort.insert(numbersToSort.end(), i.second.begin(), i.second.end());
    });
}
