#pragma once
#include "Scene.h"

class BlockManager;
class MainScene :
	public Scene
{
private:
	bool m_clicked;
	HWND m_hWnd;
	HDC				m_backBuffer;
	HBITMAP m_hBitmap;
	HBITMAP m_hOld;

	BlockManager* m_pBlockManager;
public:
	MainScene();
	~MainScene();

	virtual void init(HDC _hdc, HWND _hWnd);
	virtual void input(UINT _iMessage, WPARAM _wParam);
	virtual void update();
	virtual void render(HDC _hdc);
	virtual void release();
};

