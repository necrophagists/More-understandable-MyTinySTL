#ifndef ALGOBASE_MUSTL
#define ALGOBASE_MUSTL

#include <cstring>
#include "iterator_1.h"
#include "util_1.h"

namespace mustl {
	#ifdef max  //屏蔽该词的宏定义  在本命名空间中 max可以随便定义
    #pragma message("#undefing marco max")
    #undef max
    #endif // max

    #ifdef min
    #pragma message("#undefing marco min")
    #undef min
    #endif // min

    /*****************************************************************************************/
    // max
    // 取二者中的较大值，语义相等时保证返回第一个参数
    /*****************************************************************************************/
    template <class T>
    const T& max(const T& lhs, const T& rhs)
    {
        return lhs < rhs ? rhs : lhs;
    }

    // 重载版本使用函数对象 comp 代替比较操作
    template <class T, class Compare>
    const T& max(const T& lhs, const T& rhs, Compare comp)
    {
        return comp(lhs, rhs) ? rhs : lhs;
    }
    /*****************************************************************************************/
    // min 
    // 取二者中的较小值，语义相等时保证返回第一个参数
    /*****************************************************************************************/
    template <class T>
    const T& min(const T& lhs, const T& rhs)
    {
        return rhs < lhs ? rhs : lhs;
    }

    // 重载版本使用函数对象 comp 代替比较操作
    template <class T, class Compare>
    const T& min(const T& lhs, const T& rhs, Compare comp)
    {
        return comp(rhs, lhs) ? rhs : lhs;
    }

    /*****************************************************************************************/
    // iter_swap
    // 将两个迭代器所指对象对调
    /*****************************************************************************************/
    template <class FIter1, class FIter2>
    void iter_swap(FIter1 lhs, FIter2 rhs)
    {
        mustl::swap(*lhs, *rhs);
    }
    /*****************************************************************************************/
    // copy
    // 把 [first, last)区间内的元素拷贝到 [result, result + (last - first))内
    /*****************************************************************************************/
    //多层嵌套 copy--->迭代器 /其他平凡类型特化  迭代器--->随机访问迭代器/输入迭代器
    template <class InputIter, class OutputIter>
    OutputIter unchecked_copy_cat(InputIter first, InputIter last, OutputIter result, mustl::input_iterator_tag) {

        for (; first != last; ++first, ++result) {
            *result = *first;
        }
        return result;
    }

    template <class RandomIter, class OutputIter>
    OutputIter unchecked_copy_cat(RandomIter first, RandomIter last, OutputIter result, mustl::random_access_iterator_tag) {

        for (auto n =last-first; n>0;--n, ++first, ++result) {
             *result = *first;
        }
        return result;
    }

    template <class Iter, class OutputIter>
    OutputIter unchecked_copy(Iter first, Iter last, OutputIter result) {

        return unchecked_copy_cat(first, last, result, iterator_category(first));
    }
    // 为 trivially_copy_assignable 类型提供特化版本 //平凡复制类型---就是仅复制，无其他操作，标量或者满足要求的对象
    //char* 用底层操作(memmove memcpy...)速度快
    template <class T,class U> //std::memmove----防止内存重叠
    typename mustl::enable_if<std::is_same<typename remove_const<T>::type,U>::value &&
                            std::is_trivially_copy_assignable<U>::value,U*>::type
    unchecked_copy(T* first,T* last,U* result) {
        const auto n = static_cast<size_t>(last - first); //n为size_t类型 用static_cast强转
        if (n != 0) {
            std::memmove(result, first, n*sizeof(U));//memmove是以字节为单位移动的 因此要移动U的大小*n
        }
        return result + n;
    }

    template<class Input ,class Output>
    Output copy(Input first, Input last, Output result) {
        return unchecked_copy(first, last, result);
    }


    /*****************************************************************************************/
    // copy_backward  ---只能支持双向迭代器/随机访问迭代器
    // 将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
    /*****************************************************************************************/
    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
        unchecked_copy_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
            BidirectionalIter2 result, mustl::bidirectional_iterator_tag)
    {
        while (first != last)
            *--result = *--last;
        return result;
    }

    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
        unchecked_copy_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
            BidirectionalIter2 result, mustl::random_access_iterator_tag)
    {
        for (auto n = last - first; n > 0; --n)
            *--result = *--last;
        return result;
    }

    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
        unchecked_copy_backward(BidirectionalIter1 first, BidirectionalIter1 last,
            BidirectionalIter2 result)
    {
        return unchecked_copy_backward_cat(first, last, result,
            iterator_category(first));
    }

    // 为 trivially_copy_assignable 类型提供特化版本
    template <class Tp, class Up>
    typename std::enable_if<
        std::is_same<typename std::remove_const<Tp>::type, Up>::value&&
        std::is_trivially_copy_assignable<Up>::value,
        Up*>::type
        unchecked_copy_backward(Tp* first, Tp* last, Up* result)
    {
        const auto n = static_cast<size_t>(last - first);
        if (n != 0)
        {
            result -= n;
            std::memmove(result, first, n * sizeof(Up));  //std::memmove可以处理内存重叠
        }
        return result;
    }

    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
        copy_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
    {
        return unchecked_copy_backward(first, last, result);
    }

    /*****************************************************************************************/
    // copy_if
    // 把[first, last)内满足一元操作 unary_pred 的元素拷贝到以 result 为起始的位置上
    /*****************************************************************************************/
    template <class InputIter, class OutputIter, class UnaryPredicate>
    OutputIter
        copy_if(InputIter first, InputIter last, OutputIter result, UnaryPredicate unary_pred)
    {
        for (; first != last; ++first)
        {
            if (unary_pred(*first))
                *result++ = *first;
        }
        return result;
    }

    /*****************************************************************************************/
    // copy_n 
    // 把 [first, first + n)区间上的元素拷贝到 [result, result + n)上
    // 返回一个 pair 分别指向拷贝结束的尾部
    /*****************************************************************************************/
    template <class InputIter, class Size, class OutputIter>
    mustl::pair<InputIter, OutputIter>
        unchecked_copy_n(InputIter first, Size n, OutputIter result, mustl::input_iterator_tag)
    {
        for (; n > 0; --n, ++first, ++result)
        {
            *result = *first;
        }
        return mustl::pair<InputIter, OutputIter>(first, result);
    }

    template <class RandomIter, class Size, class OutputIter>
    mustl::pair<RandomIter, OutputIter>
        unchecked_copy_n(RandomIter first, Size n, OutputIter result,
            mustl::random_access_iterator_tag)
    {
        auto last = first + n;
        return mustl::pair<RandomIter, OutputIter>(last, mystl::copy(first, last, result));
    }

    template <class InputIter, class Size, class OutputIter>
    mustl::pair<InputIter, OutputIter>
        copy_n(InputIter first, Size n, OutputIter result)
    {
        return unchecked_copy_n(first, n, result, iterator_category(first));
    }

    /*****************************************************************************************/
    // move
    // 把 [first, last)区间内的元素移动到 [result, result + (last - first))内
    /*****************************************************************************************/
    // input_iterator_tag 版本
    template <class InputIter, class OutputIter>
    OutputIter
        unchecked_move_cat(InputIter first, InputIter last, OutputIter result,
            mustl::input_iterator_tag)
    {
        for (; first != last; ++first, ++result)
        {
            *result = mystl::move(*first);
        }
        return result;
    }

    // ramdom_access_iterator_tag 版本
    template <class RandomIter, class OutputIter>
    OutputIter
        unchecked_move_cat(RandomIter first, RandomIter last, OutputIter result,
            mustl::random_access_iterator_tag)
    {
        for (auto n = last - first; n > 0; --n, ++first, ++result)
        {
            *result = mystl::move(*first);
        }
        return result;
    }

    template <class InputIter, class OutputIter>
    OutputIter
        unchecked_move(InputIter first, InputIter last, OutputIter result)
    {
        return unchecked_move_cat(first, last, result, iterator_category(first));
    }

    // 为 trivially_copy_assignable 类型提供特化版本
    template <class Tp, class Up>
    typename std::enable_if<
        std::is_same<typename std::remove_const<Tp>::type, Up>::value&&
        std::is_trivially_move_assignable<Up>::value,
        Up*>::type
        unchecked_move(Tp* first, Tp* last, Up* result)
    {
        const size_t n = static_cast<size_t>(last - first);
        if (n != 0)
            std::memmove(result, first, n * sizeof(Up));
        return result + n;
    }

    template <class InputIter, class OutputIter>
    OutputIter move(InputIter first, InputIter last, OutputIter result)
    {
        return unchecked_move(first, last, result);
    }

    /*****************************************************************************************/
    // move_backward
    // 将 [first, last)区间内的元素移动到 [result - (last - first), result)内
    /*****************************************************************************************/
    // bidirectional_iterator_tag 版本
    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
        unchecked_move_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
            BidirectionalIter2 result, mustl::bidirectional_iterator_tag)
    {
        while (first != last)
            *--result = mystl::move(*--last);
        return result;
    }

    // random_access_iterator_tag 版本
    template <class RandomIter1, class RandomIter2>
    RandomIter2
        unchecked_move_backward_cat(RandomIter1 first, RandomIter1 last,
            RandomIter2 result, mustl::random_access_iterator_tag)
    {
        for (auto n = last - first; n > 0; --n)
            *--result = mystl::move(*--last);
        return result;
    }

    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
        unchecked_move_backward(BidirectionalIter1 first, BidirectionalIter1 last,
            BidirectionalIter2 result)
    {
        return unchecked_move_backward_cat(first, last, result,
            iterator_category(first));
    }

    // 为 trivially_copy_assignable 类型提供特化版本
    template <class Tp, class Up>
    typename std::enable_if<
        std::is_same<typename std::remove_const<Tp>::type, Up>::value&&
        std::is_trivially_move_assignable<Up>::value,
        Up*>::type
        unchecked_move_backward(Tp* first, Tp* last, Up* result)
    {
        const size_t n = static_cast<size_t>(last - first);
        if (n != 0)
        {
            result -= n;
            std::memmove(result, first, n * sizeof(Up));
        }
        return result;
    }

    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
        move_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
    {
        return unchecked_move_backward(first, last, result);
    }

    //比较序列1在first至last区间上是否和序列2相等
    template<class InputIter1,class InputIter2>
    bool equal(InputIter1 first, InputIter2 last, InputIter2 result) {
        while (first != last) {
            if (*first != *result)
                return false;
            ++result;
            ++first;
        }
        return true;
    }


    // 重载版本使用函数对象 comp 代替比较操作
    template <class InputIter1, class InputIter2, class Compared>
    bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compared comp)
    {
        for (; first1 != last1; ++first1, ++first2)
        {
            if (!comp(*first1, *first2))
                return false;
        }
        return true;
    }

    //fill_n 从first位置填充n个词 返回result
    template <class OutputIter, class Size, class T>
    OutputIter unchecked_fill_n(OutputIter first, Size n, const T& value)
    {
        for (; n > 0; --n, ++first)
        {
            *first = value;
        }
        return first;
    }
    // 为 one-byte 类型提供特化版本 (bool数组？)因此需要is_same bool 而且 is_intergal
    //memset 一样在内存上填充 （目的，填充值，个数）
    template <class Tp, class Size, class Up>
    typename std::enable_if<
        std::is_integral<Tp>::value && sizeof(Tp) == 1 &&
        !std::is_same<Tp, bool>::value &&
        std::is_integral<Up>::value && sizeof(Up) == 1,
        Tp*>::type
        unchecked_fill_n(Tp* first, Size n, Up value)
    {
        if (n > 0)
        {
            std::memset(first, (unsigned char)value, (size_t)(n));
        }
        return first + n;
    }

    template <class OutputIter, class Size, class T>
    OutputIter fill_n(OutputIter first, Size n, const T& value)
    {
        return unchecked_fill_n(first, n, value);
    }

    // fill
    // 为 [first, last)区间内的所有元素填充新值
    /*****************************************************************************************/
    template <class ForwardIter, class T>
    void fill_cat(ForwardIter first, ForwardIter last, const T& value,
        mustl::forward_iterator_tag)
    {
        for (; first != last; ++first)
        {
            *first = value;
        }
    }

    template <class RandomIter, class T>
    void fill_cat(RandomIter first, RandomIter last, const T& value,
        mustl::random_access_iterator_tag)
    {
        fill_n(first, last - first, value);
    }

    template <class ForwardIter, class T>
    void fill(ForwardIter first, ForwardIter last, const T& value)
    {
        fill_cat(first, last, value, iterator_category(first));
    }


    //lex...以字典序来排序

    template <class InputIter1, class InputIter2>
    //逐个比较的过程中，如果first1<first2 return true 反之return false
    //如果有一个先到了末尾，那么看是first1还是first2先到的
    //如果是frist1先到 返回false
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

    //针对unsigned char*特化          //用memcmp比较相同长度的先，然后再看剩下的部分长度
    bool lexicographical_compare(const unsigned char* first1,
        const unsigned char* last1,
        const unsigned char* first2,
        const unsigned char* last2)
    {
        const auto len1 = last1 - first1;
        const auto len2 = last2 - first2;
        // 先比较相同长度的部分
        const auto result = std::memcmp(first1, first2, mustl::min(len1, len2));
        // 若相等，长度较长的比较大
        return result != 0 ? result < 0 : len1 < len2;
    }

    //mismatch 找到第一处不匹配的元素---返回一对pair
    template <class InputIter1, class InputIter2>
    mustl::pair<InputIter1, InputIter2>
        mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2)
    {
        while (first1 != last1 && *first1 == *first2)
        {
            ++first1;
            ++first2;
        }
        return mystl::pair<InputIter1, InputIter2>(first1, first2);
    }

    // 重载版本使用函数对象 comp 代替比较操作
    template <class InputIter1, class InputIter2, class Compred>
    mustl::pair<InputIter1, InputIter2>
        mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compred comp)
    {
        while (first1 != last1 && comp(*first1, *first2))
        {
            ++first1;
            ++first2;
        }
        return mystl::pair<InputIter1, InputIter2>(first1, first2);
    }
}
#endif