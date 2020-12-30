#ifndef DW_STL_STACK_H_
#define DW_STL_STACK_H_

// 这个头文件包含了一个模板类 stack, 用于实现堆栈
// 默认使用deque来实现

#include "deque.h"

namespace dw_stl
{
    
    /*
    模板类堆栈stack
    参数一代表数据类型
    参数二代表底层容器类型，默认使用dw_stl::deque作为底层容器
    */
    template <class T, class Sequence = dw_stl::deque<T>>
    class stack
    {
    public:
        // 底层容器
        // 使用底层容器的型别
        typedef typename Sequence::value_type      value_type;
        typedef typename Sequence::size_type       size_type;
        typedef typename Sequence::reference       reference;
        typedef typename Sequence::const_reference const_reference;

        static_assert(std::is_same<T, value_type>::value, 
                     "The value_type of Container should be same with T");
        
    private:
        Sequence c;
    
    public:
        // 构造函数, 构造函数起始都会调用底层容器的构造函数
        stack() = default;

        // 显式构造函数
        explicit stack(size_type n) : c(n) {}

        // 其实是调用了底层容器的构造函数
        stack(size_type n, const value_type &value) : c(n, value) {}

        template <class InputIter>
        stack(InputIter first, InputIter last) : c(first, last) {}

        stack(std::initializer_list<T> ilist) : c(ilist.begin(), ilist.end()) {}

        stack(const Sequence &c_) : c(c_) {}

        stack(Sequence &&c_) noexcept(std::is_nothrow_move_constructible<Sequence>::value)
            : c(dw_stl::move(c_)) {}
        
        stack(const stack &rhs) : c(rhs.c) {}

        stack(stack &&rhs) noexcept(std::is_nothrow_move_constructible<Sequence>::value)
            : c(dw_stl::move(rhs.c)) {}

        // 左值引用
        stack &operator=(const stack &rhs)
        {
            c = rhs.c;
            return *this;
        }

        // 右值引用
        stack &operator=(stack &&rhs) noexcept(std::is_nothrow_move_assignable<Sequence>::value)
        {
            c = dw_stl::move(rhs.c);
            return *this;
        }

        // 初始化列表
        stack &operator=(std::initializer_list<T> ilist)
        {
            c = ilist;
            return *this;
        }

        ~stack() = default;

        // 访问元素相关的操作
        reference top() 
        {
            return c.back();
        }
        const_reference top() const
        {
            return c.back();
        }

        // 容量相关操作
        bool empty() const noexcept
        {
            return c.empty();
        }
        size_type size() const noexcept
        {
            return c.size();
        }

        // 修改容器相关操作
        template <class... Args>
        void emplace(Args&& ...args)
        {
            c.emplace_back(dw_stl::forward<Args>(args)...);
        }

        // push从尾端push
        void push(const value_type &value)
        {
            c.push_back(value);
        }
        void push(value_type &&value)
        {
            // 右值引用
            c.push_back(dw_stl::move(value));
        }

        void pop()
        {
            // pop从尾端pop
            c.pop_back();
        }

        // clear只是清除元素，内存依旧存在
        void clear()
        {
            while (!empty())
                pop();
        }

        void swap(stack &rhs) noexcept(noexcept(dw_stl::swap(c, rhs.c)))
        {
            dw_stl::swap(c, rhs.c);
        }
    
    public:
        // 友元函数，因为需要访问类的私有成员, 其余所有操作符都可以用下述的操作符来完成
        friend bool operator==(const stack &lhs, const stack &rhs)
        {
            return lhs.c == rhs.c;
        }
        friend bool operator<(const stack &lhs, const stack &rhs)
        {
            return lhs.c < rhs.c;
        }
    };

    // 重载比较操作符
    template <class T, class Sequence>
    bool operator==(const stack<T, Sequence> &lhs,  const stack<T, Sequence> &rhs)
    {
        return lhs == rhs;
    }

    template <class T, class Sequence>
    bool operator<(const stack<T, Sequence> &lhs, const stack<T, Sequence> &rhs)
    {
        return lhs < rhs;
    }

    template <class T, class Sequence>
    bool operator!=(const stack<T, Sequence> &lhs, const stack<T, Sequence> &rhs)
    {
        return !(lhs == rhs);
    }

    template <class T, class Sequence>
    bool operator>(const stack<T, Sequence> &lhs, const stack<T, Sequence> &rhs)
    {
        return rhs < lhs;
    }

    template <class T, class Sequence>
    bool operator<=(const stack<T, Sequence> &lhs, const stack<T, Sequence> &rhs)
    {
        return !(rhs < lhs);
    }

    template <class T, class Sequence>
    bool operator>=(const stack<T, Sequence> &lhs, const stack<T, Sequence> &rhs)
    {
        return !(lhs < rhs);
    }

    // 重载swap
    template <class T, class Sequence>
    void swap(stack<T, Sequence> &lhs, stack<T, Sequence> &rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }
}

#endif