#ifndef MYSTL_VECTOR_H_
#define MYSTL_VECTOR_H_

/*
这个头文件用于实现模板类 vector<T>

异常保证有几种等级：

(1) 不抛出异常保证（Nothrow exception guarantee）：函数决不抛出异常。
(2) 强异常保证（Strong exception guarantee）：若函数抛出异常，则程序的状态被回滚到正好在函数调用前的状态。
(3) 基础异常保证（Basic exception guarantee）：若函数抛出异常，则程序在合法状态。它可能需要清理，但所有不变量都原封不动。
(4) 无异常保证（No exception guarantee）：若函数抛出异常，则程序可能不在合法状态：可能已经发生了资源泄漏、内存谬误，或其他摧毁不变量的错误。

异常保证：
    dw_stl::vector<T>满足基本异常保证，部分函数无异常保证，并对以下函数做强异常安全保证：
        emplace_back
        emplace
        push_back
    当std::is_nothrow_move_assignable<T>::value == true时，以下函数也满足强异常保证
        reserve
        resize
        insert
*/

#include <initializer_list>
#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"
#include "allocator.h"

namespace dw_stl
{
    #ifdef max
    #pragam message("#undefing macro max")
    #undef max
    #endif

    #ifdef min
    #pragma message("#undefing macro min")
    #undef min
    #endif

    // vector模板类
    // 模板参数为T
    template <class T>
    class vector
    {
        // 判断是否为bool类型，如果是就抛出异常
        static_assert(!std::is_same<bool, T>::value, "vector<bool> is not implemented in this stl");

    public:
        // vector的型别定义
        typedef dw_stl::allocator<T>            allocator_type;
        typedef dw_stl::allocator<T>            data_allocator;

        typedef typename allocator_type::value_type         value_type;
        typedef typename allocator_type::pointer            pointer;
        typedef typename allocator_type::const_pointer      const_pointer;
        typedef typename allocator_type::reference          reference;
        typedef typename allocator_type::const_reference    const_reference;
        typedef typename allocator_type::size_type          size_type;
        typedef typename allocator_type::difference_type    difference_type;

        // vector的底层迭代器使用的是数据指针
        // 因此在iterator.h里面有为指针制定的萃取器，为普通萃取器的偏特化版本
        // vector正向迭代器
        typedef value_type*                                 iterator;
        typedef const value_type*                           const_iterator;
        
        // vector反向迭代器
        typedef dw_stl::reverse_iterator<iterator>          reverse_iterator;
        typedef dw_stl::reverse_iterator<const_iterator>    const_reverse_iterator;

    private:
        iterator begin_;        // 表示目前使用空间的头部
        iterator end_;          // 表示目前使用空间的尾部
        iterator cap_end_;          // 表示目前存储空间的尾部
    
    public:
        // 构造函数(保证不抛出异常)
        vector() noexcept
        { 
            try_init(); 
        }

        explicit vector(size_type n)
        { 
            fill_init(n, value_type()); 
        }

        vector(size_type n, const value_type& value)
        { 
            fill_init(n, value); 
        }

        // 使用其他迭代器来进行初始构造
        template <class Iter, typename std::enable_if<
            dw_stl::is_input_iterator<Iter>::value, int>::type = 0>
        vector(Iter first, Iter last)
        {
            MYSTL_DEBUG(!(last < first));
            range_init(first, last);
        }

        // 拷贝构造函数
        vector(const vector& rhs)
        {
            range_init(rhs.begin_, rhs.end_);
        }

        // 移动构造函数
        vector(cosnt vector&& rhs) noexcept
            : begin_(rhs.begin_),
              end_(rhs.end_),
              cap_end_(rhs.cap_end_)
        {
            // 将原先的指针置为空
            rhs.begin_ = nullptr;
            rhs.end_ = nullptr;
            rhs.cap_end_ = nullptr;
        }

        // 使用初始化列表作为构造参数vector<int> a = {1, 2, 3, 4, 5...}
        vector(std::initializer_list<value_type> ilist)
        {
            range_init(ilist.begin(), ilist.end());
        }

        
    };
}

#endif