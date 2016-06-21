// Robot21.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include <set>

const set<string> team = { "Tinky-Winky", "Dipsy", "Laa-Laa", "Poo", "TRASH_SPEED_METAL" };
StepInfo* this_step_info;
RobotInfo me;

pair<int, int> getClosestEnStation();
RobotInfo getMyself();
pair<int, int> closest_charge;
int getDst(pair<int, int> point1, pair<int, int> point2);
void doMovement(pair<int, int> dest);
void doAttackAttempt();



extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
	this_step_info = _stepInfo;
	me = getMyself();
	if (this_step_info->stepNumber == 1)
	{
		closest_charge = getClosestEnStation();
		doMovement(closest_charge);
	}
	else

		if (me.x == closest_charge.first && me.y == closest_charge.second) //still not reached
		{
			doAttackAttempt();		
		}
		else
		{
			doMovement(closest_charge);
		}

}

pair<int, int> getClosestEnStation()
{
	auto stations = this_step_info->chargingStations;
	int min_dst = this_step_info->gameConfig.W * this_step_info->gameConfig.H;
	pair<int, int> result_st;
	for (auto station : stations)
	{
		int dst = getDst(pair<int, int>(me.x, me.y), station);
		if (dst < min_dst)
		{
			min_dst = dst;
			result_st = station;
		}
	}

	return result_st;
}

RobotInfo getMyself()
{
	auto my_id = this_step_info->ID;
	for (auto robot : this_step_info->robotsInfo)
	{
		if (robot.ID == my_id)
			return robot;
	}
}

int getDst(pair<int, int> point1, pair<int, int> point2)
{
	int dst;
	dst = sqrt(pow(point1.first - point2.first, 2) + pow(point1.second - point2.second, 2));
	return dst;
}

void doMovement(pair<int, int> dest)
{
	auto config = this_step_info->gameConfig;
	int distance = getDst(pair<int, int>(me.x, me.y), dest);
	int max_distance = config.V_max * me.V / config.L_max * me.E / config.E_max;
	double dx = dest.first - me.x;
	double dy = dest.second - me.y;

	if (distance > max_distance)
	{
		double ang = atan2(dy, dx);
		dx = max_distance * cos(ang); //recount components
		dy = max_distance * sin(ang);
	}

	this_step_info->pRobotActions->addActionMove(dx, dy);
}

void doAttackAttempt()
{
	auto robots = this_step_info->robotsInfo;
	auto config = this_step_info->gameConfig;
	int max_attack_dst  = config.R_max * me.V / config.L_max * me.E / config.E_max;
	for (auto robot : robots)
	{
		if (getDst(pair<int, int>(me.x, me.y), pair<int, int>(robot.x, robot.y)) <= max_attack_dst) //opt. range
			if (team.find(robot.Name) == team.end()) //not in team
				if (robot.E * robot.P < me.A * robot.E * 1.8) //not so strong
				{
					this_step_info->pRobotActions->addActionAttack(robot.ID);
					return;
				}
	}
}
