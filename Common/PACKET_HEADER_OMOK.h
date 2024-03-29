#pragma once
#include <Windows.h>
#pragma pack(1)

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


//////////////내ㅑ가 만든거

struct USER_DATA_H
{
	int iIndex;
	int stone;
};

struct PACKET_USER_DATA_H
{
	PACKET_HEADER header;
	WORD wCount;
	USER_DATA_H data[20];
};




#pragma pack()

//////////////////////////////////////////////////////////
//데이터 영역

//디파인
#define CHECKER_BOARD_XY 19

//이넘
enum SCENE_STATE
{
	SCENE_LOGIN,
	SCENE_LOBBY,
	SCENE_GAME_ROOM
};





enum LINE_DIR
{
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT_UP,
	DIR_RIGHT_DOWN,
	DIR_LEFT_DOWN,
	DIR_RITHG_UP
};