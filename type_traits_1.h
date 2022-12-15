#ifndef TYPE_TRAITS_MUSTL
#define TYPE_TRAITS_MUSTL

// 这个头文件用于提取类型信息
//类型萃取
// use standard header for type_traits
#include <type_traits>
namespace mustl
{   

	//这个类用于实现std::true_type std::false_type 
	template <class T, T v>                                  //类模板 一个类型为T的参数 以及类型为T的变量v
	struct integral_constant
	{
		static constexpr T value = v;                        //value是一个bool值 为许多type_traits函数的底层部分
	};	                                                     //定义静态成员变量(函数) 1.无需建立对象即可调用该成员---- mustl::integral_constant::value;
									                         //                       2.静态变量作用域为全局 所有对象共用该成员
		                                                    //constexpr-------变量值不可变且在编译器计算


	template <bool flag>                                   //类的特化  T =bool  v=flag
	using bool_constant = integral_constant<bool, flag>;   //using之后 可以这样调用 -----bool_constant<true/false>  只需指定flag的值

	//核心部分 std::true_type false_type                    本质上是intergeal_constant类的重命名后的特化版本  是一个类型！
	typedef bool_constant<true>  true_type;                 //如果传给bool_constant 为true---》true_type  反之就为false_type
	typedef bool_constant<false>  false_type;               //可以用表达式或者带有bool类型返回值的函数来代替true、false 这样就可以进行类型萃取了

	/*****************************************************************************************/
	//下面是用true_type false_type实现的一些函数 
	template <class T>                                     //泛化版    
	struct is_int : mustl::false_type {};  
	
	template <>                                           //特化版     只有is_int传入"int"类型才加入继承了true_type的特化版 否则就进入泛化版
	struct is_int<int> : mustl::true_type {};             //此时可以调用基类的value变量来查看结果:
	                                                      //例:   int a=1;
														  //      cout<<mustl::is_int<decltype(a)>::value<<endl; //true

	//***************************************************/
	// 以下为补充函数 实现了一些位于type_traits.h内的函数
	//std::enable_if 为类模板 用于编译时进行条件判断 以及用于万能引用时的类型推导  
	template<bool,class T=void>                           //有两个模板参数  bool、类型T   当bool为true时加入特化版 可以提取T类型  
	struct enable_if{};  //泛化                           //也可以用表达式或者带有bool类型返回值的函数

	template<class T>  //特化                             //例:  typename mustl::enable_if<(1<2),int>::type a =1;
	                                                             
	struct enable_if<true, T> {
		typedef  T type;
	};
	//********************************************
	//conditional 结构体模板 类似三目运算符 <bool,t1,t2>      原理同上  true--->T1分支  false--->T2分支
	template<bool,class T1,class T2>//泛化                    //可以嵌套着用
	                                                          //mustl::conditional<(1<2),mustl::conditional<true,int,int>::type,void>::type
	struct conditional {
		typedef T1 type;
	};

	template<class T1,class T2>    //特化
	struct conditional<false,T1,T2>{
		typedef T2 type;
	}; 
	
	
	//******************************************** 类型萃取
	//remove_all_extents  取得数组内部元素的类型  可以直接丢数组进去 也可以decltype(a)获取表达式类型再丢进去
	//要么是 int a,int a[],int a[4]
	template <class T>                  
	struct remove_all_extents {
		typedef T value;
	};

	template<class T>                           //特化1  如果 remove_all_extents<a[]> 则内部调用去[]版本的  这样就能取得数组内元素的类型了
	struct remove_all_extents<T[]> {      
		typedef typename remove_all_extents<T>::value value;
	};

	template<class T,size_t N>
	struct remove_all_extents<T[N]> {      //特化2      如果 remove_all_extents<a[3]> 则内部调用去[]版本的
		typedef typename remove_all_extents<T>::value value;
	};

	//*****************************************
	//std::decay 实现退化(去掉const、左右引用修饰符；数组->指针，函数->指针)
	// std::decay<decltype(a)>::type
	template <class T1>                         //去const 泛化
	struct remove_const {
		using type = T1;
	};

	template <class T1>
	struct remove_const<const T1> {          //特化
		using type = T1;
	};
	//****************************************
	template <class T1>                        //去引用 泛化
	struct remove_reference {
		using type = T1;
	};

	template <class T1>
	struct remove_reference<T1&> {           //特化
		using type = T1;
	};

	template <class T1>
	struct remove_reference<T1&&> {           //特化
		using type = T1;
	};
	//std::decay
	template<class T1> //泛化版---去引用、const  必须先去引用后去const
	struct decay:remove_const<remove_reference<T1>>{};

	template<class T1,size_t size>  //特化     数组--->指针
	struct decay<T1[size]> {
		using type = T1*;
	};
	                               //特化     函数->函数指针  T1---函数返回值类型 ...args为可变参 比如func(int a,int b, string c)在作为类某班参数时可以
	                               // func(args)...这种形式表达  
	template <class T1,class...args> //  auto p =decay<func(1,2,"ss")>::type    ------p是func的函数指针
	struct decay<T1(args...)> {
		using type = T1 * (args...);
	};
}

#endif


