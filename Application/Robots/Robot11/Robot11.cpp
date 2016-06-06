// Robot11.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

struct Characteristics
{
	Characteristics(UINT _A, UINT _P, UINT _V) :
		A(_A), P(_P), V(_V) {};
	UINT A;
	UINT P;
	UINT V;
};



struct VictimInfo
{
	VictimInfo(int _dX, int _dY, UINT _ID) :
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


extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{

}