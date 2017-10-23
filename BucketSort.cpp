#include "BucketSort.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <iostream>
#include <thread>

typedef std::pair<unsigned int, std::vector<unsigned int>> pair;
typedef std::pair<const unsigned int, std::vector<unsigned int>> pair_c;

// compare two unsigned integer in lexicographic order
bool compare(const unsigned int& x, const unsigned int& y) {
    std::string s_x = std::to_string(x);
    std::string s_y = std::to_string(y);
    return s_x < s_y;
}

void BucketSort::sort(unsigned int numCores) {
    // use a map as buckets
    std::map<unsigned int, std::vector<unsigned int>> buckets;
    // distribute numbers into buckets
    while (!numbersToSort.empty()) {
        // get number
        auto num = numbersToSort.back();
        numbersToSort.pop_back();
        auto first_digit = num;
        // get first two digit as the bucket to put
        while (first_digit >= 100) {
            first_digit /= 10;
        }
        // if only one digit, put it into (number * 10) bucket (0 put into first bucket which is 10)
        if (first_digit < 10) {
            if (first_digit == 0)
                first_digit = 10;
            else
                first_digit *= 10;
        }
        // create bucket if not exists, and put the number into bucket
        if ( buckets.find(first_digit) == buckets.end() ) {
            buckets.insert(pair(first_digit, std::vector<unsigned int>()));
        }
        // put the number into bucket
        buckets.at(first_digit).push_back(num);
    }
    // lambda function for a single thread, sort each bucket for this thread
    auto sortFunc = [&buckets] (std::vector<unsigned int> bucket_list) {
        for (auto& i : bucket_list) {
            std::sort(buckets[i].begin(), buckets[i].end(), [] (const unsigned int& x, const unsigned int& y){
                return compare(x,y);
            } );
        }
    };
    // store the location of buckets for each thread
    std::vector<std::vector<unsigned int>> l;
    // count total buckets' sizes for each thread
    std::vector<unsigned long> l_size;
    // initialize the 'l'(start with 0 bucket for each thread) and 'l_size' (start with 0 for each thread)
    for (unsigned int i = 0; i < numCores; ++i) {
        l.emplace_back(std::vector<unsigned int>());
        l_size.push_back(0);
    }
    // store each thread
    std::vector<std::thread> containerOfThreads;
    // distribute buckets for each thread, each time add current bucket to the thread with smallest size
    for (auto& i : buckets) {
        auto min_index = std::min_element(l_size.begin(),l_size.end()) - l_size.begin();
        l[min_index].push_back(i.first);
        l_size[min_index] += i.second.size();
    }
    // start each thread
    for (auto& i : l) {
        containerOfThreads.emplace_back(std::thread(sortFunc, i));
    }
    // join each thread
    for (auto& i : containerOfThreads) {
        i.join();
    }
    // combine the sorted buckets
    for (auto& i : buckets) {
        numbersToSort.insert(numbersToSort.end(), i.second.begin(), i.second.end());
    }
}
