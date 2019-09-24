#pragma once
#include "Global.h"

class BitMap;
class Block
{
private:
	BitMap* m_bitMap[2];
	WHAT_BLOCK_STATE m_state;
	POINT m_position;


public:
	Block();
	~Block();

	void init(int _x, int _y);
	void update();
	void render(HDC _hdc);
	bool collision(POINT _mouse);
	void setStone(WHAT_BLOCK_STATE _stone);
	
	bool isEmpty();
	inline WHAT_BLOCK_STATE whatState() { return m_state; }
};

