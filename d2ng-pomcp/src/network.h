#ifndef NETWORK_H
#define NETWORK_H

#include "simulator.h"

class NETWORK_STATE : public STATE
{
public:

    std::vector<bool> Machines;

    virtual size_t hash() const {
    	using boost::hash_combine;

    	// Start with a hash value of 0    .
    	std::size_t seed = 0;

    	// Modify 'seed' by XORing and bit-shifting in
    	// one member of 'Key' after the other:
    	hash_combine(seed, boost::hash_value(Machines));

    	// Return the result.
    	return seed;
    }
};

class NETWORK : public SIMULATOR
{
public:

    enum
    {
        E_CYCLE,
        E_3LEGS
    };

    NETWORK(int numMachines, int ntype);

    virtual STATE* Copy(const STATE& state) const;
    virtual void Validate(const STATE& state) const;
    virtual STATE* CreateStartState() const;
    virtual void FreeState(STATE* state) const;
    virtual bool Step(STATE& state, int action, 
        int& observation, double& reward) const;
        
//    virtual bool Prune(int action, const HISTORY& history) const;
//    virtual int SelectRandom(const HISTORY& history) const;

    virtual void DisplayBeliefs(const BELIEF_STATE& beliefState, 
        std::ostream& ostr) const;
    virtual void DisplayState(const STATE& state, std::ostream& ostr) const;
    virtual void DisplayObservation(const STATE& state, int observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;

private:

    void MakeRingNeighbours();
    void Make3LegsNeighbours();
    
    int NumMachines;
    double FailureProb1, FailureProb2, ObsProb;
    std::vector<std::vector<int> > Neighbours;
    
    mutable MEMORY_POOL<NETWORK_STATE> MemoryPool;
};

#endif // NETWORK_H
