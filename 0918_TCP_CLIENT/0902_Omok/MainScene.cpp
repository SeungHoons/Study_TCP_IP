#include "MainScene.h"
#include "BlockManager.h"


MainScene::MainScene()
{
}


MainScene::~MainScene()
{
}


void MainScene::init(HDC _hdc, HWND _hWnd)
{
	m_hWnd = _hWnd;
	m_pBlockManager = new BlockManager();
	m_pBlockManager->init();
	m_backBuffer = CreateCompatibleDC(_hdc);
	m_hBitmap = CreateCompatibleBitmap(_hdc, 1024, 768);
	m_hOld = (HBITMAP)SelectObject(m_backBuffer, m_hBitmap);
	m_clicked = false;
}
void MainScene::input(UINT _iMessage, WPARAM _wParam)
{

}

void MainScene::update()
{
	/*if ((GetKeyState(VK_LBUTTON) & 0x0001) && m_clicked)
	{
		m_clicked = false;
	}*/
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x0001) && m_clicked)
	{
		m_clicked = false;
	}
	else if ((GetKeyState(VK_LBUTTON) & 0x8000) && !m_clicked)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(m_hWnd, &pt);

		m_pBlockManager->checkCollition(pt);
		m_clicked = true;
	}
	
}

void MainScene::render(HDC _hdc)
{
	m_pBlockManager->render(m_backBuffer);

	BitBlt(_hdc, 0, 0, 1024, 768, m_backBuffer, 0, 0, SRCCOPY);
}
void MainScene::release()
{
	SelectObject(m_backBuffer, m_hOld);
	DeleteObject(m_hBitmap);
	DeleteDC(m_backBuffer);
}

