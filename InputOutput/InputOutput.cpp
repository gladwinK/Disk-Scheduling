#include "../Headers/InputOutput.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>

// Function to print the generated queue
void printQueueGen(const std::vector<int> &queue)
{
    std::cout << "\nGenerated Queue: ";
    if (!queue.empty())
    {
        // Limit printing extremely long queues
        const size_t max_print = 100;
        for (size_t i = 0; i < std::min(queue.size(), max_print); ++i)
        {
            std::cout << queue[i] << (i == queue.size() - 1 || i == max_print - 1 ? "" : ",");
        }
        if (queue.size() > max_print)
        {
            std::cout << "...";
        }
    }
    std::cout << " (" << queue.size() << " requests)" << std::endl;
}

// Function to plot the scatter plot of the queue
void plotScatter(const std::vector<int> &queue, int max_cylinder)
{
    if (queue.empty())
    {
        std::cout << "Queue is empty, cannot plot." << std::endl;
        return;
    }
    const int plot_width = 80;                               // Adjusted width for console
    const int plot_height = std::min(40, (int)queue.size()); // Limit height
    std::vector<std::string> grid(plot_height, std::string(plot_width, '.'));

    // Determine scaling for y-axis (time/request index)
    double y_scale = (queue.size() > 1) ? static_cast<double>(plot_height - 1) / (queue.size() - 1) : 1.0;

    for (size_t i = 0; i < queue.size(); ++i)
    {
        // Map request index to plot row
        int y_pos = static_cast<int>(std::round(i * y_scale));
        y_pos = std::min(plot_height - 1, y_pos); // Clamp to max height

        // Map cylinder to plot column
        int cylinder = queue[i];
        double x_scale_factor = (max_cylinder > 0) ? static_cast<double>(plot_width - 1) / max_cylinder : 0.0;
        int column = static_cast<int>(std::round(cylinder * x_scale_factor));
        column = std::max(0, std::min(plot_width - 1, column));

        // Mark the point, potentially overwriting if multiple map to same cell
        grid[y_pos][column] = '*';
    }
    std::cout << "\nASCII Scatter Plot (x=cyl 0.." << max_cylinder << ", y=request index approx.)" << std::endl;
    // Simple axis labels
    std::cout << " |";
    for (int i = 0; i < plot_width; ++i)
        std::cout << (i == 0 ? '0' : (i == plot_width - 1 ? 'M' : '-'));
    std::cout << std::endl;

    for (int i = 0; i < plot_height; ++i)
    {
        std::cout << " |" << grid[i] << std::endl;
    }
    std::cout << " |";
    for (int i = 0; i < plot_width; ++i)
        std::cout << '-';
    std::cout << std::endl;
    std::cout << " (M=" << max_cylinder << ")" << std::endl;
}

// Function to print the welcome message for queue generation
void printGenWelcome()
{
    std::cout << "\n--- Generate Request Queue ---" << std::endl;
    std::cout << "Select generation pattern:" << std::endl;
    std::cout << "  1) Uniform Random" << std::endl;
    std::cout << "  2) Sequential" << std::endl;
    std::cout << "  3) Clustered (with density)" << std::endl; // Updated description
    std::cout << "  4) Mixed (Random + Clustered)" << std::endl;
}

// Function to get the user's choice for queue generation
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
            std::cout << "Invalid input. Please enter 1, 2, 3, or 4." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

// Function to get a positive integer input from the user
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
            std::string error_msg;
            if (!std::cin.good())
            {
                error_msg = "Invalid input. Please enter an integer.";
            }
            else if (max_val == std::numeric_limits<int>::max())
            {
                error_msg = "Invalid input. Please enter an integer >= " + std::to_string(min_val) + ".";
            }
            else
            {
                error_msg = "Invalid input. Please enter an integer between " + std::to_string(min_val) + " and " + std::to_string(max_val) + ".";
            }
            std::cout << error_msg << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}