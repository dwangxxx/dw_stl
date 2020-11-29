# dw_stl

这个项目用于实现C++ STL中的大部分功能，使用C++11编写，**借鉴了部分大佬的代码以及侯捷老师的《STL源码剖析》一书**。

## 完成进度

- construct.h：用于进行对象的构造和析构。
- allocator.g：实现了allocator类，一个简易的空间分配器，包含allocate、deallocate、construct和destroy函数。
- type_traits.h：用于萃取类型，主要是用于pair类型。
- iterator.h：本文件实现了模板类iterator以及迭代器萃取器iterator_traits，以及反向迭代器reverse_iterator。
- utils.h：此文件实现了一些通用的工具，包括 move, forward, swap 等函数，以及模板类 pair 等。
- uninitialized.h：此文件实现了对未初始化空间进行构造元素，实现了如下几个函数
```C++
uninitialized_copy(first, last, result)
uninitialized_copy_n(first, n, result)
uninitialized_fill(first, last, value)
uninitialized_fill_n(first, n, value)
uninitialized_move(first, last, result)
uninitialized_move_n(first, n, result)
```
如果上述函数传入的参数是有赋值运算符的话，那么就会调用algotithm.h文件中的copy、fill、move等函数，如果没有赋值运算符，就会直接对对象进行构造。
- algorithm.h：这个头文件包含了stl的基本算法，本文件中实现的算法均是有赋值操作符(=)的。在uninitialized中的函数(uninitialized_fill, uninitialized_copy等)会调用本文件中的fill、copy等函数。
- vector.h：本文件实现了容器vector.....(正在实现中，待完成)

## 待完成

- deque, queue, stack, map, multimap, set, multiset.......