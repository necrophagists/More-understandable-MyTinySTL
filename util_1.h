#ifndef UTIL_1
#define UTIL_1
#include <cstddef>
#include "type_traits_1.h"
namespace mustl {
	//std::move 左值转右值
	//使用static_cast强转 
	template <class T>
	typename std::remove_reference<T>::type && move(T&& arg) {  //发生了类型推导--->万能引用 无论进来的是左值右值，先去引用属性 然后强转为右值引用 
		return static_cast<typename std::remove_reference<T>::type &&>(arg);
	}
	//std::forward 完美转发 保证参数在传递过程中类型始终保持不变
	//分为 T T& T&&
	template<class T>
	T&& forward(typename std::remove_reference<T>::type& arg) {
		return static_cast<T&&>(arg);
		//传进来是左值 推导出 T =x& 展开得到 x& && ----》引用折叠之后返回左值
	}

	template <class T>
	T&& forward(typename std::remove_reference<T>::type&& arg) {
		//如果此时识别出来为左值 就报错
		// target:不是左值   
		static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
		return static_cast<T&&>(arg);
		//传进来是右值 推导出 T =x&& 展开得到 x&& && ----》引用折叠之后返回右值
	}
	//*****************************************
	//swap  使用move效率更高 直接进行所有权的转移
	template<class T1>                 //交换一般类型
	void swap(T1& lhs, T1& rhs) {
		T1 temp = mustl::move(lhs);
		lhs = mustl::move(rhs);
		rhs = mustl::move(temp);
	}

	template<class T1, size_t N>
	void swap(T1(&a)[N], T1(&b)[N]) {    //交换数组
		if (&a != &b) { //检查A B 地址是否相同 如果相同则说明A=B 无需交换
			T1* firstA = a;
			T1* lastA = a + N;
			T1* firstB = b;
			for (; firstA != lastA; ++firstA, ++firstB) {
				mustl::swap(*firstA, *firstB);
			}
		}
	}
	//*********************************************
	//**********************************************
	//std::pair 
	template<class T1,class T2>
	struct pair {
		typedef T1 first_type;
		typedef T2 second_type;

		first_type first;
		second_type second;

		//各种构造函数  加constexpr 使得在编译期执行
		// Other1 Other2均有默认值 
		//1)默认构造函数    T1 T2是否能默认构造?      
		template<class Other1 = T1, class Other2 = T2, 
			typename mustl::enable_if<
			std::is_default_constructible<Other1>::value&&
			std::is_default_constructible<Other2>::value, void>::type>
			constexpr pair() :first(), second() {};

		//2)隐式构造函数   
		//类型能够进行拷贝构造？能否进行const T1&->T1,const T2&->T2的隐式转换？
		template<class Other1 = T1, class Other2 = T2, 
			typename mustl::enable_if<
			std::is_copy_constructible<Other1>::value&&
			std::is_copy_constructible<Other2>::value&&
			std::is_convertible<const Other1&,T1>::value &&
			std::is_convertible<const Other2&, T2>::value, int>::type=0>
			constexpr pair(const T1&a,const T2&b) :first(a), second(b) {};

		//3)显式构造函数    
		//类型能够进行拷贝构造？ const T1&,const T2&是否至少一个类型无法隐式转换？
		template<class Other1 = T1, class Other2 = T2, 
			typename mustl::enable_if<
			std::is_copy_constructible<Other1>::value&&
			std::is_copy_constructible<Other2>::value&&
			(!std::is_convertible<const Other1&, T1>::value||
			!std::is_convertible<const Other2&, T2>::value), int>::type = 0>
			explicit constexpr pair(const T1& a, const T2& b) :first(a), second(b) {};//加explicit表示无法隐式调用

		pair(const pair& rhs) = default;
		pair(pair&& rhs) = default; //拷贝构造以及移动构造均用默认


		////////////////下面开始有点难理解 我的注释也不一定对
		/////之前是用T1 T2类型来初始化pair  4)-5)是使用其他类型来初始化pair，如果T1 T2可以由Other1 Other2构造,且可以隐式转换 那也可以使用Other1 Other2来初始化pair
		///6)-9)是用pair来初始化pair

		//is_constructible<T1,...args> T1是否可以被args...所构造  <int,float,double>--->int(float,double)?
		// 可以实现T1(Other1) ，T2(Other2)这样的构造吗?
		//4)其它类型的隐式构造函数   其它类型可以隐式转换为T1 T2吗？ 此时为保证参数正确传递 使用万能引用和完美转发
		template<class Other1, class Other2, 
			typename mustl::enable_if<
			std::is_constructible<T1,Other1>::value&&
			std::is_constructible<T2,Other2>::value&&
			std::is_convertible<Other1&&, T1>::value&&
			std::is_convertible<Other2&&, T2>::value, int>::type = 0>
			constexpr pair(Other1&& a, Other2&& b) :first(mustl::forward<Other1>(a)), second(mustl::forward<Other1>(b)) {};

		//5)其它类型的显示构造函数   -->pair<int,int>a =pair<int,int>(1,2)   
         //类型能够构造？ 是否至少一个类型无法隐式转换？ 此时为保证参数正确传递 使用万能引用和完美转发
		template<class Other1, class Other2, 
			typename mustl::enable_if<
			std::is_constructible<T1,Other1>::value&&
			std::is_constructible<T2,Other2>::value &&
			(!std::is_convertible<Other1, T1>::value ||
				!std::is_convertible<Other2, T2>::value), int>::type = 0>
			explicit constexpr pair(Other1&& a, Other2&& b) :first(mustl::forward<Other1>(a)), second(mustl::forward<Other1>(b)) {};

		//6使用const pair对象隐式初始化 使用 pair<other1,other2>来构造另一个pair
		template<class Other1, class Other2, 
			typename mustl::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value&&
			std::is_convertible<const Other1&, T1>::value&&
			std::is_convertible<const Other2&, T2>::value, int>::type = 0>
			constexpr pair(const pair<Other1,Other2>&a) :first(a.first), second(a.second) {};

		//7)const pair显式初始化 使用 pair<other1,other2>来构造另一个pair
		template<class Other1, class Other2, 
			typename mustl::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value&&
			(!std::is_convertible<const Other1&, T1>::value||
			!std::is_convertible<const Other2&, T2>::value), int>::type = 0>
			explicit constexpr pair(const pair<Other1, Other2>& a) :first(a.first), second(a.second) {};

		//8)非const pair来隐式初始化       万能引用+完美转发准没错 && +forward
		template<class U1, class U2, typename mustl::enable_if<
			std::is_constructible<T1, U1>::value&&
			std::is_constructible<T2, U2>::value&&
			std::is_convertible<U1, T1>::value&&
			std::is_convertible<U2, T2>::value, int>::type = 0>
			constexpr pair(pair<U1, U2>&& a):first(mustl::forward<U1>(a.first)), second(mustl::forward<U2>(a.second)){};

		//8)非const pair来显式初始化       
		template<class U1, class U2, typename mustl::enable_if<
			std::is_constructible<T1, U1>::value&&
			std::is_constructible<T2, U2>::value&&
			(!std::is_convertible<U1, T1>::value||
			!std::is_convertible<U2, T2>::value), int>::type = 0>
			explicit constexpr pair(pair<U1, U2>&&a) :first(mustl::forward<U1>(a.first)), second(mustl::forward<U2>(a.second)) {};


		//拷贝赋值运算符
		//1)const
		pair& operator=(const pair & a){
			//检查自赋值
			if (this != &a) {
				this->first = a.first;
				this->second = a.second;
			}
			return *this;
		}
		//2)移动赋值运算符
		pair& operator=(pair&& a) {
			//检查自赋值
			if (this != &a) {
				this->first = mustl::move(a.first);
				this->second = mustl::move(a.second);
			}
			return *this;
		}
		// 其它类型pair的拷贝运算符
		template <class Other1, class Other2>
		pair& operator=(const pair<Other1, Other2>& other)
		{
			first = other.first;
			second = other.second;
			return *this;
		}

		// / 其它类型pair的移动运算符  万能引用+完美转发
		template <class Other1, class Other2>
		pair& operator=(pair<Other1, Other2>&& other)
		{
			first = mustl::forward<Other1>(other.first);
			second = mustl::forward<Other2>(other.second);
			return *this;
		}

		~pair() {};

		void swap(pair& other) {
			//检查自赋值
			if (this != &other) {
				mustl::swap(first, other.first);
				mustl::swap(second, other.second)
			}
		}
	}; 

	//重载运算符
	// == <!= > <= >=
	template<class T1,class T2>
	bool operator==(const pair<T1,T2>& a, const pair<T1,T2>& b) { 
		return a.first == b.first && a.second == b.second;
	}

	template<class T1, class T2>
	bool operator<(const pair<T1, T2>& a, const pair<T1, T2>& b) {
		return (a.first<b.first) || (a.first == b.first && a.second<b.second);
	}

	template<class T1, class T2>
	bool operator!=(const pair<T1, T2>& a, const pair<T1, T2>& b) {
		return !(a==b);
	}
	template<class T1, class T2>
	bool operator>(const pair<T1, T2>& a, const pair<T1, T2>& b) {
		return b<a;
	}

	template<class T1, class T2>
	bool operator<=(const pair<T1, T2>& a, const pair<T1, T2>& b) {
		return !(b<a);
	}
	template<class T1, class T2>
	bool operator>=(const pair<T1, T2>& a, const pair<T1, T2>& b) {
		return !(a>b);
	}

	//重载swap
	template<class T1,class T2>
	void swap(pair<T1, T2>& a, pair<T1, T2>& b) {
		a.swap(b);
	}
	//make_pair   万能引用+完美转发    遇到中转 或者出现左右值引用参数混合的时候 或者参数类型范围大时 可以试试万能引用+完美转发
	template<class T1,class T2>
	pair<T1,T2> make_pair(T1 &&a, T2 &&b) {
		return pair<T1, T2>(mustl::forward<T1>(a), mustl::forward<T2>(b));
	}
}

#endif