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
    std::map<unsigned int, std::vector<unsigned>> buckets;
    while (!numbersToSort.empty()) {
        auto num = numbersToSort.back();
        numbersToSort.pop_back();
        auto first_digit = num;
        while (first_digit >= 10) {
            first_digit /= 10;
        }
        if ( buckets.find(first_digit) == buckets.end() ) {
            buckets.insert(std::pair<unsigned int, std::vector<unsigned int>>(first_digit, std::vector<unsigned int>()));
        }
        buckets.at(first_digit).push_back(num);
    }
    auto sortFunc = [&buckets] (std::vector<unsigned int> bucket_list) {
        std::for_each(bucket_list.begin(), bucket_list.end(), [&buckets] (auto& i) {
            std::sort(buckets[i].begin(), buckets[i].end(), [] (const unsigned int& x, const unsigned int& y){
                return aLessB(x,y,0);
            } );
        });
    };
    std::vector<std::vector<unsigned int>> l;
    std::vector<unsigned long> l_size;
    for (unsigned int i = 0; i < numCores; ++i) {
        l.push_back(std::vector<unsigned int>());
        l_size.push_back(0);
    }
    std::vector<std::thread> containerOfThreads;
    for (auto it_m = buckets.begin(); it_m != buckets.end(); ++it_m) {
        auto min_index = std::min_element(l_size.begin(),l_size.end()) - l_size.begin();
        l[min_index].push_back(it_m->first);
        l_size[min_index] += it_m->second.size();
    }
    std::for_each(l.begin(), l.end(), [&containerOfThreads, &sortFunc] (auto& i) {
        containerOfThreads.push_back(std::thread(sortFunc, i));
    });
    std::for_each(containerOfThreads.begin(), containerOfThreads.end(), [] (auto& i) {
        i.join();
    });
    std::for_each(buckets.begin(), buckets.end(), [&] (auto& i) {
        numbersToSort.insert(numbersToSort.end(), i.second.begin(), i.second.end());
    });
}
