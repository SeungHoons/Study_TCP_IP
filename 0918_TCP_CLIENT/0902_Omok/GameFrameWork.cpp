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

void GameFrameWork::Init(HWND hWnd, SOCKET _sock)
{
	m_hWnd = hWnd;
	HDC hdc = GetDC(hWnd);

	//네트워크
	m_socket = _sock;


	ResManager::getInst()->init(hdc);
	//씬매니저 초기화
	SceneManager::getSingleton()->init(hdc, m_hWnd, _sock);

	ReleaseDC(hWnd, hdc);
}


void GameFrameWork::Release()
{
	//SceneManager::getSingleton()->freeInst();
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
	//if (GetKeyState(VK_LEFT) & 0x8000)
	//	Player_x -= 500 * m_fElapseTime;
	//if (GetKeyState(VK_RIGHT) & 0x8000)
	//	Player_x += 500 * m_fElapseTime;
	//if (GetKeyState(VK_UP) & 0x8000)
	//	Player_y -= 500 * m_fElapseTime;
	//if (GetKeyState(VK_DOWN) & 0x8000)
	//	Player_y += 500 * m_fElapseTime;
	//if (GetKeyState(VK_SPACE) & 0x8000)
	//{
	//	if (m_bJump == false)
	//	{
	//		// 키 다운
	//		m_bJump = true;
	//	}
	//	else
	//	{
	//		//누르고 있을때
	//	}
	//}
	//else
	//{
	//	if (m_bJump)
	//	{
	//		//키 업
	//	}
	//	m_bJump = false;
	//}
		
}

void GameFrameWork::Render()
{
	HDC hdc = GetDC(m_hWnd);

	SceneManager::getSingleton()->render(hdc);

	ReleaseDC(m_hWnd, hdc);
}

void GameFrameWork::ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen = 0;
	int retval = 0;


	if (WSAGETSELECTERROR(lParam))
	{
		int err_code = WSAGETSELECTERROR(lParam);
		//err_display(WSAGETSELECTERROR(lParam));
		return;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:
	{
		char szBuf[BUFSIZE];

		retval = recv(wParam, szBuf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//cout << "err on recv!!" << endl;
			}
		}

		if (retval > 0)
		{
			memcpy(&m_byteStream.szBuf[m_byteStream.len], szBuf, retval);
			m_byteStream.len += retval;
			retval = 0;
		}

		//if (m_byteStream.len < sizeof(PACKET_HEADER))
		//{
		//	//return false;
		//}
		//else
		//{

		//}

		//PACKET_HEADER header;
		//memcpy(&header, pUser->szBuf, sizeof(header));

		//if (pUser->len < header.wLen)
		//	return false;


		ProcessPacket(szBuf, retval);
	}
	break;
	case FD_CLOSE:
		closesocket(wParam);
		break;
	}
}

void GameFrameWork::ProcessPacket(char * szBuf, int len)
{
	SceneManager::getSingleton()->ProcessPacket(szBuf, len);


	//memcpy(&pUser->szBuf, &pUser->szBuf[header.wLen], pUser->len - header.wLen);
	//pUser->len -= header.wLen;

}
