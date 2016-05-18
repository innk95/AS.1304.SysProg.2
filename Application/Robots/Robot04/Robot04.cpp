// Robot04.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

RobotInfo* me = nullptr;

struct Point
{
	int x;
	int y;
	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	};

};
double distance_between(Point p1, Point p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

Point nearest_station(StepInfo* _stepInfo, bool energy)
{
	list<pair<unsigned int/* x */, unsigned int/* y */>> station = energy ? _stepInfo->chargingStations : _stepInfo->maintenance;
	Point me_at = Point(me->x, me->y);
	double min_distance = 99999999;
	Point n_s(-1, -1);
	for (auto s = station.begin(); s != station.end(); s++)
	{
		Point st(s->first, s->second);
		double d = distance_between(me_at, st);
		if (d < min_distance)
		{
			min_distance = d;
			n_s = st;
		}
	}
	return n_s;
}

void move(Point p)
{
	
}

void find_me(StepInfo* _stepInfo)
{
	for (auto r = _stepInfo->robotsInfo.begin(); r != _stepInfo->robotsInfo.end(); ++r) {
		if (_stepInfo->ID == r->ID) {
			me = &(*r);
			break;
		}
	}
}

extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
	find_me(_stepInfo);
}

