#include "GameFrameWork.h"
#include <math.h>
#include <stdio.h>
#include "SceneManager.h"
#include "ResManager.h"

GameFrameWork::GameFrameWork()
{
	m_LastTime = std::chrono::system_clock::now();
}

GameFrameWork::~GameFrameWork()
{
}

void GameFrameWork::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	HDC hdc = GetDC(hWnd);

	ResManager::getInst()->init(hdc);
	//씬매니저 초기화
	SceneManager::getSingleton()->init(hdc, m_hWnd);

	/*m_hMemDC[0] = CreateCompatibleDC(hdc);
	m_hBitmap[0] = CreateCompatibleBitmap(hdc, 1024, 768);
	m_hOld[0] = (HBITMAP)SelectObject(m_hMemDC[0], m_hBitmap[0]);

	m_hMemDC[1] = CreateCompatibleDC(m_hMemDC[0]);
	m_hBitmap[1] = (HBITMAP)LoadImage(NULL, "back.bmp", IMAGE_BITMAP, 0, 0
		, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	m_hOld[1] = (HBITMAP)SelectObject(m_hMemDC[1], m_hBitmap[1]);

	m_hMemDC[2] = CreateCompatibleDC(m_hMemDC[0]);
	m_hBitmap[2] = (HBITMAP)LoadImage(NULL, "char.bmp", IMAGE_BITMAP, 0, 0
		, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	m_hOld[2] = (HBITMAP)SelectObject(m_hMemDC[2], m_hBitmap[2]);
	ReleaseDC(hWnd, hdc);*/
	ReleaseDC(hWnd, hdc);
}


void GameFrameWork::Release()
{
	for (int i = 0; i < 3; i++)
	{
		SelectObject(m_hMemDC[i], m_hOld[i]);
		DeleteObject(m_hBitmap[i]);
		DeleteDC(m_hMemDC[i]);
	}
	SceneManager::getSingleton()->freeInst();
}

#define PI 3.141592f

void GameFrameWork::Update()
{
	
	if (GetKeyState(VK_LBUTTON) & 0x8000)
	{
		POINT pt;
		GetCursorPos(&pt);

		ScreenToClient(m_hWnd, &pt);
	}

	std::chrono::duration<float> sec = std::chrono::system_clock::now() - m_LastTime;
	if (sec.count() < (1 / FPS))
		return;

	m_fElapseTime = sec.count();
	m_LastTime = std::chrono::system_clock::now();

	SceneManager::getSingleton()->update();

	OperateInput();

	Render();
}

void GameFrameWork::OperateInput()
{
	if (GetKeyState(VK_LEFT) & 0x8000)
		Player_x -= 500 * m_fElapseTime;
	if (GetKeyState(VK_RIGHT) & 0x8000)
		Player_x += 500 * m_fElapseTime;
	if (GetKeyState(VK_UP) & 0x8000)
		Player_y -= 500 * m_fElapseTime;
	if (GetKeyState(VK_DOWN) & 0x8000)
		Player_y += 500 * m_fElapseTime;

	if (GetKeyState(VK_SPACE) & 0x8000)
	{
		if (m_bJump == false)
		{
			// 키 다운
			m_bJump = true;

		}
		else
		{
			//누르고 있을때
		}
	}
	else
	{
		if (m_bJump)
		{
			//키 업
		}
		m_bJump = false;

	}
		
}

void GameFrameWork::Render()
{
	HDC hdc = GetDC(m_hWnd);

	SceneManager::getSingleton()->render(hdc);

	ReleaseDC(m_hWnd, hdc);
}
