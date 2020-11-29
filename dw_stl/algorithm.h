#ifndef MYSTL_ALGOBASE_H_
#define MYSTL_ALGOBASE_H_

/*
这个头文件包含了stl的基本算法
本文件中实现的算法均是有赋值操作符(=)的
在uninitialized中的函数(uninitialized_fill, uninitialized_copy等)如果传入的参数是有赋值运算符的
则会直接调用当前文件中的copy、fill等函数，否则会使用拷贝构造函数进行拷贝构造
*/

#include <cstring>
#include "iterator.h"
#include "util.h"

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

    /*******************************************************************/
    // max函数
    /*******************************************************************/
    template <class T>
    const T& max(const T& lhs, const T& rhs)
    {
        return lhs < rhs ? rhs : lhs;
    }
    
    // 重载比较函数
    template <class T, class Compare>
    const T& max(const T& lhs, const T& rhs, Compare comp)
    {
        // 比较函数返回true说明lhs < rhs
        return comp(lhs, rhs) ? rhs : lhs;
    }

    /*******************************************************************/
    // min函数
    /*******************************************************************/
    template <class T>
    const T& min(const T& lhs, const T& rhs)
    {
        return rhs < lhs ? rhs : lhs;
    }

    // 重载比较函数版本
    template <class T, class Compare>
    const T& min(const T& lhs, const T& rhs, Compare comp)
    {
        // 比较函数返回true说明rhs < lhs
        return comp(rhs, lhs) ? rhs : lhs;
    }

    /*******************************************************************/
    // copy函数
    // 将[first, last)区间的元素拷贝到以result开头空间内
    /*******************************************************************/

    // unchecked_copy_cat的input_iterator_tag版本
    template <class InputIter, class OutputIter>
    OutputIter copy_cat_dispatch(InputIter first, InputIter last, OutputIter result, dw_stl::input_iterator_tag)
    {
        for (; first != last; ++first, ++result)
            *result = *first;
        return result;
    }

    // unchecked_copy_cat的random_access_iterator_tag版本
    template <class RandomIter, class OuputIter>
    OuputIter copy_cat_dispatch(RandomIter first, RandomIter last, OuputIter result, dw_stl::random_access_iterator_tag)
    {
        for (auto n = last - first; n > 0; --n, ++first, ++result)
            *result = *first;
        return result
    }

    // 非内置类型
    template <class InputIter, class OutputIter>
    OutputIter copy_dispatch(InputIter first, InputIter last, OutputIter result)
    {
        return copy_cat_dispatch(first, last, result, iterator_category(first));
    }

    // 内置类型
    template <class Tp, class Up>
    typename std::enable_if<
        std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
        std::is_trivially_copy_assignable<Up>::value,
        Up*>::type
    copy_dispatch(Tp* first, Tp* last, Up* result)
    {
        const auto n = static_cast<size_t>(last - first);
        if (n != 0)
            // 直接使用标准库的内置memmove函数对内置类型进行拷贝
            std::memmove(result, first, n * sizeof(Up));
        return result + n;
    }

    // copy接口
    template <class InputIter, class OutputIter>
    OutputIter copy(InputIter first, InputIter last, OutputIter result)
    {
        // 即使两种迭代器类型不一样，但是迭代器指向的内容可能是一样的
        // 比如可以将map的内容复制到vector<pair<>>类型的数据中去
        return copy_dispatch(first, last, result);
    }

    /*******************************************************************/
    // copy_backward函数
    // 将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
    /*******************************************************************/

    // unchecked_copy_backward_cat的bidirectional_iterator_tag版本(命名规则为其能提供的最小功能)
    template <class BidirecitonalIter1, class BidirectionalIter2>
    BidirectionalIter2 copy_backward_cat_dispatch(BidirecitonalIter1 first, BidirecitonalIter1 last,
                                                   BidirectionalIter2 result, dw_stl::bidirectional_iterator_tag)
    {
        while (first != last)
            *--result = *--last;
        return result;
    }

    // unchecked_copy_backward_cat的random_access_iterator_tag版本(命名规则为其能提供的最小功能)
    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 copy_backward_cat_dispatch(BidirectionalIter1 first, BidirectionalIter1 last,
                                                   BidirectionalIter2 result, dw_stl::random_access_iterator_tag)
    {
        for (auto n = last - first; n > 0; --n)
            *--result = *--last;
        return result;
    }

    // 非内置类型, 选择不同迭代器
    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 copy_backward_dispatch(BidirectionalIter1 first, BidirectionalIter1 last,
                                               BidirectionalIter2 result)
    {
        return copy_backward_cat_dispatch(first, last, result, iterator_category(first));
    }

    // 内置类型
    template <class Tp, class Up>
    typename std::enable_if<
    std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
    std::is_trivially_copy_assignable<Up>::value, Up*>::type
    copy_backward_dispatch(Tp* first, Tp* last, Up* result)
    {
        const auto n = static_cast<size_t>(last - first);
        if (n != 0)
        {
            result -= n;
            std::memmove(result, first, n * sizeof(Up));
        }
        return result;
    }
    
    // copy_backward接口函数
    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 copy_backward(BidirectionalIter1 first, BidirectionalIter1 last,
                                     BidirectionalIter2 result)
    {
        return copy_backward_dispatch(first, last, result);
    }

    /*******************************************************************/
    // copy_n函数
    // 将 [first, first + n)区间内的元素拷贝到 [result, result + n)上
    // 返回一个pair分别指向拷贝的尾部(first + n, result + n)
    /*******************************************************************/
    // unchecked_copy_n的input_iterator_tag版本
    template <class InputIter, class Size, class OutputIter>
    dw_stl::pair<InputIter, OutputIter> copy_n_dispatch(InputIter first, Size n, OutputIter result, dw_stl::input_iterator_tag)
    {
        for (; n > 0; --n, ++result)
            *result = *first;
        
        return dw_stl::pair<InputIter, OutputIter>(first, result);
    }

    // unchecked_copy_n的random_access_iterator_tag版本
    template <class RandomIter, class Size, class OutputIter>
    dw_stl::pair<RandomIter, OutputIter> copy_n_dispatch(RandomIter first, Size n, OutputIter result, dw_stl::random_access_iterator_tag)
    {
        auto last = first + n;
        // 可以直接调用copy函数
        return dw_stl::pair<RandomIter, OutputIter>(last, dw_stl::copy(first, last, result));
    }

    // copy_n接口函数
    template <class InputIter, class Size, class OutputIter>
    dw_stl::pair<InputIter, OutputIter> copy_n(InputIter first, Size n, OutputIter result)
    {
        return copy_n_dispatch(first, n, result, iterator_category(first));
    }

    /*******************************************************************/
    // move函数
    // 将 [first, last)区间内的元素移动到 [result, result + (last - first))上
    /*******************************************************************/
    // unchecked_move_cat的input_iterator_tag版本
    template <class InputIter, class OutputIter>
    OutputIter move_cat_dispatch(InputIter first, OutputIter last, OutputIter result,
                                  dw_stl::input_iterator_tag)
    {
        for (; first != last; ++first, ++result)
            *result = dw_stl::move(*first);
        return result;
    }

    // unchecked_move_cat的random_access_iterator_tag版本
    template <class RandomIter, class OutputIter>
    OutputIter move_cat_dispatch(RandomIter first, RandomIter last, OutputIter result,
                                  dw_stl::random_access_iterator_tag)
    {
        for (auto n = last - first; n > 0; --n, ++first, ++result)
            *result = dw_stl::move(*first);
        return result;
    }

    // 非内置类型
    template <class InputIter, class OutputIter>
    OutputIter move_dispatch(InputIter first, InputIter last, OutputIter result)
    {
        return move_cat_dispatch(first, last, result, iterator_category(first));
    }

    // 内置类型
    // 检查是否有move操作运算符
    template <class Tp, class Up>
    typename std::enable_if<
        std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
        std::is_trivially_move_assignable<Up>::value, Up*>::type
    move_dispatch(Tp* first, Tp* last, Up* result)
    {
        const size_t n = static_cast<size_t>(last - first);
        if (n != 0)
            std::memmove(result, first, n * sizeof(Up));
        return result + n;
    }

    // move函数接口
    template <class InputIter, class OutputIter>
    OutputIter move(InputIter first, InputIter last, OutputIter result)
    {
        return move_dispatch(first, last, result);
    }

    /*******************************************************************/
    // move_backward函数
    // 将 [first, last)区间内的元素移动到 [result - (last - first), result)上
    /*******************************************************************/
    // 
}

#endif