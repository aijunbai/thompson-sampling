#include "simulator.h"

using namespace std;
using namespace UTILS;

SIMULATOR::KNOWLEDGE::KNOWLEDGE()
:   TreeLevel(LEGAL),
    RolloutLevel(LEGAL),
    SmartTreeCount(10),
    SmartTreeValue(1.0)
{
}

SIMULATOR::STATUS::STATUS()
:   Phase(TREE),
    Particles(CONSISTENT)
{
}

SIMULATOR::SIMULATOR() 
:   NumActions(0),
    NumObservations(0),
    Discount(1.0),
    RewardRange(1.0)
{
}

SIMULATOR::SIMULATOR(int numActions, int numObservations, double discount)
:   NumActions(numActions),
    NumObservations(numObservations),
    Discount(discount),
    RewardRange(1.0)
{ 
    assert(discount > 0 && discount <= 1);
}

SIMULATOR::~SIMULATOR() 
{ 
}

void SIMULATOR::Validate(const STATE& ) const
{ 
}

bool SIMULATOR::LocalMove(STATE& , const HISTORY& ,
    int , const STATUS& ) const
{
    return true;
}

void SIMULATOR::GenerateLegal(const STATE& , /*const HISTORY& ,*/
    std::vector<int>& actions, const STATUS& ) const
{
    for (int a = 0; a < NumActions; ++a)
        actions.push_back(a);
}

void SIMULATOR::GeneratePreferred(const STATE& , const HISTORY& ,
    std::vector<int>& , const STATUS& ) const
{
}

int SIMULATOR::SelectRandom(const STATE& state, const HISTORY& history,
    const STATUS& status) const
{
    static vector<int> actions;

    if (Knowledge.RolloutLevel >= KNOWLEDGE::SMART)
    {
        actions.clear();
        GeneratePreferred(state, history, actions, status);
        if (!actions.empty())
            return actions[Random(actions.size())];
    }
        
    if (Knowledge.RolloutLevel >= KNOWLEDGE::LEGAL)
    {
        actions.clear();
        GenerateLegal(state, /*history,*/ actions, status);
        if (!actions.empty())
            return actions[Random(actions.size())];
    }

    return Random(NumActions);
}

void SIMULATOR::Prior(const STATE* state, const HISTORY& history,
    VNODE* vnode, const STATUS& status) const
{
	static std::vector<int> actions;

    if (Knowledge.TreeLevel == KNOWLEDGE::PURE || state == 0) {
        vnode->SetPrior(0, 0, true); //所有动作初始化为 (0, 0)
        return;
    }
    else {
        vnode->SetPrior(+LargeInteger, -Infinity, false); //为后面设置做准备，所有动作初始化为 (+inf, -inf)
    }

    if (Knowledge.TreeLevel >= KNOWLEDGE::LEGAL) {
    	actions.clear();
    	GenerateLegal(*state, /*history,*/ actions, status);

        for (vector<int>::const_iterator i_action = actions.begin(); i_action != actions.end(); ++i_action) {
            int a = *i_action;
            QNODE& qnode = vnode->Child(a);
            qnode.SetPrior(0, 0, true);
        }
    }
    
    if (Knowledge.TreeLevel >= KNOWLEDGE::SMART) {
        actions.clear();
        GeneratePreferred(*state, history, actions, status); //产生优先动作

        for (vector<int>::const_iterator i_action = actions.begin(); i_action != actions.end(); ++i_action) {
            int a = *i_action;
            QNODE& qnode = vnode->Child(a);
            qnode.SetPrior(Knowledge.SmartTreeCount, Knowledge.SmartTreeValue, true);
        }    
    }
}

void SIMULATOR::DisplayBeliefs(const BELIEF_STATE& ,
    ostream& ) const
{
}

void SIMULATOR::DisplayState(const STATE& , ostream& ) const
{
}

void SIMULATOR::DisplayAction(int action, ostream& ostr) const 
{
    ostr << "Action " << action << endl;
}

void SIMULATOR::DisplayObservation(const STATE& , int observation, ostream& ostr) const
{
    ostr << "Observation " << observation << endl;
}

void SIMULATOR::DisplayReward(double reward, std::ostream& ostr) const
{
    ostr << "Reward " << reward << endl;
}

double SIMULATOR::GetHorizon(double accuracy, int undiscountedHorizon) const 
{ 
    if (Discount == 1)
        return undiscountedHorizon;
    return log(accuracy) / log(Discount);
}
