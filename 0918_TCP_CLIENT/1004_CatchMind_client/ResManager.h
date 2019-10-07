#pragma once
#include <Windows.h>
#include <vector>
#include<fstream>

using namespace std;

enum RES_FILE_NAME
{
	RES_FILE_BLACK_STONE,
	RES_FILE_WHITE_STONE,
	RES_FILE_BACK_GROUND,
	RES_FILE_END
};

class BitMap;
class ResManager
{
private:
	static ResManager* Inst;
	BitMap* m_image;
	vector<BitMap*> m_pVecBitmap;
	vector<int> m_vecMapLoad;
	ResManager();
	~ResManager();
public:
	static ResManager* getInst()
	{
		if (!Inst)Inst = new ResManager;
		return Inst;
	}
	void release();


	void init(HDC _hdc);
	BitMap* getBitMap(RES_FILE_NAME fileName);
	BitMap* getLoadBitMap(int _index);

	void fileLoad(HWND _hWnd);
	void fileSave(HWND _hWnd);
};

