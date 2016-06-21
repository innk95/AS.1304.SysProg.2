// Robot13.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
StepInfo* stepInfo;
RobotInfo* myInfo;

UINT Pyth(int x, int y)
{
	return UINT(sqrt(pow(x, 2) + pow(y, 2)));
}

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


Point getMStation() {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	for (auto station = stepInfo->maintenance.begin(); station != stepInfo->maintenance.end(); ++station) {
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

	int maxDistToMove = stepInfo->gameConfig.V_max * myInfo->V * myInfo->E /
		(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);

	if ((double)myInfo->E / stepInfo->gameConfig.E_max < 0.75 &&
		myInfo->E - stepInfo->gameConfig.dE_V > 0) {
		Point CstationPos = getCStation();
		if (Pyth(CstationPos.x, CstationPos.y) <= maxDistToMove) {
			stepInfo->pRobotActions->addActionMove(CstationPos.x, CstationPos.y);
			if (myInfo->E - stepInfo->gameConfig.dE_V > 0) {
				stepInfo->pRobotActions->addActionMove(1, 0);
				stepInfo->pRobotActions->addActionMove(-1, 0);
			}
		}
		else {
			stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.01/*новое значение A*/, myInfo->L*0.79 /*новое значение P*/, myInfo->L*0.20 /* новое значение V*/);
		}

	}
	else {
		Point MstationPos = getMStation();
		if (Pyth(MstationPos.x, MstationPos.y) <= maxDistToMove) {
			stepInfo->pRobotActions->addActionMove(MstationPos.x, MstationPos.y);
			stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.01/*новое значение A*/, myInfo->L*0.79/*новое значение P*/, myInfo->L*0.20 /* новое значение V*/);
			if (myInfo->E - stepInfo->gameConfig.dE_V > 0) {
				stepInfo->pRobotActions->addActionMove(1, 0);
				stepInfo->pRobotActions->addActionMove(-1, 0);
			}
		}
		else {
			Point CstationPos = getCStation();
			if (Pyth(CstationPos.x, CstationPos.y) <= maxDistToMove) {
				stepInfo->pRobotActions->addActionMove(CstationPos.x, CstationPos.y);
			}
			else {
				stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.01/*новое значение A*/, myInfo->L*0.79/*новое значение P*/, myInfo->L*0.20 /* новое значение V*/);
			}
		}
	}
delete myInfo;
}
