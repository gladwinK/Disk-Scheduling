#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H

#include <iostream>
#include <vector>
#include <string>
#include <limits>

void printQueueGen(const std::vector<int> &queue);

void plotScatter(const std::vector<int> &queue, int max_cylinder);

void printGenWelcome();

int getGenUserChoice();

int getPositiveIntInput(const std::string &prompt, int min_val = 0, int max_val = std::numeric_limits<int>::max());

#endif