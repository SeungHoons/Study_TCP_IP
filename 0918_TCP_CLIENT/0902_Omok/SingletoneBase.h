#pragma once

template <typename T>
class singletonBase
{
protected:
	//�̱��� �ν��Ͻ� ����
	static T* singleton;

	singletonBase() {};
	~singletonBase() {};

public:
	//�̱��� ��ü ��������
	static T* getSingleton();
	void releaseSingleton();
};
//�̱��� �ν��Ͻ�����
template <typename T>
T* singletonBase<T>::singleton = nullptr;

//�̱��� ��ü ��������
template <typename T>
T* singletonBase<T>::getSingleton()
{//�̱��� ��ü�� ������ ���� ������
	if (!singleton)singleton = new T;

	return singleton;
}
template <typename T>
void singletonBase<T>::releaseSingleton()
{
	//�̱��� ��ü�� �ִٸ� �޸𸮿��� ��������.
	if (singleton)
	{
		delete singleton;
		singleton = nullptr;
	}
}
