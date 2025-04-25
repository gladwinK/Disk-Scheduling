#include <vector>
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