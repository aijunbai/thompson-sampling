#ifndef POCMAN_H
#define POCMAN_H

#include "simulator.h"
#include "coord.h"
#include "grid.h"
#include "beliefstate.h"

class POCMAN_STATE : public STATE
{
public:

    COORD PocmanPos;
    std::vector<COORD> GhostPos;
    std::vector<int> GhostDir;
    std::vector<bool> Food; // bit vector
    int NumFood;
    int PowerSteps;

    virtual size_t hash() const {
    	using boost::hash_combine;

    	// Start with a hash value of 0    .
    	std::size_t seed = 0;

    	// Modify 'seed' by XORing and bit-shifting in
    	// one member of 'Key' after the other:
    	hash_combine(seed, hash_value(PocmanPos));
    	hash_combine(seed, boost::hash_value(GhostPos));
    	hash_combine(seed, boost::hash_value(GhostDir));
    	hash_combine(seed, boost::hash_value(Food));
    	hash_combine(seed, boost::hash_value(NumFood));
    	hash_combine(seed, boost::hash_value(PowerSteps));

    	// Return the result.
    	return seed;
    }
};

class POCMAN : public SIMULATOR
{
public:

    virtual STATE* Copy(const STATE& state) const;
    virtual void Validate(const STATE& state) const;
    virtual STATE* CreateStartState() const;
    virtual void FreeState(STATE* state) const;
    virtual bool Step(STATE& state, int action, 
        int& observation, double& reward) const;

    virtual bool LocalMove(STATE& state, const HISTORY& history,
        int stepObs, const STATUS& status) const;
    void GenerateLegal(const STATE& state, /*const HISTORY& history,*/
        std::vector<int>& legal, const STATUS& status) const;
    void GeneratePreferred(const STATE& state, const HISTORY& history,
        std::vector<int>& legal, const STATUS& status) const;

    virtual void DisplayBeliefs(const BELIEF_STATE& beliefState, 
        std::ostream& ostr) const;
    virtual void DisplayState(const STATE& state, std::ostream& ostr) const;
    virtual void DisplayObservation(const STATE& state, int observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;

protected:

    POCMAN(int xsize, int ysize);

    enum { 
        E_PASSABLE,
        E_SEED,
        E_POWER
    };

    GRID<int> Maze;
    int NumGhosts, PassageY, GhostRange, SmellRange, HearRange;
    COORD PocmanHome, GhostHome;
    double FoodProb, ChaseProb, DefensiveSlip;
    double RewardClearLevel, RewardDefault, RewardDie; 
    double RewardEatFood, RewardEatGhost, RewardHitWall; 
    int PowerNumSteps;

private:

    void MoveGhost(POCMAN_STATE& pocstate, int g) const;
    void MoveGhostAggressive(POCMAN_STATE& pocstate, int g) const;
    void MoveGhostDefensive(POCMAN_STATE& pocstate, int g) const;
    void MoveGhostRandom(POCMAN_STATE& pocstate, int g) const;
    void NewLevel(POCMAN_STATE& pocstate) const;
    int SeeGhost(const POCMAN_STATE& pocstate, int action) const;    
    bool HearGhost(const POCMAN_STATE& pocstate) const;
    bool SmellFood(const POCMAN_STATE& pocstate) const;
    COORD NextPos(const COORD& from, int dir) const;
    bool Passable(const COORD& pos) const { return UTILS::CheckFlag(Maze(pos), E_PASSABLE); }
    int MakeObservations(const POCMAN_STATE& pocstate) const;

    mutable MEMORY_POOL<POCMAN_STATE> MemoryPool;
};

class MICRO_POCMAN : public POCMAN
{
public:

    MICRO_POCMAN();
    virtual ~MICRO_POCMAN() { }
};

class MINI_POCMAN : public POCMAN
{
public:

    MINI_POCMAN();
    virtual ~MINI_POCMAN() { }
};

class FULL_POCMAN : public POCMAN
{
public:

    FULL_POCMAN();
    virtual ~FULL_POCMAN() { }
};

#endif // POCMAN_H
