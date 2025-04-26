#include "../Headers/DiskScheduling.h"
#include <cmath>
#include <limits>
AlgorithmResult calculateMetrics(const std::string &name,
                                 const std::vector<int> &sequence,
                                 int numRequests,
                                 const DiskPerformanceParams &diskParams) // Pass disk params
{
    AlgorithmResult result;
    result.name = name;
    result.seekSequence = sequence;

    // Handle cases with no requests or only the start head
    if (sequence.size() < 2 || numRequests == 0)
    {
        if (numRequests > 0 && sequence.size() == 1)
        { // Only start head, no requests to service *from* there
            result.totalMovement = 0;
            result.avgSeek = 0.0;
            result.maxSeek = 0;
            result.stdDevSeek = 0.0;
            result.throughput = 0.0;      // No requests completed
            result.avgResponseTime = 0.0; // No service occurred
        }
        else if (numRequests == 0)
        {
            // No requests, metrics are zero or undefined
        }
        return result;
    }

    std::vector<int> seekTimesDistances; // Store distances for StdDev Seek
    int currentMaxSeekDistance = 0;
    long long currentTotalMovement = 0;
    double totalServiceTimeMs = 0.0; // Accumulator for response time components

    // Iterate through the movements required to service the requests
    for (size_t i = 1; i < sequence.size(); ++i)
    {
        int seekDistance = std::abs(sequence[i] - sequence[i - 1]);
        currentTotalMovement += seekDistance;

        // --- Calculate time components for this specific seek/service ---
        double seekTimeMs = static_cast<double>(seekDistance) * diskParams.avgSeekTimePerCylinderMs;
        double serviceTimeForThisStepMs = seekTimeMs + diskParams.avgRotationalLatencyMs + diskParams.transferTimePerRequestMs;
        totalServiceTimeMs += serviceTimeForThisStepMs;

        // --- Store data for other metrics ---
        if (seekDistance > 0)
        {
            seekTimesDistances.push_back(seekDistance); // Store distance for StdDev Seek
        }
        if (seekDistance > currentMaxSeekDistance)
        {
            currentMaxSeekDistance = seekDistance;
        }
    }
    // Add a zero seek distance if needed for StdDev calculation when no movement occurred
    if (currentTotalMovement == 0 && sequence.size() > 1)
    {
        seekTimesDistances.push_back(0);
    }

    // --- Assign calculated metrics ---
    result.totalMovement = static_cast<int>(currentTotalMovement);
    result.maxSeek = currentMaxSeekDistance; // Max seek is based on distance

    if (numRequests > 0)
    {
        // Avg Seek Time (distance per request)
        result.avgSeek = static_cast<double>(result.totalMovement) / numRequests;

        // Std Dev Seek (based on distances)
        if (!seekTimesDistances.empty())
        {
            double sumSqDiff = 0.0;
            double avgActualSeekDistance = static_cast<double>(currentTotalMovement) / seekTimesDistances.size();
            for (int dist : seekTimesDistances)
            {
                sumSqDiff += std::pow(static_cast<double>(dist) - avgActualSeekDistance, 2);
            }
            double variance = sumSqDiff / seekTimesDistances.size();
            result.stdDevSeek = std::sqrt(variance);
        }
        else
        {
            result.stdDevSeek = 0.0;
        }

        // Throughput (requests per unit of movement)
        if (result.totalMovement > 0)
        {
            result.throughput = static_cast<double>(numRequests) / result.totalMovement;
        }
        else
        { // Movement is 0
            // If numRequests > 0 and movement is 0, all requests were at start head?
            // Throughput could be considered infinite relative to movement, but maybe better as 0?
            // Let's keep Inf for consistency, but acknowledge ambiguity.
            result.throughput = (numRequests > 0) ? std::numeric_limits<double>::infinity() : 0.0;
        }

        // Average Response Time (ms per request)
        // Check sequence size > 1 because service time is calculated per *step*
        int numServiceSteps = sequence.size() - 1;
        if (numServiceSteps > 0)
        {
            // Average the total time over the number of service steps performed.
            // This implicitly averages over the number of requests if seq_size-1 == numRequests
            result.avgResponseTime = totalServiceTimeMs / numServiceSteps;
        }
        else
        {
            result.avgResponseTime = 0.0; // No service steps occurred
        }

    } // End if (numRequests > 0)

    return result;
}