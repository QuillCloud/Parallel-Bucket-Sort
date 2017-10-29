#include "BucketSort.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <thread>

// define type pair
typedef std::pair<unsigned int, std::vector<unsigned int>> pair;

// sorted bucket using msd radix sort
// @param: 'target_v' the bucket be sort, 'tens' determine the digit location
void msd_radix_sort(std::vector<unsigned int>& target_v,const unsigned int& tens) {
    // if bucket contains less than 1 element, do nothing
    if (target_v.size() <= 1)
        return;
    // store bucket list, index from 0 to 9
    std::map<unsigned int, std::vector<unsigned int>> buckets;
    // store the first bucket, which is even ahead than 0
    std::vector<unsigned int> first_bucket;
    // distribute numbers into buckets
    while (!target_v.empty()) {
        // get number
        auto num = target_v.back();
        target_v.pop_back();
        auto digit = num;
        // get digit depends on 'tens', this digit is the index to determine which bucket to put the number
        // if cannot get digit, put number to first bucket
        if (digit < std::round(std::pow(10,tens-1))) {
            first_bucket.push_back(num);
        } else {
            // get digit
            while (digit >= std::round(std::pow(10,tens))) {
                digit /= 10;
            }
            digit %= 10;
            // create bucket if not exists
            if (buckets.find(digit) == buckets.end()) {
                buckets.insert(pair(digit, std::vector<unsigned int>()));
            }
            // put the number into bucket
            buckets.at(digit).push_back(num);
        }
    }
    // sort the buckets excluding the first bucket
    for (auto& i : buckets) {
        // recursion, tens+1 to get next digit
        msd_radix_sort(i.second, tens+1);
    }
    // add first bucket to original bucket
    target_v.insert(target_v.end(), first_bucket.begin(), first_bucket.end());
    // add rest buckets to original bucket
    for (auto& i : buckets) {
        target_v.insert(target_v.end(), i.second.begin(), i.second.end());
    }
    return;
};

void BucketSort::sort(unsigned int numCores) {
    // use a map as buckets
    std::map<unsigned int, std::vector<unsigned int>> buckets;
    // distribute numbers into buckets
    // chose first two significant digit to determine the bucket
    // so that each thread's total size of buckets will be more evenly
    while (!numbersToSort.empty()) {
        // get number
        auto num = numbersToSort.back();
        numbersToSort.pop_back();
        auto first_digit = num;
        // get first two significant digit as the bucket to put
        while (first_digit >= 100) {
            first_digit /= 10;
        }
        // if the number less than 10 and not 0, put to bucket 'number*10'
        if (first_digit < 10 && first_digit != 0) {
            first_digit *= 10;
        }
        // create bucket if not exists, and put the number into bucket
        if ( buckets.find(first_digit) == buckets.end() ) {
            buckets.insert(pair(first_digit, std::vector<unsigned int>()));
        }
        // put the number into bucket
        // if the number less than 10 and not 0, insert to the front of bucket
        if (num < 10 && num != 0) {
            buckets.at(first_digit).insert(buckets.at(first_digit).begin(), num);
        } else {
            buckets.at(first_digit).push_back(num);
        }
    }
    // lambda function for a single thread, sort each bucket for this thread
    auto sortFunc = [&buckets] (std::vector<unsigned int> bucket_list) {
        for (auto& i : bucket_list) {
            msd_radix_sort(buckets[i], 2);
        }
    };
    // store the index of buckets in map for each thread
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
    // distribute buckets for each thread, each time add bucket to the thread with the smallest size
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
