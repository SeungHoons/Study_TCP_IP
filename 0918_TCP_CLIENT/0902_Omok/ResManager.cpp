#include "ResManager.h"
#include <string>
#include "BitMap.h"
#include "macro.h"

ResManager* ResManager::Inst = nullptr;

ResManager::ResManager()
{
}


ResManager::~ResManager()
{
}


void ResManager::init(HDC _hdc)
{
	string str;
	char tempChar[128];

	//담을 백터 초기화
	for (int i = 0; i < RES_FILE_END; i++)
	{
		m_pVecBitmap.push_back(new BitMap());
	}
	//m_pVecBitmap.resize(FILE_NAME_END); //공간만 잡음

	m_pVecBitmap[RES_FILE_BLACK_STONE]->init(_hdc, "./Resource/Black_Stone.bmp");
	m_pVecBitmap[RES_FILE_WHITE_STONE]->init(_hdc, "./Resource/White_Stone.bmp");
	m_pVecBitmap[RES_FILE_BACK_GROUND]->init(_hdc, "./Resource/BackGround.bmp");
	////블록 불러오기
	//for (int i = 0; i <= FILE_BLOCK14; i++)
	//{
	//	str = "./Resource/block";
	//	//memset(&tempChar[0], 0, 2);
	//	itoa(i, tempChar, 10);
	//	if (i < 10)
	//	{
	//		str = str + "0" /*+ tempStr.c_str()*/;
	//		str = str + tempChar;
	//		str = str + ".bmp";
	//	}
	//	else
	//	{
	//		str = str + tempChar;
	//		str = str + ".bmp";
	//	}
	//	m_pVecBitmap[i]->init(_hdc, str.c_str(), i);
	//}
}

BitMap * ResManager::getBitMap(RES_FILE_NAME fileName)
{
	return m_pVecBitmap[fileName];
}

BitMap * ResManager::getLoadBitMap(int _index)
{
	if (m_vecMapLoad[_index] == -1)
		return nullptr;
	else
		return m_pVecBitmap[m_vecMapLoad[_index]];
}

void ResManager::release()
{
	for (int i = 0; i < m_pVecBitmap.size(); i++)
	{
		SAFE_DELETE(m_pVecBitmap[i]);
	}
	if (Inst)
	{
		delete Inst;
		Inst = nullptr;
	}
}


//사용할일 없음
void ResManager::fileLoad(HWND _hWnd)
{
	char str[256];

	string dd = "./Map/stage1.txt";

	std::ifstream ofname(dd);

	auto iter = m_vecMapLoad.begin();

	for (int i = 0; i < 13; i++)
	{
		for (int j = 0; j < 13; j++)
		{
			ofname >> (*iter);
			iter++;
		}
	}
	ofname.close();
}

void ResManager::fileSave(HWND _hWnd)
{
	OPENFILENAME OFN;

	char str[256];
	char lpstrFile[256] = "";

	memset(&OFN, 0, sizeof(OPENFILENAME));

	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = _hWnd;
	OFN.lpstrFilter = "Every File(*.*)\0*.*\0 TextFile\0*.txt*\0.ini\0";
	OFN.lpstrFile = lpstrFile;
	OFN.nMaxFile = 256;
	OFN.lpstrInitialDir = "c:\\";
	if (GetSaveFileName(&OFN) != 0)
	{
		sprintf(str, "%s 파일을 저장했습니다. ", OFN.lpstrFile);
		MessageBox(_hWnd, str, "파일열기 성공", MB_OK);
	}
	else
	{
		MessageBox(_hWnd, ("파일 오픈 에러입니다."), ("파일열기실패"), MB_OK);
		return;
	}
}
