#ifndef DW_STL_ALLOCATOR_H_
#define DW_STL_ALLOCATOR_H_

/*
allocator.h
construct.h
memory.h
三个头文件实现了分配器的工作，以及初始化一段空间的工作
*/
// 用于实现模板类allocator
// allocator的作用是用于管理内存的分配，释放，对象的构造和析构等

/*
实现了以下函数：
allocate
deallocate
construct
destroy
*/

#include "construct.h"
#include "util.h"

namespace dw_stl
{
    // 模板类allocator
    // T代表数据类型
    template <class T>
    class allocator 
    {
    public:
        typedef T               value_type;
        typedef T*              pointer;
        typedef const T*        const_pointer;
        typedef T&              reference;
        typedef const T&        const_reference;
        typedef size_t          size_type;
        typedef ptrdiff_t       difference_type;

        // 分配空间
        static T*   allocate();
        static T*   allocate(size_type n);
        
        // 释放空间
        static void deallocate(T* ptr);
        static void deallocate(T* ptr, size_type n);

        // 构造函数
        static void construct(T* ptr);
        static void construct(T* ptr, const T& value);
        static void construct(T* ptr, T&& value);

        template <class... Args>
        static void construct(T* ptr, Args&& ...args);

        // 调用析构函数，但是之前的空间还可以使用
        static void destroy(T* ptr);
        static void destroy(T* first, T* last);
        
    };

    // 分配一个类型为T的一个单元的内存
    template <class T>
    T* allocator<T>::allocate()
    {
        return static_cast<T*>(::operator new(sizeof(T)));
    }

    // 分配n个类型的T的内存
    template <class T>
    T* allocator<T>::allocate(size_type n)
    {
        if (n == 0)
            return nullptr;
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    /********************************************/
    // 释放ptr指向的单元
    /*********************************************/
    template <class T>
    void allocator<T>::deallocate(T* ptr)
    {
        if (ptr == nullptr)
            return;
        ::operator delete(ptr);
    }

    template <class T>
    void allocator<T>::deallocate(T* ptr, size_type )
    {
        if (ptr == nullptr)
            return;
        // operator不会调用构造函数，因此需要首先销毁对象才能调用delete
        ::operator delete(ptr);
    }

    /********************************************
    对象的构造
    *************************************************/
    // 构造函数
    template <class T>
    void allocator<T>::construct(T* ptr)
    {
        dw_stl::construct(ptr);
    }

    // 构造对象，拷贝构造函数
    template <class T>
    void allocator<T>::construct(T* ptr, const T& value)
    {
        dw_stl::construct(ptr, value);
    }

    // 构造对象，参数为构造函数的参数
    template <class T>
    template <class ...Args>
    void allocator<T>::construct(T* ptr, Args&& ...args)
    {
        dw_stl::construct(ptr, dw_stl::forward<Args>(args)...);
    }

    /*************************************
    销毁对象
    **************************************/

    // 销毁对象
    template <class T>
    void allocator<T>::destroy(T* ptr)
    {
        dw_stl::destroy(ptr);
    }

    template <class T>
    void allocator<T>::destroy(T* first, T* last)
    {
        dw_stl::destroy(first, last);
    }

}

#endif