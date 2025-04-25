#include <vector>
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