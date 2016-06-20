// Robot01.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

StepInfo* stepInfo;

struct Point
{
	Point(){};
	Point(int _x, int _y) :
		x(_x), y(_y){};
	int x;
	int y;
};

struct riv
{
	int P;
	int ID;
	int rast;
	int x, y;
};

UINT Pythagoras(int x, int y)
{
	return UINT(sqrt(x*x + y*y));
}

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

//Поиск станции подзарядки энергии
Point getCStation(int x, int y){
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	Point its;
	its.x = x;
	its.y = y;
	for (auto station = stepInfo->chargingStations.begin(); station != stepInfo->chargingStations.end(); ++station) {
		Point vec = getDistance(its, Point(station->first, station->second));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY)) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = abs(vec.x);
			min_dY = abs(vec.y);
		}
	}
	return Point(real_dX, real_dY);
}

//Поиск станции тех обсуживания
Point getMStation(int x, int y){
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	Point its;
	its.x = x;
	its.y = y;
	for (auto station = stepInfo->maintenance.begin(); station != stepInfo->maintenance.end(); ++station) {
		Point vec = getDistance(its, Point(station->first, station->second));
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
	srand(time(NULL));
	
	vector <riv> rivals;

	stepInfo = _stepInfo;

	int num,  ID = stepInfo->ID;//мой номер
	for (int i = 0; i < stepInfo->robotsInfo.size(); i++)
	{
		if (stepInfo->robotsInfo[i].ID == ID)
		{
			num = i;
			break;
		}
	}
	int E = stepInfo->robotsInfo[num].E;
	int L = stepInfo->robotsInfo[num].L;
	int A = stepInfo->robotsInfo[num].A;
	int P = stepInfo->robotsInfo[num].P;
	int V = stepInfo->robotsInfo[num].V;
	int x = stepInfo->robotsInfo[num].x;
	int y = stepInfo->robotsInfo[num].y;

	int Emax = stepInfo->gameConfig.E_max;
	int Lmax = stepInfo->gameConfig.L_max;
	int Vmax = stepInfo->gameConfig.V_max;
	int Rmax = stepInfo->gameConfig.R_max;

	int Smax = 1.0*Vmax*V / Lmax*E/Emax;
	int S_amax = 1.0*Rmax*V / Lmax*E / Emax;
	int Amax = 1.0*A * E / Emax;
	int Pmax = 1.0*P * E / Emax;
	Point energy, mech;
	energy = getCStation(x, y);
	mech = getMStation(x, y);

	int se = Pythagoras(energy.x, energy.y);
	int sm = Pythagoras(mech.x, mech.y);

	if ((se == 0) && ((E<0.95*Emax) || (stepInfo->stepNumber>975)) && (L>0.7*Lmax))
	{
		stepInfo->pRobotActions->addActionRedistribution(0, L, 0);
		return;
	}

	if ((sm == 0) && (L < Lmax))
	{
		stepInfo->pRobotActions->addActionRedistribution(0, L, 0);
		return;
	}
	
	if ((E < 0.85*Emax) || (stepInfo->stepNumber>980))
	{

		if (Smax > se)
		{
			stepInfo->pRobotActions->addActionMove(energy.x, energy.y);
			stepInfo->pRobotActions->addActionRedistribution(0, L, 0);
		}
		else
		{
			energy.x = 1.0*energy.x*Smax / se;
			energy.y = 1.0*energy.y*Smax / se;
			stepInfo->pRobotActions->addActionRedistribution(0, 0.4*L, 0.6*L);
			stepInfo->pRobotActions->addActionMove(energy.x, energy.y);
		}
		return;
	}


	if ((L< 0.85*Lmax))
	{

		if (Smax > sm)
		{
			stepInfo->pRobotActions->addActionMove(mech.x, mech.y);
			stepInfo->pRobotActions->addActionRedistribution(0, L, 0);
		}
		else
		{
			if (L>0.25*Lmax)
				stepInfo->pRobotActions->addActionRedistribution(0, 0.4*L, 0.6*L);
			else stepInfo->pRobotActions->addActionRedistribution(0, 0, L);

			mech.x = 1.0*mech.x*Smax / sm;
			mech.y = 1.0*mech.y*Smax / sm;
			stepInfo->pRobotActions->addActionMove(mech.x, mech.y);

		}
		return;
	}

	for (int i = 0; i < stepInfo->robotsInfo.size(); i++)
	{

		if (stepInfo->robotsInfo[i].Author != "Aganov")
		{
			Point its, rival;
			its.x = x;
			its.y = y;
			rival.x = stepInfo->robotsInfo[i].x;
			rival.y = stepInfo->robotsInfo[i].y;
			Point dist = getDistance(its, rival);
			int rast = Pythagoras(dist.x, dist.y);
			if ((S_amax >= rast) && (1.0*A*E > 0.6*stepInfo->robotsInfo[i].P*stepInfo->robotsInfo[i].E) && ((dist.x != 0) || (dist.y != 0)) && (stepInfo->robotsInfo[i].Alive))
			{
				riv ff;
				ff.P = stepInfo->robotsInfo[i].P*stepInfo->robotsInfo[i].E;
				ff.ID = stepInfo->robotsInfo[i].ID;
				ff.rast = rast;
				ff.x = dist.x;
				ff.y = dist.y;
				rivals.push_back(ff);
			}


		}

	}

	if (rivals.size() != 0)
	{
		int id_l=0, min_P = 100000;
		for (int i = 1; i < rivals.size(); i++)
		{
			if (min_P>rivals[i].P)
			{
				id_l = i;
				min_P = rivals[i].P;
			}

		}

		stepInfo->pRobotActions->addActionAttack(rivals[id_l].ID);
		stepInfo->pRobotActions->addActionMove(rivals[id_l].x+1, rivals[id_l].y);
		return;
	}


	stepInfo->pRobotActions->addActionRedistribution(0.35*L, 0.4*L, 0.25*L);
	if (stepInfo->stepNumber % 2) Smax = -Smax;
	stepInfo->pRobotActions->addActionMove(Smax, 0);

	
	return;
}




