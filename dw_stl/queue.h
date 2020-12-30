#ifndef DW_STL_QUEUE_H_
#define DW_STL_QUEUE_H_

/*
此文件实现了两个模板类
queue: 队列, 底层容器默认使用deque来实现
priority_queue: 优先队列，使用堆来实现
*/

#include "deque.h"
#include "vector.h"

namespace dw_stl
{
    // 模板类queue
    template <class T, class Sequence = dw_stl::deque<T>>
    class queue
    {
    public:
        // 底层容器的一些型别
        typedef typename Sequence::value_type           value_type;
        typedef typename Sequence::size_type            size_type;
        typedef typename Sequence::reference            reference;
        typedef typename Sequence::const_reference      const_reference;

        static_assert(std::is_same<T, value_type>::value,
                    "The value_type of Sequence should be same with T");
    
    private:
        // 定义一个底层容器
        Sequence c;

    public:
        // 构造函数
        queue() = default;

        explicit queue(size_type n) : c(n) {}

        queue(size_type n, const value_type &value) : c(n, value) {}

        template <class InputIter>
        queue(InputIter first, InputIter last) : c(first, last) {}

        // 拷贝构造函数
        queue(std::initializer_list<T> ilist) : c(ilist.begin(), ilist.end()) {}
        
        queue(const Sequence &c_) : c(c_) {}
        
        queue(Sequence &&c_) noexcept(std::is_nothrow_move_constructible<Sequence>::value)
            : c(dw_stl::move(c_)) {}

        queue(const queue &rhs) : c(rhs.c) {}

        queue(queue &&rhs) noexcept(std::is_nothrow_move_constructible<Sequence>::value)
            : c(dw_stl::move(rhs.c)) {}
        
        // 赋值构造函数
        queue& operator=(const queue &rhs)
        {
            c = rhs.c;
            return *this;
        }

        queue& operator=(queue &&rhs) noexcept(std::is_nothrow_move_assignable<Sequence>::value)
        {
            c = dw_stl::move(rhs.c);
            return *this;
        }

        queue& operator=(std::initializer_list<T> ilist)
        {
            c = ilist;
            return *this;
        }

        // 析构函数
        ~queue() = default;

        // 访问元素相关的操作
        reference front()
        {
            return c.front();
        }
        const_reference front() const
        {
            return c.front();
        }
        reference back()
        {
            return c.back();
        }
        const_reference back() const
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
        template <class ...Args>
        void emplace(Args... args)
        {
            c.emplace_back(dw_stl::forward<Args>(args)...);
        }
        
        void push(const value_type &value)
        {
            // 从队尾入队
            c.push_back(value);
        }
        void push(value_type &&value)
        {
            c.push_back(std::move(value));
        }

        void pop()
        {
            // 从队列前出队
            c.pop_front();
        }

        void clear()
        {
            while (!empty())
                pop();
        }

        void swap(queue &rhs) noexcept(noexcept(dw_stl::swap(c, rhs.c)))
        {
            // 调用底层容器的swap函数
            dw_stl::swap(c, rhs.c);
        }
    
    public:
        friend bool operator==(const queue &lhs, const queue &rhs)
        {
            return lhs.c == rhs.c;
        }
        friend bool operator<(const queue &lhs, const queue &rhs)
        {
            return lhs.c < rhs.c;
        }
    };

    // 重载比较操作符
    template <class T, class Sequence>
    bool operator==(const queue<T, Sequence> &lhs, const queue<T, Sequence> &rhs)
    {
        return lhs == rhs;
    }

    template <class T, class Sequence>
    bool operator!=(const queue<T, Sequence> &lhs, const queue<T, Sequence> &rhs)
    {
        return !(lhs == rhs);
    }

    template <class T, class Sequence>
    bool operator<(const queue<T, Sequence> &lhs, const queue<T, Sequence> &rhs)
    {
        return lhs < rhs;
    }

    template <class T, class Sequence>
    bool operator>(const queue<T, Sequence> &lhs, const queue<T, Sequence> &rhs)
    {
        return rhs < lhs;
    }

    template <class T, class Sequence>
    bool operator<=(const queue<T, Sequence> &lhs, const queue<T, Sequence> &rhs)
    {
        return !(rhs < lhs);
    }

    template <class T, class Sequence>
    bool operator>=(const queue<T, Sequence> &lhs, const queue<T, Sequence> &rhs)
    {
        return !(lhs < rhs);
    }

    // 重载适用于queue的swap
    template <class T, class Sequence>
    void swap(queue<T, Sequence> &lhs, queue<T, Sequence> &rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        // 调用queue类内部定义的swap函数
        lhs.swap(rhs);
    }
}

#endif