#include <iostream>
#include <ctime>
#include "../dw_stl/stack.h"
#include <vector>

int main()
{
    dw_stl::stack<int> v1;
    double start = clock();
    int i = 0;
    for (i = 0; i < 50000000; ++i)
    {
        v1.push(i);
    }
    std::cout << i << std::endl;
    double end = clock();
    double total = (end - start) / CLOCKS_PER_SEC; 
    std::cout << total << "ms" << std::endl;
    return 0;
}