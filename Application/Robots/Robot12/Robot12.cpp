// Robot12.cpp: îïðåäåëÿåò ýêñïîðòèðîâàííûå ôóíêöèè äëÿ ïðèëîæåíèÿ DLL.
//

#include "stdafx.h"
UINT Pythagoras(int x, int y)
{
	return UINT(sqrt(pow(x, 2) + pow(y, 2)));
}

struct Characteristics
{
	Characteristics(UINT _A, UINT _P, UINT _V) :
		A(_A), P(_P), V(_V){};
	UINT A;
	UINT P;
	UINT V;
};


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

VictimInfo getVictimInfo(){
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 0, real_dY = 0;
	UINT victimID = 1000;
	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
		Point vec = getDistance(Point(myInfo->x, myInfo->y), Point(it->x, it->y));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY) &&
			it->Alive && myInfo->Author.compare(it->Author) != 0) {
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

extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
//RobotInfo _myInfo;
	stepInfo = _stepInfo;
	stepInfo->pRobotActions->addActionRedistribution(1 /*новое значение A*/, 98 /*новое значение P*/, 1/* новое значение V*/);

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

	//проверяем на критичный уровень эенергии
	if ((double)myInfo->E / stepInfo->gameConfig.E_max < 1) {
		Point stationPos = getCStation();
		if (stationPos.x != 1000 && stationPos.x != 1000) {
			//далее stationPos.x = stationPos.dX и  stationPos.y = stationPos.dY
			int curDistance = Pythagoras(stationPos.x, stationPos.y);//дистанция между нами и станцией

			//если для перемещения на станцию нам хватит дистанции
			if (curDistance <= maxDistToMove) {
				stepInfo->pRobotActions->addActionMove(stationPos.x, stationPos.y);
			}
			else {
				//нам не хватило дистанции => просто пойдем по направлению к станции
				double ratio_x = abs(stationPos.x) / (abs(stationPos.x) + abs(stationPos.y));
				int shift_x = ratio_x * maxDistToMove;

				if (stationPos.y >= 0) {
					stepInfo->pRobotActions->addActionMove(shift_x, maxDistToMove - abs(shift_x));
				}
				else {
					stepInfo->pRobotActions->addActionMove(shift_x, -(maxDistToMove - abs(shift_x)));
				}
			}
		}

	}
	VictimInfo vInfo = getVictimInfo();
	if (vInfo.ID != 1000) {
		RobotInfo* victim;
		for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
			if (vInfo.ID == it->ID) {
				victim = it._Ptr;
			}
		}

		float A_r = stepInfo->gameConfig.RND_min * myInfo->A;
		float P_r = (1 - stepInfo->gameConfig.RND_min) * victim->P;
		int delta = (A_r * myInfo->E - P_r * victim->E) / stepInfo->gameConfig.E_max;
		if (delta > 0) {
			int maxDistToAttack = stepInfo->gameConfig.R_max * myInfo->V * myInfo->E /
				(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
			int curDistance = Pythagoras(vInfo.dX, vInfo.dY);//дистанция между нами и целью
			//если дистанция для атаки нормальная
			//и после атаки мы не умрем, то атакуем
			if (curDistance <= maxDistToAttack && myInfo->E - stepInfo->gameConfig.dE_A > 0) {
				stepInfo->pRobotActions->addActionAttack(vInfo.ID);
			}


		}
	}
}
