#include"sp.h"
//另外还有swap(p,q)和p.swap(q)这样的交换指针的操作
//make_shared<T>(args) 返回一个智能指针对象，指向动态分配的T对象，用参数args初始化此对象
//shared_ptr<T>p(q) p是q的拷贝，此操作会递增q中的计数器。q中的指针必须能转换成T*（定义对应的转换构造函数）
//p=q 两个智能指针，且内部的指针可以相互转换。此操作会递减p的引用计数，递增q的引用计数。若p的计数变为0 则释放内存
//p.unique() 若p的计数为1，则返回true，否则返回false
//p.use_count() 返回与p共享对象的智能指针数量

template<class T>
SP<T>::SP(T* tmp) : _ptr(tmp), count(1) {} //构造函数，计数变成1;默认初始化保存一个空指针

template<class T>
SP<T>::~SP() {
	--count;
	if (count == 0) {
		delete _ptr;
	}
}

template<class T>
T& SP<T>::operator*() {
	return *_ptr;
}

template<class T>
T* SP<T>::operator->() {
	return _ptr;
}

template<class T>
SP<T>::SP(SP<T>& tmp) :_ptr(tmp._ptr) { //所谓的拷贝构造函数  这里不用const 
		(tmp->count)++;
		count = tmp.count;
	}

template<class T>
SP<T>& SP<T>::operator=(SP<T>& tmp) { //复制赋值操作符重载
	if (_ptr != tmp._ptr) {	//排除自己给自己赋值的可能
		//先要判断原来的空间是否需要释放   被赋新的值后，计数递减；而tmp的计数递增
		count--;
		tmp.count++;
		if (count == 0) {
			delete _ptr;
		}
		_ptr = tmp._ptr;
	}
	return *this;
}

template<class T>
void SP<T>::swap(SP<T>& tmp) {
		T* tmp2 = new T;
		tmp2 = _ptr;
		_ptr = tmp._ptr;
		tmp._ptr = tmp2;
		delete tmp2;

		int tmpCount;
		tmpCount = count;
		count = tmp.count;
		tmp.count = tmpCount;
	}

//template<class T>
//void swap(SP<T>& tmp1, SP<T>& tmp2) {
//	
//}

template<class T>
int SP<T>::use_count() {
	return count;
}
