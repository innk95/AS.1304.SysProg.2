// Robot20.cpp: îïðåäåëÿåò ýêñïîðòèðîâàííûå ôóíêöèè äëÿ ïðèëîæåíèÿ DLL.
//

// Robot20.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

StepInfo* stepInf;
RobotInfo* robotInf;

struct VictimInfo
{
	VictimInfo(int _dX, int _dY, UINT _ID) :
	dX(_dX), dY(_dY), ID(_ID){};
	int dX;
	int dY;
	UINT ID;
};

struct Point
{
	Point(){};
	Point(int _x, int _y) :
		x(_x), y(_y){};
	int x;
	int y;
};

UINT Pythagoras(int x, int y) //!!!!!!!!!
{
	return UINT(sqrt(pow(x, 2) + pow(y, 2)));
}
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
	int dist2_x = left_x + (stepInf->gameConfig.W - right_x);
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
	int dist2_y = top_y + (stepInf->gameConfig.H - bottom_y);
	if (dist1_y <= dist2_y) {
		vec.y = posB.y - posA.y;
	}
	else {
		bottom_y == posB.y ? vec.y = -dist2_y : vec.y = dist2_y;
	}

	return vec;
}



VictimInfo getVictimInfo(){
	UINT min_dX = stepInf->gameConfig.W + 1;
	UINT min_dY = stepInf->gameConfig.H + 1;
	int real_dX = 0, real_dY = 0;
	UINT victimID = 1000;
	for (auto it = stepInf->robotsInfo.begin(); it != stepInf->robotsInfo.end(); ++it) {
		Point vec = getDistance(Point(robotInf->x, robotInf->y), Point(it->x, it->y));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY) &&
			it->Alive && robotInf->Author.compare(it->Author) != 0) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = abs(vec.x);
			min_dY = abs(vec.y);
			victimID = it->ID;
		}
	}

	return VictimInfo(real_dX, real_dY, victimID);
}


Point getCStation(){
	UINT min_dX = stepInf->gameConfig.W + 1;
	UINT min_dY = stepInf->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	for (auto station = stepInf->chargingStations.begin(); station != stepInf->chargingStations.end(); ++station) {
		Point vec = getDistance(Point(robotInf->x, robotInf->y), Point(station->first, station->second));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY)) {
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
	stepInf = _stepInfo;


	for (auto it = stepInf->robotsInfo.begin(); it != stepInf->robotsInfo.end(); ++it) {
		if (_stepInfo->ID == it->ID) {
			//_myInfo = *it;
			robotInf = new RobotInfo(*it);
			break;
		}
	}

		//stepInf->pRobotActions->addActionMove(1, 1);
	
	//if (robotInf->E - stepInf->gameConfig.dE_V > 0)
	//{
		Point stationPos = getCStation();

	int curDistance = Pythagoras(stationPos.x, stationPos.y);//дистанция между нами и станцией

		stepInf->pRobotActions->addActionMove(stationPos.x, stationPos.y);
	//}
		VictimInfo vInfo = getVictimInfo();

		//stepInf->pRobotActions->addActionMove(vInfo.dX /*смещение по х*/, vInfo.dY /* смещение по у*/);
	//stepInf->pRobotActions->addActionAttack(vInfo.ID/*ID цели*/);

		delete robotInf;

}
