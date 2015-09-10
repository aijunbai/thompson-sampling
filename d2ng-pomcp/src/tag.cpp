#include "tag.h"

using namespace std;
using namespace UTILS;

const int TAG::NumCells = 29;

TAG::TAG(int opponents)
:   NumOpponents(opponents)
{
    NumActions = 5;
    NumObservations = NumCells + 1;
    RewardRange = 10 * NumOpponents;

    Discount = 0.95;
}

STATE* TAG::Copy(const STATE& state) const
{
    const TAG_STATE& tagstate = safe_cast<const TAG_STATE&>(state);
    TAG_STATE* newstate = MemoryPool.Allocate();
    *newstate = tagstate;
    return newstate; 
}

void TAG::Validate(const STATE& state) const
{
    const TAG_STATE& tagstate = safe_cast<const TAG_STATE&>(state);
    assert(Inside(tagstate.AgentPos));
}

STATE* TAG::CreateStartState() const
{
    TAG_STATE* tagstate = MemoryPool.Allocate();
    tagstate->NumAlive = NumOpponents;
    tagstate->AgentPos = GetCoord(Random(NumCells));
    tagstate->OpponentPos.clear();
    for (int i = 0; i < NumOpponents; ++i)
        tagstate->OpponentPos.push_back(GetCoord(Random(NumCells)));
    return tagstate;
}

void TAG::FreeState(STATE* state) const
{
    TAG_STATE* tagstate = safe_cast<TAG_STATE*>(state);
    MemoryPool.Free(tagstate);
}

bool TAG::Step(STATE& state, int action, 
    int& observation, double& reward) const
{
    TAG_STATE& tagstate = safe_cast<TAG_STATE&>(state);

    // Tag action
    if (action == 4) // tag
    {
        observation = GetIndex(tagstate.AgentPos);
        bool tagged = false;
        for (int opp = 0; opp < NumOpponents; ++opp)
        {
            if (tagstate.OpponentPos[opp] == tagstate.AgentPos)            
            {
                reward = 10;
                tagged = true;
                tagstate.NumAlive--;
                tagstate.OpponentPos[opp] = coord::Null;
            }
        }
        if (!tagged)
        {
            reward = -10;
        }
    }

    // Move opponents
    for (int opp = 0; opp < NumOpponents; ++opp)
        if (IsAlive(tagstate, opp))
            MoveOpponent(tagstate, opp);

    // Move action
    if (action < 4)
    {
        reward = -1;
        COORD nextpos = tagstate.AgentPos + coord::Compass[action];
        if (Inside(nextpos))
            tagstate.AgentPos = nextpos;        
    }
    
    // Observation occurs in final positions, not start positions
    observation = GetObservation(tagstate, action);
    return tagstate.NumAlive == 0;
}

inline int TAG::GetObservation(const TAG_STATE& tagstate, int action) const
{
    int obs = GetIndex(tagstate.AgentPos);
    if (action < 4)
        for (int opp = 0; opp < NumOpponents; ++opp)
            if (tagstate.OpponentPos[opp] == tagstate.AgentPos)
                obs = NumCells;
    return obs;
}

inline bool TAG::Inside(const COORD& coord) const
{
    if (coord.Y >= 2)
    {
        return coord.X >= 5 && coord.X < 8 && coord.Y < 5;
    }
    else
    {
        return coord.X >= 0 && coord.X < 10 && coord.Y >= 0;    
    }
}

inline COORD TAG::GetCoord(int index) const
{
    assert(NumCells == 29);
    assert(index >= 0 && index < 29);
    if (index < 20)
        return COORD(index % 10, index / 10);
    index -= 20;
    return COORD(index % 3 + 5, index / 3 + 2);
}

inline int TAG::GetIndex(const COORD& coord) const
{
    assert(coord.X >= 0 && coord.X < 10);
    assert(coord.Y >= 0 && coord.Y < 5);
    if (coord.Y < 2)
        return coord.Y * 10 + coord.X;
    assert(coord.X >= 5 && coord.X < 8);
    return 20 + (coord.Y - 2) * 3 + coord.X - 5;
}

inline bool TAG::IsAlive(const TAG_STATE& tagstate, int opp) const
{
    return tagstate.OpponentPos[opp].Valid();
}

inline bool TAG::IsCorner(const COORD& coord) const
{
    if (!Inside(coord))
        return false;
    if (coord.Y < 2)
        return coord.X == 0 || coord.X == 9;
    else
        return coord.Y == 4 && (coord.X == 5 || coord.X == 7);
}

inline COORD TAG::GetRandomCorner() const
{
    int c = Random(6);
    switch(c)
    {
        case 0: return COORD(0, 0);
        case 1: return COORD(0, 1);
        case 2: return COORD(9, 0);
        case 3: return COORD(9, 1);
        case 4: return COORD(5, 4);
        case 5: return COORD(7, 4);
    }
}
    
void TAG::MoveOpponent(TAG_STATE& tagstate, int opp) const
{
    const COORD& agent = tagstate.AgentPos;
    COORD& opponent = tagstate.OpponentPos[opp];
    
    static vector<int> actions;
    actions.clear();

    if (opponent.X >= agent.X)
        actions.push_back(COORD::E_EAST);
    if (opponent.Y >= agent.Y)
        actions.push_back(COORD::E_NORTH);
    if (opponent.X <= agent.X)
        actions.push_back(COORD::E_WEST);
    if (opponent.Y <= agent.Y)
        actions.push_back(COORD::E_SOUTH);
    if (opponent.X == agent.X && opponent.Y > agent.Y)
        actions.push_back(COORD::E_NORTH);
    if (opponent.Y == agent.Y && opponent.X > agent.X)
        actions.push_back(COORD::E_EAST);
    if (opponent.X == agent.X && opponent.Y < agent.Y)
        actions.push_back(COORD::E_SOUTH);
    if (opponent.Y == agent.Y && opponent.X < agent.X)
        actions.push_back(COORD::E_WEST);
    
    assert(!actions.empty());
    if (Bernoulli(0.8))
    {
        int d = actions[Random(actions.size())];
        if (Inside(opponent + coord::Compass[d]))
            opponent = opponent + coord::Compass[d];
    }
}

bool TAG::LocalMove(STATE& state, const HISTORY& history,
    int /*stepObs*/, const STATUS& ) const
{
    TAG_STATE& tagstate = safe_cast<TAG_STATE&>(state);

    int opp = Random(NumOpponents);
    if (!IsAlive(tagstate, opp))
        return false;
    tagstate.OpponentPos[opp] = GetCoord(Random(NumCells));

    int realObs = history.Back().Observation;
    if (realObs < NumCells && realObs != GetIndex(tagstate.AgentPos))
        tagstate.AgentPos = GetCoord(realObs);
    int simObs = GetObservation(tagstate, history.Back().Action);
    return simObs == realObs;
}

void TAG::GeneratePreferred(const STATE& state, const HISTORY& history,
    vector<int>& actions, const STATUS& ) const
{
    const TAG_STATE& tagstate = safe_cast<const TAG_STATE&>(state);
    
    // If history is empty then we don't know where we are yet
    if (history.Size() == 0)
        return;

    // If we just saw an opponent and we are in a corner then TAG
    if (history.Back().Observation == NumCells && IsCorner(tagstate.AgentPos))
    {
        actions.push_back(4);
        return;
    }
    
    // Don't double back and don't go into walls
    for (int d = 0; d < 4; ++d)
        if (history.Back().Action != coord::Opposite(d)
            && Inside(tagstate.AgentPos + coord::Compass[d]))
            actions.push_back(d);
}

void TAG::DisplayBeliefs(const BELIEF_STATE& ,
    std::ostream& ) const
{
}

void TAG::DisplayState(const STATE& state, std::ostream& ostr) const
{
    const TAG_STATE& tagstate = safe_cast<const TAG_STATE&>(state);
    GRID<char> cgrid(10, 5);
    cgrid.SetAllValues('.');
    for (int opp = 0; opp < NumOpponents; ++opp)
        if (IsAlive(tagstate, opp))
            cgrid(tagstate.OpponentPos[opp]) = '@';
    cgrid(tagstate.AgentPos) = '*';

    for (int y = 4; y >= 0; y--)
    {
        for (int x = 0; x < 10; x++)
        {
            COORD pos(x, y);
            if (Inside(pos))
            {
                ostr << cgrid(pos) << ' ';
            }
            else
            {
                ostr << "  ";
            }
        }
        ostr << endl;
    }
}

void TAG::DisplayObservation(const STATE& , int observation, std::ostream& ostr) const
{
    if (observation == NumCells)
        ostr << "On opponent" << endl;
    else
        ostr << "Agent is at (" << GetCoord(observation).X << ", " 
            << GetCoord(observation).Y << ")" << endl;
}

void TAG::DisplayAction(int action, std::ostream& ostr) const
{
    if (action < 4)
        ostr << coord::CompassString[action] << endl;
    else
        ostr << "TAG" << endl;
}
