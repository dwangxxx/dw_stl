#ifndef MYSTL_CONSTRUCT_H_
#define MYSTL_CONSTRUCT_H_

/*
这个头文件包含两个函数：
construct: 负责完成对象的构造
destroy:   负责完成对象的销毁，销毁之后的空间还是可以使用的
不止allocator对象可以使用，其他迭代器也可以使用来进行对象的构造以及销毁
*/

#include <new>
#include "type_traits.h"
#include "iterator.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragam warning(disable : 4100)
#endif

namespace dw_stl
{
    // construct函数，构造对象
    // 构造Ty类型的变量
    template <class Ty>
    void construct(Ty* ptr)
    {
        ::new ((void*)ptr) Ty();
    }

    // 构造Ty1类型的变量，并且构造函数参数为Ty2
    // 如果是同类型，则调用拷贝构造函数
    template <class Ty1, class Ty2>
    void construct(Ty1* ptr, const Ty2& value)
    {
        // 可能会调用拷贝构造函数
        ::new ((void*)ptr) Ty1(value);
    }

    // 构造Ty类型的变量，参数未定
    template <class Ty, class... Args>
    void construct(Ty* ptr, Args&&... args)
    {
        // forward: 完美转发
        ::new ((void*)ptr) Ty(dw_stl::forward<Args>(args)...);
    }

    // destroy函数，析构对象
    // 析构一个对象
    template <class Ty>
    void destroy_one(Ty*, std::true_type) {}
 
    template <class Ty>
    void destroy_one(Ty* pointer, std::false_type) 
    {
        if (pointer != nullptr)
        {
            pointer->~Ty();
        }
    }

    // 销毁迭代器指向的对象, 如果是内置类型的话，就不用析构
    template <class ForwardIterator>
    void destroy_cat(ForwardIterator , ForwardIterator, std::true_type) {}

    // 如果是自定义的类型class，需要调用析构函数
    template <class ForwardIterator>
    void destroy_cat(ForwardIterator first, ForwardIterator last, std::false_type)
    {
        for (; first != last; ++first)
            // 析构迭代器指向的对象
            destroy(&*first);
    }
 
    template <class Ty>
    void destroy(Ty* pointer)
    {
        // std::is_trivially_destructible<Ty>返回一个type，表示其是否为内置类型
        destroy_one(pointer, std::is_trivially_destructible<Ty>{});
    }

    // 注：指针也是迭代器的一种
    template <class ForwardIterator>
    void destroy(ForwardIterator first, ForwardIterator last)
    {
        // 首先传入iterator_traits萃取迭代器指向的型别
        // 然后判断型别是内置类型还是自定义类型
        destroy_cat(first, last, std::is_trivially_destructible<typename iterator_traits<first>::value_type>{});
    }
}

#endif