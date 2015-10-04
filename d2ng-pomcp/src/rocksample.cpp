#include "rocksample.h"
#include "utils.h"
#include "distribution.h"
#include <bitset>

using namespace std;
using namespace UTILS;

ROCKSAMPLE::ROCKSAMPLE(int size, int rocks)
:   Grid(size, size),
    Size(size),
    NumRocks(rocks)
{
    NumActions = NumRocks + 5; //动作数
    NumObservations = 3; //观察数
    RewardRange = 20;

    Discount = 0.95;

    HalfEfficiencyDistance = 20;
    StartPos = COORD(0, Size / 2);

    if (size == 7 && rocks == 8)
        Init_7_8();
    else if (size == 11 && rocks == 11)
        Init_11_11();
    else
        InitGeneral();
}

void ROCKSAMPLE::InitGeneral()
{
    vector<COORD> rocks; //保持模拟器和真实环境一致性

  SimpleRNG::ins().RandomSeed(0);

    for (int i = 0; i < NumRocks; ++i) {
        COORD pos;
        do
        {
            pos = COORD(SimpleRNG::ins().Random(Size), SimpleRNG::ins().Random(Size));
        }
        while (Contains(rocks, pos));
        rocks.push_back(pos);
    }

    FillField(rocks);
}

void ROCKSAMPLE::Init_7_8()
{
    // Equivalent to RockSample_7_8.pomdpx
    cout << "Using special layout for rocksample(7, 8)" << endl;

    COORD rocks[] =
    {
        COORD(2, 0),
        COORD(0, 1),
        COORD(3, 1),
        COORD(6, 3),
        COORD(2, 4),
        COORD(3, 4),
        COORD(5, 5),
        COORD(1, 6)
    };

    FillField(rocks);
}

void ROCKSAMPLE::Init_11_11()
{
    // Equivalent to RockSample_11_11.pomdp(x)
    cout << "Using special layout for rocksample(11, 11)" << endl;

    COORD rocks[] =
    {
        COORD(0, 3),
        COORD(0, 7),
        COORD(1, 8),
        COORD(2, 4),
        COORD(3, 3),
        COORD(3, 8),
        COORD(4, 3),
        COORD(5, 8),
        COORD(6, 1),
        COORD(9, 3),
        COORD(9, 9)
    };

    FillField(rocks);
}


STATE* ROCKSAMPLE::Copy(const STATE& state) const
{
    const ROCKSAMPLE_STATE& rockstate = safe_cast<const ROCKSAMPLE_STATE&>(state);
    ROCKSAMPLE_STATE* newstate = MemoryPool.Allocate();
    *newstate = rockstate;
    return newstate;
}

void ROCKSAMPLE::Validate(const STATE& state) const
{
    const ROCKSAMPLE_STATE& rockstate = safe_cast<const ROCKSAMPLE_STATE&>(state);
    assert(Grid.Inside(rockstate.AgentPos));
}

STATE* ROCKSAMPLE::CreateStartState() const
{
    ROCKSAMPLE_STATE* rockstate = MemoryPool.Allocate();

    rockstate->AgentPos = StartPos;
    rockstate->Rocks.clear();

    for (int i = 0; i < NumRocks; i++)
    {
        RS_ENTRY entry;

        entry.Collected = false;
        entry.Valuable = SimpleRNG::ins().Bernoulli(0.5); //随机决定（50/50）
        entry.Count = 0;
        entry.Measured = 0;
        entry.ProbValuable = 0.5;
        entry.LikelihoodValuable = 1.0;
        entry.LikelihoodWorthless = 1.0;

        rockstate->Rocks.push_back(entry);
    }

    return rockstate;
}

void ROCKSAMPLE::FreeState(STATE* state) const
{
    ROCKSAMPLE_STATE* rockstate = safe_cast<ROCKSAMPLE_STATE*>(state);
    MemoryPool.Free(rockstate);
}

bool ROCKSAMPLE::Step(STATE& state, int action,
    int& observation, double& reward) const //进行一步模拟：state, action |-> state, reward, observation
{
    ROCKSAMPLE_STATE& rockstate = safe_cast<ROCKSAMPLE_STATE&>(state);
    reward = 0;
    observation = E_NONE;

    if (action < E_SAMPLE) // move
    {
        switch (action)
        {
            case COORD::E_EAST:
                if (rockstate.AgentPos.X + 1 < Size)
                {
                    rockstate.AgentPos.X++;
                    break;
                }
                else
                {
                    reward = +10;
                    rockstate.AgentPos.X++;
                    return true; //terminated
                }

            case COORD::E_NORTH:
                if (rockstate.AgentPos.Y + 1 < Size)
                    rockstate.AgentPos.Y++;
                else
                    reward = -100; //为什么设置 -100，原始论文里面好像没有？
                break;

            case COORD::E_SOUTH:
                if (rockstate.AgentPos.Y - 1 >= 0)
                    rockstate.AgentPos.Y--;
                else
                    reward = -100;
                break;

            case COORD::E_WEST:
                if (rockstate.AgentPos.X - 1 >= 0)
                    rockstate.AgentPos.X--;
                else
                    reward = -100;
                break;
        }
    }

    if (action == E_SAMPLE) // sample
    {
        int rock = Grid(rockstate.AgentPos);
        if (rock >= 0 && !rockstate.Rocks[rock].Collected)
        {
            rockstate.Rocks[rock].Collected = true;
            if (rockstate.Rocks[rock].Valuable)
                reward = +10;
            else
                reward = -10;
        }
        else
        {
            reward = -100;
        }
    }

    if (action > E_SAMPLE) // check
    {
        int rock = action - E_SAMPLE - 1;
        assert(rock < NumRocks);
        observation = GetObservation(rockstate, rock); //获得观察
        rockstate.Rocks[rock].Measured++;

        double distance = coord::EuclideanDistance(rockstate.AgentPos, RockPos[rock]);
        double efficiency = (1 + pow(2, -distance / HalfEfficiencyDistance)) * 0.5;

        if (observation == E_GOOD)
        {
            rockstate.Rocks[rock].Count++;
            rockstate.Rocks[rock].LikelihoodValuable *= efficiency;
            rockstate.Rocks[rock].LikelihoodWorthless *= 1.0 - efficiency;

        }
        else
        {
            rockstate.Rocks[rock].Count--;
            rockstate.Rocks[rock].LikelihoodWorthless *= efficiency;
            rockstate.Rocks[rock].LikelihoodValuable *= 1.0 - efficiency;
        }
        double denom = (0.5 * rockstate.Rocks[rock].LikelihoodValuable) +
            (0.5 * rockstate.Rocks[rock].LikelihoodWorthless);
        rockstate.Rocks[rock].ProbValuable = (0.5 * rockstate.Rocks[rock].LikelihoodValuable) / denom;
    }

    assert(reward != -100);
    return false; //not terminated
}

bool ROCKSAMPLE::LocalMove(STATE& state, const HISTORY& history,
    int stepObs, const STATUS& ) const //局部扰动
{
    if (NumRocks) {
        ROCKSAMPLE_STATE& rockstate = safe_cast<ROCKSAMPLE_STATE&>(state);
        int rock = SimpleRNG::ins().Random(NumRocks);
        rockstate.Rocks[rock].Valuable = !rockstate.Rocks[rock].Valuable;

        if (history.Back().Action > E_SAMPLE) // check rock
        {
            rock = history.Back().Action - E_SAMPLE - 1;
            int realObs = history.Back().Observation;

            // Condition new state on real observation
            int newObs = GetObservation(rockstate, rock);
            if (newObs != realObs)
                return false;

            // Update counts to be consistent with real observation
            if (realObs == E_GOOD && stepObs == E_BAD)
                rockstate.Rocks[rock].Count += 2;
            if (realObs == E_BAD && stepObs == E_GOOD)
                rockstate.Rocks[rock].Count -= 2;
        }
    }
    return true;
}

void ROCKSAMPLE::GenerateLegal(const STATE& state, /*const HISTORY& ,*/
    vector<int>& legal, const STATUS& ) const
{
    const ROCKSAMPLE_STATE& rockstate =
        safe_cast<const ROCKSAMPLE_STATE&>(state);

    if (Grid.Inside(rockstate.AgentPos)) {
        if (rockstate.AgentPos.Y + 1 < Size)
            legal.push_back(COORD::E_NORTH);

        legal.push_back(COORD::E_EAST);

        if (rockstate.AgentPos.Y - 1 >= 0)
            legal.push_back(COORD::E_SOUTH);

        if (rockstate.AgentPos.X - 1 >= 0)
            legal.push_back(COORD::E_WEST);

        int rock = Grid(rockstate.AgentPos);
        if (rock >= 0 && !rockstate.Rocks[rock].Collected)
            legal.push_back(E_SAMPLE);

        for (rock = 0; rock < NumRocks; ++rock)
            if (!rockstate.Rocks[rock].Collected)
                legal.push_back(rock + 1 + E_SAMPLE);
    }
}

void ROCKSAMPLE::GeneratePreferred(const STATE& state, const HISTORY& history, //手工策略
    vector<int>& actions, const STATUS& ) const //获得优先动作
{

//	static const bool UseBlindPolicy = false;
//
//	if (UseBlindPolicy)
//	{
//		actions.push_back(COORD::E_EAST);
//		return;
//	}

    const ROCKSAMPLE_STATE& rockstate =
            safe_cast<const ROCKSAMPLE_STATE&>(state);

    if (!Grid.Inside(rockstate.AgentPos)) {
        return;
    }

    // Sample rocks with more +ve than -ve observations
    int rock = Grid(rockstate.AgentPos);
    if (rock >= 0 && !rockstate.Rocks[rock].Collected)
    {
        int total = 0;
        for (int t = 0; t < history.Size(); ++t)
        {
            if (history[t].Action == rock + 1 + E_SAMPLE)
            {
                if (history[t].Observation == E_GOOD)
                    total++;
                if (history[t].Observation == E_BAD)
                    total--;
            }
        }
        if (total > 0)
        {
            actions.push_back(E_SAMPLE);
            return;
        }

    }

    // processes the rocks
    bool all_bad = true;
    bool north_interesting = false;
    bool south_interesting = false;
    bool west_interesting  = false;
    bool east_interesting  = false;

    for (int rock = 0; rock < NumRocks; ++rock)
    {
        const RS_ENTRY& entry = rockstate.Rocks[rock];
        if (!entry.Collected)
        {
            int total = 0;
            for (int t = 0; t < history.Size(); ++t)
            {
                if (history[t].Action == rock + 1 + E_SAMPLE)
                {
                    if (history[t].Observation == E_GOOD)
                        total++;
                    if (history[t].Observation == E_BAD)
                        total--;
                }
            }

            if (total >= 0)
            {
                all_bad = false;

                if (RockPos[rock].Y > rockstate.AgentPos.Y)
                    north_interesting = true;
                if (RockPos[rock].Y < rockstate.AgentPos.Y)
                    south_interesting = true;
                if (RockPos[rock].X < rockstate.AgentPos.X)
                    west_interesting = true;
                if (RockPos[rock].X > rockstate.AgentPos.X)
                    east_interesting = true;
            }
        }
    }

    // if all remaining rocks seem bad, then head east
    if (all_bad)
    {
        actions.push_back(COORD::E_EAST);
        return;
    }

    // generate a random legal move, with the exceptions that:
    //   a) there is no point measuring a rock that is already collected
    //   b) there is no point measuring a rock too often
    //   c) there is no point measuring a rock which is clearly bad or good
    //   d) we never sample a rock (since we need to be sure)
    //   e) we never move in a direction that doesn't take us closer to
    //      either the edge of the map or an interesting rock
    if (rockstate.AgentPos.Y + 1 < Size && north_interesting)
            actions.push_back(COORD::E_NORTH);

    if (east_interesting)
        actions.push_back(COORD::E_EAST);

    if (rockstate.AgentPos.Y - 1 >= 0 && south_interesting)
        actions.push_back(COORD::E_SOUTH);

    if (rockstate.AgentPos.X - 1 >= 0 && west_interesting)
        actions.push_back(COORD::E_WEST);


    for (rock = 0; rock < NumRocks; ++rock)
    {
        if (!rockstate.Rocks[rock].Collected    &&
            rockstate.Rocks[rock].ProbValuable != 0.0 &&
            rockstate.Rocks[rock].ProbValuable != 1.0 &&
            rockstate.Rocks[rock].Measured < 5  &&
            std::abs(rockstate.Rocks[rock].Count) < 2)
        {
            actions.push_back(rock + 1 + E_SAMPLE);
        }
    }
}

int ROCKSAMPLE::GetObservation(const ROCKSAMPLE_STATE& rockstate, int rock) const //noisy observation
{
    double distance = coord::EuclideanDistance(rockstate.AgentPos, RockPos[rock]);
    double efficiency = (1 + pow(2, -distance / HalfEfficiencyDistance)) * 0.5; //[0.5, 1.0]

    if (SimpleRNG::ins().Bernoulli(efficiency))
        return rockstate.Rocks[rock].Valuable ? E_GOOD : E_BAD;
    else
        return rockstate.Rocks[rock].Valuable ? E_BAD : E_GOOD;
}

void ROCKSAMPLE::DisplayBeliefs(const BELIEF_STATE& belief,
    std::ostream& ostr) const
{
    std::vector<double> prob(NumRocks);

    for (int i = 0; i < belief.GetNumSamples(); ++i) {
        const ROCKSAMPLE_STATE& rockstate = safe_cast<const ROCKSAMPLE_STATE&>(*belief.GetSample(i));

        for (int j = 0; j < NumRocks; ++j) {
            if (!rockstate.Rocks[j].Collected) {
                prob[j] += rockstate.Rocks[j].Valuable;
            }
        }
    }

    ostr << "#Belief: ";
    for (int j = 0; j < NumRocks; ++j) {
        prob[j] /= belief.GetNumSamples();

        if (prob[j] > 0.0) {
            ostr << "#" << j << "(" << prob[j] << ") ";
        }
    }
    ostr << std::endl;
}

void ROCKSAMPLE::DisplayState(const STATE& state, std::ostream& ostr) const
{
    const ROCKSAMPLE_STATE& rockstate = safe_cast<const ROCKSAMPLE_STATE&>(state);

    for (int x = 0; x < Size + 2; x++)
        ostr << "# ";
    ostr << endl;
    for (int y = Size - 1; y >= 0; y--)
    {
        ostr << "# ";
        for (int x = 0; x < Size; x++)
        {
            COORD pos(x, y);
            int rock = Grid(pos);
            const RS_ENTRY& entry = rockstate.Rocks[rock];
            if (rockstate.AgentPos == COORD(x, y))
                ostr << "A ";
            else if (rock >= 0 && !entry.Collected)
                ostr << rock << (entry.Valuable ? "$" : "X");
            else
                ostr << ". ";
        }
        ostr << "#" << endl;
    }
    for (int x = 0; x < Size + 2; x++)
        ostr << "# ";
    ostr << endl;
}

void ROCKSAMPLE::DisplayObservation(const STATE& , int observation, std::ostream& ostr) const
{
    switch (observation)
    {
    case E_NONE:
        break;
    case E_GOOD:
        ostr << "Observed good" << endl;
        break;
    case E_BAD:
        ostr << "Observed bad" << endl;
        break;
    }
}

void ROCKSAMPLE::DisplayAction(int action, std::ostream& ostr) const
{
    if (action < E_SAMPLE)
        ostr << coord::CompassString[action] << endl;
    if (action == E_SAMPLE)
        ostr << "Sample" << endl;
    if (action > E_SAMPLE)
        ostr << "Check " << action - E_SAMPLE - 1 << endl;
}

FieldVisionRockSample::FieldVisionRockSample(int size, int rocks): ROCKSAMPLE(size, rocks)
{
    NumActions = 5; //动作数
    NumObservations = 1 << rocks; //观察数
}

bool FieldVisionRockSample::Step(STATE& state, int action,
    int& observation, double& reward) const
{
    int tmp_obs;
    double tmp_reward;

    observation = 0;
    for (int i = 0; i < NumRocks; ++i) {
        tmp_obs = 0;
        ROCKSAMPLE::Step(state, i + E_SAMPLE + 1, tmp_obs, tmp_reward);
        observation = (observation << 1) + tmp_obs - 1;
    }

    assert(action <= E_SAMPLE);
    return ROCKSAMPLE::Step(state, action, tmp_obs, reward);
}

void FieldVisionRockSample::GenerateLegal(const STATE& state, /*const HISTORY& history,*/
    std::vector<int>& legal, const STATUS& ) const
{
    const ROCKSAMPLE_STATE& rockstate =
        safe_cast<const ROCKSAMPLE_STATE&>(state);

    if (Grid.Inside(rockstate.AgentPos)) {
        if (rockstate.AgentPos.Y + 1 < Size)
            legal.push_back(COORD::E_NORTH);

        legal.push_back(COORD::E_EAST);

        if (rockstate.AgentPos.Y - 1 >= 0)
            legal.push_back(COORD::E_SOUTH);

        if (rockstate.AgentPos.X - 1 >= 0)
            legal.push_back(COORD::E_WEST);

        int rock = Grid(rockstate.AgentPos);
        if (rock >= 0 && !rockstate.Rocks[rock].Collected)
            legal.push_back(E_SAMPLE);
    }
}

void FieldVisionRockSample::GeneratePreferred(const STATE& state, const HISTORY& /*history*/,
    std::vector<int>& actions, const STATUS& status) const
{
    GenerateLegal(state, actions, status);
}

bool FieldVisionRockSample::LocalMove(STATE& state, const HISTORY& /*history*/,
    int /*stepObs*/, const STATUS& ) const
{
    if (NumRocks) {
        ROCKSAMPLE_STATE& rockstate = safe_cast<ROCKSAMPLE_STATE&>(state);
        int rock = SimpleRNG::ins().Random(NumRocks);

        if (!rockstate.Rocks[rock].Collected) {
            rockstate.Rocks[rock].Valuable = !rockstate.Rocks[rock].Valuable;
        }
    }

    return true;
}

void FieldVisionRockSample::DisplayObservation(const STATE& , int observation, std::ostream& ostr) const
{
    std::bitset<32> x(observation);
    ostr << "observation: " << x << endl;
}
