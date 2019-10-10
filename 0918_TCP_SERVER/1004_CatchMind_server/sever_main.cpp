#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\..\Common\PACKET_HEADER_CatchMind.h"

#define SERVERPORT 9000
#define BUFSIZE    512
//�α� ���س��µ� �������� ����
#define MAXUSER    50

//��Ŷ�� ����
enum IOTYPE
{
	IO_READ,
	IO_WRITE
};

// ���� ���� ������ ���� ����ü�� ����
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
	IOTYPE iotype;

	//ê
	int MessageLen;
};

int NowUserNum = 0;
SOCKETINFO* User[MAXUSER];

SOCKET client_sock;
HANDLE hReadEvent, hWriteEvent;

// �񵿱� ����� ���۰� ó�� �Լ�
DWORD WINAPI WorkerThread(LPVOID arg);

//���� ���� �߰�
SOCKETINFO* AddSocketInfo(SOCKET _sock);
bool RemoveSocketInfo(SOCKETINFO *_ptr);

//������ Ȯ��
bool CheckRecv(SOCKETINFO* _ptr);

//�б� ����
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

		//���� ���� ����ü �Ҵ�
		/*SOCKETINFO* ptr = new SOCKETINFO;
		if (ptr == NULL)
			break;
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;*/

		//�񵿱� ����� ����
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
		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED*)&ptr, INFINITE);

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
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);
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


		//������ ���۷� ����
		if (ptr->recvbytes == 0)
		{
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;
			//���� ������ ��� 
			ptr->buf[ptr->recvbytes] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				ntohs(clientaddr.sin_port), ptr->buf);
		}
		else {
			ptr->sendbytes += cbTransferred;
		}


		//if (ptr->recvbytes > ptr->sendbytes)
		//{
		//	//������ ������ 
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


			// ������ �ޱ�
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFSIZE;
			//header
			memcpy(&header, ptr->buf, sizeof(PACKET_HEADER));

			//chat packet
			memcpy(&packet, ptr->buf, header.wLen);
			printf("���� : %s\n", packet.buf);

			

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
			//������ �����ٰ� Ŭ���̾�Ʈ ����
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

			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			return true;
		}
	}

	return false;
}


//������ Ȯ��
bool CheckRecv(SOCKETINFO* _ptr)
{
	//��ó�� �� ���� ���̰� 0�̰� ������� ���� ���̰� sizeof(int)���� ũ�Ű� ���� ���
	if (_ptr->recvbytes == 0)
	{
		//�� �޾��� ���
		//if(_ptr->recvbytes )
		//if (_info->packet->Comp_Recvbytes >= sizeof(int))
		//{
		//	//�տ� ��Ż����� _info->packet->Recvbytes�� �־��ش�
		//	memcpy(&_info->packet->Recvbytes, _info->packet->RecvBuf, sizeof(int));
		//}
	}

	return false;
}

//�б� ����
void IO_Read(SOCKETINFO* _info, DWORD _cbTransferred)
{
	//int retval;
	//PROTOCAL protocal = PROTOCAL::NONE_USER;
	////�� ����Ŭ �������� ���� _cbTransferred���� ������ ������� ���� ���̿� �־��ش�
	//_info->packet->Comp_Recvbytes += _cbTransferred;
	////��� ���ú��۾��� �������� üũ
	

	//_info->MessageLen = _info->
	//_info
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
