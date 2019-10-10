#pragma once

// window h
#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include <chrono>
#include <map>

using namespace std;


//my header
#include "macro.h"
#include "..\..\Common\PACKET_HEADER_CatchMind.h"

//define
#define FPS 60.0f
#define MAP_TILE_WIDHT_HEIGHT 13
#define BLCOK_START_POINT_X 12
#define BLOCK_START_POINT_Y 12
#define STONE_SIZE 30


//통신관련 디파인
#define BUFSIZE 512

struct F_POSITION
{
	float x;
	float y;
};

