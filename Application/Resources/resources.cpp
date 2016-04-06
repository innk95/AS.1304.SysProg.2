#include "resources.h"

Action::~Action()
{
	
}

RobotActions::RobotActions(const RobotActions& from)
{
	for (auto it = from.m_Actions.begin(); it != from.m_Actions.end(); ++it){
		switch ((*it)->getType())
		{
		case MOVE:{
			Move* pMove = (static_cast<Move*>(*it));
			Move* pNewMove = new Move(*pMove);			
			this->m_Actions.push_back(pNewMove);
			break;
		}
		case ATTACK:{
			Attack* pAttack = (static_cast<Attack*>(*it));
			Attack* pNewAttack = new Attack(*pAttack);
			this->m_Actions.push_back(pNewAttack);
			break;		
		}
		case REDISTRIBUTION:{
			Redistribution* pRedistribution = (static_cast<Redistribution*>(*it));
			Redistribution* pNewRedistribution = new Redistribution(*pRedistribution);
			this->m_Actions.push_back(pNewRedistribution);
			break;			
		}
		default:
			break;
		}
	}
}

RobotActions::RobotActions()
{

}

RobotActions::~RobotActions()
{
	for (auto it = m_Actions.begin(); it != m_Actions.end(); ++it){
		delete (*it);
	}
}

void RobotActions::addActionMove(unsigned int x, unsigned int y)
{
	Move* pMove = new Move(x, y);
	m_Actions.push_back(pMove);
}

void RobotActions::addActionAttack(unsigned int victimID /*ID атакуемого робота*/)
{
	Attack* pAttack = new Attack(victimID);
	m_Actions.push_back(pAttack);
}

void RobotActions::addActionRedistribution(unsigned int A, unsigned int P, unsigned int V)
{
	Redistribution* pRedistribution = new Redistribution(A, P, V);
	m_Actions.push_back(pRedistribution);
}

vector<Action*> RobotActions::getActions()
{
	return m_Actions;
}

unsigned int Action::getType(){
	return type;
}

Move::Move(int _dx, int _dy){
	type = MOVE;
	dx = _dx;
	dy = _dy;
}
Move::~Move()
{
}
Attack::Attack(unsigned int _victimID){
	type = ATTACK;
	victimID = _victimID;
}
Attack::~Attack()
{
}
Redistribution::Redistribution(unsigned int _A, unsigned int _P, unsigned int _V){
	type = REDISTRIBUTION;
	A = _A;
	P = _P;
	V = _V;
}
Redistribution::~Redistribution()
{
}
