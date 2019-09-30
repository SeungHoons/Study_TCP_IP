#pragma once
#include "Global.h"
#include "Scene.h"
class LobbyScene :
	public Scene
{
private:
	HWND m_hWnd;
	RECT m_editRect;
	HINSTANCE m_inst;
	vector<string> m_lobbyMember;
	string m_strStaticBox;
public:
	LobbyScene();
	~LobbyScene();

	virtual void init(HDC _hdc, HWND _hWnd, SOCKET _sock);
	virtual void input(UINT _iMessage, WPARAM _wParam);
	virtual void update();
	virtual void render(HDC _hdc);
	virtual void release();

	void setInstance(HINSTANCE _inst);

	//로비 맴버 목록 창
	void setLobbyInfo(string _str);
	void clearLobbyInfo();
	void setLobbyList();
};

