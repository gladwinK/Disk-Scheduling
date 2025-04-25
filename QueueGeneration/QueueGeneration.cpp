#include "../Headers/QueueGeneration.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>

// Function to parse the queue from a string
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
        {
            // Keep warnings less verbose for potentially large generated queues
            // std::cerr << "Warning: Invalid number format in queue: '" << segment << "'. Skipping." << std::endl;
        }
        catch (const std::out_of_range &e)
        {
            // std::cerr << "Warning: Number out of range in queue: '" << segment << "'. Skipping." << std::endl;
        }
    }
    return queue;
}

// Function to generate a clustered queue
std::vector<int> generateClustered(int max_cylinder, int num_requests, int desired_num_clusters, std::mt19937 &rng)
{
    std::vector<int> queue;
    queue.reserve(num_requests);

    // Handle edge cases
    if (num_requests <= 0)
        return queue;
    if (max_cylinder < 0)
        max_cylinder = 0; // Ensure non-negative

    int num_clusters = std::max(1, std::min(desired_num_clusters, num_requests));

    // Distribute requests per cluster
    int requests_per_cluster = num_requests / num_clusters;
    int remaining_requests = num_requests % num_clusters;

    // --- Determine Cluster Center Placement (C/N Basis with Separation) ---
    std::vector<int> cluster_centers;
    cluster_centers.reserve(num_clusters);
    // Calculate approximate space per cluster
    double approx_segment_width = (num_clusters > 0) ? static_cast<double>(max_cylinder + 1) / num_clusters : 0.0;

    for (int i = 0; i < num_clusters; ++i)
    {
        // Calculate the ideal center for this segment (middle of the C/N slice)
        double ideal_center_double = (static_cast<double>(i) + 0.5) * approx_segment_width;
        int ideal_center = static_cast<int>(std::round(ideal_center_double));

        // Add a small random offset to the ideal center for variation, but keep them separated
        // Offset range could be a fraction of the segment width, e.g., +/- 10%
        int center_offset_range = static_cast<int>(std::round(approx_segment_width * 0.10));
        center_offset_range = std::max(0, center_offset_range); // Ensure non-negative range

        std::uniform_int_distribution<int> center_offset_dist(-center_offset_range, center_offset_range);
        int actual_center = ideal_center + center_offset_dist(rng);

        // Clamp the final center to be within disk bounds
        actual_center = std::max(0, std::min(max_cylinder, actual_center));
        cluster_centers.push_back(actual_center);
    }
    // Optional: Shuffle centers if you don't strictly need them ordered by position
    // std::shuffle(cluster_centers.begin(), cluster_centers.end(), rng);

    // --- Generate Requests with Density around Centers ---
    // Use Normal distribution for density. Standard deviation controls spread.
    // Make std dev related to the segment width / cluster spacing. Smaller fraction = tighter cluster.
    double std_dev = std::max(1.0, approx_segment_width / 6.0); // Ensure std_dev is at least 1.0
    std::normal_distribution<double> offset_dist(0.0, std_dev); // Mean 0, calculated std dev

    int requests_generated = 0;
    for (int i = 0; i < num_clusters; ++i)
    {
        int cluster_center = cluster_centers[i];
        int current_requests_in_cluster = requests_per_cluster + (i < remaining_requests ? 1 : 0);

        for (int j = 0; j < current_requests_in_cluster; ++j)
        {
            // Generate offset using normal distribution
            double offset = offset_dist(rng);
            // Round offset to nearest integer and add to center
            int request_cyl = static_cast<int>(std::round(cluster_center + offset));

            // Clamp result to disk bounds [0, max_cylinder]
            request_cyl = std::max(0, std::min(max_cylinder, request_cyl));
            queue.push_back(request_cyl);
            requests_generated++;
        }
    }

    // Ensure exactly num_requests (might be off slightly due to clamping/rounding)
    std::uniform_int_distribution<int> fill_dist(0, max_cylinder);
    while (queue.size() < num_requests)
    {
        // Add random requests if needed (less likely with normal dist but possible)
        queue.push_back(fill_dist(rng));
    }
    if (queue.size() > num_requests)
    {
        queue.resize(num_requests); // Trim if somehow too many
    }

    return queue;
}

// Function to generate a uniformly random queue
std::vector<int> generateUniformRandom(int max_cylinder, int num_requests, std::mt19937 &rng)
{
    std::vector<int> queue;
    queue.reserve(num_requests);
    // Handle max_cylinder=0 case correctly within distribution
    std::uniform_int_distribution<int> dist(0, std::max(0, max_cylinder));
    for (int i = 0; i < num_requests; ++i)
    {
        queue.push_back(dist(rng));
    }
    return queue;
}

// Function to generate a sequential queue
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

// Function to generate a mixed queue
std::vector<int> generateMixed(int max_cylinder, int num_requests, std::mt19937 &rng)
{
    std::vector<int> queue;
    queue.reserve(num_requests);
    int num_random = num_requests * 6 / 10;        // 60% random
    int num_clustered = num_requests - num_random; // 40% clustered
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
    {
        queue.push_back(fill_dist(rng));
    }
    if (queue.size() > num_requests)
    {
        queue.resize(num_requests);
    }
    std::shuffle(queue.begin(), queue.end(), rng);
    return queue;
}
