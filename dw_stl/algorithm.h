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

    // copy_cat_dispatch的input_iterator_tag版本
    template <class InputIter, class OutputIter>
    OutputIter copy_cat_dispatch(InputIter first, InputIter last, OutputIter result, dw_stl::input_iterator_tag)
    {
        for (; first != last; ++first, ++result)
            *result = *first;
        return result;
    }

    // copy_cat_dispatch的random_access_iterator_tag版本
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

    // copy_backward_cat_dispatch的bidirectional_iterator_tag版本(命名规则为其能提供的最小功能)
    template <class BidirecitonalIter1, class BidirectionalIter2>
    BidirectionalIter2 copy_backward_cat_dispatch(BidirecitonalIter1 first, BidirecitonalIter1 last,
                                                   BidirectionalIter2 result, dw_stl::bidirectional_iterator_tag)
    {
        while (first != last)
            *--result = *--last;
        return result;
    }

    // copy_backward_cat_dispatch的random_access_iterator_tag版本(命名规则为其能提供的最小功能)
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
    // copy_n_dispatch的input_iterator_tag版本
    template <class InputIter, class Size, class OutputIter>
    dw_stl::pair<InputIter, OutputIter> copy_n_dispatch(InputIter first, Size n, OutputIter result, dw_stl::input_iterator_tag)
    {
        for (; n > 0; --n, ++result)
            *result = *first;
        
        return dw_stl::pair<InputIter, OutputIter>(first, result);
    }

    // copy_n_dispatch的random_access_iterator_tag版本
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
    // move_cat_dispatch的input_iterator_tag版本
    template <class InputIter, class OutputIter>
    OutputIter move_cat_dispatch(InputIter first, OutputIter last, OutputIter result,
                                  dw_stl::input_iterator_tag)
    {
        for (; first != last; ++first, ++result)
            *result = dw_stl::move(*first);
        return result;
    }

    // move_cat_dispatch的random_access_iterator_tag版本
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
    // move_backward_cat_dispatch的bidirectional_iterator_tag版本
    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 move_backward_cat_dispatch(BidirectionalIter1 first, BidirectionalIter1 last,
                                                  BidirectionalIter2 result, dw_stl::bidirectional_iterator_tag)
    {
        while (first != last)
            *--result = dw_stl::move(*last);
        return result;
    }

    // move_backward_cat_dispatch的random_access_iterator_tag版本
    template <class RandomIter1, class RandomIter2>
    RandomIter2 move_backward_cat_dispatch(RandomIter1 first, RandomIter1 last,
                                           RandomIter2 result, dw_stl::random_access_iterator_tag)
    {
        for (auto n = last - first; n > 0; --n)
            *--result = dw_stl::move(*--last);
        return result;
    }

    // move_backward_dispatch的迭代器版本(自定义类型)
    template <class BidirecitonalIter1, class BidirectionalIter2>
    BidirectionalIter2 move_backward_dispatch(BidirecitonalIter1 first, BidirecitonalIter1 last,
                                              BidirectionalIter2 result)
    {
        return move_backward_cat_dispatch(first, last, result, iterator_category(first));
    }

    // move_backward_dispatch的原生类型版本(特化版本)
    template <class Tp, class Up>
    typename std::enable_if<
        std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
        std::is_trivially_move_assignable<Up>::value, Up*>::type
    move_backward_dispatch(Tp* first, Tp* last, Up* result)
    {
        const size_t n = static_cast<size_t>(last - first);
        if (n != 0)
        {
            result -= n;
            // 直接调用标准库的memmove函数
            std::memmove(result, first, n * sizeof(Up));
        }
        return result;
    }

    // move_backward接口函数
    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2 move_backward(BidirectionalIter1 first, BidirectionalIter1 last,
                                     BidirectionalIter2 result)
    {
        return move_backward_dispatch(first, last, result);
    }

    /*******************************************************************/
    // equal比较函数
    // 比较[first1, last1)序列上的元素是否和first2开头的元素相等
    /*******************************************************************/
    // 函数对象：函数名、函数指针、重载了()运算符的类
    template <class InputIter1, class InputIter2>
    bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2)
    {
        for (; first1 != last1; ++first1, ++first2)
        {
            if (*first1 != *first2)
                return false;
        }
        return true;
    }
    
    // 重载版本使用函数对象 comp 代替比较操作, 可以传入函数指针、函数名、重载了()运算符的对象
    template <class InputIter1, class InputIter2, class Compare>
    bool equal(InputIter1 first1, InputIter2 last1, InputIter2 first2, Compare cmp)
    {
        for (; first1 != last1; ++first1, ++first2)
        {
            if (!cmp(*first1, *first2))
                return false;
        }

        return true;
    }

    /*******************************************************************/
    // fill_n
    // 从first位置开始填充n个值
    /*******************************************************************/
    template <class OuputIter, class Size, class T>
    OuputIter fill_n_dispatch(OuputIter first, Size n, const T& value)
    {
        for (; n > 0; --n, ++first)
            *first = value;
        return first;
    }

    // 为one-byte提供特化版本
    template <class Tp, class Size, class Up>
    typename std::enable_if<
        std::is_integral<Tp>::value && sizeof(Tp) == 1 &&
        !std::is_same<Tp, bool>::value &&
        std::is_integral<Up>::value && sizeof(Up) == 1, Tp*>::type
    fill_n_dispatch(Tp* first, Size n, Up value)
    {
        if (n > 0)
            std::memset(first, (unsigned char)value, (size_t)(n));
        return first + n;
    }

    template <class OuputIter, class Size, class T>
    OuputIter fill_n(OuputIter first, Size n, const T& value)
    {
        return fill_n_dispatch(first, n, value);
    }

    /*******************************************************************/
    // fill
    // 为[first, last)区间捏的所有元素填充新值
    /*******************************************************************/
    // forward_iterator_tag版本(因为bidirectional是继承自forward的，因此bidirectional_iterator也可以调用)
    template <class ForwardIter, class T>
    void fill_dispatch(ForwardIter first, ForwardIter last, const T& value,
                    dw_stl::forward_iterator_tag)
    {
        for (; first != last; ++first)
            *first = value;
    }

    // random_access_iterator_tag版本
    template<class RandomIter, class T>
    void fill_dispatch(RandomIter first, RandomIter last, const T& value,
                    dw_stl::random_access_iterator_tag)
    {
        // 直接调用fill_n函数
        fill_n(first, last - first, value);
    }

    // fill函数接口
    template <class ForwardIter, class T>
    void fill(ForwardIter first, ForwardIter last, const T& value)
    {
        fill_dispatch(first, last, value, iterator_category(first));
    }

    /*******************************************************************/
    // lexicographical_compare
    // 以字典序排列对两个序列进行比较，当在某个位置发现第一组不相等元素时，有下列几种情况：
    // (1)如果第一序列的元素较小，返回 true ，否则返回 false
    // (2)如果到达 last1 而尚未到达 last2 返回 true
    // (3)如果到达 last2 而尚未到达 last1 返回 false
    // (4)如果同时到达 last1 和 last2 返回 false
    /*******************************************************************/
    template <class InputIter1, class InputIter2>
    bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
                                 InputIter2 first2, InputIter2 last2)
    {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        {
            if (*first1 < *first2)
                return true;
            if (*first2 < *first1)
                return false;
        }

        return first1 == last1 && first2 != last2;
    }

    // 重载比较对象comp版本
    template <class InputIter1, class InputIter2, class Compare>
    bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
                                 InputIter2 first2, InputIter2 last2, Compare cmp)
    {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        {
            if (cmp(*first1, *first2))
                return true;
            if (cmp(*first2, *first1))
                return false;
        }

        return first1 == last1 && first2 != last2;
    }

    // 针对const unsigned char*的特化版本
    bool lexicographical_compare(const unsigned char* first1,
                                 const unsigned char* last1,
                                 const unsigned char* first2,
                                 const unsigned char* last2)
    {
        const auto len1 = last1 - first1;
        const auto len2 = last2 - first2;
        // 先比较相同长度
        // memcmp比较，如果count长度超过字符串长度，则继续比较，不会在'/0'处停下
        const auto result = std::memcmp(first1, first2, dw_stl::min(len1, len2));
        // 如果相等，长度较长的比较大
        return result != 0 ? result < 0 : len1 < len2;
    }

    /*****************************************************************************************/
    // mismatch
    // 平行比较两个序列，找到第一处失配的元素，返回一对迭代器，分别指向两个序列中失配的元素
    /*****************************************************************************************/
    template <class InputIter1, class InputIter2>
    dw_stl::pair<InputIter1, InputIter2> mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2)
    {
        while (first1 != last1 && *first1 == *first2)
        {
            ++first1;
            ++first2;
        }

        return dw_stl::pair<InputIter1, InputIter2>(first1, first2);
    }

    // 重载cmp的mismatch版本
    template <class InputIter1, class InputIter2, class Compare>
    dw_stl::pair<InputIter1, InputIter2> mismatch(InputIter1 first1, InputIter1 last,
                                                  InputIter2 first2, Compare cmp)
    {
        while (first1 != last1 && cmp(*first1, *first2))
        {
            ++first1;
            ++first2;
        }
        return dw_stl::pair<InputIter1, InputIter2>(first1, first2);
    }
}

#endif