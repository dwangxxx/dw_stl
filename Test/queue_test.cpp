#include <iostream>
#include <ctime>
#include "../dw_stl/queue.h"
#include <vector>

int main()
{
    dw_stl::queue<int> v1;
    double start = clock();
    for (int i = 0; i < 50000000; ++i)
    {
        v1.push(i);
    }
    std::cout << v1.size() << std::endl;
    double end = clock();
    double total = (end - start) / CLOCKS_PER_SEC; 
    std::cout << total << "ms" << std::endl;
    return 0;
}