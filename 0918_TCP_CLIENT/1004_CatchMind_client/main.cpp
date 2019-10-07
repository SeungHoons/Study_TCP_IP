#include "Global.h"
#include "SceneManager.h"
using namespace std;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE g_hInst;
HWND hWnd;
LPCTSTR lpszClass = TEXT("Hello World!!");


#define WM_SOCKET (WM_USER+1)
SOCKET g_sock;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET)
	{
		//cout << "err on socket" << endl;
		return -1;
	}

	SceneManager::getSingleton()->init(GetDC(hWnd), hWnd, g_sock);
	//g_GameFrame.Init(hWnd, g_sock);

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int retval = connect(g_sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		//cout << "err on connect" << endl;
		return -1;
	}

	retval = WSAAsyncSelect(g_sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
	if (retval == SOCKET_ERROR)
	{
		return -1;
	}

	while (true)
	{
		// 메시지큐에 메시지가 있으면 메시지 처리
		if (PeekMessage(&Message, NULL, 0U, 0U, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
				break;

			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{
			SceneManager::getSingleton()->update();
			//g_GameFrame.Update();
		}
	}

	SceneManager::getSingleton()->freeInst();
	//g_GameFrame.Release();

	closesocket(g_sock);
	WSACleanup();

	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_SOCKET:
		SceneManager::getSingleton()->ProcessSocketMessage(hWnd, iMessage, wParam, lParam);
		//g_GameFrame.ProcessSocketMessage(hWnd, iMessage, wParam, lParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}