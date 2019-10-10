#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\..\Common\PACKET_HEADER_CatchMind.h"

#define SERVERPORT 9000
#define BUFSIZE    512
//인구 정해놨는데 터질수도 있음
#define MAXUSER    50

//패킷의 상태
enum IOTYPE
{
	IO_READ,
	IO_WRITE
};

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
	IOTYPE iotype;

	//챗
	int MessageLen;
};

int NowUserNum = 0;
SOCKETINFO* User[MAXUSER];

SOCKET client_sock;
HANDLE hReadEvent, hWriteEvent;

// 비동기 입출력 시작과 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg);

//유저 소켓 추가
SOCKETINFO* AddSocketInfo(SOCKET _sock);
bool RemoveSocketInfo(SOCKETINFO *_ptr);

//데이터 확인
bool CheckRecv(SOCKETINFO* _ptr);

//읽기 상태
void IO_Read(SOCKETINFO* _info, DWORD _cbTransferred);

//


void err_quit(const char *msg);
void err_display(const char *msg);
void err_display(int errcode);

DWORD recvbytes, sendbytes;
DWORD flags = 0;


int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	//입출력 완료 포트 생성
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL)
		return 1;

	//cpu 개수 확인
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	//(cpu 개수 * 2)개의 작업자 스레드 생성
	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
	{
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL)
			return 1;
		CloseHandle(hThread);
	}

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");


	//데이터 통신에 사용할 변수 
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		printf("[TCP 서버] 클라이언트 접속 : IP 접속 = %s, 포트 번호 = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		SOCKETINFO* ptr = AddSocketInfo(client_sock);

		//소켓과 입출력 완료 포트 연결
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		//소켓 정보 구조체 할당
		/*SOCKETINFO* ptr = new SOCKETINFO;
		if (ptr == NULL)
			break;
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;*/

		//비동기 입출력 시장
		flags = 0;
		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes,
			&flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				err_display("wsaRecv");
			}
			continue;
		}
	}

	// 윈속 종료
	WSACleanup();
	return 0;
}


//작업자 스레드 함수
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;

	while (true)
	{
		//비동기 입출력 완료 기다리기
		DWORD cbTransferred;
		SOCKET client_sock;
		SOCKETINFO *ptr;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED*)&ptr, INFINITE);

		//클라이언트 정보 얻기
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

		//비동기 입출력 결과 확인
		if (retval == 0 || cbTransferred == 0)
		{
			if (retval == 0)
			{
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);
				err_display("WsagetOverlappedResult()");
				RemoveSocketInfo(ptr);
				continue;
			}
			RemoveSocketInfo(ptr);
			continue;
			//closesocket(ptr->sock);
			/*printf("[TCP 서버] 클라이언트 종료 : IP 주소 = &s, 포트번호 =%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			delete ptr;*/
			//continue;
		}


		//switch (ptr->iotype)
		//{
		//case IO_READ:
		//	IO_Read(ptr, cbTransferred);
		//	break;
		//case IO_WRITE:
		//	break;
		//default:
		//	break;
		//}


		//데이터 전송량 갱신
		if (ptr->recvbytes == 0)
		{
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;
			//받은 데이터 출력 
			ptr->buf[ptr->recvbytes] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				ntohs(clientaddr.sin_port), ptr->buf);
		}
		else {
			ptr->sendbytes += cbTransferred;
		}


		//if (ptr->recvbytes > ptr->sendbytes)
		//{
		//	//데이터 보내기 
		//	ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		//	ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;
		//	ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;

		//	DWORD sendbytes;
		//	retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, NULL);
		//	if (retval == SOCKET_ERROR)
		//	{
		//		if (WSAGetLastError() != WSA_IO_PENDING) {
		//			err_display("WSASend()");
		//		}
		//		continue;
		//	}
		//}


		{
			ptr->recvbytes = 0;
			PACKET_CHAT_1 packet;
			PACKET_HEADER header;


			// 데이터 받기
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFSIZE;
			//header
			memcpy(&header, ptr->buf, sizeof(PACKET_HEADER));

			//chat packet
			memcpy(&packet, ptr->buf, header.wLen);
			printf("내용 : %s\n", packet.buf);

			

			DWORD recvbytes;
			DWORD flags = 0;
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1,
				&recvbytes, &flags, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					err_display("WSARecv()");
				}				
			}

			for (int i = 0; i < NowUserNum; i++)
			{
				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				ptr->wsabuf.buf = ptr->buf;
				ptr->wsabuf.len = sizeof(ptr->buf);

				DWORD sendbytes;
				retval = WSASend(User[i]->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, NULL);
				if (retval == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING) {
						err_display("WSASend()");
					}
					continue;
				}
			}
		}
	}
	return 0;
}


SOCKETINFO* AddSocketInfo(SOCKET _sock)
{
	int retval;

	SOCKETINFO* ptr = new SOCKETINFO;

	if (ptr == NULL)
		return false;
	ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
	ptr->sock = _sock;
	ptr->recvbytes = ptr->sendbytes = 0;
	ptr->wsabuf.buf = ptr->buf;
	ptr->wsabuf.len = BUFSIZE;

	User[NowUserNum++] = ptr;

	return ptr;
}

bool RemoveSocketInfo(SOCKETINFO *_ptr)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	for (int i = 0; i < NowUserNum; i++)
	{
		if (User[i]->sock == _ptr->sock)
		{
			getpeername(_ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);
			//유저가 나갓다고 클라이언트 전송
			for (int j = 0; j < NowUserNum; j++)
			{
				
			}

			closesocket(_ptr->sock);
			delete _ptr;
			_ptr = nullptr;

			for (int k = i + 1; k < NowUserNum; ++k, ++i)
			{
				User[i] = User[k];
			}
			NowUserNum--;
			User[NowUserNum] = nullptr;

			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			return true;
		}
	}

	return false;
}


//데이터 확인
bool CheckRecv(SOCKETINFO* _ptr)
{
	//맨처음 총 받은 길이가 0이고 현재까지 받은 길이가 sizeof(int)보다 크거가 같을 경우
	if (_ptr->recvbytes == 0)
	{
		//다 받았을 경우
		//if(_ptr->recvbytes )
		//if (_info->packet->Comp_Recvbytes >= sizeof(int))
		//{
		//	//앞에 토탈사이즈를 _info->packet->Recvbytes에 넣어준다
		//	memcpy(&_info->packet->Recvbytes, _info->packet->RecvBuf, sizeof(int));
		//}
	}

	return false;
}

//읽기 상태
void IO_Read(SOCKETINFO* _info, DWORD _cbTransferred)
{
	//int retval;
	//PROTOCAL protocal = PROTOCAL::NONE_USER;
	////한 싸이클 돌았을때 들어온 _cbTransferred값을 유저가 현재까지 받은 길이에 넣어준다
	//_info->packet->Comp_Recvbytes += _cbTransferred;
	////모든 리시브작업이 끝났는지 체크
	

	//_info->MessageLen = _info->
	//_info
}

// 소켓 함수 오류 출력 후 종료
void err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[오류] %s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
