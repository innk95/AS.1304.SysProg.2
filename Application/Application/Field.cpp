#include "stdafx.h"
#include "Field.h"


Field::Field(UINT width, UINT height)
{
	m_Width = width;
	m_Height = height;
	m_pCells = new vector<Cell>(width*height);
}


Field::~Field()
{
	delete m_pCells;
}


Cell* Field::at(CPoint pos)
{
	return &(m_pCells->at(pos.x * m_Width + pos.y));
}


void Field::addChargingStation(CPoint pos)
{
	this->at(pos)->type = CHARGING_STATION;
}


void Field::addMaintenance(CPoint pos)
{
	this->at(pos)->type = MAINTENANCE;
}


void Field::occupyCell(CPoint pos, RobotInfo* _pRobot)
{
	this->at(pos)->pRobot = _pRobot;
}


void Field::freeCell(CPoint pos)
{
	this->at(pos)->pRobot = NULL;
}


bool Field::isOccupied(CPoint pos)
{
	return (this->at(pos)->pRobot == NULL) ? false : true;
}


void Field::addDeadRobot(CPoint pos, RobotInfo* _pDeadRobot)
{
	Cell* cell = this->at(pos);
	switch (cell->type)
	{
	case STANDARD:
		cell->type = BONES;
		cell->pDeadRobot = _pDeadRobot;
		break;
	case CHARGING_STATION:
		cell->type = CS_B;
		cell->pDeadRobot = _pDeadRobot;
		break;
	case MAINTENANCE:
		cell->type = M_B;
		cell->pDeadRobot = _pDeadRobot;
		break;
	default:
		break;
	}
}