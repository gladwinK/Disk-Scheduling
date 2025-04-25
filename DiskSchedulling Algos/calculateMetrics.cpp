#include "../Headers/DiskScheduling.h"
AlgorithmResult calculateMetrics(const std::string &name, const std::vector<int> &sequence, int numRequests)
{
    AlgorithmResult result;
    result.name = name;
    result.seekSequence = sequence;

    if (sequence.size() < 2 || numRequests == 0)
    {
        if (numRequests > 0 && !sequence.empty() && sequence.size() == 1)
        {
            result.totalMovement = 0;
            result.avgSeek = 0.0;
            result.maxSeek = 0;
            result.stdDevSeek = 0.0;
            result.throughput = std::numeric_limits<double>::infinity();
        }
        return result;
    }

    std::vector<int> seekTimes;
    int currentMaxSeek = 0;
    long long currentTotalMovement = 0;

    for (size_t i = 1; i < sequence.size(); ++i)
    {
        int seek = std::abs(sequence[i] - sequence[i - 1]);
        currentTotalMovement += seek;
        if (seek > 0)
        {
            seekTimes.push_back(seek);
        }
        if (seek > currentMaxSeek)
        {
            currentMaxSeek = seek;
        }
    }
    if (currentTotalMovement == 0 && sequence.size() > 1)
    {
        seekTimes.push_back(0); // Add a zero seek if no movement occurred but sequence exists
    }

    result.totalMovement = static_cast<int>(currentTotalMovement);
    result.maxSeek = currentMaxSeek;

    if (numRequests > 0)
    {
        result.avgSeek = static_cast<double>(result.totalMovement) / numRequests;

        if (!seekTimes.empty())
        {
            double sumSqDiff = 0.0;
            double avgActualSeek = static_cast<double>(currentTotalMovement) / seekTimes.size();
            for (int seek : seekTimes)
            {
                sumSqDiff += std::pow(static_cast<double>(seek) - avgActualSeek, 2);
            }
            double variance = sumSqDiff / seekTimes.size();
            result.stdDevSeek = std::sqrt(variance);
        }
        else
        {
            result.stdDevSeek = 0.0;
        }

        if (result.totalMovement > 0)
        {
            result.throughput = static_cast<double>(numRequests) / result.totalMovement;
        }
        else if (numRequests > 0)
        {
            result.throughput = std::numeric_limits<double>::infinity();
        }
        else
        {
            result.throughput = 0.0;
        }
    }
    return result;
}