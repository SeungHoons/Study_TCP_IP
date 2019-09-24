#pragma once
#include "Global.h"

class Scene
{
public:
	Scene();
	~Scene();

	virtual void init(HDC _hdc, HWND _hWnd);
	virtual void input(UINT _iMessage, WPARAM _wParam);
	virtual void update();
	virtual void render(HDC _hdc);
	virtual void release();
};

