#pragma once
#ifndef SP_H
#define SP_H
template<class T>
class SP {		//ģ��ʵ��shared_ptr
public:
	SP(T* tmp = nullptr); //Ĭ�Ϲ��캯��
	~SP();//����

	T& operator*();//�㶮��
	T* operator->();//�㶮��

	SP(SP<T>& tmp);//�㶮��

	SP<T>& operator=(SP<T>& tmp);//�㶮��

	void swap(SP<T>& tmp);//�㶮��
	//friend void swap(SP<T>& tmp1,SP<T>& tmp2);

	int use_count();//���ؼ���

private:
	T* _ptr;
	int count;
};

#endif // !SP_H
