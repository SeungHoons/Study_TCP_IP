#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include "..\..\Common\PACKET_HEADER_CatchMind.h"

#define SERVERPORT 9000
#define BUFSIZE    512
//�α� ���س��µ� �������� ����
#define MAXUSER    50

struct PACKET;
struct OVERLAPPEDEX;
struct SOCKETINFO;
struct NEXTWORK;


//��Ŷ�� ����
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

struct OVERLAPPEDEX			/*������ex����ü*/
{
	OVERLAPPED overlapped;	/*����������ü*/
	SOCKETINFO* info;		/*�ڱ��ڽ�*/
	IOTYPE iotype;			/*�б�,�������*/
};

// ���� ���� ������ ���� ����ü�� ����
struct SOCKETINFO
{
	OVERLAPPEDEX readOverlapped;
	OVERLAPPEDEX writeOverlapped;
	SOCKET sock;
	PACKET* packet;
	std::queue <NEXTWORK*> nextWork;		//�۾� ����
};



//���� ����
int NowUserNum = 0;
SOCKETINFO* User[MAXUSER];

SOCKET client_sock;
HANDLE hReadEvent, hWriteEvent;

// �񵿱� ����� ���۰� ó�� �Լ�
DWORD WINAPI WorkerThread(LPVOID arg);

//���� ���� �߰�
SOCKETINFO* AddSocketInfo(SOCKET _sock);
bool RemoveSocketInfo(SOCKETINFO *_info);

//iocp
int IO_Recv(SOCKETINFO* _info);
int IO_Send(SOCKETINFO* _info);

//�б� ����
void IO_Read(SOCKETINFO* _info, DWORD _cbTransferred);
//�����͸� �� �޾Ҵ��� üũ
bool CheckRecv(SOCKETINFO* _info);

//���� ����
void IO_Write(SOCKETINFO* _info, DWORD _cbTransferred);
//�����͸� �� ���´��� üũ
bool CheckSend(SOCKETINFO* _info, DWORD _cbTransferred);

//�۾� ť�� �־��ֱ�
int PushQueue(SOCKETINFO * _info, NEXTWORK* _nextWork);

//// ��ŷ
bool Packing(int _totalsize, char* _buf, PACKET_INDEX _pacIndex, int _size, char* _data);

//����ŷ
bool UnPacking(SOCKETINFO* _info, PACKET_INDEX* _protocal);
//�۾�
struct NEXTWORK
{
	NEXTWORK(PACKET_INDEX _pacIndex, int _size, char* _buf)
	{
		size =						/*�ѱ���*/
			sizeof(int) +			/*�ѱ���ũ��*/
			sizeof(PACKET_INDEX) +	/*��������ũ��*/
			sizeof(int) +			/*���ڱ���ũ��*/
			_size;					/*���ڱ���*/
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

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	//����� �Ϸ� ��Ʈ ����
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL)
		return 1;

	//cpu ���� Ȯ��
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	//(cpu ���� * 2)���� �۾��� ������ ����
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


	//������ ��ſ� ����� ���� 
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
		printf("[TCP ����] Ŭ���̾�Ʈ ���� : IP ���� = %s, ��Ʈ ��ȣ = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		SOCKETINFO* ptr = AddSocketInfo(client_sock);

		//���ϰ� ����� �Ϸ� ��Ʈ ����
		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		//�񵿱� ����� ����
		flags = 0;
		result = IO_Recv(ptr);
		if (result == TRANSSTATE::DISCONNECT)
		{
			RemoveSocketInfo(ptr);
		}
	}

	// ���� ����
	WSACleanup();
	return 0;
}


//�۾��� ������ �Լ�
DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;

	while (true)
	{
		//�񵿱� ����� �Ϸ� ��ٸ���
		DWORD cbTransferred;
		SOCKET client_sock;
		SOCKETINFO *ptr;
		OVERLAPPEDEX *overlappedEx = NULL;
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED*)&overlappedEx, INFINITE);
		ptr = overlappedEx->info;

		//Ŭ���̾�Ʈ ���� ���
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

		//�񵿱� ����� ��� Ȯ��
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
			/*printf("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = &s, ��Ʈ��ȣ =%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
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
			//������ �����ٰ� Ŭ���̾�Ʈ ����
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

			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

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
	//���� ť�� �� �տ� �ִ� �̼��� mission�� �ֱ�
	NEXTWORK* nextWork = _info->nextWork.front();
	//������ ���� �� ���� = �̼��� size
	_info->packet->sendbytes = nextWork->size;
	//������ ����������� ����������ü �ʱ�ȭ
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


//������ Ȯ��
bool CheckRecv(SOCKETINFO* _info)
{
	//��ó�� �� ���� ���̰� 0�̰� ������� ���� ���̰� sizeof(int)���� ũ�Ű� ���� ���
	if (_info->packet->recvbytes == 0)
	{
		//�� �޾��� ���
		if (_info->packet->compRecvByte >= sizeof(int))
		{
			//�տ� ��Ż����� _info->packet->Recvbytes�� �־��ش�
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
		//�������̿� �������� 0���� �ʱ�ȭ
		_info->packet->compSendByte = _info->packet->sendbytes = 0;
		//�� �������� ������ ť�� �ִ� ���� ��
		_info->nextWork.pop();
		//������ ť�� ���� ������ ���� ���
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
		//WSASend �ٽ� ȣ��
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
	//�� ������ ���
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

//�б� ����
void IO_Read(SOCKETINFO* _info, DWORD _cbTransferred)
{
	int retval;
	int totalsize;
	PACKET_INDEX protocal = PACKET_INDEX::PACKET_INDEX_LOGIN_RET;
	//�� ����Ŭ �������� ���� _cbTransferred���� ������ ������� ���� ���̿� �־��ش�
	_info->packet->compRecvByte += _cbTransferred;
	//��� ���ú��۾��� �������� üũ
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
		//�ٽ� ���ú� �����ֱ�
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
		(char*)&_totalsize,/*��Ż������*/
		sizeof(int)/*��Ż������ ũ��*/
	);

	memcpy
	(
		_buf + sizeof(int),/*buf+sizeof(int)*/
		(char*)&_pacIndex,/*�������� ��*/
		sizeof(PACKET_INDEX)/*�������� ũ��*/
	);

	memcpy
	(
		_buf + sizeof(int) + sizeof(PACKET_INDEX),/*buf+sizeof(int)+sizeof(PROTOCAL)*/
		(char*)&_size,/*�������ڱ���*/
		sizeof(int)/*����ũ��*/
	);

	memcpy
	(
		_buf + sizeof(int) + sizeof(PACKET_INDEX) + sizeof(int),/*buf+sizeof(int)+sizeof(PROTOCAL)+sizeof(int)*/
		_data,/*���� ���ڳ���*/
		_size/*���ڱ���*/
	);
	return true;
}

bool UnPacking(SOCKETINFO * _info, PACKET_INDEX * _protocal)
{
	memcpy
	(
		_protocal,/*��������*/
		_info->packet->recvBuf + sizeof(int),/*_info->packet->RecvBuf + sizeof(int)*/
		sizeof(PACKET_INDEX)/*�������� ũ��*/
	);

	memcpy
	(
		&_info->packet->len,/*_info->circle.pos�� ����*/
		_info->packet->recvBuf + sizeof(int) + sizeof(PACKET_INDEX),/*_info->packet->RecvBuf + sizeof(int) + sizeof(PROTOCAL)*/
		sizeof(int)/*��Ʈ�� ũ��*/
	);

	memcpy
	(
		_info->packet->recvBuf,/*����*/
		_info->packet->recvBuf + sizeof(int) + sizeof(PACKET_INDEX) + sizeof(int),/*_info->packet->RecvBuf + sizeof(int) + sizeof(PROTOCAL) + sizeof(int)*/
		_info->packet->len/*����*/
	);
	//printf("[recv] x : %d, y : %d\n", _info->circle.pos.x, _info->circle.pos.y);
	return true;
}

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[����] %s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
