#pragma once
#include <Windows.h>
#pragma pack(1)

enum USER_STATE
{
	USER_IN_LOBBY,
	USER_IN_ROOM,
	USER_PLAYING_GAME,
	USER_WHATCHING_GAME
};

enum PACKET_INDEX
{
	PACKET_INDEX_LOGIN_RET = 1,
	PACKET_INDEX_USER_LOBBY,
	PACKET_INDEX_USER_DATA,
	PACKET_INDEX_SEND_POS,
};

struct PACKET_HEADER
{
	WORD wIndex;
	WORD wLen;
};

struct USER_DATA
{
	int iIndex;
	WORD wX;
	WORD wY;
	int stone;
	//int roomNum;
};

struct USER_LOBBY_DATA
{
	int iIndex;
};

struct PACKET_LOGIN_RET
{
	PACKET_HEADER header;
	int iIndex;
};

struct PACKET_USER_LOBBY
{
	PACKET_HEADER header;
	WORD wCount;
	USER_LOBBY_DATA data[20];
};

struct PACKET_USER_DATA
{
	PACKET_HEADER header;
	WORD wCount;
	USER_DATA data[20];
};

struct PACKET_SEND_POS
{
	PACKET_HEADER header;
	USER_DATA data;
};


struct PACKET_CHAT_1
{
	PACKET_HEADER header;
	int chatLen;
	char buf[32];
};

#pragma pack()

enum SCENE_STATE
{
	SCENE_LOGIN,
	SCENE_LOBBY,
	SCENE_GAME_ROOM
};