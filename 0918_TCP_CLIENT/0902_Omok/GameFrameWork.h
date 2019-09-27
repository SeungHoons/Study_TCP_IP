#pragma once
#include"Global.h"


enum STATE
{
	IDLE,
	MOVE,
	JUMP
};

struct SERVER_INFO
{
	int len;
	char szBuf[512];
};

class GameFrameWork
{
private:
	//네트워크
	SOCKET		m_socket;
	SERVER_INFO m_byteStream;


	//클라
	HWND		m_hWnd;
	chrono::system_clock::time_point m_LastTime;
	float		m_fElapseTime;

	float		Player_x;
	float		Player_y;

	bool		m_bJump;
	//수학적으로
	STATE		m_eState;
	float		m_fJumpX;
	float		m_fJumpY;
	float		m_fCurJumpTime;

	//물리적
	float		m_fVectorX;
	float		m_fVectorY;
public:
	GameFrameWork();
	~GameFrameWork();

	void Init(HWND hWnd ,SOCKET _sock);
	void Release();
	void Update();
	void OperateInput();
	void Render();
	void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ProcessPacket(char* szBuf, int len);
};

