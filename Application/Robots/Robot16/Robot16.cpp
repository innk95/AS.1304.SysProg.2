// Robot01.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

StepInfo* stepInfo;
RobotInfo* myInfo;
int pi = 3.14;
UINT Pyth(int x, int y)
{
	return UINT(sqrt(pow(x, 2) + pow(y, 2)));
}

struct Enemy
{
	Enemy(int _dX, int _dY, UINT _ID) :
		dX(_dX), dY(_dY), ID(_ID) {};
	int dX;
	int dY;
	UINT ID;
};

struct Point
{
	Point() {};
	Point(int _x, int _y) :
		x(_x), y(_y) {};
	int x;
	int y;
};
Point getDistance(Point posA, Point posB)
{
	Point vec;
	int dist1_x = abs(posA.x - posB.x);
	int dist1_y = abs(posA.y - posB.y);

	int left_x, right_x;
	if (posA.x > posB.x) {
		right_x = posA.x;
		left_x = posB.x;
	}
	else {
		right_x = posB.x;
		left_x = posA.x;
	}
	int dist2_x = left_x + (stepInfo->gameConfig.W - right_x);
	if (dist1_x <= dist2_x) {
		vec.x = posB.x - posA.x;
	}
	else {
		right_x == posB.x ? vec.x = -dist2_x : vec.x = dist2_x;
	}

	int bottom_y, top_y;
	if (posA.y > posB.y) {
		bottom_y = posA.y;
		top_y = posB.y;
	}
	else {
		bottom_y = posB.y;
		top_y = posA.y;
	}
	int dist2_y = top_y + (stepInfo->gameConfig.H - bottom_y);
	if (dist1_y <= dist2_y) {
		vec.y = posB.y - posA.y;
	}
	else {
		bottom_y == posB.y ? vec.y = -dist2_y : vec.y = dist2_y;
	}

	return vec;
}
Point getCStation() {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	for (auto station = stepInfo->chargingStations.begin(); station != stepInfo->chargingStations.end(); ++station) {
		Point vec = getDistance(Point(myInfo->x, myInfo->y), Point(station->first, station->second));
		if (Pyth(vec.x, vec.y) < Pyth(min_dX, min_dY)) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = abs(vec.x);
			min_dY = abs(vec.y);
		}
	}
	return Point(real_dX, real_dY);
}

Enemy FindEnemy() {
	UINT min_X = stepInfo->gameConfig.W + 1;
	UINT min_Y = stepInfo->gameConfig.H + 1;
	int X = 0, Y = 0;
	UINT enemyID = 500;
	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
		Point enm = getDistance(Point(myInfo->x, myInfo->y), Point(it->x, it->y));
		if (Pyth(enm.x, enm.y) < Pyth(min_X, min_Y) &&
			it->Alive && myInfo->Author.compare(it->Author) != 0) {
			X = enm.x;
			Y = enm.y;
			min_X = abs(enm.x);
			min_Y = abs(enm.y);
			enemyID = it->ID;
		}
	}
	return Enemy(X, Y, enemyID);
}



extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{

	stepInfo = _stepInfo;

	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
		if (_stepInfo->ID == it->ID) {
			//_myInfo = *it;
			myInfo = new RobotInfo(*it);
			break;
		}
	}
	srand(time(NULL));

	int maxDistToMove = stepInfo->gameConfig.V_max * myInfo->V * myInfo->E /
	(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);

	if ((double)myInfo->E / stepInfo->gameConfig.E_max < 0.50 &&
	myInfo->E - stepInfo->gameConfig.dE_V > 0) {
	Point stationPos = getCStation();
	int curDistance = Pyth(stationPos.x, stationPos.y);
	if (curDistance <= maxDistToMove) {
		stepInfo->pRobotActions->addActionMove(stationPos.x, stationPos.y);
	}
	else { stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.00, myInfo->L, myInfo->L*0.0); }
	}
	else {
		if ((double)myInfo->E / stepInfo->gameConfig.E_max < 0.75 &&
			myInfo->E - stepInfo->gameConfig.dE_V > 0) {
		Enemy nearestEnemy = FindEnemy();
	int maxDistToAttack = stepInfo->gameConfig.R_max * myInfo->V * myInfo->E /
	(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
	int curDistance = Pyth(nearestEnemy.dX, nearestEnemy.dY);
	if (curDistance <= maxDistToAttack && myInfo->E - stepInfo->gameConfig.dE_A > 0) {
		stepInfo->pRobotActions->addActionMove(nearestEnemy.dX - myInfo->x - 1, nearestEnemy.dY - myInfo->y - 1);
		stepInfo->pRobotActions->addActionAttack(nearestEnemy.ID);
	}
	stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.01, myInfo->L*0.49, myInfo->L*0.50);
	int n1 = rand() % 10 - 5;
	int n2 = rand() % 10 - 5;
	stepInfo->pRobotActions->addActionMove(n1, n2);
		}
		else
		{
			stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.01, myInfo->L*0.49, myInfo->L*0.50);
			int n1 = rand() % 10 - 5 ;
			int n2 = rand() % 10 - 5;
			stepInfo->pRobotActions->addActionMove(n1, n2);
			}
		
	}
	delete myInfo;
}
	

