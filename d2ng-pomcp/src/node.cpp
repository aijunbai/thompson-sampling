#include "node.h"
#include "history.h"
#include "utils.h"
#include "simulator.h"

using namespace std;

//-----------------------------------------------------------------------------

int QNODE::NumChildren = 0;

void QNODE::Initialise()
{
    assert(NumChildren);

    Children.resize(NumChildren);

    mApplicable = false;
    mCount = 0;

    Observation.Clear();
    ImmediateReward.Clear();

    for (int observation = 0; observation < QNODE::NumChildren; observation++) {
        Children[observation] = 0; //初始化为空指针
    }
}

void QNODE::DisplayValue(HISTORY& history, int maxDepth, ostream& ostr, const double *qvalue) const
{
	history.Display(ostr);
	if (qvalue) {
		ostr << "q=" << *qvalue;
	}

	ImmediateReward.Print(": r=", ostr);
	Observation.Print(", o=", ostr);
	ostr << std::endl;

    for (int observation = 0; observation < NumChildren; observation++)
    {
        if (Children[observation])
        {
        	std::stringstream ss;
        	ss << "\t\t\t#" << observation;
//            Children[observation]->GetCumulativeReward().Print(ss.str().c_str(), ostr);
        }
    }

    if (history.Size() >= maxDepth)
        return;

    for (int observation = 0; observation < NumChildren; observation++)
    {
        if (Children[observation])
        {
            history.Back().Observation = observation;
            Children[observation]->DisplayValue(history, maxDepth, ostr);
        }
    }
}

void QNODE::DisplayPolicy(HISTORY& history, int maxDepth, ostream& ostr) const
{
    history.Display(ostr);

    ImmediateReward.Print("r=", ostr);
	Observation.Print(", o=", ostr);
	ostr << std::endl;

    if (history.Size() >= maxDepth)
        return;

    for (int observation = 0; observation < NumChildren; observation++)
    {
        if (Children[observation])
        {
            history.Back().Observation = observation;
            Children[observation]->DisplayPolicy(history, maxDepth, ostr);
        }
    }
}

//-----------------------------------------------------------------------------

namespace vnode {
MEMORY_POOL<VNODE> VNodePool;
};

int VNODE::NumChildren = 0;
STATISTIC VNODE::PARTICLES_STAT;
STATISTIC VNODE::HASH_STAT;

void VNODE::Initialise()
{
    assert(NumChildren);
    assert(BeliefState.Empty());

    Children.resize(VNODE::NumChildren);
    for (int action = 0; action < VNODE::NumChildren; action++)
        Children[action].Initialise();

//    GetCumulativeReward().Set(0, 0);
    CumulativeRewards.clear();
}

VNODE* VNODE::Create()
{
    VNODE* vnode = vnode::VNodePool.Allocate();
    vnode->Initialise();
    return vnode;
}

void VNODE::Free(VNODE* root, const SIMULATOR& simulator, VNODE* ignore)
{
	if (root != ignore) {
		PARTICLES_STAT.Add(root->CumulativeRewards.size());

		root->BeliefState.Free(simulator);
		vnode::VNodePool.Free(root);

		for (int action = 0; action < VNODE::NumChildren; action++)
			for (int observation = 0; observation < QNODE::NumChildren; observation++)
				if (root->Child(action).Child(observation))
					Free(root->Child(action).Child(observation), simulator, ignore);
	}
}

void VNODE::FreeAll()
{
	vnode::VNodePool.DeleteAll();
}

void VNODE::SetPrior(int count, double value, bool applicable)
{
    for (int action = 0; action < NumChildren; action++)
    {
        QNODE& qnode = Children[action];
        qnode.SetPrior(count, value, applicable);
    }
}

void VNODE::DisplayValue(HISTORY& history, int maxDepth, ostream& ostr, const std::vector<double> *qvalues) const
{
    if (history.Size() >= maxDepth)
        return;

    for (int action = 0; action < NumChildren; action++)
    {
        history.Add(action);
        const QNODE &qnode = Children[action];

        if (qnode.Applicable()) {
        	ostr << "n=" << qnode.GetCount() << " ";
        	if (qvalues) {
        		qnode.DisplayValue(history, maxDepth, ostr, &(qvalues->at(action)));
        	}
        	else {
        		qnode.DisplayValue(history, maxDepth, ostr);
        	}
        }
        history.Pop();
    }
}

void VNODE::DisplayPolicy(HISTORY& history, int maxDepth, ostream& ostr) const
{
    if (history.Size() >= maxDepth)
        return;

//    double bestq = -Infinity;
    int besta = -1;
    for (int action = 0; action < NumChildren; action++)
    {
//        if (Children[action].Dirichlet.GetValue() > bestq) //XXX
//        {
//            besta = action;
//            bestq = Children[action].Dirichlet.GetValue();
//        }
    }

    if (besta != -1)
    {
        history.Add(besta);
        Children[besta].DisplayPolicy(history, maxDepth, ostr);
        history.Pop();
    }
}

NormalGammaInfo& VNODE::GetCumulativeReward(const STATE &s) {
#if MIXTURE_NORMAL
#ifdef NDEBUG
	return CumulativeRewards[s.hash()];
#else
	std::size_t key = s.hash();
	NormalGammaInfo_POMCP::iterator it = CumulativeRewards.find(key);

	if (it != CumulativeRewards.end()) {
		HASH_STAT.Add(1.0);
		return it->second;
	}
	else {
		HASH_STAT.Add(0.0);
		return CumulativeRewards[key];
	}
#endif
#else
	return CumulativeRewards[0];
#endif
}

//-----------------------------------------------------------------------------
