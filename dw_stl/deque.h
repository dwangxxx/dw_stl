#ifndef MYSTL_DEQUE_H_
#define MYSTL_DEQUE_H_

/*
这个头文件实现了一个模板类 deque(双端队列)

异常保证：
    dw_stl::deque<T> 满足基本异常保证，部分函数无异常保证，并对以下函数做强异常安全保证：
    emplace_front
    emplace_back
    emplace
    push_front
    push_back
    insert
*/

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

namespace dw_stl
{
    #ifdef max
    #pragma message("#undefing macro max")
    #undef max
    #endif

    #ifdef min
    #pragma message("#undefing macro min")
    #undef min
    #endif

    // deque中中控器map的初始化大小
    #ifndef DEQUE_MAP_INIT_SIZE
    #define DEQUE_MAP_INIT_SIZE 8
    #endif

    // deque中每个缓冲区的大小
    template <class T>
    struct deque_buf_size
    {
        // constexpr常量表达式，表示只在编译时期计算一遍
        static constexpr size_t value = sizeof(T) < 256 ? 4096 / sizeof(T) : 16;
    };

    // deque迭代器: 使用random_access_iterator_tag，支持随机存取
    // Ref和Ptr表示是否为const_iterator
    // 如果Ref传进const T&, Ptr传进const T*，则是const_iterator
    // 如果Ref传进T&, Ptr传进T*，则是普通iterator
    template <class T, class Ref, class Ptr>
    struct deque_iterator : public iterator<random_access_iterator_tag, T>
    {
        // 默认是private
        typedef deque_iterator<T, T&, T*>               iterator;
        typedef deque_iterator<T, const T&, const T*>   const_iterator;
        typedef deque_iterator                          self;

        typedef T               value_type;
        typedef Ptr             pointer;
        typedef Ref             reference;
        typedef size_t          size_type;
        typedef ptrdiff_t       difference_type;
        // 数据类型的指针
        typedef T*              value_pointer;
        // 指向数据的指针的指针
        typedef T**             map_pointer;

        // 计算buffer_size
        static const size_type buffer_size = deque_buf_size<T>::value;

        // 迭代器所含成员数据
        value_pointer cur;      // 指向所在缓冲区的当前元素
        value_pointer first;    // 指向所在缓冲区的头部
        value_pointer last;     // 指向所在缓冲区的尾部 
        map_pointer node;       // 缓冲区所在节点

        // 构造函数
        deque_iterator() noexcept
            : cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}
        
        deque_iterator(value_pointer v, map_pointer n)
            : cur(v), first(*n), last(*n + buffer_size), node(n) {}
        
        deque_iterator(const iterator &rhs)
            : cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}
        deque_iterator(iterator &&rhs)
            : cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) 
        {
            rhs.cur = nullptr;
            rhs.first = nullptr;
            rhs.last = nullptr;
            rhs.node = nullptr;
        }

        deque_iterator(const const_iterator &rhs)
            : cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}

        self &operator=(const iterator &rhs)
        {
            if (this != &rhs)
            {
                cur = rhs.cur;
                first = rhs.first;
                last = rhs.last;
                node = rhs.node;
            }
            return *this;
        }

        // 跳转到另一个缓冲区
        void set_node(map_pointer new_node)
        {
            node = new_node;
            // 重新设置first和last指针
            first = *new_node;
            last = first + buffer_size;
        }

        // 重载运算符
        // const表示传入的this指针为const指针，对成员的修改都是不允许的, 返回迭代器指向的成员
        reference operator*() const
        {
            return *cur;
        }
        pointer operator->() const
        {
            return cur;
        }

        difference_type operator-(const self &x) const
        {
            return static_cast<difference_type>(buffer_size) * (node - x.node) 
                + (cur - first) - (x.cur - x.first);
        }

        // 返回迭代器本身
        self& operator++()
        {
            ++cur;
            // 如果到达当前缓冲区的末尾
            if (cur == last)
            {
                set_node(node + 1);
                cur = first;
            }
            return *this;
        }

        self operator++(int)
        {
            self tmp = *this;
            ++*this;
            return tmp;
        }

        self& operator--()
        {
            // 是否到达缓冲区的头
            if (cur == first)
            {
                set_node(node  - 1);
                cur = last;
            }
            --cur;
            return *this;
        }

        self operator--(int)
        {
            self tmp = *this;
            --*this;
            return tmp;
        }

        self& operator+=(difference_type n)
        {
            const auto offset = n + (cur - first);
            // 如果加上之后仍在当前缓冲区，则直接加
            if (offset >= 0 && offset < static_cast<difference_type>(buffer_size))
            {
                cur += n;
            }
            // 否则需要跳转到其他缓冲区
            else
            {
                const auto node_offset = offset > 0 ? 
                    offset / static_cast<difference_type>(buffer_size)
                    : -static_cast<difference_type>((-offset - 1) / buffer_size) - 1;
                set_node(node + node_offset);
                cur = first + (offset - node_offset * static_cast<difference_type>(buffer_size));
            }
            return *this;
        }

        self operator+(difference_type n) const
        {
            self tmp = *this;
            return tmp += n;
        }

        self& -=(difference_type n)
        {
            // 调用+=(n)函数
            return  *this += -n;
        }

        self operator-(difference_type n) const
        {
            self tmp = *this;
            return tmp -= n;
        }

        // 重载[]操作符
        reference operator[](difference_type n) const
        {
            return *(*this + n);
        }

        // 重载比较操作符, 判断迭代器的位置大小
        // 因为每一段的地址都是连续的
        bool operator==(const self &rhs) const
        {
            // 判断指针是否相等
            return cur == rhs.cur;
        }
        bool operator<(const self &rhs) const
        {
            return node == rhs.node ? (cur < rhs.cur) : (node < rhs.node);
        }
        bool operator!=(const self &rhs)
        {
            return !(*this == rhs);
        }
        bool operator>(const self& rhs)
        {
            return rhs < *this;
        }
        bool operator<=(const self& rhs)
        {
            return !(rhs < *this);
        }
        bool operator>=(const self& rhs)
        {
            return !(*this < rhs);
        }
    };

    // 模板类deque, 模板参数T代表数据类型
    template <class T>
    class deque
    {
    public:
        // deque的型别定义
        typedef dw_stl::allocator<T>                        allocator_type;
        typedef dw_stl::allocator<T>                        data_allocator;
        // 中控器map的型别
        typedef dw_stl::allocator<T*>                       map_allocator;

        typedef typename allocator_type::value_type         value_type;
        typedef typename allocator_type::pointer            pointer;
        typedef typename allocator_type::const_pointer      const_pointer;
        typedef typename allocator_type::reference          reference;
        typedef typename allocator_type::const_reference    const_reference;
        typedef typename allocator_type::size_type          size_type;
        typedef typename allocator_type::difference_type    difference_type;
        typedef pointer*                                    map_pointer;
        typedef const_pointer*                              const_map_pointer;

        typedef deque_iterator<T, T&, T*>                   iterator;
        typedef deque_iterator<T, const T&, const T*>       const_iterator;
        typedef dw_stl::reverse_iterator<iterator>          reverse_iterator;
        typedef dw_stl::reverse_iterator<const_iterator>    const_reverse_iterator;

        allocator_type get_allocator()
        {
            return allocator_type();
        }

        static const size_type buffer_size = deque_buf_size<T>::value;
    
    private:
        // 以下四个数据来实现一个deque
        iterator        begin_;     // 指向第一块缓冲区的第一个节点
        iterator        end_;       // 指向最后一块缓冲区的最后一个节点
        map_pointer     map_;       // 指向一块map, map中的每个元素都是一个指针，指向缓冲区
        size_type       map_size_;  // map中的指针数量

    public:
        // 构造函数
        deque()
        {
            fill_init(0, value_type());
        }
        // 显式构造
        explicit deque(size_type n)
        {
            fill_init(n, value_type());
        }
        deque(size_type n, const value_type &value)
        {
            fill_init(n, value);
        }

        template <class InputIter, typename std::enable_if<
            dw_stl::is_input_iterator<InputIter>::value, int>::type = 0>
        deque(InputIter first, InputIter last)
        {
            copy_init(first, last, iterator_category(first));
        }
        
        deque(std::initializer_list<value_type> ilist)
        {
            copy_init(ilist.begin(), ilist.end(), dw_stl::forward_iterator_tag());
        }

        deque(const deque &rhs)
        {
            copy_init(rhs.begin(), rhs.end(), dw_stl::forward_iterator_tag());
        }

        // 移动构造函数，右值引用
        deque(deque &&rhs) noexcept
            : begin_(rhs.begin_),
              end_(rhs.end_),
              map_(rhs.map_),
              map_size_(rhs.map_size_)
        {
            rhs.begin_ = nullptr;
            rhs.end_ = nullptr;
            rhs.map_ = nullptr;
            rhs.map_size_ = 0;
        }

        // 赋值构造函数
        deque &operator=(const deque &rhs);
        deque &operator=(deque &&rhs);

        deque &operator=(std::initializer_list<value_type> ilist)
        {
            deque tmp(ilist);
            swap(tmp);
            return *this;
        }

        // 析构函数
        ~deque()
        {
            if (map_ != nullptr)
            {
                clear();
                data_allocator::deallocate(*begin_.node, buffer_size);
                *begin_.node = nullptr;
                map_allocator::deallocate(map_, map_size_);
                map_ = nullptr;
            }
        }

    public:
        // 迭代器相关的操作
        iterator begin() noexcept
        {
            return begin_;
        }
        const_iterator begin() const noexcept
        {
            return begin_;
        }
        iterator end() noexcept
        {
            return end_;
        }
        const_iterator end() const noexcept
        {
            return end_;
        }

        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator(end());
        }
        const_reverse_iterator rbegin() const noexcept
        {
            return reverse_iterator(end());
        }
        reverse_iterator rend() noexcept
        {
            return reverse_iterator(begin());
        }
        const_reverse_iterator rend() const noexcept
        {
            return reverse_iterator(begin());
        }

        const_iterator cbegin() const noexcept
        {
            return begin();
        }
        const_iterator cend() const noexcept
        {
            return end();
        }
        const_reverse_iterator crbegin() const noexcept
        {
            return rbegin();
        }
        const_reverse_iterator crend() const noexcept
        {
            return rend();
        }

        // 容量相关操作
        bool empty() const noexcept
        {
            // 比较begin和end迭代器是否相等
            return begin() == end();
        }
        size_type size() const noexcept
        {
            return end_ - begin_;
        }
        size_type max_size() const noexcept
        {
            return static_cast<size_type>(-1);
        }
        void resize(size_type new_size)
        {
            resize(new_size, value_type());
        }
        void resize(size_type new_size, const value_type &value);
        void shrink_to_fit() noexcept;

        // 访问元素相关操作
        reference operator[](size_type n)
        {
            MYSTL_DEBUG(n < size());
            return begin_(n);
        }
        const_reference operator[](size_type n) const
        {
            MYSTL_DEBUG(n < size());
            return begin_[n];
        }

        reference at(size_type n)
        {
            THROW_LENGTH_ERROR_IF(!(n < size()), "deque<T>::at() subscript out of range");
            return (*this)[n];
        }
        const_reference at(size_type n) const
        {
            THROW_OUT_OF_RANGE_IF(!(n < size()), "deque<T>::at() subscript out of range");
            return (*this)[n];
        }

        // front函数
        reference front()
        {
            MYSTL_DEBUG(!empty());
            return *begin();
        }
        const_reference front()
        {
            MYSTL_DEBUG(!empty());
            return *begin();
        }

        // back函数
        reference back()
        {
            MYSTL_DEBUG(!empty());
            return *(end() - 1);
        }
        const_reference back()
        {
            MYSTL_DEBUG(!empty());
            return *(end() - 1);
        }

        // 修改容器相关操作
        // assign函数
        void assign(size_type n, const value_type &value)
        {
            fill_assign(n, value);
        }
        // forward_iterator也是input_iterator
        template <class InputIter, typename std::enable_if<
            dw_stl::is_input_iterator<InputIter>::value, int>::type = 0>
        void assign(InputIter first, InputIter last)
        {
            // 调用copy_assign函数
            copy_assign(first, last, iterator_category(first));
        }

        // emplace_front、emplace_back和emplace函数
        template <class ...Args>
        void emplace_front(Args&& ...args);
        template <class ...Args>
        void emplace_back(Args&& ...args);
        template <class ...Args>
        iterator emplace(iterator pos, Args&& ...args);
        
        // push_front和push_back函数
        void push_front(const value_type &value);
        void push_back(const value_type &value);

        void push_front(value_type &&value)
        {
            // 调用emplace_front函数
            emplace_front(dw_stl::move(value));
        }
        void push_back(value_type &&value)
        {
            // 调用emplace_back函数
            emplace_back(dw_stl::move(value));
        }

        // pop_back和pop_front函数
        void pop_front();
        void pop_back();

        // insert插入函数
        iterator insert(iterator position, const value_type &value);
        iterator insert(iterator position, value_type &&value);
        void insert(iterator position, size_type n, const value_type &value);
        template <class InputIter, typename std::enable_if<
            dw_stl::is_input_iterator<InputIter>::value, int>::type = 0>
        void insert(iterator position, InputIter first, InputIter last)
        {
            insert_dispatch(position, first, last, iterator_category(first));
        }

        // erase和clear函数
        iterator erase(iterator position);
        iterator erase(iterator first, iterator last);
        void clear();

        // swap函数
        void swap(deque &rhs) noexcept;

    private:
        // 实现上述接口函数的辅助函数

        // 创建中控器map和销毁中控器map
        map_pointer create_map(size_type size);
        void create_buffer(map_pointer start, map_pointer finish);
        void destroy_buffer(map_pointer start, map_pointer finish);

        // 初始化相关函数(map_init, fill_init, copy_init)
        void map_init(size_type n);
        void fill_init(size_type n, const value_type &value);
        template <class InputIter>
        void copy_init(InputIter, InputIter, input_iterator_tag);
        template <class InputIter>
        void copy_init(ForwardIter, ForwardIter, forward_iterator_tag);

        // assign相关函数(fill_assign, copy_assign, copy_assign)
        void fill_assign(size_type n, const value_type &value);
        template <class InputIter>
        void copy_assign(InputIter first, InputIter last, input_iterator_tag);
        template <class ForwardIter>
        void copy_assign(ForwardIter first, ForwardIter last, forward_iterator_tag);

        // insert相关函数
        template <class... Args>
        iterator insert_aux(iterator position, Args&& ...args);
        void fill_insert(iterator position, size_type n, const value_type &x);
        template <class ForwardIter>
        void copy_insert(iterator, ForwardIter, ForwardIter, size_type);
        template <class InputIter>
        void insert_dispatch(iterator, InputIter, InputIter, input_iterator_tag);
        template <class ForwardIter>
        void insert_dispatch(iterator, ForwardIter, ForwardIter, forward_iterator_tag);

        // reallocate相关函数
        void require_capacity(size_type n, bool front);
        // 在头部重新分配内存
        void reallocate_map_at_front(size_type need);
        // 在尾部重新分配内存
        void reallocate_map_at_back(size_type need);
    };

    /********************************************************************************/
    // 复制赋值运算符
    template <class T>
    deque<T> &deque<T>::operator=(const deque &rhs)
    {
        if(this != &rhs)
        {
            const auto len = size();
            // 如果当前size大于等于rhs的size
            if (len >= rhs.size())
            {
                // 将其复制过去之后，删除end_之后的数据
                erase(dw_stl::copy(rhs.begin_, rhs.end_), end_);
            }
            else 
            {
                iterator mid = rhs.begin() + static_cast<difference_type>(len);
                dw_stl::copy(rhs.begin_, mid, begin_);
                insert(end_, mid, rhs.end_);
            }
        }

        return *this;
    }

    // 移动赋值运算符
    template <class T>
    deque<T> &deque<T>::operator=(deque &&rhs)
    {
        // 首先清空原容器
        clear();
        // 然后使用迭代器的移动赋值函数
        begin_ = dw_stl::move(rhs.begin_);
        end_ = dw_stl::move(rhs.end_);
        map_ = rhs.map_;
        map_size_ = rhs.map_size_;
        rhs.map_ = nullptr;
        rhs.map_size_ = 0;
        return *this;
    }

    // 重置容器大小
    template <class T>
    void deque<T>::resize(size_type new_size, const value_type &value)
    {
        const auto len = size();
        // 如果new_size小于当前容量，则删除当前容器多出来的数据
        if (new_size < len)
        {
            erase(begin_ + new_size, end_);
        }
        // 如果new_size大于等于当前容量，则从end处插入数据
        else 
        {
            insert(end_, new_size - len, value);
        }
    }

    // 减小容器容量
    template <class T>
    void deque<T>::shrink_to_fit() noexcept
    {
        // 至少会留下头部缓冲区
        // 将头部之前的内存清空
        for (auto cur = map_; cur < begin_.node; ++cur)
        {
            data_allocator::deallocate(*cur, buffer_size);
            *cur = nullptr;
        }
        // 将尾部之后的内存清空
        for (auto cur = end_.node + 1; cur < map_ + map_size_; ++cur)
        {
            data_allocator::deallocate(*cur, buffer_size);
            *cur = nullptr;
        }
    }

    // 在头部就地构造元素(emplace_front)
    template <class T>
    template <class ...Args>
    void deque<T>::emplace_front(Args&& ...args)
    {
        // 如果没有超出当前缓冲区, 则直接插入
        if(begin_.cur != begin_.first)
        {
            data_allocator::construct(begin_.cur - 1, dw_stl::forward<Args>(args)...);
            --begin_.cur;
        }
        // 如果超出当前缓冲区，则申请一块新的内存
        else 
        {
            require_capacity(1, true);
            try
            {
                --begin_;
                data_allocator::construct(begin_.cur, dw_stl::forward<Args>(args)...);
            }
            catch(...)
            {
                ++begin_;
                throw;
            }
        }
    }

    // 在尾部就地构造元素(emplace_back)
    template <class T>
    template <class ...Args>
    void deque<T>::emplace_back(Args&& ...args)
    {
        if (_end.cur != end_.last - 1)
        {
            data_allocator::construct(end_.cur, dw_stl::forward<Args>(args)...);
            ++end_.cur;
        }
        else 
        {
            require_capacity(1, false);
            data_allocator::construct(end_.cur, dw_stl::forward<Args>(args)...);
            ++end_;
        }
    }

    // 在pos位置就地构造元素(emplace)
    template <class T>
    template <class ...Args>
    typename deque<T>::iterator deque<T>::emplace(iterator pos, Args&& ...args)
    {
        // 如果是在头部插入
        if (pos.cur == begin_.cur)
        {
            // 直接调用emplace_front
            emplace_front(dw_stl::forward<Args>(args)...);
            return begin_;
        }
        // 如果是在尾部插入
        else if (pos.cur == end_.cur)
        {
            // 直接调用emplace_back
            emplace_back(dw_stl::forward<Args>(args)...);
            return end_;
        }
        // 否则调用insert_aux函数
        return insert_aux(pos, dw_stl::forward<Args>(args)...);
    }

    // 在头部插入元素(push_front)
    template <class T>
    void deque<T>::push_front(const value_type &value)
    {
        // 如果没有超出当前缓冲区，则直接构造
        if (begin_.cur != begin_.first)
        {
            data_allocator::construct(begin_.cur - 1, value);
            --begin_.cur;
        }
        // 否则需要重新申请空间
        else 
        {
            require_capacity(1, true);
            try 
            {
                --begin_;
                data_allocator::construct(begin_.cur, value);
            }
            catch(...)
            {
                ++begin_;
                throw;
            }
        }
    }

    // 在尾部插入元素(push_back)
    template <class T>
    void deque<T>::push_back(const value_type &value)
    {
        // 如果在当前缓冲区内
        if (end_.cur != end_.last - 1)
        {
            data_allocator::construct(end_.cur, value);
            ++end_.cur;
        }
        // 不在当前缓冲区内
        else 
        {
            require_capacity(1, false);
            data_allocator::construct(end_.cur, value);
            ++end_;
        }
    }

    // 弹出头部元素
    template <class T>
    void deque<T>::pop_front()
    {
        MYSTL_DEBUG(!empty());
        // 如果弹出之后依然是当前缓冲区内
        if (begin_.cur != begin_.last - 1)
        {
            data_allocator::destroy(begin_.cur);
            ++begin_.cur;
        }
        else 
        {
            data_allocator::destroy(begin_.cur);
            ++begin_;
            destroy_buffer(begin_.node - 1, begin_.node - 1);
        }
    }

    // 弹出尾部元素
    template <class T>
    void deque<T>::pop_back()
    {
        MYSTL_DEBUG(!empty());
        if (end_.cur != end_.first)
        {
            --end_.cur;
            data_allocator::destroy(end_.cur);
        }
        else 
        {
            --end_;
            data_allocator::destroy(end_.cur);
            destroy_buffer(end_.node + 1, end_.node + 1);
        }
    }

    // 在position处插入元素
    template <class T>
    typename deque<T>::iterator
    deque<T>::insert(iterator position, const value_type &value)
    {
        // 如果在头部插入，直接调用push_front
        if (position.cur == begin_.cur)
        {
            push_front(value);
            return begin_;
        }
        // 如果在尾部插入，直接调用push_back
        else if(position.cur == end_.cur)
        {
            push_back(value);
            auto tmp = end_;
            --tmp;
            return tmp;
        }
        else 
        {
            return insert_aux(position, value);
        }
    }

    template <class T>
    typename deque<T>::iterator
    deque<T>::insert(iterator position, value_type &&value)
    {
        if (position.cur == begin_.cur)
        {
            emplace_front(dw_stl::move(value));
            return begin_;
        }
        else if (position.cur == end_.cur)
        {
            emplace_back(dw_stl::move(value));
            auto tmp = end_;
            --tmp;
            return tmp;
        }
        else 
        {
            return insert_aux(position, dw_stl::move(value));
        }
    }

    // 在position位置插入n个元素
    template <class T>
    void deque<T>::insert(iterator position, size_type n, const value_type &value)
    {
        if (position.cur == begin_.cur)
        {
            require_capacity(n, true);
            auto new_begin = begin_ - n;
            dw_stl::uninitialized_fill_n(new_begin, n, value);
            begin_ = new_begin;
        }
        else if (position.cur == end_.cur)
        {
            require_capacity(n, false);
            auto new_end = end_ + n;
            dw_stl::uninitialized_fill_n(end_, n, value);
            end_ = new_end;
        }
        else 
        {
            fill_insert(position, n, value);
        }
    }

    // 删除position处的元素, iterator重载了++, --等操作
    template <class T>
    typename deque<T>::iterator
    deque<T>::erase(iterator position)
    {
        auto next = position;
        ++next;
        const size_type elems_before = position - begin_;
        // 如果position之前的元素较少，则从前面开始删除
        if (elems_before < (size() / 2))
        {
            dw_stl::copy(begin_, position, next);
            pop_front();
        }
        else 
        {
            dw_stl::copy(next, end_, position);
            pop_back();
        }
        return begin_ + elems_before;
    }

    // 删除[first, last)上的元素
    template <class T>
    typename deque<T>::iterator
    deque<T>::erase(iterator first, iterator last)
    {
        if (first == begin_ && last == end_)
        {
            clear();
            return end_;
        }
        else 
        {
            const size_type len = last - first;
            const size_type elems_before = first - begin_;
            if (elems_before < ((size() - len) / 2))
            {
                dw_stl::copy_backward(begin_, first, last);
                auto new_begin = begin_ + len;
                data_allocator::destroy(begin_.cur, new_begin.cur);
                begin_ = new_begin;
            }
            else 
            {
                dw_stl::copy(last, end_, first);
                auto new_end = end_ - len;
                data_allocator::destroy(new_end.cur, end_.cur);
                end_ = new_end;
            }
            return begin_ + elems_before;
        }
    }
}

#endif