#pragma once

//디파인
#define CHECKER_BOARD_XY 19

//이넘
enum SCENE_STATE
{
	SCENE_MENU,
	SCENE_STAGE_ONE
};

enum WHAT_BLOCK_STATE
{
	BLACK_STONE,
	WHITE_STONE,
	BLOCK_EMPTY
};

struct DIRECTION_XY
{
	int x;
	int y;
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