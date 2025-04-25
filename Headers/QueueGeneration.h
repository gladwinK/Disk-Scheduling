#ifndef QUEUEGENERATION_H
#define QUEUEGENERATION_H

#include <iostream>
#include <vector>
#include <string>
#include <random>

std::vector<int> parseQueue(const std::string &s);

std::vector<int> generateClustered(int max_cylinder, int num_requests, int desired_num_clusters, std::mt19937 &rng);

std::vector<int> generateUniformRandom(int max_cylinder, int num_requests, std::mt19937 &rng);

std::vector<int> generateSequential(int max_cylinder, int num_requests, std::mt19937 &rng);

std::vector<int> generateMixed(int max_cylinder, int num_requests, std::mt19937 &rng);

#endif