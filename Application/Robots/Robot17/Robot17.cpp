// Robot11.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include <fstream> 

ofstream fout;
ifstream fin;


struct Point
{
	unsigned int x;
	unsigned int y;
	Point() {}
	Point(unsigned int _x, unsigned int _y) {
		x = _x;
		y = _y;
	}

};

Point point_of_meeting;
bool ready_to_battle;

struct characteristic
{
	double max_Length_Of_Step;
	double max_Length_Of_Attack;
	double force;
	Point position;
	int *A;
	int *P;
	int *V;
	int E;
	int L;
	unsigned int ID;
	GameConfig * _GC;
	RobotInfo * _RI;
	characteristic(GameConfig * GC, RobotInfo * RI) // имя конструктора совпадает с именем класса
	{
		max_Length_Of_Step = GC->V_max*RI->V / GC->L_max*RI->E / GC->E_max;
		max_Length_Of_Attack = GC->R_max*RI->V / GC->L_max*RI->E / GC->E_max;
		force = RI->A*RI->E / GC->E_max;
		position.x = RI->x;
		position.y = RI->y;
		A = &RI->A;
		P = &RI->P;
		V = &RI->V;
		E = RI->E;
		L = RI->L;
		_GC = GC;
		_RI = RI;
		ID = RI->ID;
	}
	double calculate_max_Length_Of_Attack(int V_new) {

		return  _GC->R_max*V_new / _GC->L_max*_RI->E / _GC->E_max;

	}
	double calculate_max_Length_Of_Step(int V_new) {

		return  _GC->V_max*V_new / _GC->L_max*_RI->E / _GC->E_max;

	}
	double calculate_Force(int A_new) {

		return  A_new*E / _GC->E_max;;

	}

	void refresh_max_Length_Of_Attack(int V_new) {

		max_Length_Of_Attack = _GC->R_max*V_new / _GC->L_max*_RI->E / _GC->E_max;

	}
	void refresh_max_Length_Of_Step(int V_new) {

		max_Length_Of_Step = _GC->V_max*V_new / _GC->L_max*_RI->E / _GC->E_max;

	}
	void refresh_Force(int A_new) {

		force = A_new*E / _GC->E_max;;

	}
};



StepInfo* stepInfo;
RobotInfo* myInfo;

Point get_Meeting_Point(StepInfo* _stepInfo) {

	vector<Point> poligon;
	for (auto r = _stepInfo->robotsInfo.begin(); r != _stepInfo->robotsInfo.end(); ++r)
	{
		if (r->Author == "Panshina" || r->Author == "Lukacheva" || r->Author == "Kostyrko" || r->Author == "Fedorova")
		{
			Point p(r->x, r->y);
			poligon.push_back(p);
		}
	}

	Point center;
	center.x = poligon.at(0).x;
	center.y = poligon.at(0).y;
	int step = 2;
	for (auto i = poligon.begin() + 1; i != poligon.end(); i++)
	{
		center.x = (center.x + i->x) / step;
		center.y = (center.y + i->y) / step;
		step++;

	}

	return center;
}

pair<unsigned int, unsigned int> get_Min_Distance_To_Border(Point A, int l_x, int l_y)
{
	unsigned int len[4] = { A.x, A.y,l_x - A.x, l_y - A.y };
	pair<unsigned int, unsigned int> MIN;

	MIN.first = len[0];
	MIN.second = 0;

	for (int i = 1; i < 4;++i) {
		if (MIN.first > len[i]) {
			MIN.first = len[i];
			MIN.second = i;
		}
	}
	return MIN;

}
bool is_equal(Point a, Point b) {
	if (a.x != b.x || a.y != b.y)
		return false;
	else
		return true;
}
pair<double, pair<int, int>> getDistance_Without_Toroidal(Point posA, Point posB, GameConfig * GC, double max_length_of_step) {
	pair<double, pair<int, int>> resp;
	pair<int, int> vec;
	double length_of_step = max_length_of_step;

	unsigned int cathetus_x = max(posA.x, posB.x) - min(posA.x, posB.x);
	unsigned int cathetus_y = max(posA.y, posB.y) - min(posA.y, posB.y);

	double hypotenuse = sqrt(cathetus_x*cathetus_x + cathetus_y * cathetus_y);

	double cos_x = double(cathetus_x) / double(hypotenuse);
	double cos_y = double(cathetus_y) / double(hypotenuse);


	if (posB.x > posA.x)
		vec.first = -1;
	if (posB.x < posA.x)
		vec.first = 1;
	if (posB.x == posA.x)
		vec.first = 0;

	if (posB.y > posA.y)
		vec.second = -1;
	if (posB.y < posA.y)
		vec.second = 1;
	if (posB.y == posA.y)
		vec.second = 0;

	length_of_step = (hypotenuse > max_length_of_step) ? max_length_of_step : hypotenuse;

	vec.first = vec.first * length_of_step * cos_x;
	vec.second = vec.second *  length_of_step *cos_y;

	resp.first = hypotenuse;
	resp.second = vec;
	return resp;

}
pair<double, pair<int, int>> getDistancde(Point posA, Point posB, GameConfig * GC, double max_length_of_step)
{
	pair<double, pair<int, int>> resp;
	pair<int, int> vec;
	double length_of_step = max_length_of_step;

	unsigned int cathetus_x = max(posA.x, posB.x) - min(posA.x, posB.x);
	unsigned int cathetus_y = max(posA.y, posB.y) - min(posA.y, posB.y);

	double hypotenuse = sqrt(cathetus_x*cathetus_x + cathetus_y * cathetus_y);

	double cos_x = double(cathetus_x) / double(hypotenuse);
	double cos_y = double(cathetus_y) / double(hypotenuse);

	double d_hypotenuse; //distance based on the toroidal field

	pair<unsigned int, unsigned int> MIN = get_Min_Distance_To_Border(posA, GC->H, GC->W);

	d_hypotenuse = MIN.first;
	switch (MIN.second) {
	case 0:
		cathetus_x = max(posA.x, posB.x) - min(posA.x, posB.x);
		cathetus_y = max(posA.y, GC->H) - min(posA.y, GC->H);
		d_hypotenuse = d_hypotenuse + sqrt(cathetus_x*cathetus_x + cathetus_y * cathetus_y);
		vec.first = 0;
		vec.second = -length_of_step;
		break;
	case 1:
		cathetus_x = max(posA.x, GC->W) - min(posA.x, GC->W);
		cathetus_y = max(posA.y, posA.y) - min(posA.y, posA.y);
		d_hypotenuse = d_hypotenuse + sqrt(cathetus_x*cathetus_x + cathetus_y * cathetus_y);
		vec.first = -length_of_step;
		vec.second = 0;
		break;
	case 2:
		cathetus_x = max(posA.x, posB.x) - min(posA.x, posB.x);
		cathetus_y = max(posA.y, 0) - min(posA.y, 0);
		d_hypotenuse = d_hypotenuse + sqrt(cathetus_x*cathetus_x + cathetus_y * cathetus_y);
		vec.first = 0;
		vec.second = length_of_step;
		break;
	case 3:
		cathetus_x = max(posA.x, 0) - min(posA.x, 0);
		cathetus_y = max(posA.y, posA.y) - min(posA.y, posA.y);
		d_hypotenuse = d_hypotenuse + sqrt(cathetus_x*cathetus_x + cathetus_y * cathetus_y);
		vec.first = length_of_step;
		vec.second = 0;
		break;
	}

	if (d_hypotenuse >= hypotenuse) {
		length_of_step = (hypotenuse > max_length_of_step) ? max_length_of_step : hypotenuse;
		if (posB.x > posA.x)
			vec.first = -1;
		if (posB.x < posA.x)
			vec.first = 1;
		if (posB.x == posA.x)
			vec.first = 0;

		if (posB.y > posA.y)
			vec.second = -1;
		if (posB.y < posA.y)
			vec.second = 1;
		if (posB.y == posA.y)
			vec.second = 0;

		vec.first = vec.first * length_of_step * cos_x;
		vec.second = vec.second *  length_of_step *cos_y;

		resp.first = hypotenuse;

	}
	else {

		resp.first = d_hypotenuse;
	}




	resp.second = vec;
	return resp;
}
pair<unsigned int, unsigned int> getNearest(list<pair<unsigned int, unsigned int>> LST, characteristic CHRCTR, StepInfo *SI) {

	pair<unsigned int, unsigned int>  nearest;
	nearest.first = -1;
	double min_dist;
	double min_dist_buff;
	Point p;

	for (auto it = LST.begin(); it != LST.end(); ++it) {
		if (nearest.first == -1) {
			nearest = *it;
			min_dist = getDistance_Without_Toroidal(Point(nearest.first, nearest.second), CHRCTR.position, &SI->gameConfig, 0).first;
		}
		else
		{
			p.x = it->first;
			p.y = it->second;
			min_dist_buff = getDistance_Without_Toroidal(p, CHRCTR.position, &SI->gameConfig, 0).first;
			if (min_dist > min_dist_buff) {
				min_dist = min_dist_buff;
				nearest = *it;
			}
		}
	}
	return nearest;
}
bool everyone_is_here(StepInfo* _stepInfo) {
	Point point_of_meeting(_stepInfo->gameConfig.W / 2, _stepInfo->gameConfig.H / 2);

	for (auto it = _stepInfo->robotsInfo.begin(); it != _stepInfo->robotsInfo.end(); ++it) {
		if (_stepInfo->ID != it->ID && (it->Author == "Panshina" || it->Author == "Lukacheva" || it->Author == "Kostyrko") && it->Alive) {
			if (!is_equal(point_of_meeting, Point(it->x, it->y)))
				return false;
		}
		return true;
	}
}


pair<unsigned int, Point> get_weekness_robot(StepInfo* _stepInfo, characteristic CHRCTR)
{
	unsigned int id = 0;
	int protection = 100;
	Point p;
	bool find = false;
	if (!find)
	{
		for (auto it = _stepInfo->robotsInfo.begin(); it != _stepInfo->robotsInfo.end(); ++it)
		{
			if (it->Author == "Timoshin")
			{
				if (it->P < protection &&it->Alive)
				{
					id = it->ID;
					p.x = it->x;
					p.y = it->y;
					protection = it->P;
					find = true;
				}
			}

		}
	}
	if (!find)
	{
		for (auto it = _stepInfo->robotsInfo.begin(); it != _stepInfo->robotsInfo.end(); ++it)
		{
			if (it->ID != CHRCTR.ID && (it->Author != "Panshina" && it->Author != "Lukacheva" && it->Author != "Kostyrko")) {
				if (it->P < protection &&it->Alive)
				{
					id = it->ID;
					p.x = it->x;
					p.y = it->y;
					protection = it->P;
				}
			}
		}
	}
	return pair<unsigned int, Point>(id, p);
}

bool in_team(unsigned int  ID, string Author, characteristic CHRCTR) {

	if (ID == CHRCTR.ID || (Author == "Panshina" || Author == "Lukacheva" || Author == "Kostyrko"))
		return true;
	else
		return false;
}


int get_summ_E_Of_Team(StepInfo* _stepInfo, characteristic CHRCTR)
{
	int summ = 0;
	for (auto it = _stepInfo->robotsInfo.begin(); it != _stepInfo->robotsInfo.end(); ++it) {
		if (in_team(it->ID, it->Author, CHRCTR)) {
			summ += it->E;
		}
	}

	return summ;
}


pair<int, int> go_To_TheNearest_chargingStation(StepInfo* _stepInfo, characteristic CHRCTR)
{
	pair<unsigned int, unsigned int>  nearest_chargingStations;
	Point p;
	pair<double, pair<int, int>>  move;
	nearest_chargingStations = getNearest(_stepInfo->chargingStations, CHRCTR, _stepInfo);
	p.x = nearest_chargingStations.first;
	p.y = nearest_chargingStations.second;
	if (!is_equal(p, CHRCTR.position)) {
		move = getDistance_Without_Toroidal(p, CHRCTR.position, &_stepInfo->gameConfig, CHRCTR.max_Length_Of_Step);
	}
	return pair<int, int>(move.second.first, move.second.second);
}

pair<int, int> go_To_TheNearest_tech_Station(StepInfo* _stepInfo, characteristic CHRCTR)
{
	pair<unsigned int, unsigned int>  nearest_tech_Stations;
	Point p;
	pair<double, pair<int, int>>  move;
	nearest_tech_Stations = getNearest(_stepInfo->maintenance, CHRCTR, _stepInfo);
	p.x = nearest_tech_Stations.first;
	p.y = nearest_tech_Stations.second;
	if (!is_equal(p, CHRCTR.position)) {
		move = getDistance_Without_Toroidal(p, CHRCTR.position, &_stepInfo->gameConfig, CHRCTR.max_Length_Of_Step);
	}
	return pair<int, int>(move.second.first, move.second.second);
}

extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{

	for (auto it = _stepInfo->robotsInfo.begin(); it != _stepInfo->robotsInfo.end(); ++it) {
		if (_stepInfo->ID == it->ID) {

			myInfo = new RobotInfo(*it); //robot receives information about its state
			break;
		}
	}

	characteristic CHRCTR(&_stepInfo->gameConfig, myInfo);
	Point p;
	pair<unsigned int, unsigned int>  nearest_chargingStations;
	pair<unsigned int, unsigned int>  nearest_maintenance;
	pair<double, pair<int, int>>   nearest_robot;
	pair<int, int> delta;
	bool attack;
	bool recharge;
	bool treatment;
	point_of_meeting.x = _stepInfo->gameConfig.W / 2;
	point_of_meeting.y = _stepInfo->gameConfig.H / 2;

	pair<double, pair<int, int>>  move;
	int d_x;
	int d_y;

	//	nearest_chargingStations = getNearest(_stepInfo->chargingStations, CHRCTR, _stepInfo);
	//nearest_maintenance = getNearest(_stepInfo->maintenance, CHRCTR, _stepInfo);



	double probable_attack_distance = CHRCTR.calculate_max_Length_Of_Attack(20);
	unsigned int id_for_attack;
	for (auto it = _stepInfo->robotsInfo.begin(); it != _stepInfo->robotsInfo.end(); ++it) {
		if (it->ID != CHRCTR.ID && (it->Author != "Panshina" && it->Author != "Lukacheva" && it->Author != "Kostyrko")) {
			p.x = it->x;
			p.y = it->y;
			pair<double, pair<int, int>>  buff;
			buff = getDistance_Without_Toroidal(p, CHRCTR.position, &_stepInfo->gameConfig, 0);
			if (it - _stepInfo->robotsInfo.begin() == 0) {
				nearest_robot = buff;
				id_for_attack = it->ID;
			}
			else {
				nearest_robot = (nearest_robot.first > buff.first) ? buff : nearest_robot;
				id_for_attack = it->ID;
			}
		}
	}


	if (!is_equal(point_of_meeting, CHRCTR.position) && ready_to_battle == false) {


		move = getDistance_Without_Toroidal(point_of_meeting, CHRCTR.position, &_stepInfo->gameConfig, CHRCTR.max_Length_Of_Step); // процедура сбора
		d_x = move.second.first;
		d_y = move.second.second;
		_stepInfo->pRobotActions->addActionRedistribution(0.4*_stepInfo->gameConfig.L_max, 0.4 * _stepInfo->gameConfig.L_max, 0.2*_stepInfo->gameConfig.L_max);
		_stepInfo->pRobotActions->addActionMove(d_x, d_y);
	}
	else {
		if (everyone_is_here(_stepInfo) && ready_to_battle == false) { // процедура боя
			ready_to_battle = true;
		}
	}

	if (ready_to_battle) {

		nearest_chargingStations = getNearest(_stepInfo->chargingStations, CHRCTR, _stepInfo);
		double dist = getDistance_Without_Toroidal(Point(nearest_chargingStations.first, nearest_chargingStations.second), CHRCTR.position, &_stepInfo->gameConfig, 0).first;
		if ((_stepInfo->gameConfig.N - _stepInfo->stepNumber - dist) <= (_stepInfo->gameConfig.E_max / _stepInfo->gameConfig.dE))
		{
			pair<int, int> station = go_To_TheNearest_chargingStation(_stepInfo, CHRCTR);
			_stepInfo->pRobotActions->addActionMove(station.first, station.second);
			if ((station.first = myInfo->x) && ((station.second = myInfo->y)))
			{
				int set_P = 0;
				int get_A = 0;
				int get_V = 0;
				if (*CHRCTR.A > 20 + _stepInfo->gameConfig.dL / 2)
				{
					set_P += _stepInfo->gameConfig.dL / 2;
					get_A = _stepInfo->gameConfig.dL / 2;
				}
				if (*CHRCTR.P >20 + _stepInfo->gameConfig.dL / 2) {
					set_P += _stepInfo->gameConfig.dL / 2;
					get_V = _stepInfo->gameConfig.dL / 2;
				}
				_stepInfo->pRobotActions->addActionRedistribution(*CHRCTR.A - get_A, *CHRCTR.P - get_V, *CHRCTR.V + set_P);
			}
		}
		else {
			attack = true;
			recharge = false;
			treatment = false;

			if (CHRCTR.L < _stepInfo->gameConfig.L_max*0.5) {
				recharge = false;
				attack = false;
				treatment = true;
			}

			int E_summ = get_summ_E_Of_Team(_stepInfo, CHRCTR);
			if (CHRCTR.E < _stepInfo->gameConfig.E_max*0.9) {
				recharge = true;
				attack = false;
				treatment = false;
			}

			if (recharge) {
				if (CHRCTR.E > 0.98  * _stepInfo->gameConfig.E_max) {
					recharge = false;
					attack = true;
				}
				else {
					/*_stepInfo->pRobotActions->addActionRedistribution(*CHRCTR.A-4, *CHRCTR.P-4, *CHRCTR.V+8);
					CHRCTR.refresh_max_Length_Of_Attack(*CHRCTR.V + 8);
					CHRCTR.refresh_max_Length_Of_Step(*CHRCTR.V + 8);*/
					delta = go_To_TheNearest_chargingStation(_stepInfo, CHRCTR);


					if (delta.first == 0 && delta.second == 0) {
						int set_P = 0;
						int get_A = 0;
						int get_V = 0;
						if (*CHRCTR.A > 20 + _stepInfo->gameConfig.dL / 2) {
							set_P += _stepInfo->gameConfig.dL / 2;
							get_A = _stepInfo->gameConfig.dL / 2;
						}
						if (*CHRCTR.V> 20 + _stepInfo->gameConfig.dL / 2) {
							set_P += _stepInfo->gameConfig.dL / 2;
							get_V = _stepInfo->gameConfig.dL / 2;
						}
						_stepInfo->pRobotActions->addActionRedistribution(*CHRCTR.A - get_A, *CHRCTR.P + set_P, *CHRCTR.V - get_V);

					}
					else {
						_stepInfo->pRobotActions->addActionMove(delta.first, delta.second);
						int set_V = 0;
						int get_A = 0;
						int get_P = 0;
						if (*CHRCTR.A > 20 + _stepInfo->gameConfig.dL / 2) {
							set_V += _stepInfo->gameConfig.dL / 2;
							get_A = _stepInfo->gameConfig.dL / 2;
						}
						if (*CHRCTR.P >20 + _stepInfo->gameConfig.dL / 2) {
							set_V += _stepInfo->gameConfig.dL / 2;
							get_P = _stepInfo->gameConfig.dL / 2;
						}
						_stepInfo->pRobotActions->addActionRedistribution(*CHRCTR.A - get_A, *CHRCTR.P - get_P, *CHRCTR.V + set_V);
					}
				}

			}

			if (treatment) {
				if (CHRCTR.L> 0.8  * _stepInfo->gameConfig.L_max) {
					recharge = false;
					attack = true;
					treatment = false;
				}
				else {
					delta = go_To_TheNearest_tech_Station(_stepInfo, CHRCTR);
					if (delta.first == 0 && delta.second == 0) {
						int set_P = 0;
						int get_A = 0;
						int get_V = 0;
						if (*CHRCTR.A >20 + _stepInfo->gameConfig.dL / 2) {
							set_P += _stepInfo->gameConfig.dL / 2;
							get_A = _stepInfo->gameConfig.dL / 2;
						}
						if (*CHRCTR.V>20 + _stepInfo->gameConfig.dL / 2) {
							set_P += _stepInfo->gameConfig.dL / 2;
							get_V = _stepInfo->gameConfig.dL / 2;
						}
						_stepInfo->pRobotActions->addActionRedistribution(*CHRCTR.A - get_A, *CHRCTR.P + set_P, *CHRCTR.V - get_V);

					}
					else {
						_stepInfo->pRobotActions->addActionMove(delta.first, delta.second);
						int set_V = 0;
						int get_A = 0;
						int get_P = 0;
						if (*CHRCTR.A > 20 + _stepInfo->gameConfig.dL / 2) {
							set_V += _stepInfo->gameConfig.dL / 2;
							get_A = _stepInfo->gameConfig.dL / 2;
						}
						if (*CHRCTR.P >20 + _stepInfo->gameConfig.dL / 2) {
							set_V += _stepInfo->gameConfig.dL / 2;
							get_P = _stepInfo->gameConfig.dL / 2;
						}
						_stepInfo->pRobotActions->addActionRedistribution(*CHRCTR.A - get_A, *CHRCTR.P - get_P, *CHRCTR.V + set_V);
					}
				}

			}

			if (attack) {
				if (CHRCTR.L == _stepInfo->gameConfig.L_max) {
					_stepInfo->pRobotActions->addActionRedistribution(CHRCTR.L*0.2, CHRCTR.L*0.6, CHRCTR.L*0.2);

				}
				else
				{
					int dl = CHRCTR.L*0.1;
					if (*CHRCTR.V > 0.25*CHRCTR.L)
						_stepInfo->pRobotActions->addActionRedistribution(CHRCTR.L + dl*0.2, CHRCTR.L + dl*0.6, CHRCTR.L - dl*0.8);
				}


				pair<unsigned int, Point> uf = get_weekness_robot(_stepInfo, CHRCTR);
				move = getDistance_Without_Toroidal(uf.second, CHRCTR.position, &_stepInfo->gameConfig, CHRCTR.max_Length_Of_Step);
				double probable_attack_distance = CHRCTR.calculate_max_Length_Of_Attack(CHRCTR.L*0.4);
				if (move.first > probable_attack_distance)
					_stepInfo->pRobotActions->addActionMove(move.second.first, move.second.second);
				else
					_stepInfo->pRobotActions->addActionAttack(uf.first);
			}

		}





		/*nearest_chargingStations = getNearest(_stepInfo->chargingStations, CHRCTR, _stepInfo);
		p.x = nearest_chargingStations.first;
		p.y = nearest_chargingStations.second;
		if (!is_equal(p, CHRCTR.position)) {

		move = getDistance(p, CHRCTR.position, &_stepInfo->gameConfig, CHRCTR.max_Length_Of_Step);
		_stepInfo->pRobotActions->addActionRedistribution(1, 1, CHRCTR.L - 2);
		d_x = move.second.first;
		d_y = move.second.second;
		_stepInfo->pRobotActions->addActionMove(d_x, d_y);
		}
		else {
		/*if (nearest_robot.first < probable_attack_distance) {
		_stepInfo->pRobotActions->addActionRedistribution(CHRCTR.L - 21, 1, 20);
		_stepInfo->pRobotActions->addActionAttack(id_for_attack);
		}
		_stepInfo->pRobotActions->addActionRedistribution(40, 40, 20);
		pair<unsigned int, Point> uf = get_weekness_robot(_stepInfo, CHRCTR);

		move = getDistance(uf.second, CHRCTR.position, &_stepInfo->gameConfig, CHRCTR.max_Length_Of_Step);
		_stepInfo->pRobotActions->addActionMove(move.second.first, move.second.second);
		}*/

	}

}
