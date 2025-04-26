// #include <iostream>
// #include <vector>
// #include <string>
// #include <sstream>
// #include <cmath>
// #include <algorithm>
#include <iomanip>
// #include <limits>

#include "./Headers/DiskScheduling.h"
#include "./Headers/QueueGeneration.h"
#include "./Headers/InputOutput.h"
#include <iostream>
#include <vector>
#include <string>

int main()
{
    int startHead;
    int maxCylinder;
    std::vector<int> initialQueue;
    std::string queueInputMode;
    DiskPerformanceParams diskParams; // Create struct to hold disk parameters

    // --- Get Basic Config ---
    std::cout << "--- Disk Scheduling Simulation (C++) ---" << std::endl;
    startHead = getPositiveIntInput("Enter Start Head Position: ", 0);
    maxCylinder = getPositiveIntInput("Enter Max Cylinder: ", 0);

    // --- Get Disk Performance Parameters ---
    std::cout << "\n--- Enter Disk Performance Parameters ---" << std::endl;
    diskParams.avgSeekTimePerCylinderMs = getPositiveDoubleInput("Average Seek Time per Cylinder (ms): ", 0.0);
    double rpm = getPositiveDoubleInput("Disk Rotational Speed (RPM): ", 1.0);
    double transferRateMBps = getPositiveDoubleInput("Disk Transfer Rate (MB/s): ", 0.001);
    double avgRequestSizeKB = getPositiveDoubleInput("Average Request Size (KB): ", 0.1);

    // Calculate derived performance parameters
    diskParams.avgRotationalLatencyMs = (rpm > 0) ? (30000.0 / rpm) : 0.0; // (60 * 1000 / RPM) / 2
    double transferRateKBpms = (transferRateMBps * 1024.0) / 1000.0;       // MB/s -> KB/s -> KB/ms
    diskParams.transferTimePerRequestMs = (transferRateKBpms > 0) ? (avgRequestSizeKB / transferRateKBpms) : 0.0;

    std::cout << std::fixed << std::setprecision(2); // Set precision for displaying calculated params
    std::cout << " -> Calculated Avg Rotational Latency: " << diskParams.avgRotationalLatencyMs << " ms" << std::endl;
    std::cout << " -> Calculated Transfer Time per Request: " << diskParams.transferTimePerRequestMs << " ms" << std::endl;
    std::cout << std::defaultfloat; // Reset precision

    // --- Choose Queue Input Mode ---
    while (true)
    {
        std::cout << "\nEnter 'm' for manual queue entry or 'g' to generate queue: ";
        std::cin >> queueInputMode;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (queueInputMode == "m" || queueInputMode == "M")
        {
            std::cout << "Enter Request Queue (comma-separated): ";
            std::string queueStr;
            std::getline(std::cin, queueStr);
            initialQueue = parseQueue(queueStr);
            if (initialQueue.empty())
            {
                std::cerr << "Warning: Manual queue entry resulted in an empty queue. Retrying."
                          << std::endl;
                continue;
            }
            break;
        }
        else if (queueInputMode == "g" || queueInputMode == "G")
        {
            // Setup RNG
            unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::mt19937 rng(seed);

            printGenWelcome();                  // From InputOutput.h
            int genChoice = getGenUserChoice(); // From InputOutput.h
            int numRequestsGen = getPositiveIntInput("Number of Requests to Generate: ", 1);

            // Use functions from QueueGeneration.h
            switch (genChoice)
            {
            case 1:
                initialQueue = generateUniformRandom(maxCylinder, numRequestsGen, rng);
                break;
            case 2:
                initialQueue = generateSequential(maxCylinder, numRequestsGen, rng);
                break;
            case 3:
            {
                int nc = getPositiveIntInput("Desired Number of Clusters: ", 1, numRequestsGen);
                initialQueue = generateClustered(maxCylinder, numRequestsGen, nc, rng);
            }
            break;
            case 4:
                initialQueue = generateMixed(maxCylinder, numRequestsGen, rng);
                break;
            }
            // Use functions from InputOutput.h
            printQueueGen(initialQueue);
            plotScatter(initialQueue, maxCylinder);
            break;
        }
        else
        {
            std::cout << "Invalid choice. Please enter 'm' or 'g'."
                      << std::endl;
        }
    }

    // --- Validate Final Queue ---
    if (initialQueue.empty())
    {
        std::cout << "\nError: Request queue is empty." << std::endl;
        return 1;
    }
    bool validRequests = true;
    for (int req : initialQueue)
    {
        if (req < 0 || req > maxCylinder)
        {
            std::cerr << "Error: Request " << req << " invalid." << std::endl;
            validRequests = false;
        }
    }
    if (startHead < 0 || startHead > maxCylinder)
    {
        std::cerr << "Error: Start head " << startHead << " invalid." << std::endl;
        validRequests = false;
    }
    if (!validRequests)
    {
        std::cerr << "Exiting." << std::endl;
        return 1;
    }

    // --- Display Configuration (Using function from InputOutput.h) ---
    displayConfiguration(startHead, maxCylinder, diskParams, initialQueue);

    // --- Run Simulations & Calculate Metrics (Using functions from DiskScheduling.h) ---
    std::vector<AlgorithmResult> results;
    int numRequests = initialQueue.size();

    results.push_back(calculateMetrics("FCFS", fcfs(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("SSTF", sstf(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("SCAN", scan(startHead, maxCylinder, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("C-SCAN", cscan(startHead, maxCylinder, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("LOOK", look(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("C-LOOK", clook(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("HDSA", hdsa(startHead, initialQueue), numRequests, diskParams));

    // --- Display Summary Table (Using function from InputOutput.h) ---
    displaySummaryTable(results, numRequests);

    // --- Display Notes (Using function from InputOutput.h) ---
    displayNotes(numRequests);

    return 0;
}