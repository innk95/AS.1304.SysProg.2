#pragma once
#include <string>
#include <vector>
#include <list>
using namespace std;

struct GameConfig//Íàñòðîéêè èãðû
{
	unsigned int W;
	unsigned int H;
	unsigned int N;
	unsigned int T;
	unsigned int E_max;
	unsigned int L_max;
	unsigned int V_max;
	unsigned int R_max;
	unsigned int dL;
	unsigned int dE_S;
	unsigned int dE_V;
	unsigned int dE_A;
	unsigned int dE_P;
	unsigned int dE;
	unsigned int N_E;
	unsigned int N_L;
	float RND_min;
	float RND_max;
	unsigned int K;
};

struct RobotInfo //Õàðàêòåðèñòèêè ðîáîòà
{
	string Name;
	string Author;
	unsigned int ID;
	int E;
	int L;
	int A;
	int P;
	int V;
	unsigned int x;
	unsigned int y;
	bool Alive;
	unsigned int TotalScore;
	unsigned int RoundScore;
	unsigned int TotalFrags;
	unsigned int RoundFrags;
};

#define MOVE			0
#define ATTACK			1
#define REDISTRIBUTION	2

//Âñïîìîãàòåëüíûå êëàññû
class Action
{
protected:	
	unsigned int type;
public:
	virtual ~Action();
	unsigned int getType();
};
class Move : public Action
{
public:
	int dx;
	int dy;
	Move(int _dx, int _dy);
	~Move();
};
class Attack : public Action
{
public:
	unsigned int victimID;
	Attack(unsigned int _victimID);
	~Attack();
};
class Redistribution : public Action
{
public:
	unsigned int A;
	unsigned int P;
	unsigned int V;
	Redistribution(unsigned int _A, unsigned int _P, unsigned int _V);
	~Redistribution();
};

class RobotActions
{
	vector<Action*> m_Actions;

public:
	RobotActions(const RobotActions& from);
	RobotActions();
	~RobotActions();	
	void addActionMove(int x, int y);
	void addActionAttack(unsigned int victimID /*ID àòàêóåìîãî ðîáîòà*/);
	void addActionRedistribution(unsigned int A, unsigned int P, unsigned int V);
	vector<Action*> getActions();
};

struct StepInfo //ñòðóêòóðà, ïåðåäàâàåìàÿ ðîáîòó
{
	vector<RobotInfo> robotsInfo;											//Ñîñòîÿíèÿ âñåõ ðîáîòîâ
	unsigned int stepNumber;												//Íîìåð øàãà
	GameConfig gameConfig;													//Íàñòðîéêè èãðû
	unsigned int ID;														//ID ðîáîòà
	list<pair<unsigned int/* x */, unsigned int/* y */>> chargingStations;	//Ïóíêòû ïîäçàðÿäêè
	list<pair<unsigned int/* x */, unsigned int/* y */>> maintenance;		//Ïóíêòû òåõ. îáñëóæèâàíèÿ
	list < pair<unsigned int/* ID */, RobotActions>> actionsList;			//Ëèñò äåéñòâèé äðóãèõ ó÷àñòíèêîâ
	RobotActions* pRobotActions;											//Óêàçàòåëü íà êëàññ, êîòîðûé õðàíèò îòâåò ðîáîòà (åãî âûáðàííûå äåéñòâèÿ íà òåêóùåì øàãå) çàïîëíÿåòñÿ ðîáîòîì
};
