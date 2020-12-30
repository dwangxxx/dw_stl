#ifndef DW_STL_UTIL_H_
#define DW_STL_UTIL_H_

// 这个文件包含一些通用工具
// 包括 move, forward, swap 等函数，以及 pair 等 

#include <cstddef>
#include "type_traits.h"

// C++中函数形参设置为右值引用则为万能引用，可以接收左值也可以接收右值引用，会根据推导机制进行推导
// C++万能引用和完美转发是因为右值的出现设置的
/*
引用折叠机制
*/

namespace dw_stl
{
    // move函数和forward函数主要应用模板函数的右值引用推导以及右值引用折叠机制
    // move函数, noexcept表示不会抛出异常
    // 将一个左值引用转化为右值引用
    // remove_reference<T>::type表示取出类型T的去掉引用之后的原始类型
    template <class T>
    typename std::remove_reference<T>::type&& move(T&& arg) noexcept
    {
        // 强制转换，不管左值右值均转换为左值
        return static_cast<typename std::remove_reference<T>::type&&>(arg);
    }

    /******************************************************/

    // forward函数, 完美转发, 保持传入函数的实参的类型不变
    // 保持实参的类型
    // 当传入的是左值引用时 int& int
    template <class T>
    T&& forward(typename std::remove_reference<T>::type& arg) noexcept
    {
        return static_cast<T&&>(arg);
    }

    // 传入的是右值引用时 int&&
    template <class T>
    T&& forward(typename std::remove_reference<T>::type&& arg) noexcept
    {
        return static_cast<T&&>(arg);
    }

    /**********************************************************/

    // swap函数, 交换两个数, 使用move语义实现, 节省了三次拷贝构造函数的调用
    // 使用了移动构造
    // 全局的swap函数
    // 现在的swap函数都是需要参数具有右值赋值函数和移动拷贝构造函数
    template <class Tp>
    void swap(Tp& lhs, Tp& rhs)
    {
        auto tmp(dw_stl::move(lhs));
        lhs = dw_stl::move(rhs);
        rhs = dw_stl::move(tmp);
    }

    template <class ForwardIter1, class ForwardIter2>
    ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2)
    {
        for (; first1 != last1; ++first1, (void) ++ first2)
            // 交换迭代器指向的内容
            dw_stl::swap(*first1, *first2);
        return first2;
    }

    /*********************************************************************************/
    // pair实现

    /*
    结构体模板 pair
    其中两个模板参数Ty1和Ty2分别表示两个数据的类型
    用 first 和 second分别取出第一个和第二个数据
    */
    template <class Ty1, class Ty2>
    struct pair
    {
        typedef Ty1 firstType;
        typedef Ty2 secondType;

        firstType first;    // 第一个数据
        secondType second;  // 第二个数据

        // 默认构造函数, 首先判断给定的两种类型是否有默认拷贝构造函数
        // std::enable_if判断是否为真，只有威震的时候使用type才不会出错
        template <class Other1 = Ty1, class Other2 = Ty2,
            typename = typename std::enable_if<
            std::is_default_constructible<Other1>::value &&
            std::is_default_constructible<Other2>::value, void>::type>
        constexpr pair() : first(), second()
        {}

        // 隐式构造函数，允许进行隐式转换的函数(左值引用)
        template <class U1 = Ty1, class U2 = Ty2,
            typename std::enable_if<
            std::is_copy_constructible<U1>::value &&
            std::is_copy_constructible<U2>::value &&
            std::is_convertible<const U1&, Ty1>::value &&
            std::is_convertible<const U2&, Ty2>::value, int>::type = 0>
        constexpr pair(const Ty1& a, const Ty2& b) : first(a), second(b)
        {}

        // 显式构造函数，不允许进行隐式转换，只能进行显式转换(左值引用)
        template <class U1 = Ty1, class U2 = Ty2,
            typename std::enable_if<
            std::is_copy_constructible<U1>::value &&
            std::is_copy_constructible<U2>::value &&
            (!std::is_convertible<const U1&, Ty1>::value ||
             !std::is_convertible<const U2&, Ty2>::value), int>::type = 0>
        explicit constexpr pair(const Ty1& a, const Ty2& b) : first(a), second(b)
        {}

        // 表示禁用拷贝构造函数
        pair(const pair& rhs) = default;
        pair(pair&& rhs) = default;

        /********************************************/
        // 为其他类型定义隐式构造函数 (右值引用)
        template <class Other1, class Other2,
            typename std::enable_if<
            std::is_constructible<Ty1, Other1>::value &&
            std::is_constructible<Ty1, Other2>::value &&
            std::is_convertible<Other1&&, Ty1>::value &&
            std::is_convertible<Other2&&, Ty2>::value, int>::type = 0>
        constexpr pair(Other1&& a, Other2&& b) 
        : first(dw_stl::forward<Other1>(a)), second(dw_stl::forward<Other2>(b))
        {}

        // 为其他类型定义显式构造函数(右值引用)
        // constexpr为常量表达式关键字
        template <class Other1, class Other2,
            typename std::enable_if<
            std::is_constructible<Ty1, Other1>::value &&
            std::is_constructible<Ty2, Other2>::value &&
            (!std::is_convertible<Other1, Ty1>::value ||
             !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
        explicit constexpr pair(Other1&& a, Other2&& b)
        : first(dw_stl::forward<Other1>(a)), second(dw_stl::forward<Other2>(b))
        {}
        /*************************************************/

        // 实现拷贝构造函数的功能
        // 为其他pair类型定义隐式构造函数(Other与Ty之间可以相互转换)(左值引用)
        template <class Other1, class Other2,
            typename std::enable_if<
            std::is_constructible<Ty1, const Other1&>::value &&
            std::is_constructible<Ty2, const Other2&>::value &&
            std::is_convertible<const Other1&, Ty1>::value &&
            std::is_convertible<const Other2&, Ty2>::value, int>::type = 0>
        constexpr pair(const pair<Other1, Other2>& other)
        : first(other.first), second(other.second)
        {}

        // 为其他pair类型定义显式构造函数(Other与Ty之间不能相互转换)(左值引用)
        template <class Other1, class Other2,
            typename std::enable_if<
            std::is_constructible<Ty1, const Other1&>::value &&
            std::is_constructible<Ty2, const Other2&>::value &&
            (!std::is_convertible<const Other1&, Ty1>::value ||
             !std::is_convertible<const Other2&, Ty2>::value), int>::type = 0>
        explicit constexpr pair(const pair<Other1, Other2>&& other)
        : first(other.first), second(other.second)
        {}

        // 为其他pair类型实现隐式构造函数(Other与Ty之间可以相互转换)(右值引用)
        template <class Other1, class Other2,
            typename std::enable_if<
            std::is_constructible<Ty1, Other1>::value &&
            std::is_constructible<Ty2, Other2>::value &&
            std::is_convertible<Other1, Ty1>::value &&
            std::is_convertible<Other2, Ty2>::value, int>::type = 0>
        constexpr pair(pair<Other1, Other2>&& other)
        : first(dw_stl::forward<Other1>(other.first)), second(dw_stl::forward<Other2>(other.second))
        {}

        // 为其他pair类型实现显式构造函数(Other与Ty之间可以相互转换)(右值引用)
        template <class Other1, class Other2,
            typename std::enable_if<
            std::is_constructible<Ty1, Other1>::value &&
            std::is_constructible<Ty2, Other2>::value &&
            (!std::is_convertible<Other1, Ty1>::value ||
             !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
        constexpr pair(pair<Other1, Other2>&& other)
        : first(dw_stl::forward<Other1>(other.first)), second(dw_stl::forward<Other2>(other.second))
        {}

        /***********************************************/
        // 为当前pair(即类型相同)
        // 拷贝赋值构造函数
        pair& operator=(const pair& rhs)
        {
            if (this != &rhs)
            {
                first = rhs.first;
                second = rhs.second;
            }

            return *this;
        }

        // 移动复制构造函数
        pair& operator=(pair&& rhs)
        {
            // 避免了内部参数的拷贝，对参数进行移动
            if (this != &rhs)
            {
                first = dw_stl::move(rhs.first);
                second = dw_stl::move(rhs.second);
            }
            return *this;
        }
        /****************************************************/

        /****************************************************/
        // 其他类型pair的复制构造函数(即pair中的成员类型和当前pair不同)
        // 拷贝赋值构造函数
        template <class Other1, class Other2>
        pair& operator=(const pair<Other1, Other2>& other)
        {
            first = other.first;
            second = other.second;
            return *this;
        }

        // 移动赋值构造函数
        template <class Other1, class Other2>
        pair& operator=(const pair<Other1, Other2>&& other)
        {
            // 完美转发
            first = dw_stl::forward<Other1>(other.first);
            second = dw_stl::forward<Other2>(other.second);
            return *this;
        }

        ~pair() = default;

        // pair swap函数
        // pair表示只能和同样的类型匹配, 在类里面直接返回类名表示和当前类的类型一样
        void swap(pair& other)
        {
            if (this != &other)
            {
                dw_stl::swap(first, other.first);
                dw_stl::swap(second, other.second);
            }
        }

        /**************************************************************/
        // 重载比较操作符
        bool operator==(const pair& rhs)
        {
            return first == rhs.first && second == rhs.second;
        }

        bool operator<(const pair& rhs)
        {
            return first < rhs.first && second < rhs.second;
        }

        bool operator!=(const pair& rhs)
        {
            return !(*this == rhs);
        }

        bool operator>(const pair& rhs)
        {
            return *this < rhs;
        }

        bool operator<=(const pair& rhs)
        {
            return !(*this > rhs);
        }

        bool operator>=(const pair& rhs)
        {
            return !(*this < rhs);
        }

        /**************************************************************/

    };

    // 重载swap函数
    template <class Ty1, class Ty2>
    void swap(pair<Ty1, Ty2>& lhs, pair<Ty1, Ty2>& rhs)
    {
        lhs.swap(rhs);
    }

    // 全局make_pair函数，让两个数据成为一个pair
    template <class Ty1, class Ty2>
    pair<Ty1, Ty2> make_pair(Ty1&& first, Ty2&& second)
    {
        return pair<Ty1, Ty2>(dw_stl::forward<Ty1>(first), dw_stl::forward<Ty2>(second));
    }
}

#endif 