#include "pocman.h"
#include "distribution.h"
#include "utils.h"

using namespace std;
using namespace UTILS;

POCMAN::POCMAN(int xsize, int ysize)
:   Maze(xsize, ysize),
    NumGhosts(-1),
    PassageY(-1),
    GhostRange(-1),
    SmellRange(1),
    HearRange(2),
    FoodProb(0.5),
    ChaseProb(0.75),
    DefensiveSlip(0.25),
    RewardClearLevel(+1000),
    RewardDefault(-1),
    RewardDie(-100),
    RewardEatFood(+10),
    RewardEatGhost(+25),
    RewardHitWall(-25),
    PowerNumSteps(15)
{
    NumActions = 4;
    NumObservations = 1 << 10;
        // See ghost N
        // See ghost E
        // See ghost S
        // See ghost W
        // Can move N
        // Can move E
        // Can move S
        // Can move W
        // Smell food
        // Hear ghost

    Discount = 0.95;
    mName << "pocman_" << xsize << "_" << ysize;
}

MICRO_POCMAN::MICRO_POCMAN()
:   POCMAN(7, 7)
{
    int maze[7][7] =
    {
        { 3, 3, 3, 3, 3, 3, 3 },
        { 3, 3, 0, 3, 0, 3, 3 },
        { 3, 0, 3, 3, 3, 0, 3 },
        { 3, 3, 3, 0, 3, 3, 3 },
        { 3, 0, 3, 3, 3, 0, 3 },
        { 3, 3, 0, 3, 0, 3, 3 },
        { 3, 3, 3, 3, 3, 3, 3 }
    };

    for (int x = 0; x < 7; x++)
        Maze.SetCol(x, maze[x]);
    NumGhosts = 1;
    GhostRange = 3;
    PocmanHome = COORD(3, 0);
    GhostHome = COORD(3, 4);
}

MINI_POCMAN::MINI_POCMAN()
:   POCMAN(10, 10)
{
    int maze[10][10] =
    {
        { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 },
        { 3, 0, 0, 3, 0, 0, 3, 0, 0, 3 },
        { 3, 0, 3, 3, 3, 3, 3, 3, 0, 3 },
        { 3, 3, 3, 0, 0, 0, 0, 3, 3, 3 },
        { 0, 0, 3, 0, 1, 1, 3, 3, 0, 0 },
        { 0, 0, 3, 0, 1, 1, 3, 3, 0, 0 },
        { 3, 3, 3, 0, 0, 0, 0, 3, 3, 3 },
        { 3, 0, 3, 3, 3, 3, 3, 3, 0, 3 },
        { 3, 0, 0, 3, 0, 0, 3, 0, 0, 3 },
        { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 }
    };

    for (int x = 0; x < 10; x++)
        Maze.SetCol(x, maze[x]);

    NumGhosts = 3;
    GhostRange = 4;
    PocmanHome = COORD(4, 2);
    GhostHome = COORD(4, 4);
    PassageY = 5;
}

FULL_POCMAN::FULL_POCMAN()
:   POCMAN(17, 19)
{
    // Transposed maze
    int maze[19][17] = {
        { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, },
        { 3, 0, 0, 3, 0, 0, 0, 3, 0, 3, 0, 0, 0, 3, 0, 0, 3, },
        { 7, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7, },
        { 3, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 3, },
        { 3, 3, 3, 3, 0, 3, 3, 3, 0, 3, 3, 3, 0, 3, 3, 3, 3, },
        { 0, 0, 0, 3, 0, 0, 0, 3, 0, 3, 0, 0, 0, 3, 0, 0, 0, },
        { 0, 0, 0, 3, 0, 1, 1, 1, 1, 1, 1, 1, 0, 3, 0, 0, 0, },
        { 0, 0, 0, 3, 0, 1, 0, 1, 1, 1, 0, 1, 0, 3, 0, 0, 0, },
        { 1, 1, 1, 3, 0, 1, 0, 1, 1, 1, 0, 1, 0, 3, 1, 1, 1, },
        { 0, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 1, 0, 3, 0, 0, 0, },
        { 0, 0, 0, 3, 0, 1, 1, 1, 1, 1, 1, 1, 0, 3, 0, 0, 0, },
        { 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, },
        { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, },
        { 3, 0, 0, 3, 0, 0, 0, 3, 0, 3, 0, 0, 0, 3, 0, 0, 3, },
        { 7, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 3, 7, },
        { 0, 3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, },
        { 3, 3, 3, 3, 0, 3, 3, 3, 0, 3, 3, 3, 0, 3, 3, 3, 3, },
        { 3, 0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 3, },
        { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3  }
    };

    // Transpose to rows
    for (int x = 0; x < 19; x++)
        Maze.SetRow(x, maze[18 - x]);

    NumGhosts = 4;
    GhostRange = 6;
    PocmanHome = COORD(8, 6);
    GhostHome = COORD(8, 10);
    PassageY = 10;
}

STATE* POCMAN::Copy(const STATE& state) const
{
    const POCMAN_STATE& pocstate = safe_cast<const POCMAN_STATE&>(state);
    POCMAN_STATE* newstate = MemoryPool.Allocate();
    *newstate = pocstate;
    return newstate;
}

void POCMAN::Validate(const STATE& state) const
{
    const POCMAN_STATE& pocstate = safe_cast<const POCMAN_STATE&>(state);
    assert(Maze.Inside(pocstate.PocmanPos));
    assert(Passable(pocstate.PocmanPos));
    for (int g = 0; g < NumGhosts; g++)
    {
        assert(Maze.Inside(pocstate.GhostPos[g]));
        assert(Passable(pocstate.GhostPos[g]));
    }
}

STATE* POCMAN::CreateStartState() const
{
    POCMAN_STATE* startState = MemoryPool.Allocate();
    startState->GhostPos.resize(NumGhosts);
    startState->GhostDir.resize(NumGhosts);
    startState->Food.resize(Maze.GetXSize() * Maze.GetYSize());
    NewLevel(*startState);
    return startState;
}

void POCMAN::FreeState(STATE* state) const
{
    POCMAN_STATE* pocstate = safe_cast<POCMAN_STATE*>(state);
    MemoryPool.Free(pocstate);
}

COORD POCMAN::NextPos(const COORD& from, int dir) const
{
    COORD nextPos;
    if (from.X == 0 && from.Y == PassageY && dir == COORD::E_WEST)
        nextPos = COORD(Maze.GetXSize() - 1, from.Y);
    else if (from.X == Maze.GetXSize() - 1 && from.Y == PassageY && dir == COORD::E_EAST)
        nextPos = COORD(0, from.Y);
    else
        nextPos = from + coord::Compass[dir];

    if (Maze.Inside(nextPos) && Passable(nextPos))
        return nextPos;
    else
        return coord::Null;
}

bool POCMAN::Step(STATE& state, int action,
    int& observation, double& reward) const
{
    POCMAN_STATE& pocstate = safe_cast<POCMAN_STATE&>(state);
    reward = RewardDefault;
    observation = 0;

    // cout << coord::CompassChar[action];
    COORD newpos = NextPos(pocstate.PocmanPos, action);
    if (newpos.Valid())
        pocstate.PocmanPos = newpos;
    else
        reward += RewardHitWall;

    if (pocstate.PowerSteps > 0)
        pocstate.PowerSteps--;

    int hitGhost = -1;
    for (int g = 0; g < NumGhosts; g++)
    {
        if (pocstate.GhostPos[g] == pocstate.PocmanPos)
            hitGhost = g;
        MoveGhost(pocstate, g);
        if (pocstate.GhostPos[g] == pocstate.PocmanPos)
            hitGhost = g;
    }

    if (hitGhost >= 0)
    {
        if (pocstate.PowerSteps > 0)
        {
            reward += RewardEatGhost;
            pocstate.GhostPos[hitGhost] = GhostHome;
            pocstate.GhostDir[hitGhost] = -1;
        }
        else
        {
            reward += RewardDie;
            return true;
        }
    }

    observation = MakeObservations(pocstate);

    int pocIndex = Maze.Index(pocstate.PocmanPos);
    if (pocstate.Food[pocIndex])
    {
        pocstate.Food[pocIndex] = false;
        pocstate.NumFood--;
        if (pocstate.NumFood == 0)
        {
            reward += RewardClearLevel;
            return true;
        }
        if (CheckFlag(Maze(pocstate.PocmanPos.X, pocstate.PocmanPos.Y), E_POWER))
            pocstate.PowerSteps = PowerNumSteps;
        reward += RewardEatFood;
    }

    return false;
}

int POCMAN::MakeObservations(const POCMAN_STATE& pocstate) const
{
    int observation = 0;
    for (int d = 0; d < 4; d++)
    {
        if (SeeGhost(pocstate, d) >= 0)
            SetFlag(observation, d);
        COORD wpos = NextPos(pocstate.PocmanPos, d);
        if (wpos.Valid() && Passable(wpos))
            SetFlag(observation, d + 4);
    }
    if (SmellFood(pocstate))
        SetFlag(observation, 8);
    if (HearGhost(pocstate))
        SetFlag(observation, 9);
    return observation;
}

bool POCMAN::LocalMove(STATE& state, const HISTORY& history,
    int , const STATUS& ) const
{
    POCMAN_STATE& pocstate = safe_cast<POCMAN_STATE&>(state);

    int numGhosts = SimpleRNG::ins().Random(1, 3); // Change 1 or 2 ghosts at a time
    for (int i = 0; i < numGhosts; ++i)
    {
        int g = SimpleRNG::ins().Random(NumGhosts);
        pocstate.GhostPos[g] = COORD(
            SimpleRNG::ins().Random(Maze.GetXSize()),
            SimpleRNG::ins().Random(Maze.GetYSize()));
        if (!Passable(pocstate.GhostPos[g])
            || pocstate.GhostPos[g] == pocstate.PocmanPos)
            return false;
    }

    COORD smellPos;
    for (smellPos.X = -SmellRange; smellPos.X <= SmellRange; smellPos.X++)
    {
        for (smellPos.Y = -SmellRange; smellPos.Y <= SmellRange; smellPos.Y++)
        {
            COORD pos = pocstate.PocmanPos + smellPos;
            if (smellPos != COORD(0, 0) &&
                Maze.Inside(pos) &&
                CheckFlag(Maze(pos), E_SEED))
                pocstate.Food[Maze.Index(pos)] = SimpleRNG::ins().Bernoulli(FoodProb * 0.5);
        }
    }

    // Just check the last time-step, don't check for full consistency
    if (history.Size() == 0)
        return true;
    int observation = MakeObservations(pocstate);
    return history.Back().Observation == observation;
}

void POCMAN::MoveGhost(POCMAN_STATE& pocstate, int g) const
{
    if (coord::ManhattanDistance(
            pocstate.PocmanPos, pocstate.GhostPos[g]) < GhostRange)
    {
        if (pocstate.PowerSteps > 0)
            MoveGhostDefensive(pocstate, g);
        else
            MoveGhostAggressive(pocstate, g);
    }
    else
    {
        MoveGhostRandom(pocstate, g);
    }
}

void POCMAN::MoveGhostAggressive(POCMAN_STATE& pocstate, int g) const
{
    if (!SimpleRNG::ins().Bernoulli(ChaseProb))
    {
        MoveGhostRandom(pocstate, g);
        return;
    }

    int bestDist = Maze.GetXSize() + Maze.GetYSize();
    COORD bestPos = pocstate.GhostPos[g];
    int bestDir = -1;
    for (int dir = 0; dir < 4; dir++)
    {
        int dist = coord::DirectionalDistance(
            pocstate.PocmanPos, pocstate.GhostPos[g], dir);
        COORD newpos = NextPos(pocstate.GhostPos[g], dir);
        if (dist <= bestDist && newpos.Valid()
            && coord::Opposite(dir) != pocstate.GhostDir[g])
        {
            bestDist = dist;
            bestPos = newpos;
        }
    }

    pocstate.GhostPos[g] = bestPos;
    pocstate.GhostDir[g] = bestDir;
}

void POCMAN::MoveGhostDefensive(POCMAN_STATE& pocstate, int g) const
{
    if (SimpleRNG::ins().Bernoulli(DefensiveSlip) && pocstate.GhostDir[g] >= 0)
    {
        pocstate.GhostDir[g] = -1;
        return;
    }

    int bestDist = 0;
    COORD bestPos = pocstate.GhostPos[g];
    int bestDir = -1;
    for (int dir = 0; dir < 4; dir++)
    {
        int dist = coord::DirectionalDistance(
            pocstate.PocmanPos, pocstate.GhostPos[g], dir);
        COORD newpos = NextPos(pocstate.GhostPos[g], dir);
        if (dist >= bestDist && newpos.Valid()
            && coord::Opposite(dir) != pocstate.GhostDir[g])
        {
            bestDist = dist;
            bestPos = newpos;
        }
    }

    pocstate.GhostPos[g] = bestPos;
    pocstate.GhostDir[g] = bestDir;
}

void POCMAN::MoveGhostRandom(POCMAN_STATE& pocstate, int g) const
{
    // Never switch to opposite direction
    // Currently assumes there are no dead-ends.
    COORD newpos;
    int dir;
//    int tempc = 0;
    do
    {
        dir = SimpleRNG::ins().Random(4);
        newpos = NextPos(pocstate.GhostPos[g], dir);
    }
    while (coord::Opposite(dir) == pocstate.GhostDir[g] || !newpos.Valid());
    pocstate.GhostPos[g] = newpos;
    pocstate.GhostDir[g] = dir;
}

void POCMAN::NewLevel(POCMAN_STATE& pocstate) const
{
    pocstate.PocmanPos = PocmanHome;
    for (int g = 0; g < NumGhosts; g++)
    {
        pocstate.GhostPos[g] = GhostHome;
        pocstate.GhostPos[g].X += g % 2;
        pocstate.GhostPos[g].Y += g / 2;
        pocstate.GhostDir[g] = -1;
    }

    pocstate.NumFood = 0;
    for (int x = 0; x < Maze.GetXSize(); x++)
    {
        for (int y = 0; y < Maze.GetYSize(); y++)
        {
            int pocIndex = Maze.Index(x, y);
            if (CheckFlag(Maze(x, y), E_SEED)
                && (CheckFlag(Maze(x, y), E_POWER)
                    || SimpleRNG::ins().Bernoulli(FoodProb)))
            {
                pocstate.Food[pocIndex] = 1;
                pocstate.NumFood++;
            }
            else
            {
                pocstate.Food[pocIndex] = 0;
            }
        }
    }

    pocstate.PowerSteps = 0;
}

int POCMAN::SeeGhost(const POCMAN_STATE& pocstate, int action) const
{
    COORD eyepos = pocstate.PocmanPos + coord::Compass[action];
    while (Maze.Inside(eyepos) && Passable(eyepos))
    {
        for (int g = 0; g < NumGhosts; g++)
            if (pocstate.GhostPos[g] == eyepos)
                return g;
        eyepos += coord::Compass[action];
    }
    return -1;
}

bool POCMAN::HearGhost(const POCMAN_STATE& pocstate) const
{
    for (int g = 0; g < NumGhosts; g++)
        if (coord::ManhattanDistance(
            pocstate.GhostPos[g], pocstate.PocmanPos) <= HearRange)
            return true;
    return false;
}

bool POCMAN::SmellFood(const POCMAN_STATE& pocstate) const
{
    COORD smellPos;
    for (smellPos.X = -SmellRange; smellPos.X <= SmellRange; smellPos.X++)
        for (smellPos.Y = -SmellRange; smellPos.Y <= SmellRange; smellPos.Y++)
            if (Maze.Inside(pocstate.PocmanPos + smellPos)
                && pocstate.Food[Maze.Index(pocstate.PocmanPos + smellPos)])
                return true;
    return false;
}

void POCMAN::GenerateLegal(const STATE& state, /*const HISTORY& ,*/
    vector<int>& legal, const STATUS& ) const
{
    const POCMAN_STATE& pocstate = safe_cast<const POCMAN_STATE&>(state);

    // Don't move into walls
    for (int a = 0; a < 4; ++a)
    {
        COORD newpos = NextPos(pocstate.PocmanPos, a);
        if (newpos.Valid())
            legal.push_back(a);
    }
}

void POCMAN::GeneratePreferred(const STATE& state, const HISTORY& history,
    vector<int>& actions, const STATUS& ) const
{
    const POCMAN_STATE& pocstate = safe_cast<const POCMAN_STATE&>(state);
    if (history.Size())
    {
        int action = history.Back().Action;
        int observation = history.Back().Observation;

        // If power pill and can see a ghost then chase it
        if (pocstate.PowerSteps > 0 && ((observation & 15) != 0))
        {
            for (int a = 0; a < 4; ++a)
                if (CheckFlag(observation, a))
                    actions.push_back(a);
        }

        // Otherwise avoid observed ghosts and avoid changing directions
        else
        {
            for (int a = 0; a < 4; ++a)
            {
                COORD newpos = NextPos(pocstate.PocmanPos, a);
                if (newpos.Valid() && !CheckFlag(observation, a)
                    && coord::Opposite(a) != action)
                    actions.push_back(a);
            }
        }
    }
}

void POCMAN::DisplayBeliefs(const BELIEF_STATE& beliefState,
    ostream& ostr) const
{
    GRID<int> counts(Maze.GetXSize(), Maze.GetYSize());
    counts.SetAllValues(0);
    for (int i = 0; i < beliefState.GetNumSamples(); i++)
    {
        const POCMAN_STATE* pocstate =
            safe_cast<const POCMAN_STATE*>(
                beliefState.GetSample(i));

        for (int g = 0; g < NumGhosts; g++)
            counts(pocstate->GhostPos[g])++;
    }

    for (int y = Maze.GetYSize() - 1; y >= 0; y--)
    {
        for (int x = 0; x < Maze.GetXSize(); x++)
        {
            ostr.width(6);
            ostr.precision(2);
            ostr << fixed << (double) counts(x, y) / beliefState.GetNumSamples();
        }
        ostr << endl;
    }
}

void POCMAN::DisplayState(const STATE& state, ostream& ostr) const
{
    const POCMAN_STATE& pocstate = safe_cast<const POCMAN_STATE&>(state);
    ostr << endl;
    for (int x = 0; x < Maze.GetXSize() + 2; x++)
        ostr << "# ";
    ostr << endl;
    for (int y = Maze.GetYSize() - 1; y >= 0; y--)
    {
        if (y == PassageY)
            ostr << "< ";
        else
            ostr << "# ";
        for (int x = 0; x < Maze.GetXSize(); x++)
        {
            COORD pos(x, y);
            int index = Maze.Index(pos);
            char c = ' ';
            if (!Passable(pos))
                c = '#';
            if (pocstate.Food[index])
                c = CheckFlag(Maze(x, y), E_POWER) ? '+' : '.';
            for (int g = 0; g < NumGhosts; g++)
                if (pos == pocstate.GhostPos[g])
                    c = (pos == pocstate.PocmanPos ? 'O' :
                        (pocstate.PowerSteps == 0 ? 'A' + g : 'a' + g));
            if (pos == pocstate.PocmanPos)
                c = pocstate.PowerSteps > 0 ? '!' : '@';
            ostr << c << ' ';
        }
        if (y == PassageY)
            ostr << ">" << endl;
        else
            ostr << "#" << endl;
    }
    for (int x = 0; x < Maze.GetXSize() + 2; x++)
        ostr << "# ";
    ostr << endl;
}

void POCMAN::DisplayObservation(const STATE& state, int observation, ostream& ostr) const
{
    const POCMAN_STATE& pocstate = safe_cast<const POCMAN_STATE&>(state);
    GRID<char> obs(Maze.GetXSize(), Maze.GetYSize());
    obs.SetAllValues(' ');

    // Pocman
    obs(pocstate.PocmanPos) = pocstate.PowerSteps > 0 ? '!' : '@';

    for (int d = 0; d < 4; d++)
    {
        // See ghost
        if (CheckFlag(observation, d))
        {
            COORD eyepos = pocstate.PocmanPos + coord::Compass[d];
            while (Maze.Inside(eyepos) && Passable(eyepos))
            {
                obs(eyepos) = (pocstate.PowerSteps == 0 ? 'G': 'g');
                eyepos += coord::Compass[d];
            }
        }

        // Feel wall
        if (!CheckFlag(observation, d + 4)
            && Maze.Inside(pocstate.PocmanPos + coord::Compass[d]))
            obs(pocstate.PocmanPos + coord::Compass[d]) = '#';
    }

    // Hear ghost
    if (CheckFlag(observation, 9))
    {
        COORD hearPos;
        for (hearPos.X = -HearRange; hearPos.X <= HearRange; hearPos.X++)
            for (hearPos.Y = -HearRange; hearPos.Y <= HearRange; hearPos.Y++)
                if (coord::ManhattanDistance(hearPos, pocstate.PocmanPos) <= HearRange
                    && Maze.Inside(pocstate.PocmanPos + hearPos)
                    && obs(pocstate.PocmanPos + hearPos) == ' ')
                    obs(pocstate.PocmanPos + hearPos) = (pocstate.PowerSteps == 0 ? 'H': 'h');
    }

    // Smell food
    if (CheckFlag(observation, 8))
    {
        COORD smellPos;
        for (smellPos.X = -SmellRange; smellPos.X <= SmellRange; smellPos.X++)
            for (smellPos.Y = -SmellRange; smellPos.Y <= SmellRange; smellPos.Y++)
                if (Maze.Inside(pocstate.PocmanPos + smellPos)
                    && obs(pocstate.PocmanPos + smellPos) == ' ')
                    obs(pocstate.PocmanPos + smellPos) = '.';
    }

    ostr << endl;
    for (int x = 0; x < Maze.GetXSize() + 2; x++)
        ostr << "# ";
    ostr << endl;
    for (int y = Maze.GetYSize() - 1; y >= 0; y--)
    {
        ostr << "# ";
        for (int x = 0; x < Maze.GetXSize(); x++)
            ostr << obs(x, y) << ' ';
        ostr << "#" << endl;
    }
    for (int x = 0; x < Maze.GetXSize() + 2; x++)
        ostr << "# ";
    ostr << endl;
}

void POCMAN::DisplayAction(int action, ostream& ostr) const
{
    ostr << "Pocman moves " << coord::CompassString[action] << endl;
}
