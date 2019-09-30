#include "LobbyScene.h"



LobbyScene::LobbyScene()
{
}


LobbyScene::~LobbyScene()
{
}

void LobbyScene::init(HDC _hdc, HWND _hWnd, SOCKET _sock)
{

	CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
		ES_AUTOHSCROLL, 10, 10, 200, 25, _hWnd, (HMENU)1, m_inst, NULL);
	
	CreateWindow(TEXT("static"), m_strStaticBox.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER |
		ES_AUTOHSCROLL | ES_READONLY, 100, 110, 200, 205, _hWnd, (HMENU)2, m_inst, NULL);
}

void LobbyScene::input(UINT _iMessage, WPARAM _wParam)
{
}

void LobbyScene::update()
{

}

void LobbyScene::render(HDC _hdc)
{
}

void LobbyScene::release()
{
}

void LobbyScene::setInstance(HINSTANCE _inst)
{
	m_inst = _inst;
}

void LobbyScene::setLobbyInfo(string _str)
{
	m_lobbyMember.push_back(_str);
}

void LobbyScene::clearLobbyInfo()
{
	m_lobbyMember.clear();
}

void LobbyScene::setLobbyList()
{
	m_strStaticBox.clear();
	m_strStaticBox += "현재 접속중인 인원 :\n";
	for (int i = 0; i < m_lobbyMember.size(); i++)
	{
		m_strStaticBox += m_lobbyMember[i];
		m_strStaticBox += '\n';
	}
}
