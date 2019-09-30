#pragma once
#include "Global.h"
#include "SingletoneBase.h"

class Scene;
class Player
{
public:
	int x;
	int y;
	WHAT_BLOCK_STATE stone;
	int sock;
};
class SceneManager : public singletonBase<SceneManager>
{
private:
	//네트워크
	int			m_iIndex;
	map<int, Player*> m_mapPlayer;
	SOCKET m_sock;

	HDC m_resouceMemDC;
	HWND m_hWnd;
	Scene* m_pNowScene;
	SCENE_STATE m_eNowSceneState;
	SCENE_STATE m_eNextScene;
	bool m_bSceneChage;			//씬이 변경될것인가
	bool m_bGameEnd;

public:
	SceneManager();
	~SceneManager();

	void init(HDC _hdc, HWND _hWnd, SOCKET _sock);
	void update();
	void render(HDC _hdc);
	void input(UINT _iMessage, WPARAM _wParam);

	void freeInst();
	void sceneChange(SCENE_STATE _state);

	void ProcessPacket(char * szBuf, int len);
	void sendPos();

	int getMyIndex() { return m_iIndex; }
	SOCKET getSock() { return m_sock; }


	inline void nextSceneChange(SCENE_STATE _state) // 씬 변경 예약함수
	{ 
		m_eNextScene = _state;
		m_bSceneChage = true;
	}
	inline SCENE_STATE getNextScene(void)   // 예약된 씬을 얻어옴
	{
		return m_eNextScene;
	}
	inline bool getSceneChange (void)    // 예약유무를 얻어옴
	{
		return m_bSceneChage;
	}
	inline void setGameEnd(bool _bEnd)      // 게임이 종료될 것인가?
	{
		m_bGameEnd = _bEnd;
	}
	inline bool getGameEnd(void)      // 게임이 종료되었는가?
	{
		return m_bGameEnd;
	}
	inline SCENE_STATE getNowState(void) // 현재 씬을 반환
	{
		return m_eNowSceneState;
	}
};

