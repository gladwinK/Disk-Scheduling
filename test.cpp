#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <random>
#include <chrono>

// --- Forward Declarations ---
// (Keep forward declarations as before)
void printQueueGen(const std::vector<int> &queue);
void plotScatter(const std::vector<int> &queue, int max_cylinder);
void printGenWelcome();
int getGenUserChoice();
int getPositiveIntInput(const std::string &prompt, int min_val = 0, int max_val = std::numeric_limits<int>::max());
double getPositiveDoubleInput(const std::string &prompt, double min_val = 0.0, double max_val = std::numeric_limits<double>::max()); // Added for double inputs
std::vector<int> generateClustered(int max_cylinder, int num_requests, int desired_num_clusters, std::mt19937 &rng);
std::vector<int> generateUniformRandom(int max_cylinder, int num_requests, std::mt19937 &rng);
std::vector<int> generateSequential(int max_cylinder, int num_requests, std::mt19937 &rng);
std::vector<int> generateMixed(int max_cylinder, int num_requests, std::mt19937 &rng);

// --- Disk Scheduling Simulation Components ---


// Parses the comma-separated string (no changes)
std::vector<int> parseQueue(const std::string &s)
{
    // ... (parseQueue code remains the same) ...
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

// Calculates metrics based on the seek sequence - MODIFIED to include Response Time


// --- Disk Scheduling Algorithms (FCFS, SSTF, SCAN, C-SCAN, LOOK, C-LOOK, HDSA) ---
// --- run_sstf_subset helper function ---
// (No changes needed in the algorithm logic itself)
// ... (Omitted for brevity - use previous versions) ...
// FCFS: First-Come, First-Served
std::vector<int> fcfs(int startHead, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    for (int req : requests)
    {
        sequence.push_back(req);
    }
    return sequence;
}

// SSTF: Shortest Seek Time First
std::vector<int> sstf(int startHead, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    std::vector<int> remaining = requests;
    int currentHead = startHead;

    while (!remaining.empty())
    {
        int closestRequestIndex = -1;
        int minDistance = std::numeric_limits<int>::max();

        for (size_t i = 0; i < remaining.size(); ++i)
        {
            int distance = std::abs(remaining[i] - currentHead);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestRequestIndex = i;
            }
        }
        if (closestRequestIndex != -1)
        {
            currentHead = remaining[closestRequestIndex];
            sequence.push_back(currentHead);
            remaining.erase(remaining.begin() + closestRequestIndex);
        }
        else
        {
            break;
        }
    }
    return sequence;
}

// SCAN (Elevator Algorithm) - Starts moving DOWN towards 0 first.
std::vector<int> scan(int startHead, int maxCylinder, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    if (requests.empty())
        return sequence;

    std::vector<int> remaining = requests;
    std::sort(remaining.begin(), remaining.end());

    int currentHead = startHead;
    std::vector<int> lower, upper;
    for (int req : remaining)
    {
        if (req < startHead)
            lower.push_back(req);
        else
            upper.push_back(req);
    }
    std::sort(lower.rbegin(), lower.rend()); // Descending for downward pass

    // --- Movement Logic: DOWNWARDS FIRST ---
    for (int req : lower)
    {
        sequence.push_back(req);
        currentHead = req;
    }
    if (!lower.empty() || !upper.empty())
    { // Only move to end if requests exist
        if (currentHead != 0)
        {
            sequence.push_back(0);
            currentHead = 0;
        }
    }
    for (int req : upper)
    {
        sequence.push_back(req);
        currentHead = req;
    }
    return sequence;
}

// C-SCAN (Circular SCAN) - Assumes initial move towards higher cylinders
std::vector<int> cscan(int startHead, int maxCylinder, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    if (requests.empty())
        return sequence;

    std::vector<int> remaining = requests;
    std::sort(remaining.begin(), remaining.end());
    int currentHead = startHead;
    std::vector<int> lower, upper;
    for (int req : remaining)
    {
        if (req < startHead)
            lower.push_back(req);
        else
            upper.push_back(req);
    }

    for (int req : upper)
    {
        sequence.push_back(req);
        currentHead = req;
    }
    if (!upper.empty() || !lower.empty())
    {
        if (currentHead != maxCylinder)
        {
            sequence.push_back(maxCylinder);
        }
        if (!lower.empty())
        {
            sequence.push_back(0); // Add 0 to sequence for the jump landing
            currentHead = 0;
            for (int req : lower)
            {
                sequence.push_back(req);
                currentHead = req;
            }
        }
    }
    return sequence;
}

// LOOK - Assumes initial move towards higher cylinders
std::vector<int> look(int startHead, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    if (requests.empty())
        return sequence;

    std::vector<int> remaining = requests;
    std::sort(remaining.begin(), remaining.end());
    std::vector<int> lower, upper;
    for (int req : remaining)
    {
        if (req < startHead)
            lower.push_back(req);
        else
            upper.push_back(req);
    }
    std::sort(lower.rbegin(), lower.rend()); // Sort descending

    for (int req : upper)
    {
        sequence.push_back(req);
    }
    for (int req : lower)
    {
        sequence.push_back(req);
    }
    return sequence;
}

// C-LOOK - Assumes initial move towards higher cylinders
std::vector<int> clook(int startHead, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    if (requests.empty())
        return sequence;

    std::vector<int> remaining = requests;
    std::sort(remaining.begin(), remaining.end());
    std::vector<int> lower, upper;
    for (int req : remaining)
    {
        if (req < startHead)
            lower.push_back(req);
        else
            upper.push_back(req);
    }

    for (int req : upper)
    {
        sequence.push_back(req);
    }
    if (!lower.empty())
    {
        for (int req : lower)
        {
            sequence.push_back(req);
        }
    }
    return sequence;
}

// --- HDSA Implementation ---
// Helper function to perform SSTF on a given queue subset
int run_sstf_subset(int currentHead, std::vector<int> &queue_subset, std::vector<int> &overall_sequence)
{
    while (!queue_subset.empty())
    {
        int closestRequestIndex = -1;
        int minDistance = std::numeric_limits<int>::max();
        for (size_t i = 0; i < queue_subset.size(); ++i)
        {
            int distance = std::abs(queue_subset[i] - currentHead);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestRequestIndex = i;
            }
        }
        if (closestRequestIndex != -1)
        {
            currentHead = queue_subset[closestRequestIndex];
            overall_sequence.push_back(currentHead);
            queue_subset.erase(queue_subset.begin() + closestRequestIndex);
        }
        else
        {
            break;
        }
    }
    return currentHead;
}
// HDSA: Hybrid Disk Scheduling Algorithm
std::vector<int> hdsa(int startHead, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    if (requests.empty())
        return sequence;

    std::vector<int> P, Q;
    for (int req : requests)
    {
        if (req < startHead)
            P.push_back(req);
        else if (req > startHead)
            Q.push_back(req);
    }

    int x = std::numeric_limits<int>::max();
    if (!P.empty())
        x = startHead - *std::min_element(P.begin(), P.end());
    int y = std::numeric_limits<int>::max();
    if (!Q.empty())
        y = *std::max_element(Q.begin(), Q.end()) - startHead;

    int currentHead = startHead;
    if (x > y)
    {
        currentHead = run_sstf_subset(currentHead, Q, sequence);
        currentHead = run_sstf_subset(currentHead, P, sequence);
    }
    else
    {
        currentHead = run_sstf_subset(currentHead, P, sequence);
        currentHead = run_sstf_subset(currentHead, Q, sequence);
    }
    return sequence;
}

// --- Generator Functions ---
// (No changes needed in generator logic)
// ... (Omitted for brevity - use previous versions) ...
std::vector<int> generateUniformRandom(int max_cylinder, int num_requests, std::mt19937 &rng)
{
    std::vector<int> queue;
    queue.reserve(num_requests);
    std::uniform_int_distribution<int> dist(0, std::max(0, max_cylinder));
    for (int i = 0; i < num_requests; ++i)
    {
        queue.push_back(dist(rng));
    }
    return queue;
}
std::vector<int> generateSequential(int max_cylinder, int num_requests, std::mt19937 &rng)
{
    std::vector<int> queue;
    queue.reserve(num_requests);
    if (max_cylinder <= 0)
    {
        for (int i = 0; i < num_requests; ++i)
            queue.push_back(0);
        return queue;
    }
    std::uniform_int_distribution<int> start_dist(0, max_cylinder);
    std::uniform_int_distribution<int> direction_dist(0, 1);
    int max_step = std::max(1, std::min(max_cylinder / 10, 5));
    std::uniform_int_distribution<int> step_dist(1, max_step);
    int current_cylinder = start_dist(rng);
    bool increasing = direction_dist(rng);
    for (int i = 0; i < num_requests; ++i)
    {
        current_cylinder = std::max(0, std::min(max_cylinder, current_cylinder));
        queue.push_back(current_cylinder);
        if (i < num_requests - 1)
        {
            int next_cylinder = current_cylinder;
            int attempts = 0;
            do
            {
                int move = step_dist(rng);
                int potential_next_cylinder = increasing ? (current_cylinder + move) : (current_cylinder - move);
                bool bounced = false;
                if (potential_next_cylinder >= max_cylinder)
                {
                    potential_next_cylinder = max_cylinder;
                    if (increasing)
                    {
                        increasing = false;
                        bounced = true;
                    }
                }
                else if (potential_next_cylinder <= 0)
                {
                    potential_next_cylinder = 0;
                    if (!increasing)
                    {
                        increasing = true;
                        bounced = true;
                    }
                }
                next_cylinder = potential_next_cylinder;
                attempts++;
            } while (next_cylinder == current_cylinder && max_cylinder > 0 && attempts < 5);
            current_cylinder = next_cylinder;
        }
    }
    return queue;
}
std::vector<int> generateClustered(int max_cylinder, int num_requests, int desired_num_clusters, std::mt19937 &rng)
{
    std::vector<int> queue;
    queue.reserve(num_requests);
    if (num_requests <= 0)
        return queue;
    if (max_cylinder < 0)
        max_cylinder = 0;
    int num_clusters = std::max(1, std::min(desired_num_clusters, num_requests));
    int requests_per_cluster = num_requests / num_clusters;
    int remaining_requests = num_requests % num_clusters;
    std::vector<int> cluster_centers;
    cluster_centers.reserve(num_clusters);
    double approx_segment_width = (num_clusters > 0) ? static_cast<double>(max_cylinder + 1) / num_clusters : 0.0;
    for (int i = 0; i < num_clusters; ++i)
    {
        double ideal_center_double = (static_cast<double>(i) + 0.5) * approx_segment_width;
        int ideal_center = static_cast<int>(std::round(ideal_center_double));
        int center_offset_range = static_cast<int>(std::round(approx_segment_width * 0.10));
        center_offset_range = std::max(0, center_offset_range);
        std::uniform_int_distribution<int> center_offset_dist(-center_offset_range, center_offset_range);
        int actual_center = ideal_center + center_offset_dist(rng);
        actual_center = std::max(0, std::min(max_cylinder, actual_center));
        cluster_centers.push_back(actual_center);
    }
    double std_dev = std::max(1.0, approx_segment_width / 6.0);
    std::normal_distribution<double> offset_dist(0.0, std_dev);
    int requests_generated = 0;
    for (int i = 0; i < num_clusters; ++i)
    {
        int cluster_center = cluster_centers[i];
        int current_requests_in_cluster = requests_per_cluster + (i < remaining_requests ? 1 : 0);
        for (int j = 0; j < current_requests_in_cluster; ++j)
        {
            double offset = offset_dist(rng);
            int request_cyl = static_cast<int>(std::round(cluster_center + offset));
            request_cyl = std::max(0, std::min(max_cylinder, request_cyl));
            queue.push_back(request_cyl);
            requests_generated++;
        }
    }
    std::uniform_int_distribution<int> fill_dist(0, max_cylinder);
    while (queue.size() < num_requests)
        queue.push_back(fill_dist(rng));
    if (queue.size() > num_requests)
        queue.resize(num_requests);
    return queue;
}
std::vector<int> generateMixed(int max_cylinder, int num_requests, std::mt19937 &rng)
{
    std::vector<int> queue;
    queue.reserve(num_requests);
    int num_random = num_requests * 6 / 10;
    int num_clustered = num_requests - num_random;
    if (num_random > 0)
    {
        std::vector<int> random_part = generateUniformRandom(max_cylinder, num_random, rng);
        queue.insert(queue.end(), random_part.begin(), random_part.end());
    }
    if (num_clustered > 0)
    {
        int clusters_for_mixed = std::max(1, std::min(num_clustered, num_clustered / 5 + 1));
        std::vector<int> clustered_part = generateClustered(max_cylinder, num_clustered, clusters_for_mixed, rng);
        queue.insert(queue.end(), clustered_part.begin(), clustered_part.end());
    }
    std::uniform_int_distribution<int> fill_dist(0, std::max(0, max_cylinder));
    while (queue.size() < num_requests)
        queue.push_back(fill_dist(rng));
    if (queue.size() > num_requests)
        queue.resize(num_requests);
    std::shuffle(queue.begin(), queue.end(), rng);
    return queue;
}

// --- Generator Output/UI Functions ---
// (No changes needed in these)
// ... (Omitted for brevity - use previous versions) ...
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
        std::cout << "Queue is empty, cannot plot." << std::endl;
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
void printGenWelcome()
{
    std::cout << "\n--- Generate Request Queue ---" << std::endl;
    std::cout << "Select generation pattern:" << std::endl;
    std::cout << "  1) Uniform Random" << std::endl;
    std::cout << "  2) Sequential" << std::endl;
    std::cout << "  3) Clustered (with density)" << std::endl;
    std::cout << "  4) Mixed (Random + Clustered)" << std::endl;
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
            std::cout << "Invalid input. Please enter 1, 2, 3, or 4." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

// --- Utility Input Functions ---

// Robust function to get a positive integer within optional bounds
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

// Robust function to get a positive double within optional bounds (NEW)
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
            // Create bounds string carefully for doubles
            std::stringstream bounds_ss;
            bool has_min = (min_val > 0.0 || min_val > -std::numeric_limits<double>::max()); // More robust check
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
            { // Less common for max double limit
                bounds_ss << " <= " << max_val;
            }
            error_msg += bounds_ss.str() + ".";

            std::cout << error_msg << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

// --- Main Program ---
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
    double rpm = getPositiveDoubleInput("Disk Rotational Speed (RPM): ", 1.0);              // RPM must be positive
    double transferRateMBps = getPositiveDoubleInput("Disk Transfer Rate (MB/s): ", 0.001); // Min positive rate
    double avgRequestSizeKB = getPositiveDoubleInput("Average Request Size (KB): ", 0.1);   // Min positive size

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
            // Manual Entry... (no changes here)
            std::cout << "Enter Request Queue (comma-separated): ";
            std::string queueStr;
            std::getline(std::cin, queueStr);
            initialQueue = parseQueue(queueStr);
            if (initialQueue.empty())
            {
                std::cerr << "Warning: Manual queue entry resulted in an empty queue. Retrying." << std::endl;
                continue;
            }
            break;
        }
        else if (queueInputMode == "g" || queueInputMode == "G")
        {
            // Generate Queue... (no changes here)
            unsigned seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::mt19937 rng(seed);
            printGenWelcome();
            int genChoice = getGenUserChoice();
            int numRequestsGen = getPositiveIntInput("Number of Requests to Generate: ", 1);
            switch (genChoice)
            { /* ... same generation calls ... */
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
            printQueueGen(initialQueue);
            plotScatter(initialQueue, maxCylinder);
            break;
        }
        else
        {
            std::cout << "Invalid choice. Please enter 'm' or 'g'." << std::endl;
        }
    }

    // --- Validate Final Queue ---
    // ... (Validation code remains the same) ...
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

    // --- Display Configuration ---
    std::cout << "\n--- Configuration ---" << std::endl;
    std::cout << "Start Head:    " << startHead << std::endl;
    std::cout << "Max Cylinder:  " << maxCylinder << std::endl;
    // Display Disk Params
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Seek Time/Cyl: " << diskParams.avgSeekTimePerCylinderMs << " ms" << std::endl;
    std::cout << "Avg Rot Latency:" << diskParams.avgRotationalLatencyMs << " ms" << std::endl;
    std::cout << "Avg Xfer Time: " << diskParams.transferTimePerRequestMs << " ms" << std::endl;
    std::cout << std::defaultfloat;
    // Display Queue
    std::cout << "Initial Queue (" << initialQueue.size() << " requests): ";
    const size_t max_print_config = 50;
    for (size_t i = 0; i < std::min(initialQueue.size(), max_print_config); ++i)
    {
        std::cout << initialQueue[i] << (i == initialQueue.size() - 1 || i == max_print_config - 1 ? "" : ", ");
    }
    if (initialQueue.size() > max_print_config)
        std::cout << "...";
    std::cout << std::endl;

    // --- Run Simulations & Calculate Metrics --- MODIFIED: Pass diskParams
    std::vector<AlgorithmResult> results;
    int numRequests = initialQueue.size();

    results.push_back(calculateMetrics("FCFS", fcfs(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("SSTF", sstf(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("SCAN", scan(startHead, maxCylinder, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("C-SCAN", cscan(startHead, maxCylinder, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("LOOK", look(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("C-LOOK", clook(startHead, initialQueue), numRequests, diskParams));
    results.push_back(calculateMetrics("HDSA", hdsa(startHead, initialQueue), numRequests, diskParams));

    // --- Find Best Algorithm ---
    // (No changes needed here, still based on Total Movement)
    // ... (Omitted for brevity) ...
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

    // --- Display Summary Table --- MODIFIED: Added Avg Resp Time column
    std::cout << "\n--- Algorithm Comparison Summary ---" << std::endl;
    std::cout << std::left << std::setw(11) << "Algorithm" << "| "
              << std::right << std::setw(10) << "Total Move" << " | "
              << std::right << std::setw(10) << "Avg Seek" << " | "
              << std::right << std::setw(10) << "Max Seek" << " | "
              << std::right << std::setw(11) << "StdDev Seek" << " | "
              << std::right << std::setw(10) << "Throughput" << " |" // Adjusted width slightly
              << std::right << std::setw(14) << "Avg Resp(ms)"       // New column
              << std::endl;
    std::cout << "-----------|------------|------------|------------|-------------|------------|--------------" << std::endl; // Adjusted separator

    // Set default precision for most columns
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

        // Throughput with higher precision
        std::cout << std::fixed << std::setprecision(4);
        if (result.throughput == std::numeric_limits<double>::infinity())
        {
            std::cout << std::right << std::setw(10) << "Inf";
        }
        else
        {
            std::cout << std::right << std::setw(10) << result.throughput;
        }
        std::cout << " |"; // Separator for new column

        // Avg Response Time with 2 decimal places
        std::cout << std::fixed << std::setprecision(2);
        std::cout << std::right << std::setw(14) << result.avgResponseTime;

        std::cout << std::endl;
        // Reset default precision for next loop (although already set above)
        // std::cout << std::fixed << std::setprecision(2);
    }

    // --- Display Notes --- MODIFIED: Added note about response time
    std::cout << "\nNote: [BEST] indicates the algorithm with the lowest Total Head Movement." << std::endl;
    std::cout << "Note: Avg Seek, StdDev Seek, and Throughput are relative to the number of requests serviced (" << numRequests << ")." << std::endl;
    std::cout << "Note: Avg Resp(ms) = Avg(Seek Time + Rotational Latency + Transfer Time) per request service." << std::endl;
    std::cout << "Note: Queueing Delay (time before scheduling) is not included in Avg Response Time." << std::endl;

    return 0;
}