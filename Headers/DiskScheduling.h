
#include <vector>

#ifndef DISKSCHEDULING_H
#define DISKSCHEDULING_H

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <algorithm>

// Structure to hold the results for one algorithm
struct AlgorithmResult
{
    std::string name;
    int totalMovement = 0;
    double avgSeek = 0.0;
    int maxSeek = 0;
    double stdDevSeek = 0.0;
    double throughput = 0.0;
    std::vector<int> seekSequence;
};

std::vector<int> fcfs(int startHead, const std::vector<int> &requests);
std::vector<int> sstf(int startHead, const std::vector<int> &requests);
std::vector<int> scan(int startHead, int maxCylinder, const std::vector<int> &requests);
std::vector<int> cscan(int startHead, int maxCylinder, const std::vector<int> &requests);
std::vector<int> look(int startHead, const std::vector<int> &requests);
std::vector<int> clook(int startHead, const std::vector<int> &requests);
std::vector<int> hdsa(int startHead, const std::vector<int> &requests);

AlgorithmResult calculateMetrics(const std::string &name, const std::vector<int> &sequence, int numRequests);
#endif // DISKSCHEDULING_H