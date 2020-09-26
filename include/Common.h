#pragma once

#include <string>
#include <vector>

std::string str(std::vector<int> vec)
{
    std::string s = "(";

    for(int i = 0; i < vec.size(); i++)
    {
        s += std::to_string(vec[i]);
        if(i != vec.size() - 1)
        {
            s += ",";
        }
    }
    s += ")";

    return s;
}