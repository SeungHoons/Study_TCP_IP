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
	//��Ʈ��ũ
	SOCKET		m_socket;
	SERVER_INFO m_byteStream;


	//Ŭ��
	HWND		m_hWnd;
	chrono::system_clock::time_point m_LastTime;
	float		m_fElapseTime;

	float		Player_x;
	float		Player_y;

	bool		m_bJump;
	//����������
	STATE		m_eState;
	float		m_fJumpX;
	float		m_fJumpY;
	float		m_fCurJumpTime;

	//������
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

