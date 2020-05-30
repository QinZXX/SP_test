#include"sp.h"
//���⻹��swap(p,q)��p.swap(q)�����Ľ���ָ��Ĳ���
//make_shared<T>(args) ����һ������ָ�����ָ��̬�����T�����ò���args��ʼ���˶���
//shared_ptr<T>p(q) p��q�Ŀ������˲��������q�еļ�������q�е�ָ�������ת����T*�������Ӧ��ת�����캯����
//p=q ��������ָ�룬���ڲ���ָ������໥ת�����˲�����ݼ�p�����ü���������q�����ü�������p�ļ�����Ϊ0 ���ͷ��ڴ�
//p.unique() ��p�ļ���Ϊ1���򷵻�true�����򷵻�false
//p.use_count() ������p������������ָ������

template<class T>
SP<T>::SP(T* tmp) : _ptr(tmp), count(1) {} //���캯�����������1;Ĭ�ϳ�ʼ������һ����ָ��

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
SP<T>::SP(SP<T>& tmp) :_ptr(tmp._ptr) { //��ν�Ŀ������캯��  ���ﲻ��const 
		(tmp->count)++;
		count = tmp.count;
	}

template<class T>
SP<T>& SP<T>::operator=(SP<T>& tmp) { //���Ƹ�ֵ����������
	if (_ptr != tmp._ptr) {	//�ų��Լ����Լ���ֵ�Ŀ���
		//��Ҫ�ж�ԭ���Ŀռ��Ƿ���Ҫ�ͷ�   �����µ�ֵ�󣬼����ݼ�����tmp�ļ�������
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
