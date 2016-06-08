// DynamicLL.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

//#include "resources.h"

struct vrag
{
	int num, P,x,y;

};

struct Point
{
	Point(){};
	Point(int _x, int _y) :
		x(_x), y(_y){};
	int x;
	int y;
};

UINT Pythagoras(int x, int y)
{
	return UINT(sqrt(pow(x, 2) + pow(y, 2)));
}

StepInfo* stepInfo;
RobotInfo* myInfo;
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

Point getCStation(){
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	for (auto station = stepInfo->chargingStations.begin(); station != stepInfo->chargingStations.end(); ++station) {
		Point vec = getDistance(Point(myInfo->x, myInfo->y), Point(station->first, station->second));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY)) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = abs(vec.x);
			min_dY = abs(vec.y);
		}
	}
	return Point(real_dX, real_dY);
}

Point getmaintenance(){
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	for (auto station = stepInfo->maintenance.begin(); station != stepInfo->maintenance.end(); ++station) {
		Point vec = getDistance(Point(myInfo->x, myInfo->y), Point(station->first, station->second));
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
	//RobotInfo _myInfo;
	stepInfo = _stepInfo;

	vector <vrag> vragi;

	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
		if (_stepInfo->ID == it->ID) {
			//_myInfo = *it;
			myInfo = new RobotInfo(*it);
			break;
		}
	}
	srand(time(NULL));
	//myInfo = &_myInfo;
	int maxDistToMove = stepInfo->gameConfig.V_max * myInfo->V * myInfo->E /
		(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
	int maxDistToAtack = stepInfo->gameConfig.R_max * myInfo->V * myInfo->E /
		(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);

	//проверяем на критичный уровень эенергии
	if (((double)myInfo->E / stepInfo->gameConfig.E_max < 0.9) || (stepInfo->stepNumber>975))
	{
		Point stationPos = getCStation();

			int curDistance = Pythagoras(stationPos.x, stationPos.y);//дистанция между нами и станцией

			//если для перемещения на станцию нам хватит дистанции
			if (curDistance <= maxDistToMove) {
				if ((stationPos.x != 0) || (stationPos.y != 0))
				stepInfo->pRobotActions->addActionMove(stationPos.x, stationPos.y);
				stepInfo->pRobotActions->addActionRedistribution(0, myInfo->L, 0);
			}
			else {
				//нам не хватило дистанции => просто пойдем по направлению к станции
				double ratio = 1.0*maxDistToMove / curDistance;
				int shift_x = ratio * stationPos.x;
				int shift_y = ratio * stationPos.y;
				if ((shift_x != 0) || (shift_y!=0))
				stepInfo->pRobotActions->addActionMove(shift_x, shift_y);
				stepInfo->pRobotActions->addActionRedistribution(0, 0.4*myInfo->L, 0.6*myInfo->L);
			}
	return;
	}

	if ((double)myInfo->L / stepInfo->gameConfig.L_max < 0.9)
	{
		Point stationPos = getmaintenance();

		int curDistance = Pythagoras(stationPos.x, stationPos.y);//дистанция между нами и станцией

		//если для перемещения на станцию нам хватит дистанции
		if (curDistance <= maxDistToMove) {
			if ((stationPos.x != 0) || (stationPos.y != 0))
			stepInfo->pRobotActions->addActionMove(stationPos.x, stationPos.y);
			stepInfo->pRobotActions->addActionRedistribution(0, myInfo->L, 0);
		}
		else {
			//нам не хватило дистанции => просто пойдем по направлению к станции
			double ratio = 1.0*maxDistToMove / curDistance;
			int shift_x = ratio * stationPos.x;
			int shift_y = ratio * stationPos.y;
			if ((shift_x != 0) || (shift_y != 0))
			stepInfo->pRobotActions->addActionMove(shift_x, shift_y);
			stepInfo->pRobotActions->addActionRedistribution(0, 0.4*myInfo->L, 0.6*myInfo->L);
		}
		return;
	}

	for (int i = 0; i < stepInfo->robotsInfo.size();i++)
	{
		if ((!(stepInfo->robotsInfo[i].Alive)) || (stepInfo->robotsInfo[i].Author=="Dimitrieva")) continue;

		Point my, he, dist;
		my.x = myInfo->x;
		my.y = myInfo->y;
		he.x = stepInfo->robotsInfo[i].x;
		he.y = stepInfo->robotsInfo[i].y;
		
		dist = getDistance(my, he);
		
		if ((dist.x == 0) && (dist.y == 0)) continue;
		
		int dd = Pythagoras(dist.x, dist.y);
		
		if ((dd <= maxDistToAtack) && (myInfo->A*1.5 >= stepInfo->robotsInfo[i].P))
		{
			vrag vr;
			vr.P = stepInfo->robotsInfo[i].P;
			vr.num = i;
			vr.x = dist.x;
			vr.y = dist.y;
			vragi.push_back(vr);
		}


	}

	if (vragi.size())
	{
		//int ID,
		int min_P = 10000, min_i = 0;

		for (int i = 1; i < vragi.size(); i++)
		{
			if (min_P>vragi[i].P)
			{
				min_i = i;
				min_P = vragi[i].P;
			}
		}

		stepInfo->pRobotActions->addActionMove(vragi[min_i].x, vragi[min_i].y);
		stepInfo->pRobotActions->addActionAttack(stepInfo->robotsInfo[vragi[min_i].num].ID);

	}


	int compass = stepInfo->stepNumber % 4;
	stepInfo->pRobotActions->addActionRedistribution(0.3*myInfo->L, 0.45*myInfo->L, 0.25*myInfo->L);
	if (compass == 0) stepInfo->pRobotActions->addActionMove(maxDistToMove, 0);
	if (compass == 1) stepInfo->pRobotActions->addActionMove(0, maxDistToMove);
	if (compass == 2) stepInfo->pRobotActions->addActionMove(-maxDistToMove, 0);
	if (compass == 3) stepInfo->pRobotActions->addActionMove(0, -maxDistToMove);
	delete myInfo;
	return;
}



