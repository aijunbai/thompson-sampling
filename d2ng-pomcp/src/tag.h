#ifndef TAG_H
#define TAG_H

#include "simulator.h"
#include "coord.h"
#include "grid.h"

class TAG_STATE : public STATE
{
public:

    COORD AgentPos;
    std::vector<COORD> OpponentPos;
    int NumAlive;

    virtual size_t hash() const {
    	using boost::hash_combine;

    	// Start with a hash value of 0    .
    	std::size_t seed = 0;

    	// Modify 'seed' by XORing and bit-shifting in
    	// one member of 'Key' after the other:
    	hash_combine(seed, hash_value(AgentPos));
    	hash_combine(seed, boost::hash_value(OpponentPos));
    	hash_combine(seed, boost::hash_value(NumAlive));

    	// Return the result.
    	return seed;
    }
};

class TAG : public SIMULATOR
{
public:

    TAG(int numrobots);

    virtual STATE* Copy(const STATE& state) const;
    virtual void Validate(const STATE& state) const;
    virtual STATE* CreateStartState() const;
    virtual void FreeState(STATE* state) const;
    virtual bool Step(STATE& state, int action, 
        int& observation, double& reward) const;
        
    void GeneratePreferred(const STATE& state, const HISTORY& history,
        std::vector<int>& legal, const STATUS& status) const;
    virtual bool LocalMove(STATE& state, const HISTORY& history,
        int stepObs, const STATUS& status) const;

    virtual void DisplayBeliefs(const BELIEF_STATE& beliefState, 
        std::ostream& ostr) const;
    virtual void DisplayState(const STATE& state, std::ostream& ostr) const;
    virtual void DisplayObservation(const STATE& state, int observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;

protected:

    void MoveOpponent(TAG_STATE& tagstate, int opp) const;
    int GetObservation(const TAG_STATE& tagstate, int action) const;
    bool Inside(const COORD& coord) const;
    COORD GetCoord(int index) const;
    int GetIndex(const COORD& coord) const;
    bool IsAlive(const TAG_STATE& tagstate, int opp) const;
    bool IsCorner(const COORD& coord) const;
    COORD GetRandomCorner() const;

    int NumOpponents;
    static const int NumCells;
    
private:

    mutable MEMORY_POOL<TAG_STATE> MemoryPool;
};

#endif
