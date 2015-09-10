#ifndef NODE_H
#define NODE_H

#include "beliefstate.h"
#include "utils.h"
#include "statistic.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/math/distributions.hpp>
#include <boost/functional/hash.hpp>

class HISTORY;
class SIMULATOR;
class QNODE;
class VNODE;

//-----------------------------------------------------------------------------

class QNODE
{
public:
	QNODE(): mApplicable(false), mCount(0)  {

	}

    void Initialise();

    void Update(int observation, double reward, int count = 0) {
    	Observation.Add(observation);
    	ImmediateReward.Add(reward);
    	mCount += count;
    }

    bool Applicable() const {
    	return mApplicable;
    }

    int GetCount() const {
    	return mCount;
    }

    void SetPrior(int count, double value, int applicable) {
    	mApplicable = applicable;

    	if (mApplicable) {
    		ImmediateReward.Set(count, value);
    	}
    	else {
    		ImmediateReward.Clear();
    	}
    }

    const DirichletInfo_POMCP<int> &GetObservation() const {
    	return Observation;
    }

    const DirichletInfo_POMCP<double> &GetImmediateReward() const {
    	return ImmediateReward;
    }

    VNODE*& Child(int c) { Assertion(c); return Children[c]; }
    VNODE* Child(int c) const { Assertion(c); return Children[c]; }

    void DisplayValue(HISTORY& history, int maxDepth, std::ostream& ostr, const double *qvalue = 0) const;
    void DisplayPolicy(HISTORY& history, int maxDepth, std::ostream& ostr) const;

    static int NumChildren;

private:
    void Assertion(int c) const {
    	assert(c >= 0 && c < int(Children.size()) && c < NumChildren);
    }

    std::vector<VNODE*> Children;

	bool mApplicable;
	int mCount;

	DirichletInfo_POMCP<int> Observation;
	DirichletInfo_POMCP<double> ImmediateReward;
};

//-----------------------------------------------------------------------------

class VNODE : public MEMORY_OBJECT
{
public:
    void Initialise();

    static VNODE* Create();
    static void Free(VNODE* root, const SIMULATOR& simulator, VNODE* ignore = 0);
    static void FreeAll();

    QNODE& Child(int c) {  Assertion(c); return Children[c]; }
    const QNODE& Child(int c) const { Assertion(c); return Children[c]; }
    BELIEF_STATE& Beliefs() { return BeliefState; }
    const BELIEF_STATE& Beliefs() const { return BeliefState; }

    void SetPrior(int count, double value, bool applicable);

    void DisplayValue(HISTORY& history, int maxDepth, std::ostream& ostr, const std::vector<double> *qvalues = 0) const;
    void DisplayPolicy(HISTORY& history, int maxDepth, std::ostream& ostr) const;

    NormalGammaInfo& GetCumulativeReward(const STATE &s);

    double ThompsonSampling(bool sampling) {
    	double count = 0.0;
    	double sum = 0.0;

#if not MIXTURE_NORMAL
    	assert(CumulativeRewards.size() == 1);
#endif

    	for (boost::unordered_map<size_t, NormalGammaInfo>::iterator it = CumulativeRewards.begin(); it != CumulativeRewards.end(); ++it) {
    		if (it->second.GetCount() > 0.0) {
    			sum += it->second.GetCount() * it->second.ThompsonSampling(sampling);
    			count += it->second.GetCount();
    		}
    	}

    	assert(count > 0.0);

    	return sum / count;
    }

    static int NumChildren;
    static STATISTIC PARTICLES_STAT;
    static STATISTIC HASH_STAT;

private:
    void Assertion(int c) const {
    	assert(c >= 0 && c < int(Children.size()) && c < NumChildren);
    }

    NormalGammaInfo_POMCP CumulativeRewards;
    std::vector<QNODE> Children;
    BELIEF_STATE BeliefState;
};

namespace vnode {
extern MEMORY_POOL<VNODE> VNodePool;

inline int GetNumAllocated() { return VNodePool.GetNumAllocated(); }
};

#endif // NODE_H
