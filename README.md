# dw_stl

这个项目用于实现C++ STL中的大部分功能，使用C++11编写，**借鉴了部分大佬的代码以及侯捷老师的《STL源码剖析》一书**。

# 完成进度

- construct.h：用于进行对象的构造和析构。
- allocator.g：实现了allocator类，一个简易的空间分配器，包含allocate、deallocate、construct和destroy函数。
- type_traits.h：用于萃取类型，主要是用于pair类型。
- iterator.h：本文件实现了模板类iterator以及迭代器萃取器iterator_traits，以及反向迭代器reverse_iterator。
- utils.h：此文件实现了一些通用的工具，包括 move, forward, swap 等函数，以及模板类 pair 等。
- uninitialized.h：此文件实现了对未初始化空间进行构造元素，实现了如下几个函数

```C++
uninitialized_copy(first, last, result);
uninitialized_copy_n(first, n, result);
uninitialized_fill(first, last, value);
uninitialized_fill_n(first, n, value);
uninitialized_move(first, last, result);
uninitialized_move_n(first, n, result);
```

如果上述函数传入的参数是有赋值运算符的话，那么就会调用algotithm.h文件中的copy、fill、move等函数，如果没有赋值运算符，就会直接对对象进行构造。
- algorithm.h：这个头文件包含了stl的基本算法，本文件中实现的算法均是有赋值操作符(=)的。在uninitialized中的函数(uninitialized_fill, uninitialized_copy等)会调用本文件中的fill、copy等函数。
- vector.h：本文件实现了容器vector。其中实现了push_back, emplace_back, 移动构造函数, 为vector重载了swap函数等。
- deque.h: 本文件实现了容器deque。其中实现了push_front, push_back, pop_front, pop_back, front, back, emplace_front, emplace_back等函数，为deque重载了swap函数, 也实现了移动构造函数等。
- stack.h: 实现了stack模板类，底层容器默认使用的是deque，可以显式指定底层容器的类型，可选list。
- queue.h: 实现了queue模板类，底层容器默认使用的是deuque，可以显式指定底层容器的类型，可选list。此文件还会实现一个priority_queue(优先队列)。
- heap_algo.h: 实现了堆函数push_heap, pop_heap, sort_heap, make_heap等函数。

# 待完成

- unordered_set, unordered_map, map, multimap, set, multiset.......

# 测试

测试在Test文件夹下，当前只是对其正确性以及性能进行简单的测试。

* vector测试

![vector_test](https://github.com/dwangxxx/dw_stl/blob/main/test_result/vector_test.png)

* deque测试

![deque_test](https://github.com/dwangxxx/dw_stl/blob/main/test_result/deque_test.png)

* queue测试

![queue_test](https://github.com/dwangxxx/dw_stl/blob/main/test_result/queue_test.png)

* stack测试

![stack_test](https://github.com/dwangxxx/dw_stl/blob/main/test_result/stack_test.png)

* priority_queue测试

![pri_queue_test](https://github.com/dwangxxx/dw_stl/blob/main/test_result/pri_queue_test.png)