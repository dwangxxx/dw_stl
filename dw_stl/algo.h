#ifndef DW_STL_ALGO_H_
#define DW_STL_ALGO_H_

#include "iterator.h"

namespace dw_stl
{
    /*****************************************************************************************/
    // iter_swap
    // 将两个迭代器所指对象对调
    /*****************************************************************************************/
    template <class ForwardIter1, class ForwardIter2>
    void iter_swap(ForwardIter1 lhs, ForwardIter2 rhs)
    {
        dw_stl::swap(*lhs, *rhs);
    }
    /*****************************************************************************************/
    // reverse
    // 将[first, last)区间内的元素反转
    /*****************************************************************************************/
    // reverse_dispatch 的 bidirectional_iterator_tag 版本
    template <class BidirectionalIter>
    void reverse_dispatch(BidirectionalIter first, BidirectionalIter last,
                        bidirectional_iterator_tag)
    {
        while (true)
        {
            if (first == last || first == --last)
            return;
            dw_stl::iter_swap(first++, last);
        }
    }

    // reverse_dispatch 的 random_access_iterator_tag 版本
    template <class RandomIter>
    void reverse_dispatch(RandomIter first, RandomIter last,
                        random_access_iterator_tag)
    {
        while (first < last)
            dw_stl::iter_swap(first++, --last);
    }

    template <class BidirectionalIter>
    void reverse(BidirectionalIter first, BidirectionalIter last)
    {
        dw_stl::reverse_dispatch(first, last, iterator_category(first));
    }
}
#endif