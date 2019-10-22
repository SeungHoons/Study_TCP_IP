#include "LobbyScene.h"



LobbyScene::LobbyScene()
{
}


LobbyScene::~LobbyScene()
{
}

void LobbyScene::init(HDC _hdc, HWND _hWnd, SOCKET _sock)
{
	m_sock = _sock;

	m_lobbyChatCtrlInput =  CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
		ES_AUTOHSCROLL, 100, 600, 500, 25, _hWnd, (HMENU)1, m_inst, NULL);

	m_lobbyChatCtrlOutput =  CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE |  ES_LEFT 
		, 100, 450, 500, 150, _hWnd, (HMENU)2, m_inst, NULL);
	
	m_lobbyListContorl = CreateWindow(TEXT("static"), m_strStaticBox.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER
		, 700, 10, 170, 205, _hWnd, (HMENU)-1, m_inst, NULL);
}

void LobbyScene::input(UINT _iMessage, WPARAM _wParam)
{
}

void LobbyScene::update()
{
	if(GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		char tempChar[256];
		GetWindowText(m_lobbyChatCtrlInput, tempChar, 256);
		if(!strcmp(tempChar, "\0"))
		//if (tempChar == '\0')
		{
		}
		else
		{
			sendTxt(m_sock);
			m_strChatingLog += "\r\n";
			m_strChatingLog += tempChar;
			SetWindowText(m_lobbyChatCtrlInput, "");
			SetWindowText(m_lobbyChatCtrlOutput, m_strChatingLog.c_str());
			SendMessage(m_lobbyChatCtrlOutput, EM_SETSEL, 0, -1);
			SendMessage(m_lobbyChatCtrlOutput, EM_SETSEL, -1, -1);
			SendMessage(m_lobbyChatCtrlOutput, EM_SCROLLCARET, 0, 0);
			//SetScrollPos(m_lobbyChatCtrlOutput, SB_VERT, 500, TRUE);
		}
	}
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
	m_strStaticBox += "==현재 접속중인 인원==\n";
	for (int i = 0; i < m_lobbyMember.size(); i++)
	{
		m_strStaticBox += m_lobbyMember[i];
		m_strStaticBox += "\n";
	}
	SetWindowText(m_lobbyListContorl, m_strStaticBox.c_str());
}

void LobbyScene::sendTxt(SOCKET _sock)
{
	int totalsize;
	char tempBuf[BUFSIZE];
	char buf[BUFSIZE];
	GetWindowText(m_lobbyChatCtrlInput, tempBuf, BUFSIZE);

	totalsize = sizeof(int) + sizeof(PACKET_INDEX) + sizeof(int) + strlen(tempBuf);
	Packing(totalsize, buf, PACKET_INDEX::PACKET_INDEX_USER_CHAT, strlen(tempBuf), tempBuf);
	send(_sock, buf, totalsize, 0);

	//PACKET_CHAT_1 packetChat;
	////packetChat.header.wLen = sizeof(PACKET_HEADER) + sizeof(int) + sizeof(char[512]);
	//packetChat.header.wLen = sizeof(PACKET_CHAT_1);
	//GetWindowText(m_lobbyChatCtrlInput, packetChat.buf, 256);
	//packetChat.chatLen = strlen(packetChat.buf);
	//send(_sock, (const char*)&packetChat, sizeof(packetChat), 0);
}

void LobbyScene::recvTxt()
{
}

BOOL LobbyScene::Packing(int _totalsize, char* _buf, PACKET_INDEX _protocal, int _size, char* _data)
{
	ZeroMemory(_buf, sizeof(_buf));
	memcpy
	(
		_buf,/*buf*/
		(char*)&_totalsize,/*토탈사이즈*/
		sizeof(int)/*토탈사이즈 크기*/
	);

	memcpy
	(
		_buf + sizeof(int),/*buf+sizeof(int)*/
		(char*)&_protocal,/*프로토콜 값*/
		sizeof(PACKET_INDEX)/*프로토콜 크기*/
	);

	memcpy
	(
		_buf + sizeof(int) + sizeof(PACKET_INDEX),/*buf+sizeof(int)+sizeof(PROTOCAL)*/
		(char*)&_size,/*넣을문자길이*/
		sizeof(int)/*문자크기*/
	);

	memcpy
	(
		_buf + sizeof(int) + sizeof(PACKET_INDEX) + sizeof(int),/*buf+sizeof(int)+sizeof(PROTOCAL)+sizeof(int)*/
		_data,/*넣을 문자내용*/
		_size/*문자길이*/
	);
	return true;
}