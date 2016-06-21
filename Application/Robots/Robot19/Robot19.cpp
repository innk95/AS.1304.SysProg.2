// Robot19.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"

using namespace std;



const set<string> team = {"Tinky-Winky", "Dipsy", "Laa-Laa", "Poo", "TRASH_SPEED_METAL"};
enum stats {none, speed, attack, protection};


struct actStruct
{
	int dst_x = -1; 
	int dst_y = -1;
	int enemy_id = -1;
	stats required = none;

}state;


pair<unsigned, unsigned> nearestStation(const list<pair<unsigned, unsigned>> &stations, int own_x, int own_y, int w, int h);
pair<int, int> getDirection(const int &own_x, const int &own_y, const int &dst_x, const int &dst_y, int w, int h);
void move(const int &dst_x, const int &dst_y, StepInfo* info, const RobotInfo& me); //one step max movement
void attackN(StepInfo* info, const RobotInfo& me);
void redist(StepInfo* info, const RobotInfo& me, stats required);
const RobotInfo& findRobot(const vector<RobotInfo>& robots, const unsigned& Id);
double getDistance(const int &x, const int &y);
int getMaxMoveDistance(const GameConfig& cfg, const RobotInfo& me);
int getMaxAttackDistance(const GameConfig& cfg, const RobotInfo& me);
void action(StepInfo* info, const RobotInfo& me);
void stateRecount(StepInfo* info, const RobotInfo& me);
int getAttackerId(list<pair<unsigned, RobotActions>> &actions, const RobotInfo& me);
int getVictimId(const vector<RobotInfo>& robots, const GameConfig& cfg);
bool isTargetSafe(const int& target_id, StepInfo* info, const RobotInfo& me);


extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
	
	const auto& me = findRobot(_stepInfo->robotsInfo, _stepInfo->ID);
	if (_stepInfo->stepNumber == 1) //alliance deployment
	{
		auto dest_charge = nearestStation(_stepInfo->chargingStations, me.x, me.y, _stepInfo->gameConfig.W, _stepInfo->gameConfig.H);
		state.required = speed;
	}
	stateRecount(_stepInfo, me);
	action(_stepInfo, me);

}



pair<unsigned, unsigned> nearestStation(const list<pair<unsigned, unsigned>> &stations, int own_x, int own_y, int w, int h)
{
	return *min_element(stations.begin(), stations.end(), [&](const pair <unsigned, unsigned> &c1, const pair <unsigned, unsigned> &c2)
		-> bool
	{
		auto tmp = abs(static_cast<int> (c1.first) - own_x);
		auto min_dx1 = min(tmp, w - tmp);
		tmp = abs(static_cast<int> (c1.second) - own_y);
		auto min_dy1 = min(tmp, h - tmp);
		tmp = abs(static_cast<int> (c2.first) - own_x);
		auto min_dx2 = min(tmp, w - tmp);
		tmp = abs(static_cast<int> (c2.second) - own_y);
		auto min_dy2 = min(tmp, h - tmp);

		if (getDistance(min_dx1, min_dy1) < getDistance(min_dx2, min_dy2))
			return true;

		return false;
	});
}



pair<int, int> getDirection(const int &own_x, const int &own_y, const int &dst_x, const int &dst_y, int w, int h)
{
	int dir_x;
	int dir_y;

	auto abs_dist = abs(own_x - dst_x);
	if (own_x > dst_x)
	{
		if (abs_dist < w - abs_dist)
			dir_x = -1;
		else
			dir_x = 1;
	}
	else
		if (own_x < dst_x)
		{
			if (abs_dist < w - abs_dist)
				dir_x = 1;
			else
				dir_x = -1;
		}
		else
			dir_x = 0;

	abs_dist = abs(own_y - dst_y);

	if (own_y > dst_y)
	{
		if (abs_dist < h - abs_dist)
			dir_y = -1;
		else
			dir_y = 1;
	}
	else
		if (own_y < dst_y)
		{
			if (abs_dist < h - abs_dist)
				dir_y = 1;
			else
				dir_y = -1;
		}
		else
			dir_y = 0;

	return pair<int, int>(dir_x, dir_y);
}

void move(const int &dst_x, const int &dst_y, StepInfo* info, const RobotInfo& me)
{
	auto own_x = static_cast<int>(me.x);
	auto own_y = static_cast<int>(me.y);
	auto direction = getDirection(own_x, own_y, dst_x, dst_y, info->gameConfig.W, info->gameConfig.H);

	auto abs_x = abs(own_x - dst_x);
	auto abs_y = abs(own_y - dst_y);

	if (!abs_x && !abs_y)
	{
		state.dst_x = -1;
		state.dst_y = -1;
		return;
	}
		
	auto distance_x = min(abs_x, info->gameConfig.W - abs_x);
	auto distance_y = min(abs_y, info->gameConfig.H - abs_y);

	int distance = getDistance(distance_x, distance_y);
	int max_distance = getMaxMoveDistance(info->gameConfig, me);

	while (distance > max_distance)
	{
		if (distance_x > 0)
			distance_x--;
		if (distance_y > 0)
			distance_y--;
		
		distance = getDistance(distance_x, distance_y);
	}

	info->pRobotActions->addActionMove(distance_x * direction.first, distance_y * direction.second);
}


void attackN(StepInfo* info, const RobotInfo& me)
{
	auto target = findRobot(info->robotsInfo, state.enemy_id);
	int max_distance = getMaxAttackDistance(info->gameConfig, me);
	
	int x_dst = abs(static_cast<int>(target.x - me.x));
	int y_dst = abs(static_cast<int>(target.y - me.y));

	int distance = getDistance(x_dst, y_dst);

	if (target.E * target.P <  1.5 * me.E * me.A && target.Alive)
	{

		if (distance > max_distance)
			move(target.x, target.y, info, me);
		else
			info->pRobotActions->addActionAttack(state.enemy_id);
	}
	else
	{
		state.enemy_id = -1;
	}
	
}

void redist(StepInfo* info, const RobotInfo& me, stats required)
{
	const auto& cfg = info->gameConfig;

	switch (required)
	{
		case speed:
		{
			int dA = 0;
			int dP = 0;
			
			if (me.A > cfg.L_max / 3)
				dA = cfg.dL / 2;
			
			if (me.P > cfg.L_max / 3)
				dP = cfg.dL / 2;
				
			info->pRobotActions->addActionRedistribution(me.A - dA, me.P - dP, me.V + dA + dP);		
			break;
		}

		case protection:
		{
			int dA = 0;
			int dV = 0;

			if (me.A > cfg.L_max / 3)
				dA = cfg.dL / 2;

			if (me.V > cfg.L_max / 3)
				dV = cfg.dL / 2;

			info->pRobotActions->addActionRedistribution(me.A - dA, me.P + dA + dV, me.V - dV);

			break;
		}

		case attack:
		{
			
			int dV = 0;
			int dP = 0;

			if (me.V > cfg.L_max / 3)
				dV = cfg.dL / 2;

			if (me.P > cfg.L_max / 3)
				dP = cfg.dL / 2;

			info->pRobotActions->addActionRedistribution(me.A + dP + dV, me.P - dP, me.V - dV);

			break;
		}
		
		case none:
		{
			
			if (me.A > me.P)
			{
				info->pRobotActions->addActionRedistribution(me.A - 1, me.P + 1, me.V);
			}
			else
			{
				info->pRobotActions->addActionRedistribution(me.A + 1, me.P - 1, me.V);
			}
			
			break;
		}


	}
	

}



const RobotInfo& findRobot(const vector<RobotInfo>& robots, const unsigned& Id)
{
	return *find_if(robots.begin(), robots.end(), [&](const RobotInfo& robot)
		-> bool
	{
		return robot.ID == Id;
	});
}


double getDistance (const int &x, const int &y)
{
	return sqrt(pow(x, 2) + pow(y, 2));
}

int getMaxMoveDistance(const GameConfig& cfg, const RobotInfo& me)
{
	return static_cast<int> (cfg.V_max * me.V / cfg.L_max * me.E / cfg.E_max);
}

int getMaxAttackDistance(const GameConfig& cfg, const RobotInfo& me)
{
	return static_cast<int> (cfg.R_max * me.V / cfg.L_max * me.E / cfg.E_max);
}

void action(StepInfo* info, const RobotInfo& me)
{
		
	redist(info, me, state.required);

	if (state.dst_x != -1 && state.dst_y != -1)
	{
		move(state.dst_x, state.dst_y, info, me);
		return;
	}

	if (state.enemy_id != -1)
	{
		attackN(info, me);
		return;
	}
}


int getAttackerId(list<pair<unsigned, RobotActions>> &actions, const RobotInfo& me)
{
	for (auto &robot : actions)
	{
		auto cur_actions = robot.second.getActions();
		for (auto act : cur_actions)
		{
			
			if (Attack* casted_action = dynamic_cast<Attack*>(act))
			{
				if (casted_action->victimID == me.ID)
				{
					return robot.first; //first this step attacker
				}
			}
		}
	}

	return -1;
}

int getVictimId(const vector<RobotInfo>& robots, const GameConfig& cfg)
{
	int min_production = cfg.E_max * cfg.L_max; //init value to compare
	int victim_id = -1;

	for (const auto &robot : robots)
	{
		if (team.find(robot.Name) == team.end() && robot.Alive) //robot isn't part of alliance
		{
			if (robot.E * robot.P < min_production)
			{
				min_production = robot.E * robot.P;
				victim_id = robot.ID;
			}
		}
	}

	return victim_id;
}

bool isTargetSafe(const int& target_id, StepInfo* info, const RobotInfo& me)
{
	if (target_id == -1)
	{
		return false;
	}

	auto target = findRobot(info->robotsInfo, target_id);
	if (target.E * target.P < 1.5 * me.E * me.A && target.Alive)
	{
		return true;
	}
	
	return false;
}

void stateRecount(StepInfo* info, const RobotInfo& me)
{
	
	const auto &cfg = info->gameConfig;
	state.required = protection;


	if (me.E < 0.7 * cfg.E_max || info->stepNumber > 0.95 * cfg.N) //the first is energy, no way. the last is too, lol
	{
		auto charge = nearestStation(info->chargingStations, me.x, me.y, cfg.W, cfg.H);
		state.dst_x = charge.first;
		state.dst_y = charge.second;
		state.required = speed;
		return;
	}

	if (me.L < 0.65 * cfg.L_max) //2nd (TODO: balance stats)
	{
		auto tech = nearestStation(info->maintenance, me.x, me.y, cfg.W, cfg.H);
		state.dst_x = tech.first;
		state.dst_y = tech.second;
		state.required = speed;
		return;
	}

	ifstream get_target("target.txt");
	if (get_target.good())
	{
		int target_id;
		get_target >> target_id;
		get_target.close();
		if (isTargetSafe(target_id, info, me))
		{		
			state.enemy_id = target_id;
			return;
		}	
	}

	ofstream define_target("target.txt", ios::trunc);

	int attacker_id = getAttackerId(info->actionsList, me);
    if (attacker_id != -1) //yep, was attacked last step 
    {
		state.enemy_id = attacker_id;
		if (define_target.good())
			define_target << attacker_id;
		state.required = protection;
    }
	else
	{
		int victim_id = getVictimId(info->robotsInfo, info->gameConfig);
		state.enemy_id = victim_id;
		if (define_target.good())
			define_target << victim_id;
		state.required = attack;
	}

	define_target.close();
	return;
}
