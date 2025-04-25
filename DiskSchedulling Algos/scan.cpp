#include <vector>
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