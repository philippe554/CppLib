#include <vector>

#include "Range.h"

class Slice
{
public:
    Slice(int index)
    {
        ranges.emplace_back(index, index + 1);
    }

    Slice(int start, int end)
    {
        ranges.emplace_back(start, end);
    }

    Slice(std::vector<int> shape)
    {
        for(auto size : shape)
        {
            ranges.emplace_back(0, size);
        }
    }

    Slice(Slice& slice1, Slice& slice2)
    {
        if(slice1.ranges.size() != slice2.ranges.size())
        {
            throw std::runtime_error("Index error");
        }

        for(int i = 0; i < slice1.ranges.size(); i++)
        {
            int start = slice1.ranges[i].start + slice2.ranges[i].start;
            int end = start + (slice2.ranges[i].end - slice2.ranges[i].start);

            if(end > slice1.ranges[i].end)
            {
                throw std::runtime_error("Index error");
            }

            ranges.emplace_back(start, end);

            if(end - start == 1)
            {
                ranges.back().hidden = true;
            }
        }
    }

    Slice& operator&(Slice&& other)
    {
        for(auto range : other.ranges)
        {
            ranges.push_back(range);
        }

        return *this;
    }

    std::vector<Range> ranges;
};
