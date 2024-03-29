#pragma once
#include "Global.h"


class BitMap;
class Block;
class BlockManager
{
private:
	//8���� 
	DIRECTION_XY DIRECTION_EIGHT[8] = { { -1, 0 }, { 1, 0 }, { 0, -1 },{ 0, 1 }, { -1, -1 }, { 1, 1 } ,{ -1, 1 }, { 1, -1 } };

	BitMap* m_pBitMap;
	Block* m_pBlock[CHECKER_BOARD_XY][CHECKER_BOARD_XY];
	WHAT_BLOCK_STATE m_nowPlayer;
public:
	BlockManager();
	~BlockManager();

	void init();
	void update();
	void render(HDC _hdc);

	void checkCollition(POINT _pt);
	void checkFiveStone(int _y, int _x);
	bool checkFiveAndResetCount(int& _num);
	void sendPos(int _y, int _x);
	void recvPos(int _y, int _x, int _stone);
	void setStone(WHAT_BLOCK_STATE _stone);
	//void winPlayer();
	

};

