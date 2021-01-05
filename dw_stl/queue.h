#ifndef DW_STL_QUEUE_H_
#define DW_STL_QUEUE_H_

/*
此文件实现了两个模板类
queue: 队列, 底层容器默认使用deque来实现
priority_queue: 优先队列，使用堆来实现
*/

#include "deque.h"
#include "vector.h"
#include "functional.h"
#include "heap_algo.h"

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

    /*
    模板类priority_queue
    第一个参数代表数据类型
    第二个参数代表容器类型，默认使用vector作为底层容器
    第三个参数代表比较函数，缺省使用dw_stl::less作为比较方式(得到的是大顶堆)
    主要有几个函数:
    push pop top size empty
    */
    template <class T, class Container = dw_stl::vector<T>, 
        class Compare = dw_stl::less<typename Container::value_type>>
    class priority_queue
    {
    public:
        typedef Container   container_type;
        typedef Compare     value_compare;
        // 使用底层容器的型别
        typedef typename Container::value_type  value_type;
        typedef typename Container::size_type   size_type;
        typedef typename Container::reference   reference;
        typedef typename Container::const_reference     const_reference;

        static_assert(std::is_same<T, value_type>::value, "The value_type of Container should be same with T");

    private:
        container_type c_;      // 使用底层容器来表现priority_queue
        value_compare  cmp_;    // 权值比较标准
    
    public:
        // 构造函数
        priority_queue() = default;

        priority_queue(const Compare &c) : c_(), cmp_(c) {}
        explicit priority_queue(size_type n) : c_(n)
        {
            // 首先将其调整成为一个堆
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }
        priority_queue(size_type n, const value_type& value) : c_(n, value)
        {
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }

        template <class InputIter>
        priority_queue(InputIter first, InputIter last) : c_(first, last)
        {
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }

        priority_queue(std::initializer_list<T> ilist) : c_(ilist)
        {
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }

        priority_queue(const Container& s) : c_(s)
        {
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }

        // 右值引用, 移动构造
        priority_queue(Container &&s) : c_(dw_stl::move(s))
        {
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }

        priority_queue(const priority_queue &rhs) : c_(rhs.c_), cmp_(rhs.cmp_)
        {
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }

        priority_queue(priority_queue &&rhs) : c_(dw_stl::move(rhs.c_), cmp_(rhs.cmp_))
        {
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
        }

        // 赋值构造函数
        priority_queue &operator=(const priority_queue &rhs)
        {
            c_ = rhs.c_;
            cmp_ = rhs.cmp_;
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
            return *this;
        }
        // 移动赋值函数
        priority_queue &operator=(const priority_queue &&rhs)
        {
            c_ = dw_stl::move(rhs.c_);
            cmp_ = rhs.cmp_;
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
            return *this;
        }

        priority_queue &operator=(std::initializer_list<T> ilist)
        {
            c_ = ilist;
            cmp_ = value_compare();
            dw_stl::make_heap(c_.begin(), c_.end(), cmp_);
            return *this;
        }

        ~priority_queue() = default;

    public:
        // 访问元素相关操作
        const_reference top() const
        {
            return c_.front();
        }

        // 容量相关操作
        bool empty() const noexcept
        {
            return c_.empty();
        }
        size_type size() const noexcept
        {
            return c_.size();
        }

        // 修改容器相关操作
        template <class... Args>
        void emplace(Args&& ...args)
        {
            // 首先插入容器
            c_.emplace_back(dw_stl::forward<Args>(args)...);
            // 然后再调整成堆
            dw_stl::push_heap(c_.begin(), c_.end(), cmp_);
        }

        void push(const value_type& value)
        {
            c_.push_back(value);
            dw_stl::push_heap(c_.begin(), c_.end(), cmp_);
        }

        void push(value_type &&value)
        {
            c_.push_back(dw_stl::move(value));
            dw_stl::push_heap(c_.begin(), c_.end(), cmp_);
        }

        void pop()
        {
            // 首先将第一个元素和最后一个元素交换
            dw_stl::pop_heap(c_.begin(), c_.end(), cmp_);
            // 弹出最后一个元素
            c_.pop_back();
        }

        void clear()
        {
            while (!empty())
                pop();
        }

        void swap(priority_queue &rhs) noexcept(noexcept(dw_stl::swap(c_, rhs.c_)) &&
                                                noexcept(dw_stl::swap(cmp_, rhs.cmp_)))
        {
            dw_stl::swap(c_, rhs.c_);
            dw_stl::swap(cmp_, rhs.cmp_);
        }
    
    public:
        friend bool operator==(const priority_queue &lhs, const priority_queue &rhs)
        {
            return lhs.c_ == rhs.c_;
        }

        friend bool operator!=(const priority_queue &lhs, const priority_queue &rhs)
        {
            return lhs.c_ == rhs.c_;
        }
    };

    // 重载比较操作符
    template <class T, class Container, class Compare>
    bool operator==(priority_queue<T, Container, Compare> &lhs, 
                    priority_queue<T, Container, Compare> &rhs)
    {
        return lhs == rhs;
    }

    template <class T, class Container, class Compare>
    bool operator!=(priority_queue<T, Container, Compare> &lhs,
                    priority_queue<T, Container, Compare> &rhs)
    {
        return lhs != rhs;
    }

    // 重载swap函数
    template <class T, class Container, class Compare>
    void swap(priority_queue<T, Container, Compare> &lhs,
              priority_queue<T, Container, Compare> &rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }
}

#endif