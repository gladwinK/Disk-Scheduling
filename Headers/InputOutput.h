#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include "DiskScheduling.h"
// input Functions
int getPositiveIntInput(const std::string &prompt, int min_val = 0, int max_val = std::numeric_limits<int>::max());

double getPositiveDoubleInput(const std::string &prompt, double min_val = 0.0, double max_val = std::numeric_limits<double>::max());

std::vector<int> parseQueue(const std::string &s);

int getGenUserChoice();

void printGenWelcome();
void printQueueGen(const std::vector<int> &queue);
void plotScatter(const std::vector<int> &queue, int max_cylinder);
void displayConfiguration(int startHead, int maxCylinder, const DiskPerformanceParams &diskParams, const std::vector<int> &initialQueue);
void displaySummaryTable(const std::vector<AlgorithmResult> &results, int numRequests);
void displayNotes(int numRequests);
#endif // INPUTOUTPUT_H