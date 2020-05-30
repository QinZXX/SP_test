#pragma once
#ifndef SP_H
#define SP_H
template<class T>
class SP {		//模拟实现shared_ptr
public:
	SP(T* tmp = nullptr); //默认构造函数
	~SP();//析构

	T& operator*();//你懂得
	T* operator->();//你懂得

	SP(SP<T>& tmp);//你懂得

	SP<T>& operator=(SP<T>& tmp);//你懂得

	void swap(SP<T>& tmp);//你懂得
	//friend void swap(SP<T>& tmp1,SP<T>& tmp2);

	int use_count();//返回计数

private:
	T* _ptr;
	int count;
};

#endif // !SP_H
