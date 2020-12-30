#ifndef DW_STL_TYPE_TRAITS_H_
#define DW_STL_TYPE_TRAITS_H_

// 此头文件用来萃取类型信息

// 使用标准头文件中的type_traits来进行类型萃取
#include <type_traits>

namespace dw_stl
{
    // 定义真假
    template <class T, T v>
    struct _integral_constant
    {
        static constexpr T value = v;
    };

    template <bool b>
    using _bool_constant = _integral_constant<bool, b>;

    typedef _bool_constant<true> _true_type;
    typedef _bool_constant<false> _false_type;

    // type traits，用于萃取判断是否是pair类型
    
    template <class T1, class T2>
    struct pair;

    // 不是pair类型
    template <class T>
    struct is_pair : dw_stl::_false_type {};

    // 是pair类型
    template <class T1, class T2>
    struct is_pair<dw_stl::pair<T1, T2>> : dw_stl::_true_type {};

}

#endif