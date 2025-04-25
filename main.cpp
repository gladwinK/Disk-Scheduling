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

int main()
{
    int startHead;
    int maxCylinder;
    std::vector<int> initialQueue;
    std::string queueInputMode;

    // --- Get Basic Config ---
    std::cout << "--- Disk Scheduling Simulation (C++) ---" << std::endl;
    startHead = getPositiveIntInput("Enter Start Head Position: ", 0);
    maxCylinder = getPositiveIntInput("Enter Max Cylinder: ", 0);

    // --- Choose Queue Input Mode ---
    while (true)
    {
        std::cout << "Enter 'm' for manual queue entry or 'g' to generate queue: ";
        std::cin >> queueInputMode;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (queueInputMode == "m" || queueInputMode == "M")
        {
            // --- Manual Queue Entry ---
            std::cout << "Enter Request Queue (comma-separated): ";
            std::string queueStr;
            std::getline(std::cin, queueStr);
            initialQueue = parseQueue(queueStr);
            if (initialQueue.empty())
            {
                std::cerr << "Error: Manual queue entry resulted in an empty queue. Try again or use generation." << std::endl;
                // Optional: continue loop instead of exiting?
                // return 1;
                continue; // Let user try again
            }
            break;
        }
        else if (queueInputMode == "g" || queueInputMode == "G")
        {
            // --- Generate Queue ---
            unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::mt19937 rng(seed);

            printGenWelcome();
            int genChoice = getGenUserChoice();
            int numRequestsGen = getPositiveIntInput("Number of Requests to Generate: ", 1);

            switch (genChoice)
            {
            case 1:
                std::cout << "Generating Uniform Random workload..." << std::endl;
                initialQueue = generateUniformRandom(maxCylinder, numRequestsGen, rng);
                break;
            case 2:
                std::cout << "Generating Sequential workload..." << std::endl;
                initialQueue = generateSequential(maxCylinder, numRequestsGen, rng);
                break;
            case 3:
            {
                std::cout << "Generating Clustered workload (with density)..." << std::endl;
                int num_clusters_input = getPositiveIntInput("Desired Number of Clusters: ", 1, numRequestsGen);
                // Call the REVISED clustered generator
                initialQueue = generateClustered(maxCylinder, numRequestsGen, num_clusters_input, rng);
            }
            break;
            case 4:
                std::cout << "Generating Mixed workload..." << std::endl;
                // This will indirectly call the revised clustered generator
                initialQueue = generateMixed(maxCylinder, numRequestsGen, rng);
                break;
            }

            printQueueGen(initialQueue);            // Show the generated queue
            plotScatter(initialQueue, maxCylinder); // Show the plot
            break;                                  // Exit loop
        }
        else
        {
            std::cout << "Invalid choice. Please enter 'm' or 'g'." << std::endl;
        }
    }

    // --- Validate Final Queue ---
    if (initialQueue.empty())
    {
        std::cout << "\nError: Request queue is empty. Cannot simulate." << std::endl;
        return 1;
    }
    bool validRequests = true;
    for (int req : initialQueue)
    {
        if (req < 0 || req > maxCylinder)
        {
            std::cerr << "Error: Request " << req << " is outside the valid cylinder range [0, "
                      << maxCylinder << "]." << std::endl;
            validRequests = false;
            // Don't exit immediately, report all invalid requests if possible
        }
    }
    if (startHead < 0 || startHead > maxCylinder)
    {
        std::cerr << "Error: Start head " << startHead << " is outside the valid cylinder range [0, "
                  << maxCylinder << "]." << std::endl;
        validRequests = false;
    }
    if (!validRequests)
    {
        std::cerr << "Exiting due to invalid input values." << std::endl;
        return 1;
    }

    // --- Display Configuration ---
    std::cout << "\n--- Configuration ---" << std::endl;
    std::cout << "Start Head:    " << startHead << std::endl;
    std::cout << "Max Cylinder:  " << maxCylinder << std::endl;
    std::cout << "Initial Queue (" << initialQueue.size() << " requests): ";
    // Limit printing very long queues
    const size_t max_print_config = 50;
    for (size_t i = 0; i < std::min(initialQueue.size(), max_print_config); ++i)
    {
        std::cout << initialQueue[i] << (i == initialQueue.size() - 1 || i == max_print_config - 1 ? "" : ", ");
    }
    if (initialQueue.size() > max_print_config)
    {
        std::cout << "...";
    }
    std::cout << std::endl;

    // --- Run Simulations & Calculate Metrics ---
    std::vector<AlgorithmResult> results;
    int numRequests = initialQueue.size();

    results.push_back(calculateMetrics("FCFS", fcfs(startHead, initialQueue), numRequests));
    results.push_back(calculateMetrics("SSTF", sstf(startHead, initialQueue), numRequests));
    results.push_back(calculateMetrics("SCAN", scan(startHead, maxCylinder, initialQueue), numRequests));
    results.push_back(calculateMetrics("C-SCAN", cscan(startHead, maxCylinder, initialQueue), numRequests));
    results.push_back(calculateMetrics("LOOK", look(startHead, initialQueue), numRequests));
    results.push_back(calculateMetrics("C-LOOK", clook(startHead, initialQueue), numRequests));
    results.push_back(calculateMetrics("HDSA", hdsa(startHead, initialQueue), numRequests));

    // --- Find Best Algorithm ---
    int minTotalMovement = std::numeric_limits<int>::max();
    bool movementOccurred = false;
    if (!results.empty())
    {
        for (const auto &result : results)
        {
            if (!movementOccurred || result.totalMovement < minTotalMovement)
            {
                minTotalMovement = result.totalMovement;
            }
            if (result.totalMovement >= 0)
            {                            // Consider 0 movement as valid
                movementOccurred = true; // Flag that we have results
            }
        }
        if (!movementOccurred && !results.empty())
        {                                                // If loop finished but no valid movement found (e.g. all errored somehow)
            minTotalMovement = results[0].totalMovement; // Default to first
        }
        else if (!movementOccurred)
        { // No results at all
            minTotalMovement = 0;
        }
    }

    // --- Display Summary Table ---
    std::cout << "\n--- Algorithm Comparison Summary ---" << std::endl;
    std::cout << std::left << std::setw(11) << "Algorithm" << "| "
              << std::right << std::setw(10) << "Total Move" << " | "
              << std::right << std::setw(10) << "Avg Seek" << " | "
              << std::right << std::setw(10) << "Max Seek" << " | "
              << std::right << std::setw(11) << "StdDev Seek" << " | "
              << std::right << std::setw(10) << "Throughput" << std::endl;
    std::cout << "-----------|------------|------------|------------|-------------|------------" << std::endl;

    std::cout << std::fixed << std::setprecision(2);
    for (const auto &result : results)
    {
        bool isBest = (numRequests > 0 && result.totalMovement == minTotalMovement);
        std::string nameWithBest = result.name;
        if (isBest)
        {
            nameWithBest += " [BEST]";
        }

        std::cout << std::left << std::setw(11) << nameWithBest << "| "
                  << std::right << std::setw(10) << result.totalMovement << " | "
                  << std::right << std::setw(10) << result.avgSeek << " | "
                  << std::right << std::setw(10) << result.maxSeek << " | "
                  << std::right << std::setw(11) << result.stdDevSeek << " | ";

        std::cout << std::fixed << std::setprecision(4);
        if (result.throughput == std::numeric_limits<double>::infinity())
        {
            std::cout << std::right << std::setw(10) << "Inf";
        }
        else
        {
            std::cout << std::right << std::setw(10) << result.throughput;
        }
        std::cout << std::endl;
        std::cout << std::fixed << std::setprecision(2);
    }

    // --- Display Notes ---
    std::cout << "\nNote: [BEST] indicates the algorithm with the lowest Total Head Movement." << std::endl;
    std::cout << "Note: Avg Seek and Throughput are relative to the number of requests serviced (" << numRequests << ")." << std::endl;
    std::cout << "Note: StdDev Seek is calculated based on the variance of actual non-zero seek distances performed." << std::endl;

    return 0;
}