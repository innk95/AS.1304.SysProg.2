// DynamicLL.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
//#include "resources.h"



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


Characteristics getRandomCharacteristics(UINT L)
{
	UINT A = rand() % L;
	UINT P = rand() % (L - A);
	UINT V = L - A - P;
	return Characteristics(A, P, V);
}


extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
	//RobotInfo _myInfo;
	stepInfo = _stepInfo;

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
	if ((double)myInfo->E / stepInfo->gameConfig.E_max < 0.6 &&
		myInfo->E - stepInfo->gameConfig.dE_V > 0) {
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
		else {
			stepInfo->pRobotActions->addActionMove(1, 1);
		}
	}
	else {//если с энергией все норм, то пытаемся атаковать ближайшего противника
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
				else {
					//расстояние, которое необходимо преодолеть, чтобы дистанция 
					//между нами и целью была допустимой для атаки
					int shift = curDistance - maxDistToAttack;
					//если для перемещения с последующей атакой нам хватит дистанции
					//и на все это хватит энергии, то...
					if (shift < maxDistToMove &&
						myInfo->E - stepInfo->gameConfig.dE_V - stepInfo->gameConfig.dE_A > 0) {

						double ratio_x = abs(vInfo.dX) / (abs(vInfo.dX) + abs(vInfo.dY));
						int shift_x = ratio_x * shift;

						if (vInfo.dY >= 0) {
							stepInfo->pRobotActions->addActionMove(shift_x, shift - abs(shift_x));
							myInfo->x += shift_x;
							myInfo->y += shift - abs(vInfo.dX);
						}
						else {
							stepInfo->pRobotActions->addActionMove(shift_x, -(shift - abs(shift_x)));
							myInfo->x += shift_x;
							myInfo->y += -(shift - abs(vInfo.dX));
						}

						maxDistToAttack = stepInfo->gameConfig.R_max * myInfo->V * (myInfo->E - stepInfo->gameConfig.dE_V) /
							(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
						Point newDist = getDistance(Point(myInfo->x, myInfo->y), Point(victim->x, victim->y));
						if (Pythagoras(newDist.x, newDist.y) <= maxDistToAttack)
							stepInfo->pRobotActions->addActionAttack(vInfo.ID);
					}
					else {
						//нам не хватило дистанции => просто пойдем по направлению к
						//противнику и проверим есть ли у нас на это энергия
						if (myInfo->E - stepInfo->gameConfig.dE_V > 0) {

							double ratio_x = abs(vInfo.dX) / (abs(vInfo.dX) + abs(vInfo.dY));
							int shift_x = ratio_x * maxDistToMove;

							if (vInfo.dY >= 0) {
								stepInfo->pRobotActions->addActionMove(shift_x, maxDistToMove - abs(shift_x));
							}
							else {
								stepInfo->pRobotActions->addActionMove(shift_x, -(maxDistToMove - abs(shift_x)));
							}

						}
					}
				}
			}
			else {
				if (myInfo->E - stepInfo->gameConfig.dE_V > 0) {
					int direction = (rand() + myInfo->x) % 8;
					switch (direction) {
					case 0:
						stepInfo->pRobotActions->addActionMove(1, 0);
						break;
					case 1:
						stepInfo->pRobotActions->addActionMove(-1, 0);
						break;
					case 2:
						stepInfo->pRobotActions->addActionMove(0, 1);
						break;
					case 3:
						stepInfo->pRobotActions->addActionMove(0, -1);
						break;
					case 4:
						stepInfo->pRobotActions->addActionMove(1, 1);
						break;
					case 5:
						stepInfo->pRobotActions->addActionMove(-1, 1);
						break;
					case 6:
						stepInfo->pRobotActions->addActionMove(-1, -1);
						break;
					case 7:
						stepInfo->pRobotActions->addActionMove(1, -1);
						break;
					default:
						break;
					}//рандомное хождение
				}
			}
		}
	}

	delete myInfo;
}



