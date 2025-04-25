#include <vector>
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