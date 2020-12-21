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
        
        // value_pointer的指针
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
        template <class ForwardIter>
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
        // 直接清空
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

    // 清空deque
    template <class T>
    void deque<T>::clear()
    {
        // 只是清空，但是内存还会保留
        // clear会保留头部的缓冲区
        for (map_pointer cur = begin_.node + 1; cur < end_.node; ++cur)
        {
            // *cur表示map指向的缓冲区的地址
            data_allocator::destroy(*cur, *cur + buffer_size);
        }
        // 如果有两个以上的缓冲区
        if (begin_.node != end_.node)
        {
            dw_stl::destroy(begin_.cur, begin_.last);
            dw_stl::destroy(end_.first, end_.cur);
        }
        else 
        {
            dw_stl::destroy(begin_.cur, end_.cur);
        }
        shrink_to_fit();
        end_ = begin_;
    }

    // 交换两个deque
    template <class T>
    void deque<T>::swap(deque& rhs) noexcept
    {
        // 首先判断是否是自交换
        if (this != &rhs)
        {
            mystl::swap(begin_, rhs.begin_);
            mystl::swap(end_, rhs.end_);
            mystl::swap(map_, rhs.map_);
            mystl::swap(map_size_, rhs.map_size_);
        }
    }

    /*************************************************************************/
    // 辅助函数

    // 创建map中控器
    template <class T>
    typename deque<T>::map_pointer
    deque<T>::create_map(size_type size)
    {
        map_pointer mp = nullptr;
        mp = map_allocator::allocate(size);
        // 初始化
        for (size_type i = 0; i < size; ++i)
            *(mp + 1) = nullptr
        return mp;
    }

    // create_buffer函数
    template <class T>
    void deque<T>::create_buffer(map_pointer start, map_pointer finish)
    {
        map_pointer cur;
        // 分配buffer
        try 
        {
            for (cur = start; cur < finish; ++cur)
                *cur = data_allocator::allocate(buffer_size);
        }
        // 如果分配失败，则将已经分配的回收，保证异常安全
        catch (...)
        {
            while (cur != start)
            {
                --cur;
                // 将状态恢复到发生异常之前的状态
                data_allocator::deallocate(*cur, buffer_size);
                *cur = nullptr;
            }
            throw;
        }
    }

    // destroy_buffer函数
    template <class T>
    void deque<T>::destroy_buffer(map_pointer start, map_pointer finish)
    {
        for (map_pointer n = start; n <= finish; ++n)
        {
            data_allocator::deallocate(*n, buffer_size);
            *n = nullptr;
        }
    }

    // map_init初始化函数
    template <class T>
    void deque<T>::map_init(size_type n)
    {
        // 需要分配的缓冲区的个数
        const size_type n_node = n / buffer_size + 1;
        map_size_ = dw_stl::max(static_cast<size_type>(DEQUE_MAP_INIT_SIZE), n_node + 2);
        try 
        {
            map_ = create_map(map_size_);
        }
        catch (...)
        {
            map_ = nullptr;
            map_size_ = 0;
            throw;
        }
        // 让start和finish都指向map_最中央的区域，方便向头尾扩充
        map_pointer start = map_ + (map_size_ - n_node)/ 2;
        map_pointer finish = start + n_node - 1;
        try
        {
            create_buffer(start, finish);
        }
        catch (...)
        {
            map_allocator::deallocate(map_, map_size_);
            map_ = nullptr;
            map_size_ = 0;
            throw;
        }
        // begin_和end_之间相差n个元素的距离
        begin_.set_node(start);
        end_.set_node(finish);
        begin_.cur = begin_.first;
        end_.cur = end_.first + (n % buffer_size);
    }

    // fill_init函数
    template <class T>
    void deque<T>::fill_init(size_type n, const value_type &value)
    {
        map_init(n);
        if (n != 0)
        {
            for (auto cur = begin_.node; cur < end_.node; ++cur)
            {
                dw_stl::uninitialized_fill(*cur, *cur + buffer_size, value);
            }
            dw_stl::uninitialized_fill(end_.first, end_.cur, value);
        }
    }
    
    // copy_init函数, 初始化，即内存还没有初始化
    template <class T>
    template <class InputIter>
    void deque<T>::copy_init(InputIter first, InputIter last, input_iterator_tag)
    {
        const size_type n = dw_stl::distance(first, last);
        // 首先分配内存来保存map
        map_init(n);
        for (; first != last; ++first)
            emplace_back(*first);
    }

    template <class T>
    template <class ForwardIter>
    void deque<T>::copy_init(ForwardIter first, ForwardIter last, forward_iterator_tag)
    {
        const size_type n = dw_stl::distance(first, last);
        // 首先进行内存的初始化
        map_init(n);
        for (auto cur = begin_.node; cur < end_.node; ++cur)
        {
            auto next = first;
            dw_stl::advance(next, buffer_size);
            // 因为地址是分段连续的，因此只能进行分段连续的复制
            // memmove需要地址连续
            dw_stl::uninitialized_copy(first, next, *cur);
            first = next;
        }
        dw_stl::uninitialized_copy(first, last, end_.first);
    }

    // fill_assign函数
    template <class T>
    void deque<T>::fill_assign(size_type n, const value_type &value)
    {
        if (n > size())
        {
            dw_stl::fill(begin(), end(), value);
            insert(end(), n - size(), value);
        }
        else 
        {
            erase(begin() + n, end());
            dw_stl::fill(begin(), end(), value);
        }
    }

    // copy_assign函数, 内存已经初始化过了
    // 为InputIter和ForwardIter实现最优的选择
    template <class T>
    template <class InputIter>
    void deque<T>::copy_assign(InputIter first, InputIter last, input_iterator_tag)
    {
        auto first1 = begin();
        auto last1 = end();
        for (; first != last && first1 != last1; ++first, ++first1)
        {
            *first1 = *first;
        }
        // 如果原来的没有到终点，则将原来多余的元素erase掉
        if (first1 != last1)
        {
            erase(first1, last1);
        }
        else 
        {
            insert_dispatch(end_, first, last, input_iterator_tag{});
        }
    }

    template <class T>
    template <class ForwardIter>
    void deque<T>::copy_assign(ForwardIter first, ForwardIter last, forward_iterator_tag)
    {
        const size_type len1 = size();
        const size_type len2 = dw_stl::distance(first, last);
        // 如果原来的size小于要copy的size
        if (len1 < len2)
        {
            auto next = first;
            dw_stl::advance(next, len1);
            dw_stl::copy(first, next, begin_);
            insert_dispatch(end_, next, last, forward_iterator_tag{});
        }
        else 
        {
            erase(dw_stl::copy(first, last, beign_), end_);
        }
    }

    // insert_aux函数
    template <class T>
    template <class... Args>
    typename deque<T>::iterator deque<T>::insert_aux(iterator position, Args&& ...args)
    {
        const size_type elems_before = position - begin_;
        value_type value_copy = value_type(dw_stl::forward<Args>(args)...);
        // 如果前半段元素较少，在前半段插入
        if (elems_before < (size() / 2))
        {
            emplace_front(front());
            auto front1 = begin_;
            ++front1;
            auto front2 = front1;
            ++front2;
            position = begin_ + elems_before;
            auto pos = position;
            ++pos;
            dw_stl::copy(front2, pos, front1);
        }
        // 在后半段插入
        else
        {
            emplace_back(back());
            auto back1 = end_;
            --back1;
            auto back2 = back1;
            --back2;
            position = begin_ + elems_before;
            dw_stl::copy_backward(position, back2, back1);
        }
        *position = dw_stl::move(value_copy);
        return position;
    }

    // fill_insert函数
    template <class T>
    void deque<T>::fill_insert(iterator position, size_type n, const value_type &value)
    {
        const size_type elems_before = position - begin_;
        const size_type len = size();
        auto value_copy = value;
        // 在前部插入
        if (elems_before < (len / 2))
        {
            // 申请内存
            require_capacity(n, true);
            // 原来的迭代器可能会失效
            auto old_begin = begin_;
            auto new_begin = begin_ - n;
            position = begin_ + elems_before;
            try 
            {
                // 如果需要移动的元素比n大，则有一部分需要使用未初始化，有一部分不用
                if (elems_before >= n)
                {
                    auto begin_n = begin_ + n;
                    dw_stl::uninitialized_copy(begin_, begin_n, new_begin);
                    begin_ = new_begin;
                    dw_stl::copy(begin_n, position, old_begin);
                    dw_stl::fill(position - n, position, value_copy);
                }
                else 
                {
                    dw_stl::uninitialized_fill(dw_stl::uninitialized_copy(begin_, position, new_begin), begin_, value);
                    begin_ = new_begin;
                    dw_stl::fill(old_begin, position, value_copy);
                }
            }
            catch (...)
            {
                if (new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        // 往后移动
        else 
        {
            require_capacity(n, false);
            // 原来的迭代器会失效
            auto old_end = end_;
            auto new_end = end_ + n;
            const size_type elems_after = len - elems_before;
            position = end_ - elems_after;
            try 
            {
                if (elems_after > n)
                {
                    auto end_n = end_ - n;
                    dw_stl::uninitialized_copy(end_n, end_, end_);
                    end_ = new_end;
                    dw_stl::copy_backward(position, end_n, old_end);
                    dw_stl::fill(position, position + n, value_copy);
                }
                else 
                {
                    dw_stl::uninitialized_fill(end_, position + n, value_copy);
                    dw_stl::uninitialized_copy(position, end_, position + n);
                    end_ = new_end;
                    dw_stl::fill(position, old_end, value_copy);
                }
            }
            catch (...)
            {
                if (new_end.node != end_.node)
                    destroy(end_.node + 1, new_end.node);
                throw();
            }
        }
    }

    // copy_insert函数
    template <class T>
    template <class ForwardIter>
    void deque<T>::copy_insert(iterator position, ForwardIter first, ForwardIter last, size_type n)
    {
        const size_type elems_before = position - begin_;
        auto len = size();
        if (elems_before < (len / 2))
        {
            require_capacity(n, true);
            // 原来的迭代器可能会失效
            auto old_begin = begin_;
            auto new_begin = begin_ - n;
            position = begin_ + elems_before;
            try 
            {
                if (elems_before >= n)
                {
                    auto begin_n = begin_ + n;
                    dw_stl::uninitialized_copy(begin_, begin_n, new_begin);
                    begin_ = new_begin;
                    dw_stl::copy(begin_n, position, old_begin);
                    dw_stl::copy(first, last, position - n);
                }
                else
                {
                    auto mid = first;
                    dw_stl::advance(mid, n - elems_before);
                    dw_stl::uninitialized_copy_dispatch(first, mid, dw_stl::uninitialized_copy(begin_, position, new_begin));
                    begin_ = new_begin;
                    dw_stl::copy(mid, last, old_begin);
                }
            }
            catch (...)
            {
                if (new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        else 
        {
            require_capacity(n,false);
            // 原来的迭代器可能会失效
            auto old_end = end_;
            auto new_end = end_ + n;
            const auto elems_after = len - elems_before;
            position = end_ - elems_after;
            try 
            {
                if (elems_after > n)
                {
                    auto end_n = end_ - n;
                    dw_stl::uninitialized_copy(end_n, end_, end_);
                    end_ = new_end;
                    dw_stl::copy_backward(position, end_n, old_end);
                    dw_stl::copy(first, last, position);
                }
                else 
                {
                    auto mid = first;
                    dw_stl::advance(mid, elems_after);
                    dw_stl::uninitialized_copy(position, end_, dw_stl::uninitialized_copy(mid, last, end_));
                    end_ = new_end;
                    dw_stl::copy(first, mid, position);
                }
            }
            catch (...)
            {
                if (new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
    }

    // insert_dispatch函数
    template <class T>
    template <class InputIter>
    void deque<T>::insert_dispatch(iterator position, InputIter first, InputIter last, input_iterator_tag)
    {
        if (last <= first)  return;
        const size_type n = dw_stl::distance(first, last);
        const size_type elems_before = position - begin_;
        if (elems_before < (size() / 2))
        {
            require_capacity(n, true);
        }
        else 
        {
            require_capacity(n, false);
        }
        position = begin_ + elems_before;
        auto cur = --last;
        for (size_type i = 0; i < n; ++i, --cur)
        {
            insert(position, *cur);
        }
    }

    template <class T>
    template <class ForwardIter>
    void deque<T>::insert_dispatch(iterator position, ForwardIter first, ForwardIter last, forward_iterator_tag)
    {
        if (last <= first)  return;
        const size_type n = dw_stl::distance(first, last);
        // 如果是在头部插入
        if (position.cur == begin_.cur)
        {
            require_capacity(n, true);
            auto new_begin = begin_ - n;
            try 
            {
                dw_stl::uninitialized_copy(first, last, new_begin);
                begin_ = new_begin;
            }
            catch (...)
            {
                if (new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        // 如果是在尾部插入
        else if (position.cur == end_.cur)
        {
            require_capacity(n, false);
            auto new_end = end_ + n;
            try 
            {
                dw_stl::uninitialized_copy(first, last, end_);
                end_ = new_end;
            }
            catch(...)
            {
                if (new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
        else 
        {
            // 调用copy_insert函数
            copy_insert(position, first, last, n);
        }
    }

    // require_capacity函数, front参数表示在队列头部或者队列尾部申请内存
    template <class T>
    void deque<T>::require_capacity(size_type n, bool front)
    {
        // 如果剩余元素不够，则在头部申请内存
        if (front && (static_cast<size_type>(begin_.cur - begin_.first) < n))
        {
            const size_type need_buffer = (n - (begin_.cur - begin_.first)) / buffer_size + 1;
            // 同时需要重新申请map和buffer
            if (need_buffer > static_cast<size_type>(begin_.node - map_))
            {
                reallocate_map_at_front(need_buffer);
                return;
            }
            // 只需要申请buffer
            create_buffer(begin_.node - need_buffer, begin_.node - 1);
        }
        // 在尾部申请内存
        else if(!front && (static_cast<size_type>(end_.last - end_.cur - 1) < n))
        {
            // 求出需要申请多少块buffer
            const size_type need_buffer = (n - (end_.last - end_.cur - 1)) / buffer_size;
            if (need_buffer > static_cast<size_type>((map_ + map_size_) - end_.node - 1));
            {
                // 需要重新申请map
                reallocate_map_at_back(need_buffer);
                return;
            }
            create_buffer(end_.node + 1, end_.node + need_buffer);
        }
    }

    // reallocate_map_at_front函数
    // need_buffer是指需要申请的缓冲区的个数
    template <class T>
    void deque<T>::reallocate_map_at_front(size_type need_buffer)
    {
        // 申请新的map
        const size_type new_map_size = dw_stl::max(map_size_ << 1, map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
        map_pointer new_map = create_map(new_map_size);
        const size_type old_buffer = end_.node - begin_.node + 1;
        const size_type new_buffer = old_buffer + need_buffer;

        auto begin = new_map + (new_map_size - new_buffer) / 2;
        auto mid = begin + need_buffer;
        auto end = mid + old_buffer;
        // 将新的map中的指针指向原来的buffer (mid -> end)
        for (auto begin1 = mid, begin2 = begin_.node; begin1 != end; ++begin1, ++begin2)
            *begin1 = *begin2;
        // 开辟新的buffer, (begin -> mid - 1)
        create_buffer(begin, mid - 1);
        
        // 更新数据
        map_allocator::deallocate(map_, map_size_);
        map_ = new_map;
        map_size_ = new_map_size;
        // 设置begin_迭代器
        begin_ = iterator(*mid + (begin_.cur - begin_.first), mid);
        // 设置end_迭代器
        end_ = iterator(*(end - 1) + (end_.cur - end_.first), end - 1);
    }

    // reallocate_map_at_back函数
    template <class T>
    void deque<T>::reallocate_map_at_back(size_type need_buffer)
    {
        const size_type new_map_size = dw_stl::max(map_size_ << 1, map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
        map_pointer new_map = create_map(new_map_size);
        const size_type old_buffer = end_.node - begin_.node + 1;
        const size_type new_buffer = old_buffer + need_buffer;

        auto begin = new_map + ((new_map_size - new_buffer) / 2);
        auto mid = begin + old_buffer;
        auto end = mid + need_buffer;
        // 将新的map指向原来的缓冲区
        for (auto begin1 = begin, begin2 = begin_.node; begin1 != mid; ++begin1, ++begin2)
            *begin1 = *begin2;
        // 开辟新的缓冲区
        create_buffer(mid, end - 1);

        // 更新数据
        map_allocator::deallocate(map_, map_size_);
        map_ = new_map;
        map_size_ = new_map_size;
        begin_ = iterator(*begin + (begin_.cur - begin_.first), begin);
        // iterator(value_pointer v, map_pointer n)
        // 参数为当前指针以及当前的map指针
        end_ = iterator(*(mid - 1) + (end_.cur - end_.first), mid - 1);
    }

    // 重载比较操作符
    template <class T>
    bool operator==(const deque<T>& lhs, const deque<T>& rhs)
    {
        return lhs.size() == rhs.size() && dw_stl::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template <class T>
    bool operator!=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class T>
    bool operator<(const deque<T>& lhs, const deque<T>& rhs)
    {
        return dw_stl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template <class T>
    bool operator>(const deque<T>& lhs, const deque<T>& rhs)
    {
        return rhs < lhs;
    }

    template <class T>
    bool operator<=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class T>
    bool operator>=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(lhs < rhs);
    }

    // 重载全局swap，对
    template <class T>
    void swap(deque<T>& lhs, deque<T>& rhs)
    {
        lhs.swap(rhs);
    }
}

#endif