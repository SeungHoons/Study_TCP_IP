#include "BlockManager.h"
#include "Block.h"
#include "ResManager.h"
#include "BitMap.h"

BlockManager::BlockManager()
{
}


BlockManager::~BlockManager()
{
}

void BlockManager::init()
{
	m_nowPlayer = BLACK_STONE;
	m_pBitMap = ResManager::getInst()->getBitMap(RES_FILE_NAME::RES_FILE_BACK_GROUND);

	float tem_x;
	float tem_y;
	for (int i = 0; i < CHECKER_BOARD_XY; i++)
	{
		for (int j = 0; j < CHECKER_BOARD_XY; j++)
		{
			tem_x = j * 37.5;
			tem_y = i * 37.5;

			m_pBlock[i][j] = new Block();
			m_pBlock[i][j]->init((tem_x)+BLCOK_START_POINT_X, (tem_y)+BLOCK_START_POINT_Y);

			/*m_vecBlock.push_back(new Block());
			m_vecBlock.back()->init((tem_x) + BLCOK_START_POINT_X, (tem_y)+BLOCK_START_POINT_Y);*/

		}
	}
}

void BlockManager::update()
{
}

void BlockManager::render(HDC _hdc)
{
	m_pBitMap->render(_hdc, 0, 0);
	for (int i = 0; i < CHECKER_BOARD_XY; i++)
	{
		for (int j = 0; j < CHECKER_BOARD_XY; j++)
		{
			m_pBlock[i][j]->render(_hdc);
		}
	}

	//for (auto iter = m_vecBlock.begin(); iter != m_vecBlock.end(); iter++)
	//{
	//	(*iter)->render(_hdc);
	//}
}

void BlockManager::checkCollition(POINT _pt)
{
	for (int i = 0; i < CHECKER_BOARD_XY; i++)
	{
		for (int j = 0; j < CHECKER_BOARD_XY; j++)
		{
			if (m_pBlock[i][j]->collision(_pt))
			{
				if (m_pBlock[i][j]->isEmpty())
				{
					m_pBlock[i][j]->setStone(m_nowPlayer);
					checkFiveStone(i, j);

					if (m_nowPlayer == BLACK_STONE)
						m_nowPlayer = WHITE_STONE;
					else
						m_nowPlayer = BLACK_STONE;
				}
			}
		}
	}


	//for (int i= 0; i< m_vecBlock.size(); i++)
	//{
	//	if (m_vecBlock[i]->collision(_pt))
	//	{
	//		if (m_vecBlock[i]->isEmpty())
	//		{
	//			m_vecBlock[i]->setStone(m_nowPlayer);
	//			checkFiveStone(i);
	//			if (m_nowPlayer == BLACK_STONE)
	//				m_nowPlayer = WHITE_STONE;
	//			else
	//				m_nowPlayer = BLACK_STONE;
	//		}
	//	}
	//}
}

void BlockManager::checkFiveStone(int _y, int _x)
{
	int count = 1;
	int x = _x;
	int y = _y;


	for (int i = 0; i < 8; i += 2)
	{
		x = _x;
		y = _y;
		int plus_x = (DIRECTION_EIGHT[(LINE_DIR)i].x);
		int plus_y = (DIRECTION_EIGHT[(LINE_DIR)i].y);
		while (y>0 && x > 0 && (m_pBlock[y + plus_y][x + plus_x]->whatState() == m_nowPlayer))
		{
			x += plus_x;
			y += plus_y;
		}
		plus_x = (DIRECTION_EIGHT[(LINE_DIR)i+1].x);
		plus_y = (DIRECTION_EIGHT[(LINE_DIR)i+1].y);
		while ((m_pBlock[y][x]->whatState() == m_nowPlayer && x < 18 && y < 18  ))
		{
			y += plus_y;
			x += plus_x;
			count++;
		}
		//5개 체크
		if (checkFiveAndResetCount(count))
		{
			//winPlayer()
			break;
		}
	}


	////가로
	//while (x > 0 && (m_pBlock[_y][x-1]->whatState() == m_nowPlayer ))
	//{
	//	x--;
	//}
	//while ((m_pBlock[_y][x++]->whatState() == m_nowPlayer && x < 18))
	//{
	//	count++;
	//}
	//checkFiveAndResetCount(count);

	////세로
	//while (y > 0 && (m_pBlock[y - 1][_x]->whatState() == m_nowPlayer))
	//{
	//	y--;
	//}
	//while ((m_pBlock[y++][_x]->whatState() == m_nowPlayer && y < 18))
	//{
	//	count++;
	//}
	//checkFiveAndResetCount(count);
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
