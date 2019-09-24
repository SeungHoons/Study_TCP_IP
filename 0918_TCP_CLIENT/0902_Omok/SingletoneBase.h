#pragma once

template <typename T>
class singletonBase
{
protected:
	//싱글톤 인스턴스 선언
	static T* singleton;

	singletonBase() {};
	~singletonBase() {};

public:
	//싱글톤 객체 가져오기
	static T* getSingleton();
	void releaseSingleton();
};
//싱글톤 인스턴스선언
template <typename T>
T* singletonBase<T>::singleton = nullptr;

//싱글톤 객체 가져오기
template <typename T>
T* singletonBase<T>::getSingleton()
{//싱글톤 객체가 없으면 새로 만들자
	if (!singleton)singleton = new T;

	return singleton;
}
template <typename T>
void singletonBase<T>::releaseSingleton()
{
	//싱글톤 객체가 있다면 메모리에서 해제하자.
	if (singleton)
	{
		delete singleton;
		singleton = nullptr;
	}
}
