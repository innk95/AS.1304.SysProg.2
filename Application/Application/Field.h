#pragma once
#include "stdafx.h"


struct Cell
{
	BYTE type = 0;
	RobotInfo* pRobot = NULL;
	RobotInfo* pDeadRobot = NULL;
};

#define STANDARD			0
#define CHARGING_STATION	1
#define MAINTENANCE			2
#define BONES				3
#define CS_B				4
#define M_B					5


class Field
{	
	UINT m_Width;
	UINT m_Height;
	vector<Cell>* m_pCells;
public:
	Field(UINT width, UINT height);
	~Field();

	Cell* at(CPoint pos);
	void addChargingStation(CPoint pos);
	void addMaintenance(CPoint pos);
	void occupyCell(CPoint pos, RobotInfo* _pRobot);
	void freeCell(CPoint pos);
	void addDeadRobot(CPoint pos, RobotInfo* _pDeadRobot);
	bool isOccupied(CPoint pos);

};

