#ifndef MYSTL_UNINITIALIZED_H_
#define MYSTL_UNINITIALIZED_H_

// 这个头文件用于对未初始化空间构造元素

/*
实现了以下函数：
uninitialized_copy(first, last, result)
uninitialized_copy_n(first, n, result)
uninitialized_fill(first, last, value)
uninitialized_fill_n(first, n, value)
uninitialized_move(first, last, result)
uninitialized_move_n(first, n, result)
*/

#include "construct.h"
#include "iterator.h"
#include "type_traits.h"
#include "util.h"

namespace dw_stl
{
    /******************************************************************
    uninitialized_copy函数
    args:
        first, last, result
    功能：
        负责把[first, last)上的内容复制到以result为起始处的空间，返回复制结束的位置
    *****************************************************************************/
    // POD类型
    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy_dispatch(InputIter first, InputIter last, ForwardIter result, std::true_type)
    {
        // 调用algobase的copy函数
        // 可以直接使用等号进行赋值的
        return dw_stl::copy(first, last, result);
    }

    // 不是POD类型
    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_copy_dispatch(InputIter first, InputIter last, ForwardIter result, std::false_type)
    {
        auto cur = result;
        try
        {
            // 将迭代器first指向的对象构造到result指向的对象中去
            for (; first != last; ++first, ++cur)
            {
                // 构造函数
                dw_stl::construct(&*cur, *first);
            }
        }
        catch
        {
            for (; result != cur; ++result)
                dw_stl::destroy(&*result);
        }
    }

    template <class InputIter, class FrowardIter>
    FrowardIter uninitialized_copy(InputIter first, InputIter last, FrowardIter result)
    {
        // 判断是否有可以忽略的赋值构造函数, 如果有的话就可以直接进行copy
        return dw_stl::uninitialized_copy_dispatch(first, last, result, 
                                            std::is_trivially_copy_assignable<
                                            typename iterator_traits<FrowardIter>::value_type>{});
    }

    /******************************************************************
    uninitialized_copy_n函数
    args:
        first, n, result
    功能：
        负责把[first, first + n)上的内容复制到以result为起始处的空间，返回复制结束的位置
    *****************************************************************************/
    // 原生数据类型的构造
    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_copy_n_dispatch(InputIter first, Size n, ForwardIter result, std::true_type)
    {
        return dw_stl::copy_n(first, n, first).second;
    }

    // 类的构造
    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_copy_n_dispatch(InputIter first, Size n, ForwardIter result, std::false_type)
    {
        // copy and swap技术
        auto cur = result;
        try 
        {
            for (; n > 0; --n, ++cur, ++first)
            {
                dw_stl::construct(&*cur, *first);
            }
        }
        // 提供强异常处理，加入发生异常将回退到函数调用之前的状态
        catch (...)
        {
            for (; result != cur; ++result)
                dw_stl::destroy(&*result);
        }

        return cur;
    }

    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_copy_n(InputIter first, Size n, ForwardIter result)
    {
        return dw_stl::uninitialized_copy_n_dispatch(first, n, result,
                                               std::is_trivially_copy_assignable<
                                               typename iterator_traits<ForwardIter>::value_type>{});
    }

    /******************************************************************
    uninitialized_fill函数
    args:
        first, last, value
    功能：
        负责在[first, last)上填上value的内容
    *****************************************************************************/
    template <class ForwardIter, class T>
    void uninitialized_fill_dispatch(ForwardIter first, ForwardIter last, const T& value, std::true_type)
    {
        dw_stl::fill(first, last, value);
    }

    template <class ForwardIter, class T>
    void uninitialized_fill_dispatch(ForwardIter first, ForwardIter last, const T& value, std::false_type)
    {
        auto cur = first;
        try 
        {
            for (; cur != last; ++cur)
                dw_stl::construct(&*cur, value);
        }
        catch (...)
        {
            for (; first != cur; ++first)
                dw_stl::destroy(&*first);
        }
    }

    template <class ForwardIter, class T>
    void uninitialized_fill(ForwardIter first, ForwardIter last, const T& value)
    {
        dw_stl::uninitialized_fill_dispatch(first, last, value, 
                                      std::is_trivially_copy_assignable<
                                      typename iterator_traits<ForwardIter>::
                                      value_type>{});
    }

    /******************************************************************
    uninitialized_fill_n函数
    args:
        first, n, value
    功能：
        负责在[first, first + n)上填上value的内容
    *****************************************************************************/
    template <class ForwardIter, class Size, class T>
    ForwardIter uninitialized_fill_n_dispatch(ForwardIter first, Size n, const T& value, std::true_type)
    {
        return dw_stl::fill_n(first, n, value);
    }

    template <class ForwardIter, class Size, class T>
    ForwardIter uninitialized_fill_n_dispatch(ForwardIter first, Size n, const T& value, std::false_type)
    {
        auto cur = first;
        try
        {
            for (; n > 0; --n, ++cur)
                dw_stl::construct(&*first, value);
        }
        catch (...)
        {
            // 如果发生异常，销毁对象，提供强异常保证
            for (; first != cur; ++first)
                dw_stl::destroy(&*first);
        }
    }

    // 是否是POD类型的，即是否有赋值运算符
    template <class ForwardIter, class Size, class T>
    ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T& value)
    {
        return dw_stl::uninitialized_fill_n_dispatch(first, n, value,
                                                std::is_trivially_copy_assignable<
                                                typename iterator_traits<ForwardIter>::value_type>{});
    }

    /******************************************************************
    uninitialized_move函数
    args:
        first, last, result
    功能：
        负责将[first, last)上的内容移动到以 result 为起始的空间
    *****************************************************************************/
    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_move_dispatch(InputIter first, InputIter last, ForwardIter result, std::true_type)
    {
        // 调用库中的move函数
        return dw_stl::move(first, last, result);
    }

    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_move_dispatch(InputIter first, InputIter last, ForwardIter result, std::false_type)
    {
        auto cur = result;
        try 
        {
            for (; first != last; ++first, ++cur)
                // 使用右值引用，直接对象转移
                dw_stl::construct(&*cur, dw_stl::move(*first));
        }
        catch (...)
        {
            for(; result != cur; ++result)
                dw_stl::destroy(&*result);
        }

        return cur;
    }

    template <class InputIter, class ForwardIter>
    ForwardIter uninitialized_move(InputIter first, InputIter last, ForwardIter result)
    {
        return dw_stl::uninitialized_move_dispatch(first, last, result,
                                          std::is_trivially_copy_assignable<
                                          typename iterator_traits<ForwardIter>::value_type>{});
    }

    /******************************************************************
    uninitialized_move_n函数
    args:
        first, n, result
    功能：
        负责将[first, first + n)上的内容移动到以 result 为起始的空间
    *****************************************************************************/
    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_move_n_dispatch(InputIter first, Size n, ForwardIter result, std::true_type)
    {
        // 如果是true_type类型的，就调用库的move函数
        return dw_stl::move(first, first + n, result);
    }

    // 否则就需要手动运行拷贝构造函数
    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_move_n_dispatch(InputIter first, Size n, ForwardIter result, std::false_type)
    {
        auto cur = result;
        try 
        {
            for (; n > 0; --n, ++first, ++cur)
                dw_stl::construct(&*cur, dw_stl::move(*first));
        }
        catch (...)
        {
            for (; result != cur; ++result)
                dw_stl::destroy(&*result);
        }

        return cur;
    }

    // 检查是否拥有复制赋值运算符
    template <class InputIter, class Size, class ForwardIter>
    ForwardIter uninitialized_move_n(InputIter first, Size n, ForwardIter result)
    {
        return dw_stl::uninitialized_move_n_dispatch(first, n, result,
                                               std::is_trivially_copy_assignable<
                                               typename iterator_traits<ForwardIter>::value_type>{});
    }
}

#endif