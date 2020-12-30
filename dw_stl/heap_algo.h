#ifndef DW_STL_HEAP_ALGO_H_
#define DW_STL_HEAP_ALGO_H_

/*
本文件实现了包含heap的四个算法：
push_heap, pop_heap, sort_heap, make_heap
*/

#include "iterator.h"

namespace dw_stl
{
    /*****************************************************************************************/
    // push_heap
    // 该函数接受两个迭代器，表示一个 heap 容器的首尾，并且新元素已经插入到底部容器的最尾端，调整 heap
    /*****************************************************************************************/
    // 默认是大顶堆(自底向上调整)
    template <class RandomIter, class Distance, class T>
    void push_heap_aux(RandomIter first, Distance shiftIdx, Distance topIdx, T value)
    {
        auto parent = (shiftIdx - 1) / 2;
        // 如果父节点的数小于当前数
        while (shiftIdx > topIdx && *(first + parent) < value)
        {
            // 大顶堆
            *(first + shiftIdx) = *(first + parent);
            shiftIdx = parent;
            parent = (shiftIdx - 1) / 2;
        }
        *(first + shiftIdx) = value;
    }

    template <class RandomIter, class Distance>
    void push_heap_d(RandomIter first, RandomIter last, Distance*)
    {
        dw_stl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1));
    }

    template <class RandomIter>
    void push_heap(RandomIter first, RandomIter last)
    {
        dw_stl::push_heap_d(first, last, distance_type(first));
    }

    /*****************************************************************/
    // 重载push_heap函数，允许自定义比较函数cmp
    template <class RandomIter, class Distance, class T, class Compare>
    void push_heap_aux(RandomIter first, Distance shiftIdx, Distance topIdx, T value, Compare cmp)
    {
        auto parent = (shiftIdx - 1) / 2;
        // cmp函数比较优先级
        // cmp(lhs, rhs)如果返回true，则表示lhs的优先级小于rhs的优先级，优先级高的往前面移动
        while (shiftIdx > topIdx && cmp(*(first + parent), value))
        {
            // 将value往上移动
            *(first + shiftIdx) = *(first + parent);
            shiftIdx = parent;
            parent = (shiftIdx - 1) / 2;
        }
        // 最后固定的位置
        *(first + shiftIdx) = value;
    }

    template <class RandomIter, class Compare, class Distance>
    void push_heap_d(RandomIter first, RandomIter last, Distance*, Compare cmp)
    {
        dw_stl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1), cmp);
    }

    template <class RandomIter, class Compare>
    void push_heap(RandomIter first, RandomIter last, Compare cmp)
    {
        dw_stl::push_heap_d(first, last, distance_type(first), cmp);
    }

    /*****************************************************************************************/
    // pop_heap
    // 该函数接受两个迭代器，表示 heap 容器的首尾，将 heap 的根节点取出放到容器尾部，调整 heap
    /*****************************************************************************************/
    // 默认大顶堆
    template <class RandomIter, class T, class Distance>
    void adjust_heap(RandomIter first, Distance shiftIdx, Distance len, T value)
    {
        // 左孩子节点
        auto lchild = 2 * shiftIdx + 1;
        while (lchild < len)
        {
            // 找到比较大的儿子节点
            if (lchild + 1 < len && *(first + lchild) < *(first + lchild + 1))
                lchild++;
            // 如果孩子节点大于父节点，则交换
            if (*(first + lchild) > value)
                *(first + shiftIdx) = *(first + lchild);
            shiftIdx = lchild;
            lchild = 2 * shiftIdx + 1;
        }
        // 找到插入的位置
        *(first + shiftIdx) = value;
    }

    template <class RandomIter, class T, class Distance>
    void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance*)
    {
        // 先将迭代器首部的值调整到尾部，然后调整[first, last - 1)使之重新成为一个heap
        *result = *first;
        dw_stl::adjust_heap(first, static_cast<Distance>(0), last - first, value);
    }

    template <class RandomIter>
    void pop_heap(RandomIter first, RandomIter last)
    {
        dw_stl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first));
    }

    // 重载pop_heap函数，允许自定义比较函数cmp, value为新插入首部的值
    template <class RandomIter, class T, class Distance, class Compare>
    void adjust_heap(RandomIter first, Distance shiftIdx, Distance len, T value, Compare cmp)
    {
        // 自顶向下调整
        auto lchild = 2 * shiftIdx + 1;
        while (lchild < len)
        {
            // cmp(lhs, rhs)返回true表示lhs的优先级小于rhs的优先级
            if (lchild + 1 < len && cmp(*(first + lchild + 1), *(first + lchild)))
                lchild++;
            if (value, cmp(*(first + lchild)))
                *(first + shiftIdx) = *(first + lchild);
            shiftIdx = lchild;
            lchild = 2 * shiftIdx + 1;
        }
        // 找到最后的插入点
        *(first + shiftIdx) = value;
    }

    template <class RandomIter, class T, class Distance, class Compare>
    void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result,
                      T value, Distance*, Compare cmp)
    {
        // 现将尾指针设置为首部值
        *result = *first;
        // 然后自定向下调整heap
        dw_stl::adjust_heap(first, static_cast<Distance>(0), last - first, value, cmp);
    }

    template <class RandomIter, class Compare>
    void pop_heap(RandomIter first, RandomIter last, Compare cmp)
    {
        dw_stl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first), cmp);
    }

    /*****************************************************************************************/
    // sort_heap(使用堆排序)
    // 该函数接受两个迭代器，表示 heap 容器的首尾，不断执行 pop_heap 操作，直到首尾最多相差1
    /*****************************************************************************************/
    template <class RandomIter>
    void sort_heap(RandomIter first, RandomIter last)
    {
        // 每执行一次pop_heap，最大的元素都被放到尾部，直到容器最多只有一个元素，便完成排序
        while (last - first > 1)
        {
            dw_stl::pop_heap(first, last--);
        }
    }

    // 重载函数，允许自定义比较函数cmp
    template <class RandomIter, class Compare>
    void sort_heap(RandomIter first, RandomIter last, Compare cmp)
    {
        while (last - first > 1)
            dw_stl::sort_heap(first, last--, cmp);
    }

    /*****************************************************************************************/
    // make_heap    
    // 该函数接受两个迭代器，表示 heap 容器的首尾，把容器内的数据变为一个 heap
    /*****************************************************************************************/
    template <class RandomIter, class Distance>
    void make_heap_aux(RandomIter first, RandomIter last, Distance*)
    {
        if (last - first < 2)
            return;
        auto len = last - first;
        auto shiftIdx = (len - 2) / 2;
        // 从shiftIdx开始进行自顶向下的调整
        while (true)
        {
            // 重排以shiftIdx为首的子树
            dw_stl::adjust_heap(first, shiftIdx, len, *(first + shiftIdx));
            if (shiftIdx == 0)
                return;
            shiftIdx--;
        }
    }

    template <class RandomIter>
    void make_heap(RandomIter first, RandomIter last)
    {
        dw_stl::make_heap_aux(first, last, distance_type(first));
    }

    // 重载make_heap函数，允许自定义比较函数cmp
    template <class RandomIter, class Distance, class Compare>
    void make_heap_aux(RandomIter first, RandomIter last, Distance*, Compare cmp)
    {
        if (last - first < 2)
            return;
        auto len = last - first;
        // 开始为最后一个不为子节点的节点
        auto shiftIdx = (len - 2) / 2;
        while (true)
        {
            // 重排以shiftIdx为首的子树
            dw_stl::adjust_heap(first, shiftIdx, len, *(first + shiftIdx), cmp);
            if (shiftIdx == 0)
                return;
            shiftIdx--;
        }
    }

    template <class RandomIter, class Compare>
    void make_heap(RandomIter first, RandomIter last, Compare cmp)
    {
        dw_stl::make_heap_aux(first, last, distance_type(first), cmp);
    }
}
#endif