#pragma once
#include "Global.h"


class BitMap;
class Block
{
public:
	WHAT_BLOCK_STATE state;
};
class BlockManager
{
private:
	//8πÊ«‚ 
	DIRECTION_XY DIRECTION_EIGHT[8] = { { -1, 0 }, { 1, 0 }, { 0, -1 },{ 0, 1 }, { -1, -1 }, { 1, 1 } ,{ -1, 1 }, { 1, -1 } };


	BitMap* m_pBitMap;
	//vector<Block*> m_vecBlock;
	Block* m_pBlock[CHECKER_BOARD_XY][CHECKER_BOARD_XY];
	WHAT_BLOCK_STATE m_nowPlayer;
public:
	BlockManager();
	~BlockManager();

	void init();
	void update();

	int checkFiveStone(int _y, int _x, int _stone);
	bool checkFiveAndResetCount(int& _num);
	int setStone(int _y, int _x, WHAT_BLOCK_STATE _stone);
	

};

