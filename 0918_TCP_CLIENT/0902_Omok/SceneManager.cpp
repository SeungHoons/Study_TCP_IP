#include "SceneManager.h"
#include "Scene.h"
#include "MainScene.h"
#include "ResManager.h"


SceneManager::SceneManager()
{
}


SceneManager::~SceneManager()
{
}

void SceneManager::init(HDC _hdc, HWND _hWnd)
{
	//ResManager::getSingleton()->init(GetDC(_hWnd));

	//m_pNowScene = new MenuScene();		//�޴� ����� 
	m_hWnd = _hWnd;
	m_pNowScene = new MainScene();
	m_bGameEnd = false;
	m_bSceneChage = false;
	m_eNowSceneState = SCENE_STAGE_ONE;
	m_pNowScene->init(_hdc, _hWnd);
}

void SceneManager::update()
{
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
		m_pNowScene->release(); // ���� �ı��Ǳ� ���� �ش���� �Ҵ���������
		delete m_pNowScene;
		m_pNowScene = NULL;
	}
}

void SceneManager::sceneChange(SCENE_STATE _state)
{
	if (m_pNowScene != NULL)
	{
		m_pNowScene->release();           // �� ����
		delete m_pNowScene;
		m_pNowScene = NULL;
	}
	switch (_state)
	{
	case SCENE_MENU:
		break;
	case SCENE_STAGE_ONE:
		break;
	default:
		break;
	}
	m_pNowScene->init(GetDC(m_hWnd) ,m_hWnd);           // �� �ʱ�ȭ
	m_bSceneChage = false;
}
