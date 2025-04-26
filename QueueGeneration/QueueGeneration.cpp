#include "../Headers/QueueGeneration.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <limits>

#include "../Headers/DiskScheduling.h"
#include "../Headers/InputOutput.h"

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
