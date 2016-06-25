// Robot04.cpp: ���������� ���������������� ������� ��� ���������� DLL.
//

#include "stdafx.h"
#include <fstream>

#define ROBOT_NULL 999

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

struct State
{
	bool energy_crit = false;
	bool l_crit = false;
	unsigned int victim = ROBOT_NULL;
};

RobotInfo* me = nullptr;
StepInfo* step_info = nullptr;
Point* me_at = nullptr;
State current_state;


void store_state()
{
	ofstream of("state", ios::out | ios::binary);
	of.write((char*)&current_state, sizeof(State));
	of.flush();
	of.close();
}

void restore_state()
{
	if (step_info->stepNumber == 0) {
		current_state = State();
	}
	else {
		ifstream is("state", ios::in | ios::binary);
		is.read((char*)&current_state, sizeof(State));
		is.close();
	}
}

double distance_between(Point p1, Point p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

double max_move_distance()
{
	GameConfig config = step_info->gameConfig;
	return config.V_max * me->V * me->E / (config.L_max * config.E_max);
}

double max_attack_distance()
{
	GameConfig config = step_info->gameConfig;
	return config.R_max * me->V * me->E / (config.E_max * config.L_max);
}

Point nearest_station(bool energy)
{
	list<pair<unsigned int/* x */, unsigned int/* y */>> station = energy ? step_info->chargingStations : step_info->maintenance;
	double min_distance = 99999999;
	Point n_s(-1, -1);
	for (auto s = station.begin(); s != station.end(); s++)
	{
		Point st(s->first, s->second);
		double d = distance_between(*me_at, st);
		if (d < min_distance)
		{
			min_distance = d;
			n_s = st;
		}
	}
	return n_s;
}

RobotInfo find_robot(int id)
{
	vector<RobotInfo> robotsInfo = step_info->robotsInfo;
	for (int i = 0; i < robotsInfo.size(); i++)
	{
		if (robotsInfo[i].ID == id) return robotsInfo[i];
	}

	return RobotInfo();
}

void move(Point p)
{
	double distance = distance_between(*me_at, p);
	double max_distance = max_move_distance();
	double dx = me_at->x - p.x;
	double dy = me_at->y - p.y;
	step_info->pRobotActions->addActionRedistribution(0*me->L, 0.6*me->L, 0.4*me->L);

	if (distance < max_distance)
	{
		step_info->pRobotActions->addActionMove(-dx, -dy);
	} else
	{
		double angle = atan(dy / dx);
		if (dx > 0) angle += M_PI;
		double x = round(cos(angle) * max_distance);
		double y = round(sin(angle) * max_distance);
		step_info->pRobotActions->addActionMove(x, y);
	}
}

void attack(unsigned int id)
{
	RobotInfo info = find_robot(id);
	Point robot_location(info.x, info.y);
	double distance = distance_between(*me_at, robot_location);
	double max_distance = max_attack_distance();
	if (distance < max_distance)
	{
		step_info->pRobotActions->addActionRedistribution(0.6*me->L, 0.4*me->L, 0.0*me->L);
		step_info->pRobotActions->addActionAttack(id);
		//step_info->pRobotActions->addActionRedistribution(0 * me->L, 0.9*me->L, 0.09 * me->L);

	} else
	{
		move(robot_location);
	}
}


void find_me()
{
	for (auto r = step_info->robotsInfo.begin(); r != step_info->robotsInfo.end(); ++r) {
		if (step_info->ID == r->ID) {
			me = &(*r);
			me_at = new Point(me->x, me->y);
			break;
		}
	}
}

unsigned int get_week_robot()
{
	unsigned int id = ROBOT_NULL;
	int protection = 100;
	for (auto r = step_info->robotsInfo.begin(); r != step_info->robotsInfo.end(); ++r) {
		if (r->Author.compare("Bezzemeltsev") 
			&& r->Author.compare("NPC")
			&& r->Author.compare("Timoshin")
			&& r->Author.compare("Gutkina")
			&& r->Author.compare("Dimitrieva")
			) {
			if (r->P < protection &&r->Alive)
			{
				id = r->ID;
				protection = r->P;
			}
		}
	}
	return id;
}

bool is_energy_critical()
{
	return (((double)me->E) / step_info->gameConfig.E_max) < 0.8;
}

bool is_l_critical()
{
	return ((double)me->L / step_info->gameConfig.L_max) < 0.8;
}

unsigned int atecked_at_last()
{
	unsigned int robot = ROBOT_NULL;
	list < pair<unsigned int/* ID */, RobotActions>> actionsList = step_info->actionsList;

	for (auto it = actionsList.begin(); it != actionsList.end(); it++)
	{
		auto r_actions = (*it).second.getActions();
		for (auto a = r_actions.begin(); a != r_actions.end(); a++)
		{
			Action* act = *a;
			if (Attack* m = dynamic_cast<Attack*>(act))
			{
				if (m->victimID == me->ID) robot = (*it).first;
			}
		}
	}

	return robot;
}


void do_action()
{
	if (is_energy_critical() || step_info->stepNumber > 900) current_state.energy_crit = true;
	if (is_l_critical()) current_state.l_crit = true;



	if (current_state.energy_crit)
	{
		if ((double)me->E / step_info->gameConfig.E_max > 0.98) current_state.energy_crit = false;
		Point station = nearest_station(true);
		//step_info->pRobotActions->addActionRedistribution(0.1 * me->L, 0.85*me->L, 0.05 * me->L);
		move(station);
		return;
	}
	if (current_state.l_crit)
	{
		if (((double)me->L / step_info->gameConfig.L_max) > 0.98) current_state.l_crit = false;
		Point station = nearest_station(false);
		//step_info->pRobotActions->addActionRedistribution(0.1 * me->L, 0.85*me->L, 0.05 * me->L);
		move(station);

		return;
	}
	
	if (current_state.victim != ROBOT_NULL)
	{
		if (find_robot(current_state.victim).Alive)
		{
			attack(current_state.victim);
			return;
		}

		current_state.victim = ROBOT_NULL;
	}

	unsigned int worst_enemy = atecked_at_last();

	if (worst_enemy != ROBOT_NULL)
	{
		current_state.victim = worst_enemy;
		attack(worst_enemy);
		return;
	}

	worst_enemy = get_week_robot();

	if (worst_enemy != ROBOT_NULL)
	{
		current_state.victim = worst_enemy;
		attack(worst_enemy);
		return;
	}

	Point station = nearest_station(true);
	move(station);
}

extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
	step_info = _stepInfo;
	restore_state();
	find_me();

	do_action();

	store_state();
}

