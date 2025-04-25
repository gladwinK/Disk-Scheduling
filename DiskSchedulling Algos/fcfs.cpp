
#include <vector>
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