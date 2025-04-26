#include "../Headers/InputOutput.h"
#include "../Headers/DiskScheduling.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <sstream>
int getPositiveIntInput(const std::string &prompt, int min_val, int max_val)
{
    int value;
    while (true)
    {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.good() && value >= min_val && value <= max_val)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        else
        {
            std::string error_msg = "Invalid input. Please enter an integer";
            if (min_val != 0 || max_val != std::numeric_limits<int>::max())
            {
                error_msg += " between " + std::to_string(min_val) + " and " + std::to_string(max_val);
            }
            else if (min_val != 0)
            {
                error_msg += " >= " + std::to_string(min_val);
            }
            error_msg += ".";
            std::cout << error_msg << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

double getPositiveDoubleInput(const std::string &prompt, double min_val, double max_val)
{
    double value;
    while (true)
    {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.good() && value >= min_val && value <= max_val)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        else
        {
            std::string error_msg = "Invalid input. Please enter a number";
            std::stringstream bounds_ss;
            bool has_min = (min_val > 0.0 || min_val > -std::numeric_limits<double>::max());
            bool has_max = (max_val < std::numeric_limits<double>::max());
            if (has_min && has_max)
            {
                bounds_ss << " between " << min_val << " and " << max_val;
            }
            else if (has_min)
            {
                bounds_ss << " >= " << min_val;
            }
            else if (has_max)
            {
                bounds_ss << " <= " << max_val;
            }
            error_msg += bounds_ss.str() + ".";
            std::cout << error_msg << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

std::vector<int> parseQueue(const std::string &s)
{
    std::vector<int> queue;
    std::stringstream ss(s);
    std::string segment;
    while (std::getline(ss, segment, ','))
    {
        try
        {
            segment.erase(0, segment.find_first_not_of(" \t\n\r\f\v"));
            segment.erase(segment.find_last_not_of(" \t\n\r\f\v") + 1);
            if (!segment.empty())
            {
                queue.push_back(std::stoi(segment));
            }
        }
        catch (const std::invalid_argument &e)
        { /* Quiet warning */
        }
        catch (const std::out_of_range &e)
        { /* Quiet warning */
        }
    }
    return queue;
}

int getGenUserChoice()
{
    int choice;
    while (true)
    {
        std::cout << "Generation Choice (1-4): ";
        std::cin >> choice;
        if (std::cin.good() && choice >= 1 && choice <= 4)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
        else
        {
            std::cout << "Invalid input. Please enter 1, 2, 3, or 4."
                      << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void printGenWelcome()
{
    std::cout << "\n--- Generate Request Queue ---" << std::endl;
    std::cout << "Select generation pattern:" << std::endl;
    std::cout << "  1) Uniform Random" << std::endl;
    std::cout << "  2) Sequential" << std::endl;
    std::cout << "  3) Clustered (with density)" << std::endl;
    std::cout << "  4) Mixed (Random + Clustered)" << std::endl;
}

void printQueueGen(const std::vector<int> &queue)
{
    std::cout << "\nGenerated Queue: ";
    if (!queue.empty())
    {
        const size_t max_print = 100;
        for (size_t i = 0; i < std::min(queue.size(), max_print); ++i)
        {
            std::cout << queue[i] << (i == queue.size() - 1 || i == max_print - 1 ? "" : ",");
        }
        if (queue.size() > max_print)
            std::cout << "...";
    }
    std::cout << " (" << queue.size() << " requests)" << std::endl;
}

void plotScatter(const std::vector<int> &queue, int max_cylinder)
{
    if (queue.empty())
    {
        std::cout << "Queue is empty, cannot plot."
                  << std::endl;
        return;
    }
    const int plot_width = 80;
    const int plot_height = std::min(40, (int)queue.size());
    std::vector<std::string> grid(plot_height, std::string(plot_width, '.'));
    double y_scale = (queue.size() > 1) ? static_cast<double>(plot_height - 1) / (queue.size() - 1) : 1.0;
    for (size_t i = 0; i < queue.size(); ++i)
    {
        int y_pos = static_cast<int>(std::round(i * y_scale));
        y_pos = std::min(plot_height - 1, y_pos);
        int cylinder = queue[i];
        double x_scale_factor = (max_cylinder > 0) ? static_cast<double>(plot_width - 1) / max_cylinder : 0.0;
        int column = static_cast<int>(std::round(cylinder * x_scale_factor));
        column = std::max(0, std::min(plot_width - 1, column));
        grid[y_pos][column] = '*';
    }
    std::cout << "\nASCII Scatter Plot (x=cyl 0.." << max_cylinder << ", y=request index approx.)" << std::endl;
    std::cout << " |";
    for (int i = 0; i < plot_width; ++i)
        std::cout << (i == 0 ? '0' : (i == plot_width - 1 ? 'M' : '-'));
    std::cout << std::endl;
    for (int i = 0; i < plot_height; ++i)
        std::cout << " |" << grid[i] << std::endl;
    std::cout << " |";
    for (int i = 0; i < plot_width; ++i)
        std::cout << '-';
    std::cout << std::endl;
    std::cout << " (M=" << max_cylinder << ")" << std::endl;
}

void displayConfiguration(int startHead, int maxCylinder, const DiskPerformanceParams &diskParams, const std::vector<int> &initialQueue)
{
    std::cout << "\n--- Configuration ---" << std::endl;
    std::cout << "Start Head:    " << startHead << std::endl;
    std::cout << "Max Cylinder:  " << maxCylinder << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Seek Time/Cyl: " << diskParams.avgSeekTimePerCylinderMs << " ms" << std::endl;
    std::cout << "Avg Rot Latency:" << diskParams.avgRotationalLatencyMs << " ms" << std::endl;
    std::cout << "Avg Xfer Time: " << diskParams.transferTimePerRequestMs << " ms" << std::endl;
    std::cout << std::defaultfloat;
    std::cout << "Initial Queue (" << initialQueue.size() << " requests): ";
    const size_t max_print_config = 50;
    for (size_t i = 0; i < std::min(initialQueue.size(), max_print_config); ++i)
    {
        std::cout << initialQueue[i] << (i == initialQueue.size() - 1 || i == max_print_config - 1 ? "" : ", ");
    }
    if (initialQueue.size() > max_print_config)
        std::cout << "...";
    std::cout << std::endl;
}

void displaySummaryTable(const std::vector<AlgorithmResult> &results, int numRequests)
{
    int minTotalMovement = std::numeric_limits<int>::max();
    bool movementOccurred = false;
    if (!results.empty())
    {
        for (const auto &result : results)
        {
            if (!movementOccurred || result.totalMovement < minTotalMovement)
                minTotalMovement = result.totalMovement;
            if (result.totalMovement >= 0)
                movementOccurred = true;
        }
        if (!movementOccurred && !results.empty())
            minTotalMovement = results[0].totalMovement;
        else if (!movementOccurred)
            minTotalMovement = 0;
    }

    std::cout << "\n--- Algorithm Comparison Summary ---" << std::endl;
    std::cout << std::left << std::setw(11) << "Algorithm" << "| "
              << std::right << std::setw(10) << "Total Move" << " | "
              << std::right << std::setw(10) << "Avg Seek" << " | "
              << std::right << std::setw(10) << "Max Seek" << " | "
              << std::right << std::setw(11) << "StdDev Seek" << " | "
              << std::right << std::setw(10) << "Throughput" << " |"
              << std::right << std::setw(14) << "Avg Resp(ms)" // New column
              << std::endl;
    std::cout << "-----------|------------|------------|------------|-------------|------------|--------------" << std::endl; // Adjusted separator

    std::cout << std::fixed << std::setprecision(2);
    for (const auto &result : results)
    {
        bool isBest = (numRequests > 0 && result.totalMovement == minTotalMovement);
        std::string nameWithBest = result.name;
        if (isBest)
            nameWithBest += " [BEST]";
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
        std::cout << " |";

        std::cout << std::fixed << std::setprecision(2);
        std::cout << std::right << std::setw(14) << result.avgResponseTime;

        std::cout << std::endl;
    }
}

void displayNotes(int numRequests)
{
    std::cout << "\nNote: [BEST] indicates the algorithm with the lowest Total Head Movement."
              << std::endl;
    std::cout << "Note: Avg Seek, StdDev Seek, and Throughput are relative to the number of requests serviced (" << numRequests << ")."
              << std::endl;
    std::cout << "Note: Avg Resp(ms) = Avg(Seek Time + Rotational Latency + Transfer Time) per request service."
              << std::endl;
    std::cout << "Note: Queueing Delay (time before scheduling) is not included in Avg Response Time." << std::endl;
}
