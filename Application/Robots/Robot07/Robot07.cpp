// Robot07.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

using namespace std;

StepInfo* cur_info;

const double energy_weight = 0.65;
const double restore_weight = 0.95;
const double attack_weight = 1./4;
const double protection_weight = 1./4;
const double velocity_weight = 1./4;
const double step_weight = 0.95;
const double fight_weight = 1.5;

const set<string> team = { "Tinky-Winky", "Dipsy", "Laa-Laa", "Poo" };
const set<string> neutrals = {"THRASH_SPEED_METAL"};

bool restore_flag = false;

struct Cell
{
	int x;
	int y;
	Cell(int X, int Y):
		x(X), y(Y)
	{
		
	}

	bool operator==(const Cell& eq)
	{
		return (eq.x == x && eq.y == y);
	}
		
};

void go(RobotInfo& me, Cell to);
double get_distance(Cell& c1, Cell& c2);
RobotInfo get_robot(int id);
Cell get_halt(RobotInfo& me, bool energy);
RobotInfo get_enemy();
void attack(RobotInfo& me, RobotInfo& target);

inline bool condition_critical(RobotInfo& me)
{
	auto max_tech = cur_info->gameConfig.L_max;
	if (me.L < cur_info->gameConfig.L_max)
		if (me.A < attack_weight * max_tech || me.P < protection_weight * max_tech ||
			me.V < velocity_weight * max_tech)
			return true;

	return false;
}

inline bool energy_critical(RobotInfo& me)
{
	auto max_e = cur_info->gameConfig.E_max;
	if (me.E < energy_weight * max_e || cur_info->stepNumber > step_weight * cur_info->gameConfig.N)
		return true;
	return false;
}

inline void on_idle(RobotInfo& me)
{
	static bool revert = false;
	if (revert)
		cur_info->pRobotActions->addActionRedistribution(me.A - 1, me.P + 1, me.V);
	else
		cur_info->pRobotActions->addActionRedistribution(me.A + 1, me.P - 1, me.V);

	revert ^= true;
}


extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
	cur_info = _stepInfo;
	auto me = get_robot(cur_info->ID);
	auto params = cur_info->gameConfig;

	//if (me.E > restore_weight * params.E_max)
	//{
	//	restore_flag = false;
	//}

	
	if (energy_critical(me))
	{
		//if (restore_flag)
		//	return;
		Cell nrj = get_halt(me, true);
		Cell myself(me.x, me.y);
		if (myself == nrj)
		{
			on_idle(me);
			//restore_flag = true;
			return; 
		}

		go(me, nrj);
		
	}
	else
		if (condition_critical(me))
		{
			Cell tech = get_halt(me, false);
			Cell myself(me.x, me.y);
			//if (myself == tech && me.L < restore_weight * params.L_max)
			//{
			//	on_idle(me);
			//	return;
			//}
			//else
			//{
				go(me, tech);
			//}
		}
		else
		{
			auto target = get_enemy();
			if (target.ID == -1)
			{
				on_idle(me);
			}
			else
			{
				attack(me, target);
			}
		}
}

void go(RobotInfo& me, Cell to)
{
	auto& params = cur_info->gameConfig;
	Cell cur(me.x, me.y);
	auto distance = get_distance(cur, to);
	int max_distance = params.V_max * me.V / params.L_max * me.E / params.E_max; 
	double dx = to.x - cur.x;
	double dy = to.y - cur.y;

	if (distance > max_distance)
	{
		double alpha = atan2(dy,dx);
		dx = max_distance * cos(alpha);
		dy = max_distance * sin(alpha);
	}

	cur_info->pRobotActions->addActionMove(dx, dy);
}

double get_distance(Cell& c1, Cell& c2)
{
	return sqrt(pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2));
}

RobotInfo get_robot(int id)
{
	auto list = cur_info->robotsInfo;
	for (auto robot : list)
	{
		if (robot.ID == id)
			return robot;
	}
}

Cell get_halt(RobotInfo& me, bool energy)
{
	list<pair<unsigned int, unsigned int>> stations;
	if (energy)
		stations = cur_info->chargingStations;
	else
		stations = cur_info->maintenance;
	
	double distance = cur_info->gameConfig.W * cur_info->gameConfig.H;
	Cell myself(me.x, me.y);
	Cell result(myself);

	for (auto station : stations)
	{
		Cell place(station.first, station.second);
		double cur_distance = get_distance(myself, place);
		if (cur_distance < distance)
		{
			distance = cur_distance;
			result = place;
		}
	}


	return result;
}

RobotInfo get_enemy()
{
	auto list = cur_info->actionsList;
	for (auto robot : list)
	{
		auto actions = robot.second.getActions();
		for (auto action : actions)
		{
			if (action->getType() == ATTACK)
			{
				Attack* casted = dynamic_cast<Attack*>(action);
				if (casted->victimID == cur_info->ID)
				{
					return get_robot(robot.first);
				}
			}
		}
	}


	auto victims = cur_info->robotsInfo;
	RobotInfo target;
	bool got_it = false;

	double criteria = cur_info->gameConfig.E_max * cur_info->gameConfig.L_max;

	for (auto robot : victims)
	{
		if (robot.Alive && team.find(robot.Name) == team.end() && neutrals.find(robot.Name) == neutrals.end())
		{
			
			if (robot.E * robot.P < criteria)
			{
				criteria = robot.E * robot.P;
				target = robot;
				got_it = true;
			}
		}
	}

	if (!got_it)
	{
		target.ID = -1;
	}

	return target;

}

void attack(RobotInfo & me, RobotInfo & target)
{		
	auto params = cur_info->gameConfig;
	if (target.E * target.P < fight_weight * me.E * me.A  && target.Alive)
	{
		Cell myself(me.x, me.y);
		Cell enemy(target.x, target.y);
		auto distance = get_distance(myself, enemy);
	
		auto max_distance = params.R_max * me.V / params.L_max * me.E / params.E_max;
		if (distance > max_distance)
		{
			go(me, enemy);
		}
		else
		{
			cur_info->pRobotActions->addActionAttack(target.ID);
		}
	}
	else
	{
		on_idle(me);
	}
}

