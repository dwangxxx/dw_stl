#ifndef DW_STL_MEMORY_H_
#define DW_STL_MEMORY_H_

// 这个头文件负责更高级的动态内存管理
// 包含一些基本函数、空间配置器，以及一个模板类 auto_ptr

#include <cstddef>
#include <cstdlib>
#include <climits>

#include "algorithm.h"
#include "allocator.h"
#include "construct.h"
#include "uninitialized.h"

namespace dw_stl
{
    // 获取对象地址
    // constexpr表示一个常量表达式，可以理解成编译时常量
    template <class Tp>
    constexpr Tp* address_of(Tp& value) noexcept
    {
        return &value;
    }

    // 获取临时缓冲区
    template <class T>
    pair<T*, ptrdiff_t> get_buffer_helper(ptrdiff_t len, T*)
    {
        // 如果超过了整数最大范围
        if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T)))
            len = INT_MAX / sizeof(T);
        while (len > 0)
        {
            T* tmp = static_cast<T*>(malloc(static_cast<size_t>(len) * sizeof(T)));
            if (tmp)
                return pair<T*, ptrdiff_t>(tmp, len);
            // 如果申请失败，将len减少一半重新申请
            len /= 2;
        }
        return pair<T*, ptrdiff_t>(nullptr, 0);
    }

    template <class T>
    pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len)
    {
        // 调用get_buffer_helper
        return get_buffer_helper(len, static_cast<T*>(0));
    }

    // 释放临时缓冲区
    template <class T>
    void release_temporary_buffer(T* ptr)
    {
        free(ptr);
    }

    /***************************************************************/
    // 类模板 ： temporary_buffer, 进行临时缓冲区的申请与释放
    // 申请临时缓冲区，会调用construct里面的函数
    template <class ForwardIterator, class T>
    class temporary_buffer
    {
    private:
        ptrdiff_t original_len;     // 缓冲区申请的大小
        ptrdiff_t len;              // 缓冲区实际的大小
        T* buffer;                  // 指向缓冲区的指针

    public:
        temporary_buffer(ForwardIterator first, ForwardIterator last);

        ~temporary_buffer()
        {
            dw_stl::destroy(buffer, buffer + len);
            free(buffer);
        }

        ptrdiff_t size() const noexcept { return len; }
        ptrdiff_t requested_size() const noexcept { return original_len; }
        T* begin() noexcept { return buffer; }
        T* end() noexcept { return buffer + len; }
    private:
        void allocate_buffer();
        void initialized_buffer(const T& value, std::true_type) 
        {
            dw_stl::fill_n(buffer, len, value);
        }
        // 初始化缓冲区
        void initialized_buffer(const T& value, std::false_type)
        {
            dw_stl::uninitialized_fill_n(buffer, len, value);
        }
    private:
        // 拷贝构造函数
        temporary_buffer(const temporary_buffer&);
        void operator=(const temporary_buffer&);
    };

    // 构造函数
    template <class ForwardIterator, class T>
    temporary_buffer<ForwardIterator, T>::temporary_buffer(ForwardIterator first, ForwardIterator last)
    {
        try 
        {
            len = dw_stl::distance(first, last);
            allocate_buffer();
            if (len > 0)
            {
                initialized_buffer(*first, std::is_trivially_default_constructible<T>());
            }
        }
        catch (...)
        {
            free(buffer);
            buffer = nullptr;
            len = 0;
        }
    }

    // allocate_buffer分配内存函数
    template <class ForwardIterator, class T>
    void temporary_buffer<ForwardIterator, T>::allocate_buffer()
    {
        original_len = len;
        if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T)))
            len = INT_MAX / sizeof(T);
        while (len > 0)
        {
            buffer = static_cast<T*>(malloc(len * sizeof(T)));
            if (buffer)
                break;
            // 申请失败时减少空间大小
            len /= 2;
        }
    }

    /*************************************************************************/
    // auto_ptr 模板类
    template <class T>
    class auto_ptr
    {
    public:
        typedef T elem_type;
    
    private:
        T* m_ptr;   // 实际指针

    public:
        explicit auto_ptr(T* p = nullptr) : m_ptr(p) {}
        auto_ptr(auto_ptr& rhs) : m_ptr(rhs.release()) {}
        
        template <class U>
        auto_ptr(auto_ptr<U>& rhs) : m_ptr(rhs.release()) {}

        auto_ptr& operator=(auto_ptr& rhs)
        {
            if (this != &rhs)
            {
                delete m_ptr;
                m_ptr = rhs.release();
            }
            return *this;
        }

        template <class U>
        auto_ptr& operator=(auto_ptr<T>& rhs)
        {
            if (this->get != rhs.get())
            {
                delete m_ptr;
                m_ptr = rhs.release();
            }
            return *this;
        }

        ~auto_ptr() { delete m_ptr; }
    
    public:
        // 重载操作符
        T& operator*() const 
        {
            return *m_ptr;
        }
        T* operator->() const 
        {
            return m_ptr;
        }

        // 获取指针
        T* get() const 
        {
            return m_ptr;
        }

        // 释放指针
        T* release()
        {
            // 转移控制权
            T* tmp = m_ptr;
            m_ptr = nullptr;
            return tmp;
        }

        // 重置指针
        void reset(T* p = nullptr)
        {
            if (m_ptr != p)
            {
                delete m_ptr;
                m_ptr = p;
            }
        }
    }; // class auto_ptr

} // namespace dw_stl

#endif