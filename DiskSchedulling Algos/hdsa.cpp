#include <vector>
// Helper function to perform SSTF on a given queue subset
int run_sstf_subset(int currentHead, std::vector<int> &queue_subset, std::vector<int> &overall_sequence)
{
    while (!queue_subset.empty())
    {
        int closestRequestIndex = -1;
        int minDistance = std::numeric_limits<int>::max();
        for (size_t i = 0; i < queue_subset.size(); ++i)
        {
            int distance = std::abs(queue_subset[i] - currentHead);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestRequestIndex = i;
            }
        }
        if (closestRequestIndex != -1)
        {
            currentHead = queue_subset[closestRequestIndex];
            overall_sequence.push_back(currentHead);
            queue_subset.erase(queue_subset.begin() + closestRequestIndex);
        }
        else
        {
            break;
        }
    }
    return currentHead;
}
// HDSA: Hybrid Disk Scheduling Algorithm
std::vector<int> hdsa(int startHead, const std::vector<int> &requests)
{
    std::vector<int> sequence;
    sequence.push_back(startHead);
    if (requests.empty())
        return sequence;

    std::vector<int> P, Q;
    for (int req : requests)
    {
        if (req < startHead)
            P.push_back(req);
        else if (req > startHead)
            Q.push_back(req);
    }

    int x = std::numeric_limits<int>::max();
    if (!P.empty())
        x = startHead - *std::min_element(P.begin(), P.end());
    int y = std::numeric_limits<int>::max();
    if (!Q.empty())
        y = *std::max_element(Q.begin(), Q.end()) - startHead;

    int currentHead = startHead;
    if (x > y)
    {
        currentHead = run_sstf_subset(currentHead, Q, sequence);
        currentHead = run_sstf_subset(currentHead, P, sequence);
    }
    else
    {
        currentHead = run_sstf_subset(currentHead, P, sequence);
        currentHead = run_sstf_subset(currentHead, Q, sequence);
    }
    return sequence;
}