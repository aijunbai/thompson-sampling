#ifndef ROOMS_H
#define ROOMS_H

#include "simulator.h"
#include "coord.h"
#include "grid.h"

class ROOMS_STATE: public STATE
{
public:
    COORD AgentPos;

    virtual size_t hash() const {
        using boost::hash_combine;

        // Start with a hash value of 0    .
        std::size_t seed = 0;

        // Modify 'seed' by XORing and bit-shifting in
        // one member of 'Key' after the other:
        hash_combine(seed, hash_value(AgentPos));

        // Return the result.
        return seed;
    }
};

class ROOMS: public SIMULATOR
{
public:
    ROOMS(const char *map_name, bool state_abstraction = false);
    virtual ~ROOMS();

    virtual STATE* Copy(const STATE& state) const;
    virtual void Validate(const STATE& state) const;
    virtual STATE* CreateStartState() const;
    virtual void FreeState(STATE* state) const;
    virtual bool Step(STATE& state, int action,
        int& observation, double& reward) const;

    virtual void GenerateLegal(const STATE& state, /*const HISTORY& history,*/
        std::vector<int>& legal, const STATUS& status) const;
    virtual void GeneratePreferred(const STATE& state, const HISTORY& history,
        std::vector<int>& legal, const STATUS& status) const;
    virtual bool LocalMove(STATE& state, const HISTORY& history,
        int stepObservation, const STATUS& status) const;

    virtual void DisplayBeliefs(const BELIEF_STATE& beliefState,
        std::ostream& ostr) const;
    virtual void DisplayState(const STATE& state, std::ostream& ostr) const;
    virtual void DisplayObservation(const STATE& state, int observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;

protected:
    void Parse(const char *file_name);
    int GetObservation(const ROOMS_STATE& state) const;

    bool mStateAbstraction;
    GRID<int> *mGrid;
    int mRooms;
    COORD mStartPos;
    COORD mGoalPos;

private:
    mutable MEMORY_POOL<ROOMS_STATE> mMemoryPool;
};

#endif // ROOMS_H
