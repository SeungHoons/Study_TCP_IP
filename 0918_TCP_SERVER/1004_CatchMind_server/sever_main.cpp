#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include "..\..\Common\PACKET_HEADER_CatchMind.h"

#define SERVERPORT 9000
#define BUFSIZE    512
//인구 정해놨는데 터질수도 있음
#define MAXUSER    50

struct PACKET;
struct OVERLAPPEDEX;
struct SOCKETINFO;
struct NEXTWORK;


//패킷의 상태
enum IOTYPE
{
	IO_READ,
	IO_WRITE
};




struct PACKET
{
	char sendBuf[BUFSIZE];
	char recvBuf[BUFSIZE];

	int recvbytes;
	int compRecvByte;
	int sendbytes;
	int compSendByte;

	int len;
};

struct OVERLAPPEDEX			/*오버랩ex구조체*/
{
	OVERLAPPED overlapped;	/*오버랩구조체*/
	SOCKETINFO* info;		/*자기자신*/
	IOTYPE iotype;			/*읽기,쓰기상태*/
};

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	OVERLAPPEDEX readOverlapped;
	OVERLAPPEDEX writeOverlapped;
	SOCKET sock;
	PACKET* packet;
	std::queue <NEXTWORK*> nextWork;		//작업 관리
};



//유저 관리
int NowUserNum = 0;
SOCKETINFO* User[MAXUSER];

SOCKET client_sock;
HANDLE hReadEvent, hWriteEvent;

// 비동기 입출력 시작과 처리 함수
DWORD WINAPI WorkerThread(LPVOID arg);

//유저 소켓 추가
SOCKETINFO* AddSocketInfo(SOCKET _sock);
bool RemoveSocketInfo(SOCKETINFO *_info);

//iocp
int IO_Recv(SOCKETINFO* _info);
int IO_Send(SOCKETINFO* _info);

//읽기 상태
void IO_Read(SOCKETINFO* _info, DWORD _cbTransferred);
//데이터를 다 받았는지 체크
bool CheckRecv(SOCKETINFO* _info);

//쓰기 상태
void IO_Write(SOCKETINFO* _info, DWORD _cbTransferred);
//데이터를 다 보냈는지 체크
bool CheckSend(SOCKETINFO* _info, DWORD _cbTransferred);

//작업 큐에 넣어주기
int PushQueue(SOCKETINFO * _info, NEXTWORK* _nextWork);

//// 패킹
bool Packing(int _totalsize, char* _buf, PACKET_INDEX _pacIndex, int _size, char* _data);

//언패킹
bool UnPacking(SOCKETINFO* _info, PACKET_INDEX* _protocal);
//작업
struct NEXTWORK
{
	NEXTWORK(PACKET_INDEX _pacIndex, int _size, char* _buf)
	{
		size =						/*총길이*/
			sizeof(int) +			/*총길이크기*/
			sizeof(PACKET_INDEX) +	/*프로토콜크기*/
			sizeof(int) +			/*문자길이크기*/
			_size;					/*문자길이*/
		buf = new char[size];
		Packing(size, buf, _pacIndex, _size, _buf);
	}
	char* buf;
	int size;
};

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
	int result;

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

		//비동기 입출력 시장
		flags = 0;
		result = IO_Recv(ptr);
		if (result == TRANSSTATE::DISCONNECT)
		{
			RemoveSocketInfo(ptr);
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
		OVERLAPPEDEX *overlappedEx = NULL;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED*)&overlappedEx, INFINITE);
		ptr = overlappedEx->info;

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
				WSAGetOverlappedResult(ptr->sock, &ptr->readOverlapped.overlapped, &temp1, FALSE, &temp2);
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


		switch (overlappedEx->iotype)
		{
		case IO_READ:
			IO_Read(ptr, cbTransferred);
			break;
		case IO_WRITE:
			IO_Write(ptr, cbTransferred);
			break;
		default:
			break;
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
	ptr->packet = new PACKET;
	ZeroMemory(ptr->packet, sizeof(PACKET));
	ZeroMemory(&ptr->readOverlapped, sizeof(ptr->readOverlapped));
	ZeroMemory(&ptr->writeOverlapped, sizeof(ptr->writeOverlapped));
	ptr->readOverlapped.iotype = IO_READ;
	ptr->writeOverlapped.iotype = IO_WRITE;
	ptr->readOverlapped.info = ptr;
	ptr->writeOverlapped.info = ptr;
	ptr->sock = _sock;
	ptr->packet->recvbytes = ptr->packet->sendbytes = 0;
	ptr->packet->compRecvByte = ptr->packet->compSendByte = 0;
	ptr->packet->len = 0;

	User[NowUserNum++] = ptr;

	return ptr;
}

bool RemoveSocketInfo(SOCKETINFO *_info)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	for (int i = 0; i < NowUserNum; i++)
	{
		if (User[i]->sock == _info->sock)
		{
			getpeername(_info->sock, (SOCKADDR*)&clientaddr, &addrlen);
			//유저가 나갓다고 클라이언트 전송
			for (int j = 0; j < NowUserNum; j++)
			{
				
			}

			closesocket(_info->sock);
			delete _info;
			_info = nullptr;

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

int IO_Recv(SOCKETINFO * _info)
{
	int retval;
	WSABUF wsabuf;

	ZeroMemory(&_info->readOverlapped.overlapped, sizeof(_info->readOverlapped.overlapped));
	ZeroMemory(&wsabuf, sizeof(wsabuf));

	wsabuf.buf = _info->packet->recvBuf + _info->packet->compRecvByte;
	wsabuf.len = BUFSIZE;

	retval = WSARecv(_info->sock, &wsabuf, 1, &recvbytes, &flags, &_info->readOverlapped.overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSARecv()");
			return TRANSSTATE::DISCONNECT;
		}
		return TRANSSTATE::SOC_FALSE;
	}
	return TRANSSTATE::SOC_TRUE;
}

int IO_Send(SOCKETINFO * _info)
{
	WSABUF wsabuf;
	ZeroMemory(&wsabuf, sizeof(wsabuf));
	//유저 큐의 맨 앞에 있는 미션을 mission에 넣기
	NEXTWORK* nextWork = _info->nextWork.front();
	//유저가 보낼 총 길이 = 미션의 size
	_info->packet->sendbytes = nextWork->size;
	//유저의 쓰기오버랩의 오버랩구조체 초기화
	ZeroMemory(&_info->writeOverlapped.overlapped, sizeof(_info->writeOverlapped.overlapped));

	wsabuf.buf = nextWork->buf + _info->packet->compSendByte;
	wsabuf.len = _info->packet->sendbytes - _info->packet->compSendByte;

	int retval = WSASend(_info->sock, &wsabuf, 1, &sendbytes, 0, &_info->writeOverlapped.overlapped, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			err_display("WSASend()");
			return TRANSSTATE::DISCONNECT;
		}
		return TRANSSTATE::SOC_FALSE;
	}
	return TRANSSTATE::SOC_TRUE;
}


//데이터 확인
bool CheckRecv(SOCKETINFO* _info)
{
	//맨처음 총 받은 길이가 0이고 현재까지 받은 길이가 sizeof(int)보다 크거가 같을 경우
	if (_info->packet->recvbytes == 0)
	{
		//다 받았을 경우
		if (_info->packet->compRecvByte >= sizeof(int))
		{
			//앞에 토탈사이즈를 _info->packet->Recvbytes에 넣어준다
			memcpy(&_info->packet->recvbytes, _info->packet->recvBuf, sizeof(int));
		}
	}

	if (_info->packet->compRecvByte >= _info->packet->recvbytes && _info->packet->recvbytes != 0)
	{
		if (_info->packet->compRecvByte == _info->packet->recvbytes)
		{
			_info->packet->compRecvByte = _info->packet->recvbytes = 0;
			return true;
		}
		else if (_info->packet->compRecvByte > _info->packet->recvbytes)
		{
			memmove(_info->packet->recvBuf, _info->packet->recvBuf + _info->packet->recvbytes, _info->packet->compRecvByte - _info->packet->recvbytes);
			_info->packet->compRecvByte = _info->packet->compRecvByte - _info->packet->recvbytes;
			_info->packet->recvbytes = 0;
		}
	}
	return false;
}

void IO_Write(SOCKETINFO * _info, DWORD _cbTransferred)
{
	int retval;

	if (CheckSend(_info, _cbTransferred) == true)
	{
		//보낼길이와 보낸길이 0으로 초기화
		_info->packet->compSendByte = _info->packet->sendbytes = 0;
		//다 보냈으니 유저의 큐에 있는 정보 팝
		_info->nextWork.pop();
		//유저의 큐에 아직 내용이 있을 경우
		if (_info->nextWork.empty() != true)
		{
			retval = IO_Send(_info);
			if (retval == TRANSSTATE::DISCONNECT)
			{
				RemoveSocketInfo(_info);
			}
		}
	}
	else
	{
		//WSASend 다시 호출
		retval = IO_Send(_info);
		if (retval == TRANSSTATE::DISCONNECT)
		{
			RemoveSocketInfo(_info);
		}
	}
}

bool CheckSend(SOCKETINFO * _info, DWORD _cbTransferred)
{
	_info->packet->compSendByte += _cbTransferred;
	//다 보냈을 경우
	if (_info->packet->compSendByte == _info->packet->sendbytes)
	{
		_info->packet->compSendByte = _info->packet->sendbytes = 0;
		return true;
	}
	else
	{
		return false;
	}
}

//읽기 상태
void IO_Read(SOCKETINFO* _info, DWORD _cbTransferred)
{
	int retval;
	int totalsize;
	PACKET_INDEX protocal = PACKET_INDEX::PACKET_INDEX_LOGIN_RET;
	//한 싸이클 돌았을때 들어온 _cbTransferred값을 유저가 현재까지 받은 길이에 넣어준다
	_info->packet->compRecvByte += _cbTransferred;
	//모든 리시브작업이 끝났는지 체크
	if (CheckRecv(_info))
	{
		UnPacking(_info, &protocal);

		switch (protocal)
		{
		case PACKET_INDEX_LOGIN_RET:
			break;
		case PACKET_INDEX_USER_LOBBY:
			break;
		case PACKET_INDEX_USER_CHAT:
			printf("%d : %s\n",_info->sock, _info->packet->recvBuf);
			totalsize = sizeof(PACKET_INDEX) + sizeof(int) + strlen(_info->packet->recvBuf);
			for (int i = 0; i < NowUserNum; i++)
			{
				if (PushQueue(User[i], new NEXTWORK(PACKET_INDEX::PACKET_INDEX_USER_CHAT, sizeof(strlen(_info->packet->recvBuf)), _info->packet->recvBuf)) == TRANSSTATE::DISCONNECT)
				{
					RemoveSocketInfo(_info);
				}
			}
			break;
		case PACKET_INDEX_USER_DATA:
			break;
		case PACKET_INDEX_SEND_POS:
			break;
		default:
			break;
		}
		//다시 리시브 날려주기
		retval = IO_Recv(_info);
		if (retval == TRANSSTATE::DISCONNECT)
		{
			RemoveSocketInfo(_info);
		}
	}
	else
	{
		retval = IO_Recv(_info);
		if (retval == TRANSSTATE::DISCONNECT)
		{
			RemoveSocketInfo(_info);
		}
	}
}

int PushQueue(SOCKETINFO * _info, NEXTWORK * _nextWork)
{
	if (_info->nextWork.empty() == true)
	{
		_info->nextWork.push(_nextWork);


		return IO_Send(_info);
	}
	else
	{
		_info->nextWork.push(_nextWork);
	}

	return TRANSSTATE::SOC_TRUE;
}

bool Packing(int _totalsize, char* _buf, PACKET_INDEX _pacIndex, int _size, char* _data)
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
		(char*)&_pacIndex,/*프로토콜 값*/
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

bool UnPacking(SOCKETINFO * _info, PACKET_INDEX * _protocal)
{
	memcpy
	(
		_protocal,/*프로토콜*/
		_info->packet->recvBuf + sizeof(int),/*_info->packet->RecvBuf + sizeof(int)*/
		sizeof(PACKET_INDEX)/*프로토콜 크기*/
	);

	memcpy
	(
		&_info->packet->len,/*_info->circle.pos의 길이*/
		_info->packet->recvBuf + sizeof(int) + sizeof(PACKET_INDEX),/*_info->packet->RecvBuf + sizeof(int) + sizeof(PROTOCAL)*/
		sizeof(int)/*인트형 크기*/
	);

	memcpy
	(
		_info->packet->recvBuf,/*버퍼*/
		_info->packet->recvBuf + sizeof(int) + sizeof(PACKET_INDEX) + sizeof(int),/*_info->packet->RecvBuf + sizeof(int) + sizeof(PROTOCAL) + sizeof(int)*/
		_info->packet->len/*길이*/
	);
	//printf("[recv] x : %d, y : %d\n", _info->circle.pos.x, _info->circle.pos.y);
	return true;
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
