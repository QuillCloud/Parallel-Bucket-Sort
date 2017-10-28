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
void msd_radix_sort(std::vector<unsigned int>& target_v,const unsigned long tens) {
    if (target_v.size() <= 1)
        return;
    std::map<unsigned int, std::vector<unsigned int>> buckets;
    std::vector<unsigned int> first_bucket;
    while (!target_v.empty()) {
        // get number
        auto num = target_v.back();
        target_v.pop_back();
        auto digit = num;
        // get first two digit as the bucket to put
        if (digit < std::round(std::pow(10,tens))) {
            first_bucket.push_back(num);
        } else {
            while (digit >= std::round(std::pow(10,tens))) {
                digit /= 10;
            }
            digit %= 10;
            if (buckets.find(digit) == buckets.end()) {
                buckets.insert(pair(digit, std::vector<unsigned int>()));
            }
            // put the number into bucket
            buckets.at(digit).push_back(num);
        }
    }
    for (auto& i : buckets) {
        msd_radix_sort(i.second, tens+1);
    }
    target_v.insert(target_v.end(), first_bucket.begin(), first_bucket.end());
    for (auto& i : buckets) {
        target_v.insert(target_v.end(), i.second.begin(), i.second.end());
    }
    return;
};

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
        if (first_digit < 10 && first_digit != 0) {
            first_digit *= 10;
        }
        // create bucket if not exists, and put the number into bucket
        if ( buckets.find(first_digit) == buckets.end() ) {
            buckets.insert(pair(first_digit, std::vector<unsigned int>()));
        }
        // put the number into bucket
        if (num < 10 && num != 0) {
            buckets.at(first_digit).insert(buckets.at(first_digit).begin(), num);
        } else {
            buckets.at(first_digit).push_back(num);
        }
    }
    // lambda function for a single thread, sort each bucket for this thread
    auto sortFunc = [&buckets] (std::vector<unsigned int> bucket_list) {
        for (auto& i : bucket_list) {
            msd_radix_sort(buckets[i], 1);
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
