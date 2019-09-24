#pragma once

// window h
#include <Windows.h>
#include <vector>
#include <chrono>

using namespace std;


//my header
#include "macro.h"
#include "..\..\Common\PACKET_HEADER.h"

//define
#define FPS 60.0f
#define MAP_TILE_WIDHT_HEIGHT 13
#define BLCOK_START_POINT_X 12
#define BLOCK_START_POINT_Y 12
#define STONE_SIZE 30



struct F_POSITION
{
	float x;
	float y;
};

