#pragma once
#include <Windows.h>
#include <string>

using namespace std;

class BitMap
{
private:

	HDC		m_hMemDC;
	HBITMAP m_hBitMap;
	HBITMAP m_hOldBitMap;
	int m_index;
	SIZE m_size;


public:
	BitMap();
	~BitMap();

	void init(HDC _hdc, string _str, int _index);
	void init(HDC _hdc, string _str);
	void render(HDC hdc, int x, int y);
	void render(HDC hdc, int x, int y, float multiply);
	void render(HDC hdc, int x, int y, int cx, int cy);
	void release();

	SIZE getSize() { return m_size; }
	int getIndex() { return m_index; }
};
