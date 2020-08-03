#include <iostream>
#include <chrono>

#include "../include/Array.h"

int main()
{
    auto test = zeros<int>({3, 5, 5});

    test[Slice(0,3) & Slice(0,2) & Slice(0,5)] += ones<int>({3, 2, 5}); std::cout << "\n";

    test[Slice(1) & Slice(0,2) & Slice(0,5)] += ones<int>({1, 2, 5}); std::cout << "\n";

    test[Slice(0,3) & Slice(2) & Slice(0,5)] += ones<int>({3, 2, 5})[Slice(0,3) & Slice(0) & Slice(0,5)]; std::cout << "\n";

    test[Slice(0,3) & Slice(2) & Slice(0,5)] += ones<int>({3, 5}); std::cout << "\n";

    test[Slice(1) & Slice(3,5) & Slice(3,5)] = ones<int>({1, 1, 1, 1, 2, 2}); std::cout << "\n";

    test[Slice(1) & Slice(4,5) & Slice(0,5)] += ones<int>({5}); std::cout << "\n";

    test[{1,2,2}] = 2;

    /*auto start = std::chrono::high_resolution_clock::now();

    auto test2 = zeros<int>({256, 256, 16});

    test2[Slice(0,256) & Slice(0,256) & Slice(0,16)] = ones<int>({16});

    auto finish = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count()*1000 << "ms\n";*/

    /*start = std::chrono::high_resolution_clock::now();

    auto test3 = ones<int>({8,8});

    for(int i = 0; i < 10000; i++)
    {
        test3 += ones<int>({8,8});
    }

    finish = std::chrono::high_resolution_clock::now();
    elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count()*1000 << "ms\n";*/

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            std::cout << test[{1, i, j}] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}