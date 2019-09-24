#include "Block.h"
#include "ResManager.h"
#include "BitMap.h"



Block::Block()
{
}


Block::~Block()
{
}

void Block::init(int _x, int _y)
{
	m_bitMap[BLACK_STONE] = ResManager::getInst()->getBitMap(RES_FILE_BLACK_STONE);
	m_bitMap[WHITE_STONE] = ResManager::getInst()->getBitMap(RES_FILE_WHITE_STONE);
	m_state = BLOCK_EMPTY;
	//m_state = WHITE_STONE;

	m_position.x = _x;
	m_position.y = _y;
}

void Block::update()
{
}

void Block::render(HDC _hdc)
{
	if (m_state != BLOCK_EMPTY)
	{
		m_bitMap[m_state]->render(_hdc, m_position.x, m_position.y,STONE_SIZE,STONE_SIZE);
	}
}

bool Block::collision(POINT _mouse)
{
	RECT rc = { m_position.x,m_position.y,m_position.x + STONE_SIZE,m_position.y + STONE_SIZE };

	if (PtInRect(&rc, _mouse))
	{
		return true;
	}
	return false;
}

void Block::setStone(WHAT_BLOCK_STATE _stone)
{
	m_state = _stone;
}

bool Block::isEmpty()
{
	if (m_state == BLOCK_EMPTY)
		return true;
	else
		return false;
}
