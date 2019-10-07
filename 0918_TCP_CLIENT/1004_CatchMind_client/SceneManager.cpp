#include "SceneManager.h"
#include "Scene.h"
#include "ResManager.h"
#include "LobbyScene.h"

SceneManager::SceneManager()
{
}


SceneManager::~SceneManager()
{
}

void SceneManager::init(HDC _hdc, HWND _hWnd, SOCKET _sock)
{
	//≥◊∆Æøˆ≈© 	
	m_iIndex = 0;
	m_sock = _sock;

	m_hWnd = _hWnd;
	//m_pNowScene = new MainScene();
	m_pNowScene = new LobbyScene();
	m_bGameEnd = false;
	m_bSceneChage = false;
	m_eNowSceneState = SCENE_LOBBY;
	m_pNowScene->init(_hdc, _hWnd, _sock);
}

void SceneManager::update()
{
	std::chrono::duration<float> sec = std::chrono::system_clock::now() - m_LastTime;
	if (sec.count() < (1 / FPS))
		return;

	m_fElapseTime = sec.count();
	m_LastTime = std::chrono::system_clock::now();


	m_pNowScene->update();
	InvalidateRect(m_hWnd, NULL, FALSE);
}

void SceneManager::render(HDC _hdc)
{
	m_pNowScene->render(_hdc);
}

void SceneManager::input(UINT _iMessage, WPARAM _wParam)
{
	m_pNowScene->input(_iMessage, _wParam);
	//InvalidateRect(m_hWnd, NULL, FALSE);
}

void SceneManager::freeInst()
{
	if (m_pNowScene != NULL)
	{
		m_pNowScene->release(); // æ¿¿Ã ∆ƒ±´µ«±‚ ¿¸ø° «ÿ¥Áæ¿¿ª «“¥Á«ÿ¡¶«ÿ¡‹
		delete m_pNowScene;
		m_pNowScene = NULL;
	}
}

void SceneManager::sceneChange(SCENE_STATE _state)
{
	if (m_pNowScene != NULL)
	{
		m_pNowScene->release();           // æ¿ «ÿ¡¶
		delete m_pNowScene;
		m_pNowScene = NULL;
	}
	switch (_state)
	{
	case SCENE_LOGIN:
		break;
	case SCENE_LOBBY:
		break;
	case SCENE_GAME_ROOM:
		//m_pNowScene = new MainScene;
		//m_eNowSceneState = SCENE_GAME_ROOM;
		break;
	default:
		break;
	}
	m_pNowScene->init(GetDC(m_hWnd) ,m_hWnd , m_sock);           // æ¿ √ ±‚»≠
	m_bSceneChage = false;
}

void SceneManager::ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void SceneManager::ProcessPacket(char * szBuf, int len)
{
	PACKET_HEADER header;

	memcpy(&header, szBuf, sizeof(header));

	switch (header.wIndex)
	{
	case PACKET_INDEX_LOGIN_RET:
	{
		PACKET_LOGIN_RET packet;
		memcpy(&packet, szBuf, header.wLen);

		m_iIndex = packet.iIndex;
	}
	break;

	case PACKET_INDEX_USER_LOBBY:
	{
		PACKET_USER_LOBBY packet;
		memcpy(&packet, szBuf, header.wLen);

		for (auto iter = m_mapPlayer.begin(); iter != m_mapPlayer.end(); iter++)
		{
			delete iter->second;
		}
		m_mapPlayer.clear();

		for (int i = 0; i < packet.wCount; i++)
		{
			Player* pNew = new Player();
			pNew->sock = packet.data[i].iIndex;
			m_mapPlayer.insert(make_pair(packet.data[i].iIndex, pNew));
		}

		((LobbyScene*)m_pNowScene)->clearLobbyInfo();
		char a[128];
		for (int i = 0; i < m_mapPlayer.size(); i++)
		{
			itoa(packet.data[i].iIndex, a, 10);
			((LobbyScene*)m_pNowScene)->setLobbyInfo(string(a));
		}
		((LobbyScene*)m_pNowScene)->setLobbyList();
	}
		break;
	case PACKET_INDEX_USER_DATA:
	{
		PACKET_USER_DATA_H packet;
		memcpy(&packet, szBuf, header.wLen);

		for (auto iter = m_mapPlayer.begin(); iter != m_mapPlayer.end(); iter++)
		{
			delete iter->second;
		}
		m_mapPlayer.clear();

		for (int i = 0; i < packet.wCount; i++)
		{
			Player* pNew = new Player();
			int a = packet.data[i].iIndex;
			pNew->stone = (WHAT_BLOCK_STATE)packet.data[i].stone;
			m_mapPlayer.insert(make_pair(packet.data[i].iIndex, pNew));

		}
		//((MainScene*)m_pNowScene)->getBlockMG()->setStone(m_mapPlayer[m_iIndex]->stone);
	}
	break;
	case PACKET_INDEX_SEND_POS:
	{
		PACKET_SEND_POS packet;
		memcpy(&packet, szBuf, header.wLen);

		//((MainScene*)m_pNowScene)->recvPos(
		//	packet.data.wY,
		//	packet.data.wX,
		//	m_mapPlayer[packet.data.iIndex]->stone
		//	/*m_mapPlayer[packet.data.iIndex]->y,
		//	m_mapPlayer[packet.data.iIndex]->x,
		//	m_mapPlayer[packet.data.iIndex]->stone*/
		//);

		//m_mapPlayer[packet.data.iIndex]->x = packet.data.wX;
		//m_mapPlayer[packet.data.iIndex]->y = packet.data.wY;
	}
	break;
	}
}

void SceneManager::sendPos()
{
	PACKET_SEND_POS packet;
	packet.header.wIndex = PACKET_INDEX_SEND_POS;
	packet.header.wLen = sizeof(packet);
	packet.data.iIndex = m_iIndex;
	packet.data.wX = m_mapPlayer[m_iIndex]->x;
	packet.data.wY = m_mapPlayer[m_iIndex]->y;
	send(m_sock, (const char*)&packet, sizeof(packet), 0);
	//send(g_sock, (const char*)&packet, sizeof(packet), 0);
}