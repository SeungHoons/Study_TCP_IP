#pragma once
#include "Global.h"
#include "Scene.h"
class LobbyScene :
	public Scene
{
private:
	//��Ʈ��ũ
	SOCKET m_sock;

	HWND m_hWnd;
	RECT m_editRect;
	HINSTANCE m_inst;
	vector<string> m_lobbyMember;

	//���� �ο� ���
	string m_strStaticBox;
	HWND m_lobbyListContorl;
	//ê
	string m_strChatingLog;
	HWND m_lobbyChatCtrlInput;
	HWND m_lobbyChatCtrlOutput;
public:
	LobbyScene();
	~LobbyScene();

	virtual void init(HDC _hdc, HWND _hWnd, SOCKET _sock);
	virtual void input(UINT _iMessage, WPARAM _wParam);
	virtual void update();
	virtual void render(HDC _hdc);
	virtual void release();

	void setInstance(HINSTANCE _inst);

	//�κ� �ɹ� ��� â
	void setLobbyInfo(string _str);
	void clearLobbyInfo();
	void setLobbyList();

	void sendTxt(SOCKET _sock);
	void recvTxt();
};

