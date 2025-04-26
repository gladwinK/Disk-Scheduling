#ifndef DISK_SCHEDULING_H
#define DISK_SCHEDULING_H

#include <vector>
#include <string>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <limits>

struct DiskPerformanceParams
{
    double avgSeekTimePerCylinderMs = 0.1; // Default example value (ms)
    double avgRotationalLatencyMs = 4.0;   // Default example value (ms)
    double transferTimePerRequestMs = 1.0; // Default example value (ms)
};

struct AlgorithmResult
{
    std::string name;
    int totalMovement = 0;
    double avgSeek = 0.0;
    int maxSeek = 0;
    double stdDevSeek = 0.0;
    double throughput = 0.0;
    double avgResponseTime = 0.0; // Average time per request (Seek+Latency+Transfer) in ms
    std::vector<int> seekSequence;
};

int run_sstf_subset(int currentHead, std::vector<int> &queue_subset, std::vector<int> &overall_sequence);

std::vector<int> fcfs(int startHead, const std::vector<int> &requests);
std::vector<int> sstf(int startHead, const std::vector<int> &requests);
std::vector<int> scan(int startHead, int maxCylinder, const std::vector<int> &requests);
std::vector<int> cscan(int startHead, int maxCylinder, const std::vector<int> &requests);
std::vector<int> look(int startHead, const std::vector<int> &requests);
std::vector<int> clook(int startHead, const std::vector<int> &requests);
std::vector<int> hdsa(int startHead, const std::vector<int> &requests);

AlgorithmResult calculateMetrics(const std::string &name,
                                 const std::vector<int> &sequence,
                                 int numRequests,
                                 const DiskPerformanceParams &diskParams);

#endif // DISK_SCHEDULING_H