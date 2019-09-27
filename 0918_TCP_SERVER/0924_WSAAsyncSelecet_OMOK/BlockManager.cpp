#include "BlockManager.h"

BlockManager::BlockManager()
{
}


BlockManager::~BlockManager()
{
}

void BlockManager::init()
{
	m_nowPlayer = BLACK_STONE;

	for (int i = 0; i < CHECKER_BOARD_XY; i++)
	{
		for (int j = 0; j < CHECKER_BOARD_XY; j++)
		{
			m_pBlock[i][j] = new Block();
			m_pBlock[i][j]->state = BLOCK_EMPTY;
		}
	}
}

void BlockManager::update()
{
}


int BlockManager::checkFiveStone(int _y, int _x, int _stone)
{
	int count = 0;
	int x = _x;
	int y = _y;


	for (int i = 0; i < 8; i += 2)
	{
		x = _x;
		y = _y;
		count = 0;
		int plus_x = (DIRECTION_EIGHT[(LINE_DIR)i].x);
		int plus_y = (DIRECTION_EIGHT[(LINE_DIR)i].y);
		while (y>0 && x > 0 && (m_pBlock[y + plus_y][x + plus_x]->state == _stone))
		{
			x += plus_x;
			y += plus_y;
		}
		plus_x = (DIRECTION_EIGHT[(LINE_DIR)i+1].x);
		plus_y = (DIRECTION_EIGHT[(LINE_DIR)i+1].y);
		while ((m_pBlock[y][x]->state == _stone && x < 18 && y < 18  ))
		{
			y += plus_y;
			x += plus_x;
			count++;
		}
		//5°³ Ã¼Å©
		if (checkFiveAndResetCount(count))
		{
			return _stone;
			break;
		}
	}

	return BLOCK_EMPTY;
}

bool BlockManager::checkFiveAndResetCount(int& _num)
{
	if (_num == 5)
	{
		return true;
	}
	_num = 1;
	return false;
}

int BlockManager::setStone(int _y, int _x, WHAT_BLOCK_STATE _stone)
{
	m_pBlock[_y][_x]->state = _stone;
	return checkFiveStone(_y, _x, _stone);
}

