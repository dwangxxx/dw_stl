#ifndef MYSTL_ITERATOR_H_
#define MYSTL_ITERATOR_H_

// 这个函数用于迭代器设计，包含了一些模板结构体与全局函数
// 包含iterator类，其余自行实现的迭代器都最好继承这个iterator模板
// 包含iterator_traits类，用于进行迭代器型别的萃取

#include <cstddef>
#include "type_traits.h"

namespace dw_stl
{
    // 首先定义五种迭代器类型，只是用作标记，因此里面没有内容，不占空间
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag : public input_iterator_tag {};
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};

    // iterator类模板，其他自行实现的迭代器需要继承自这个类
    // 里面一共有五个型别，用于迭代器型别的萃取
    // 正向迭代器模板
    // 任何实现的迭代器都需要重载操作符，比如++, --, ==这些操作符
    template <class Category, class T, class Distance = ptrdiff_t, 
            class Pointer = T*, class Reference = T&>
    struct iterator
    {
        // 一共有五种迭代器型别
        // 迭代器类别，算法底层萃取出类别之后选取最适合的算法进行操作
        typedef Category            iterator_category;
        // 迭代器所指元素的型别
        typedef T                   value_type;
        // 迭代器所指元素的指针
        typedef Pointer             Pointer;
        // 迭代器所指元素的引用
        typedef Reference           Reference;
        // 迭代器之间的距离
        typedef Distance            difference_type;
    };

    // 判断迭代器是否有迭代器类型信息, 用于后面的iterator_traits定义
    template <class T>
    struct has_iterator_cat
    {
    private:
        struct two {char a; char b};
        template <class U> static two test(...);
        template <class U> static char test(typename U::iterator_category* = 0);
    public:
        // 如果返回char类型，则表示含有iterator_category信息
        // 应用了模板匹配原理
        static const bool value = sizeof(test<T>(0)) == sizeof(char);    
    };

    // 模板结构体, iterator_traits底层实现类
    template <class Iterator, bool>
    struct iterator_traits_impl {};

    // 模板偏特化, 如果为true，则匹配
    template <class Iterator>
    struct iterator_traits_impl<Iterator, true>
    {
        typedef typename Iterator::iterator_category    iterator_category;
        typedef typename Iterator::value_type           value_type;
        typedef typename Iterator::pointer              pointer;
        typedef typename Iterator::reference            reference;
        typedef typename Iterator::difference_type      diffenence_type;
    };
    
    // 结构体模板，iterator_traits_helper
    template <class Iterator, bool>
    struct iterator_traits_helper {};
    
    // iterator_traits_helper偏特化, bool为true则匹配
    // 继承自iterator_traits_impl
    template <class Iterator>
    struct iterator_traits_helper<Iterator, true>
        : publib iterator_traits_impl<Iterator,  
            // 判断当前category能否转换为input_iterator或者output_category
            // 如果不能转换，则继承的iterator_traits_impl类为空
            std::is_convertible<typename Iterator::iterator_category, input_iterator_tag>::value ||
            std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::vale>
    {};

    /*定义迭代器萃取器*/
    /************************/
    // 继承自iterator_traits_helper类
    template <class Iterator>
    struct iterator_traits
        : public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

    // 针对原生指针的特化版本，因为不是所有的迭代器都是类，还有原生指针的版本
    template <class T>
    struct iterator_traits<T*>
    {
        // 原生指针的迭代器类型为随机存取
        typedef random_access_iterator_tag          iterator_category;
        typedef T                                   value_type;
        typedef T*                                  pointer;
        typedef T&                                  reference;
        typedef ptrdiff_t                           difference_type;
    };

    template <class T>
    struct iterator_traits<const T*>
    {
        // 原生指针的迭代器类型为随机存取
        typedef random_access_iterator_tag          iterator_category;
        typedef T                                   value_type;
        typedef const T*                                  pointer;
        typedef const T&                                  reference;
        typedef ptrdiff_t                           difference_type;
    };


    // 定义类萃取是否为某种迭代器
    // 如果没有category型别，则是false
    template <class T, class U, bool = has_iterator_cat<iterator_traits<T>>::value>

    // 继承自_bool_constant，里面有一个成员为bool类型
    struct has_iterator_cat_of
        : public _bool_constant<std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value>
    {};

    // 判断是否为某种迭代器
    // 如果迭代器没有category型别，则直接返回，不是迭代器
    template <class T, class U>
    struct has_iterator_cat_of<T, U, false> : public _false_type {};

    // 判断是否为input_category, 继承自has_iterator_cat_of, 里面有一个bool型成员
    template <class Iter>
    struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_tag> {};

    // 判断是否为output_category
    template <class Iter>
    struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};
    
    // 是否为forward_category
    template <class Iter>
    struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

    // 是否为bidirectional_iterator
    template <class Iter>
    struct is_bidirectional_iterator : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

    // 是否为random access iterator
    template <class Iter>
    struct is_random_access_iterator : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

    // 判断是否为迭代器
    template <class Iter>
    struct is_iterator:
        public _bool_constant<is_input_iterator<Iterator>::value || is_output_iterator<Iterator>::value>
    {};

    // iterator_cateogry函数，萃取某个迭代器的iterator_category类型
    template <class Iterator>
    typename iterator_traits<Iterator>::iterator_catogory
    iterator_category(const Iterator&)
    {
        typedef typename iterator_traits<Iterator>::iterator_category Category;
        return Category();
        // 返回对象
    }

    // 萃取某个迭代器的distance_type类型
    template <class Iterator>
    typename iterator_traits<Iterator>::difference_type*
    distance_type(const Iterator&)
    {
        return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
    }

    // 萃取某个迭代器的value_type
    template <class Iterator>
    typename iterator_traits<Iterator>::value_type*
    value_type(const Iterator&)
    {
        return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
    }

    /**********************************************/
    // 计算迭代器之间的距离distance，不同的迭代器有不同的版本
    
    // input_iterator_tag版本
    template <class InputIterator>
    typename iterator_traits<InputIterator>::difference_type
    distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag)
    {
        // 先萃取出distance type
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last)
        {
            ++first;
            ++n;
        }

        return n;
    }

    // random_access_iterator_tag版本
    // random_iterator可以进行加减操作
    template <class RandomIter>
    typename iterator_traits<RandomIter>::difference_type
    distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag)
    {
        return last - first;
    }

    // 定义distance函数，由distance_dispatch派发
    // InputIterator定义为最底层的
    template <class InputIterator>
    typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last)
    {
        // 首选使用iterator_category()函数萃取出first的类型
        return distance_dispatch(first, last, iterator_category(first));
    }
    /***************************************************/

    /****************************************************/
    // advance函数，让迭代器前进n个距离
    // 不同类型迭代器都有自己的实现版本
    
    // input_iterator_tag版本
    template <class InputIterator, class Distance>
    void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag)
    {
        while (n--)
            ++i;
    }

    // bidirection_iterator_tag版本, 可以往前可以往后
    template <class BidirectionalIterator, class Distance>
    void advance_dispatch(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
    {
        if (n >= 0)
            while (n--) ++i;
        else
            while (++n) --i;      
    }

    // random_access_iterator_tag版本
    template <class RandomIter, class Distance>
    void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag)
    {
        i += n;
    }

    // advance函数，将其分派给advanc_dispatch
    template <class InputIterator, class Distance>
    void advance(InputIterator& i, Distance n)
    {
        advance_dispatch(i, n, iterator_category(i));
    }
    /***************************************************************/

    /****************************************************************/
    // 反向迭代器模板类, 迭代器都需要定义自己的型别, 因此反向迭代器也需要定义五种对应的型别
    // 传入的是正向迭代器对象
    template <class Iterator>
    class reverse_iterator
    {
    private:
        Iterator current;   // 记录当前对应的正向迭代器
    
    // 数据成员
    public:
        // 反向迭代器的五种相应型别, 与对应的正向迭代器相同
        typedef typename iterator_traits<Iterator>::iterator_category   iterator_category;
        typedef typename iterator_traits<Iterator>::value_type          value_type;
        typedef typename iterator_traits<Iterator>::difference_type     difference_type;
        typedef typename iterator_traits<Iterator>::pointer             pointer;
        typedef typename iterator_traits<Iterator>::reference           reference;

        // 正向迭代器类型
        typedef Iterator                        iterator_type;
        // 反向迭代器类型
        typedef reverse_iterator<Iterator>      self;

    // 构造函数
    public:
        // 构造函数
        reverse_iterator()  {}
        explicit reverse_iterator(iterator_type i) : current(i) {}
        reverse_iterator(const self& rhs) : current(rhs.current) {}

    // 成员函数
    public:
        // 取出对应的正向迭代器
        iterator_type base() const
        {
            return current;
        }

        // 重载*操作符
        reference operator*() const
        {
            // 实际对应的是正向迭代器的前一个位置
            auto tmp = current;
            return *--tmp;
        }
        
        // 重载->操作符
        pointer operator->() const
        {
            return &(operator*());
        }

        // 前进变为后退
        // ++，先加后返回++i
        self& operator++()
        {
            --current;
            return *this;
        }
        // 执行完再加i++
        self operator++(int)
        {
            self tmp = *this;
            --current;
            return tmp;
        }

        // 后退变为前进
        // --i
        self& operator--()
        {
            ++current;
            return *this;
        }
        // i--
        self operator--()
        {
            self tmp = current;
            ++current;
            return tmp;
        }

        // 重载运算符+=
        self& operator+=(difference_type n)
        {
            current -= n;
            return *this;
        }
        // 重载运算符+
        self operator+(difference_type n) const
        {
            return self(current - n);
        }

        // 重载运算符-=
        self& operator-=(difference_type n)
        {
            current += n;
            return *this;
        }
        // 重载运算符-
        self operator-(difference_type n)
        {
            return self(current + n);
        }
        
        // 重载[]操作符，即访问*this + n的迭代器
        reference operator[](difference_type n) const
        {
            return *(*this + n);
        }

        // 重载运算符-，将两个反向迭代器进行相减
        difference_type operator-(const self& rhs)
        {
            return rhs.base() - current;
        }

        // 重载等于运算符
        bool operator==(const self& rhs)
        {
            return current == rhs.base();
        }

        // 重载小于运算符
        bool operator<(const self& rhs)
        {
            return current < rhs.base();
        }
        
        // 重载!=运算符
        bool operator!=(const self& rhs)
        {
            return current != rhs.base();
        }

        // 重载大于运算符
        bool operator>(const self& rhs)
        {
            return current > rhs.base();
        }
        
        // 重载<=运算符
        bool operator<=(const self& rhs)
        {
            return !(lhs > rhs);
        }

        // 重载>=运算符
        bool operator>=(const self& rhs)
        {
            return !(lhs < rhs);
        }
    };
    /****************************************************************/

}

#endif