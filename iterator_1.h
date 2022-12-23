#ifndef ITERTOR_MUSTL
#define ITERTOR_MUSTL

#include "type_traits_1.h"
#include <cstddef>
namespace mustl {
   //定义五种迭代器类
	//随机访问迭代器∈双向迭代器∈前向迭代器∈输入迭代器 
	//输出迭代器
	struct input_iterator_tag{};
	struct output_iterator_tag{};
	struct forward_iterator_tag :public input_iterator_tag{};
	struct bidirectional_iterator_tag :public forward_iterator_tag{};
	struct random_access_iterator_tag : public bidirectional_iterator_tag{};

	//iterator模板 内部包含五元素    迭代器类型、迭代器内部元素类型、迭代器之间的距离，指向元素的指针，指向元素的引用
	template<class Category,class T,class Distance =ptrdiff_t,class Pointer =T*,class Reference =T&>
	struct iterator {
		typedef Category                             iterator_category;
		typedef T                                    value_type;
		typedef Pointer                              pointer;
		typedef Reference                            reference;
		typedef Distance                             difference_type; //指示两个迭代器之间的距离
	};

	//判断是否存在迭代器？
	template<class T>
	class has_iterator_cat {
	  private:
		  struct two {  //定义一个私有 struct 供公有函数调用 大小为两个char的大小
			  char a;
			  char b;
		   };

		  //SFINAE技巧 匹配失败不是错误 会去寻找合适的模板  如果类型U为迭代器类型 那么就返回值大小为一个char  else 返回值为two---两个char大小
		  template<class U>
		  static two test(...) {//静态成员函数不能再类内定义
			  return two;
		  }
		  
		  template <class U>
		  static char test(typename U::iterator_category* p = nullptr) {
			                char c;
			                return c;
		  }
	  public:
		  static const bool value = sizeof(test<T>(0)) == sizeof(char); //静态成员调用静态函数 静态成员可以赋值 不能在类内初始化
	};


	///泛化      如果bool=true就重定义属性
	template <class Iterator,bool>   //
	struct iterator_traits_impl {};

	template<class Iterator>
	struct iterator_traits_impl<Iterator, true> {
		//类型重定义 
		typedef typename Iterator::iterator_category iterator_category;
		typedef typename Iterator::value_type        value_type;
		typedef typename Iterator::pointer           pointer;
		typedef typename Iterator::reference         reference;
		typedef typename Iterator::difference_type   difference_type;
	};

	//_helper   利用之前的impl做基类 实现嵌套
	//bool为true时特化 然后继承impl 当Iterator可以隐式转换为两个基类(输入iter、输出iter)中的一个时，impl的bool为true 此时调用该特化版本
	template <class Iterator, bool>
	struct iterator_traits_helper {};

	template<class Iterator>
	struct iterator_traits_helper<Iterator, true> :public  
	iterator_traits_impl<Iterator,std::is_convertible<typename Iterator::iterator_category,input_iterator_tag>::value ||
		                          std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::value>
	{};

	//利用之前的萃取迭代器类型  
	template <class Iterator>    //先判断是否有迭代器？ 有(true)--->iterator_traits_helper特化版本
	struct iterator_traits:public iterator_traits_helper<Iterator,has_iterator_cat<Iterator>::value>{};

	//针对原生指针 以及const指针的特化版本
// 针对原生指针的偏特化版本
	template <class T>
	struct iterator_traits<T*>
	{
		typedef random_access_iterator_tag           iterator_category;
		typedef T                                    value_type;
		typedef T* pointer;
		typedef T& reference;
		typedef ptrdiff_t                            difference_type;
	};

	template <class T>
	struct iterator_traits<const T*>
	{
		typedef random_access_iterator_tag           iterator_category;
		typedef T                                    value_type;
		typedef const T* pointer;
		typedef const T& reference;
		typedef ptrdiff_t                            difference_type;
	};

	//判断T是否为U迭代器  1.先判断是否为迭代器 2.再判断T能否隐式转换为U迭代器
    template<class T,class U, bool =has_iterator_cat<iterator_traits<T>>::value>  //因为之前定义了iterator_traits的各种版本
	struct has_iterator_cat_of: public bool_constant<std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value>{};

	//has_oterator_Cat_of<T,U,true/false>


	template <class T,class U>
	struct has_iterator_cat_of<T,U,false>:public false_type{};

	template<class Iter>
	struct is_input_itertor :public has_iterator_cat_of<Iter, input_iterator_tag> {};

	template <class Iter>
	struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};

	template <class Iter>
	struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

	template <class Iter>
	struct is_bidirectional_iterator : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

	template <class Iter>
	struct is_random_access_iterator : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

	//is_iterator 判断是否为输入或输出迭代器即可
	template<class Iter>
	struct is_iterator: public bool_constant<is_input_itertor<Iter>::value||is_output_iterator<Iter>::value>{};

	// 萃取某个迭代器的 category
	template <class Iterator>
	typename iterator_traits<Iterator>::iterator_category
		iterator_category(const Iterator& )
	{
		typedef typename iterator_traits<Iterator>::iterator_category Category;  //得返回一个类型对象
		return Category();
	}

	// 萃取某个迭代器的 distance_type
	template <class Iterator>
	typename iterator_traits<Iterator>::difference_type*   //返回值为指针类型 用static_cast强转
		distance_type(const Iterator& iter)
	{
		return static_cast<typename iterator_traits<Iterator>::difference_type*>(iter);
	}

	// 萃取某个迭代器的 value_type
	template <class Iterator>
	typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator&iter)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type*>(iter);
	}

	//distance  ---随机迭代器和输入迭代器之间算法不一样
	template <class InputIter>
	typename iterator_traits<InputIter>::different_type
	distance_range(InputIter first, InputIter end, input_iterator_tag) {
		typename iterator_traits<InputIterator>::difference_type n = 0;
		while (first != end)
		{
			++first;
			++n;
		}
		return n;
	}

	template<class RandomIter>
	typename iterator_traits<RandomIter>::different_type
	distance_range(RandomIter first, RandomIter end, random_access_iterator_tag) {
		typename iterator_traits<RandomIter>::difference_type n = 0;
		return end-first;
	}

	template <class Iter>
	typename iterator_traits<Iter>::different_type
	distance(Iter first, Iter end) {
		return distance_range(first, end, iterator_category(first));
	}

	// 以下函数用于让迭代器前进 n 个距离

	// advance 的 input_iterator_tag 的版本
	template <class InputIterator, class Distance>
	void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag)
	{
		while (n--)
			++i;
	}

	// advance 的 bidirectional_iterator_tag 的版本
	template <class BidirectionalIterator, class Distance>
	void advance_dispatch(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
	{
		if (n >= 0)
			while (n--)  ++i;
		else
			while (n++)  --i;
	}

	// advance 的 random_access_iterator_tag 的版本
	template <class RandomIter, class Distance>
	void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag)
	{
		i += n;
	}

	template <class InputIterator, class Distance>
	void advance(InputIterator& i, Distance n)
	{
		advance_dispatch(i, n, iterator_category(i));
	}

	template<class Iterator>
	class reverse_iterator {
	private:
		Iterator current; //当前正向的对象
	public:
		typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
		typedef typename iterator_traits<Iterator>::value_type        value_type;
		typedef typename iterator_traits<Iterator>::difference_type   difference_type;
		typedef typename iterator_traits<Iterator>::pointer           pointer;
		typedef typename iterator_traits<Iterator>::reference         reference;

		typedef Iterator                                              iterator_type;
		typedef reverse_iterator<Iterator>                            self;

		// 构造函数
		reverse_iterator() {}
		explicit reverse_iterator(iterator_type i) :current(i) {} //
		reverse_iterator(const self& rhs) :current(rhs.current) {}

		// 取出对应的正向迭代器
		iterator_type base() const
		{
			return current;
		}
		//重载*运算符 为current的前一个
		reference operator*()const {
			auto temp = current;
			return *(--temp);
		}
		//重载——》运算符      ++i返回引用 i++返回临时变量
		pointer operator->()const {
			return &(operator*());
		}

		// 前进(++)变为后退(--) ++i
		self& operator++()
		{
			--current;
			return *this;
		}

		self operator++(int) //i++
		{
			self temp = *this;
			--current;
			return temp;
		}
		// 后退(--)变为前进(++)
		self& operator--()
		{
			++current;
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			++current;
			return tmp;
		}

		//+= -=   返回引用
		self& operator+=(difference_type n) {
			current -= n;
			return *this;
		}
		self& operator-=(difference_type n) {
			current += n;
			return *this;
		}

		//+ - 返回临时变量
		self operator+(difference_type n)const {
			return self(current - n);
		}
		self operator-(difference_type n)const {
			return self(current + n);
		}

		//[] 返回引用 看起来是零食变量 其实不是
		reference operator[](difference_type n)const {
			return *((*this + n)); //-----》*((*(this+n)))
		}

		// 重载比较操作符  因为类中重载运算符第一个对象为this 因此只能传入一个参数 如果要实现两个参数的话 需要定义为全局作用域的友元函数
		template <class Iterator>
		friend bool operator==(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
		{
			return lhs.base() == rhs.base();
		}

		template <class Iterator>
		friend bool operator<(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
		{
			return rhs.base() < lhs.base();
		}

		template <class Iterator>
		friend bool operator!=(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
		{
			return !(lhs == rhs);
		}

		template <class Iterator>
		friend bool operator>(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
		{
			return rhs < lhs;
		}

		template <class Iterator>
		friend bool operator<=(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
		{
			return !(rhs < lhs);
		}

		template <class Iterator>
		friend bool operator>=(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
		{
			return !(lhs < rhs);
		}
	};
}
#endif
