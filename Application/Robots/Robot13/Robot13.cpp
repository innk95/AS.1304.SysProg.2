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

Point getCStation() {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	for (auto station = stepInfo->chargingStations.begin(); station != stepInfo->chargingStations.end(); ++station) {
		Point vec = Point(station->first - myInfo->x, station->second - myInfo->y);
		if (Pyth(vec.x, vec.y) < Pyth(min_dX, min_dY)) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = vec.x;
			min_dY = vec.y;
		}
	}
	return Point(real_dX, real_dY);
}


Point getMStation() {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	for (auto station = stepInfo->maintenance.begin(); station != stepInfo->maintenance.end(); ++station) {
		Point vec = Point(station->first - myInfo->x, station->second - myInfo->y);
		if (Pyth(vec.x, vec.y) < Pyth(min_dX, min_dY)) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = vec.x;
			min_dY = vec.y;
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

	if (((double)myInfo->E / stepInfo->gameConfig.E_max < 0.85 &&
		myInfo->E - stepInfo->gameConfig.dE_V > 0) || stepInfo->stepNumber > 0.95* stepInfo->gameConfig.N) {
		Point CstationPos = getCStation();
		if (Pyth(CstationPos.x, CstationPos.y) <= maxDistToMove) {
			stepInfo->pRobotActions->addActionMove(CstationPos.x, CstationPos.y);
			if (myInfo->E - stepInfo->gameConfig.dE_V > 0) {
				stepInfo->pRobotActions->addActionMove(1, 0);
				stepInfo->pRobotActions->addActionMove(-1, 0);
			}
		}
		else {
			if ((myInfo->y = 0) && (myInfo->y = stepInfo->gameConfig.H) && (myInfo->x = 0) && (myInfo->x = stepInfo->gameConfig.W)) { stepInfo->pRobotActions->addActionMove(maxDistToMove*0.3, maxDistToMove*0.3); }
	
			if (CstationPos.y > myInfo->y) { stepInfo->pRobotActions->addActionMove(0, -rand() % 5); }
			else
			{	stepInfo->pRobotActions->addActionMove(0,  rand() % 5);
			if (CstationPos.x > myInfo->x) { stepInfo->pRobotActions->addActionMove(-rand() % 5, 0); }
			else { stepInfo->pRobotActions->addActionMove(rand() % 5, 0); }
			}
	        
			
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
			stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.01/*новое значение A*/, myInfo->L*0.79/*новое значение P*/, myInfo->L*0.20 /* новое значение V*/);
		    stepInfo->pRobotActions->addActionMove(rand() % 5, rand() % 5);
				
			
		}
	}
delete myInfo;
}
