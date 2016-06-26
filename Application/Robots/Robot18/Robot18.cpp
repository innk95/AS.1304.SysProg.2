// Robot18.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
StepInfo* stepInfo;
RobotInfo* myInfo;
UINT Pyth(int x, int y)
{
	return UINT(sqrt(pow(x, 2) + pow(y, 2)));
}
void domove(pair<int, int> point,int maxmove) {

	if (Pyth(point.first, point.second) < maxmove) {
		stepInfo->pRobotActions->addActionMove(point.first, point.second);
	}
	else {
		double ang = atan2(point.second, point.first);
		point.first = maxmove * cos(ang);
		point.second = maxmove * sin(ang);
		stepInfo->pRobotActions->addActionMove(point.first, point.second);
	}

}

pair <int,int> getCharge()
{
	auto stations = stepInfo->chargingStations;
	int min_dst = stepInfo->gameConfig.W * stepInfo->gameConfig.H;
	pair <int,int> closest;
	for (auto station : stations)
	{
		int dst = Pyth(station.first - myInfo->x, station.second - myInfo->y);
		if (dst < min_dst)
		{
			min_dst = dst;
			closest.first = station.first- myInfo->x;
			closest.second = station.second - myInfo->y;
		}
	}

	return closest;
	
}
pair <int, int> getMaintenance()
{
	auto stations = stepInfo->maintenance;
	int min_dst = stepInfo->gameConfig.W * stepInfo->gameConfig.H;
	pair <int, int> closest;
	for (auto station : stations)
	{
		int dst = Pyth(station.first - myInfo->x, station.second - myInfo->y);
		if (dst < min_dst)
		{
			min_dst = dst;
			closest.first = station.first - myInfo->x;
			closest.second = station.second - myInfo->y;
		}
	}

	return closest;

}

void randomstep() {
	int i = rand() % 5;
	int j = rand() % 1;
	switch (j) {
	case 0:stepInfo->pRobotActions->addActionMove(i , 0 );
		break;
	case 1: stepInfo->pRobotActions->addActionMove(0, i);
		break;

	}
}
void FindAndAttack(int maxattack,int maxmove)
{
	auto enemies = stepInfo->robotsInfo;
	for (auto enemy : enemies) {
		if (Pyth(enemy.x - myInfo->x, enemy.y - myInfo->y) <= maxattack && myInfo->A / (enemy.P+1)>1.5) {
			stepInfo->pRobotActions->addActionAttack(enemy.ID);
			return;
		}
		else randomstep();
	}

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
	int maxDistToAttack = stepInfo->gameConfig.R_max * myInfo->V * myInfo->E /
		(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
	pair <int, int> target;
	if (stepInfo->stepNumber == 1) {
		stepInfo->pRobotActions->addActionRedistribution(myInfo->L*0.30, myInfo->L*0.55, myInfo->L*0.15); }
	if (stepInfo->stepNumber > 0.95*stepInfo->gameConfig.N) {
		target = getCharge();
	domove(target,maxDistToMove);
	}
	if (((double)myInfo->E / stepInfo->gameConfig.E_max < 0.80 && myInfo->E - stepInfo->gameConfig.dE_V > 0)) {
		target = getCharge();
		domove(target, maxDistToMove);
	}
	else {
		if (myInfo->L < stepInfo->gameConfig.L_max) { target = getMaintenance();
		domove(target, maxDistToMove);
		}
		else { FindAndAttack(maxDistToAttack, maxDistToMove); }
	}
		delete myInfo;
}
