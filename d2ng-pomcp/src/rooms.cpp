#include "rooms.h"
#include "utils.h"
#include "distribution.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <boost/unordered_map.hpp>

using namespace std;
using namespace UTILS;

ROOMS::ROOMS(const char *map_name, bool state_abstraction):
    mStateAbstraction(state_abstraction),
    mGrid(0), mRooms(0)
{
    Parse(map_name);

    NumActions = 4; //动作数
    NumObservations = mStateAbstraction? mRooms: mGrid->GetXSize() * mGrid->GetYSize();
    Discount = 0.9;
}

ROOMS::~ROOMS()
{
    delete mGrid;
}

void ROOMS::Parse(const char *file_name)
{
    ifstream fin(file_name);

    if (!fin) {
        cerr << "can not open map file: " << file_name << endl;
        return;
    }

    uint xsize, ysize;

    fin.ignore(LINE_MAX, ' ');
    fin >> xsize >> ysize;
    fin.ignore(LINE_MAX, ' ');
    fin >> mRooms;
    fin.ignore(LINE_MAX, ' ');
    fin >> mStartPos.X >> mStartPos.Y;
    fin.ignore(LINE_MAX, ' ');
    fin >> mGoalPos.X >> mGoalPos.Y;

    mGrid = new GRID<int>(xsize, ysize);

    int row = 0;
    string line;

    while (getline(fin, line)) {
        if (line.size() < ysize) {
            continue;
        }

        for (uint col = 0; col < line.size(); ++col) {
            int x = col;
            int y = ysize - 1 - row;
            mGrid->operator ()(x, y) = line[col];
        }
        row += 1;
    }
}

STATE* ROOMS::Copy(const STATE& state) const
{
    const ROOMS_STATE& rooms_state = safe_cast<const ROOMS_STATE&>(state);
    ROOMS_STATE* newstate = mMemoryPool.Allocate();
    *newstate = rooms_state;
    return newstate;
}

void ROOMS::Validate(const STATE& state) const
{
    const ROOMS_STATE& rooms_state = safe_cast<const ROOMS_STATE&>(state);
    assert(mGrid->Inside(rooms_state.AgentPos));
}

STATE* ROOMS::CreateStartState() const
{
    ROOMS_STATE* rooms_state = mMemoryPool.Allocate();
    rooms_state->AgentPos = mStartPos;
    return rooms_state;
}

void ROOMS::FreeState(STATE* state) const
{
    ROOMS_STATE* rooms_state = safe_cast<ROOMS_STATE*>(state);
    mMemoryPool.Free(rooms_state);
}

bool ROOMS::Step(STATE& state, int action,
    int& observation, double& reward) const //进行一步模拟：state, action |-> state, reward, observation
{
    assert(action < NumActions);

    ROOMS_STATE& rooms_state = safe_cast<ROOMS_STATE&>(state);
    reward = 0.0;

    if (SimpleRNG::ins().Bernoulli(4.0/9.0)) { //fail
        action = SimpleRNG::ins().Random(NumActions);
    }

    COORD pos = rooms_state.AgentPos + coord::Compass[action];
    if (mGrid->operator ()(pos) != 'x') { //not wall
        rooms_state.AgentPos = pos;
    }
    observation = GetObservation(rooms_state);

    if (rooms_state.AgentPos == mGoalPos) {
        reward = 1.0;
        return true;
    }

    return false; //not terminated
}

bool ROOMS::LocalMove(STATE& state, const HISTORY& history, int, const STATUS& ) const //局部扰动
{
    ROOMS_STATE rooms_state = safe_cast<ROOMS_STATE&>(state);
    if (GetObservation(rooms_state) == history.Back().Observation) {
        return true;
    }
    return false;
}

void ROOMS::GenerateLegal(const STATE& state, /*const HISTORY& ,*/
    vector<int>& legal, const STATUS& ) const
{
    const ROOMS_STATE& rooms_state =
            safe_cast<const ROOMS_STATE&>(state);

    assert(mGrid->Inside(rooms_state.AgentPos));

    legal.push_back(COORD::E_NORTH);
    legal.push_back(COORD::E_EAST);
    legal.push_back(COORD::E_SOUTH);
    legal.push_back(COORD::E_WEST);
}

void ROOMS::GeneratePreferred(const STATE& state, const HISTORY&, //手工策略
    vector<int>& actions, const STATUS& status) const //获得优先动作
{
    GenerateLegal(state, actions, status);
}

int ROOMS::GetObservation(const ROOMS_STATE& rooms_state) const
{
    return mStateAbstraction?
                mGrid->operator ()(rooms_state.AgentPos) - '0':
                mGrid->Index(rooms_state.AgentPos);
}

void ROOMS::DisplayBeliefs(const BELIEF_STATE& belief,
    std::ostream& ostr) const
{
    boost::unordered_map<COORD, int> m;
    for (int i = 0; i < belief.GetNumSamples(); ++i) {
        const ROOMS_STATE& state = safe_cast<const ROOMS_STATE&>(*belief.GetSample(i));
        m[state.AgentPos] += 1;
    }

    vector<pair<double, const COORD*> > sorted;
    for (boost::unordered_map<COORD, int>::iterator it = m.begin(); it != m.end(); ++it) {
        double p = double(it->second) / double(belief.GetNumSamples());
        sorted.push_back(make_pair(p, &(it->first)));
    }
    sort(sorted.begin(), sorted.end(), greater<pair<double, const COORD*> >());

    ostr << "#Belief: ";
    for (uint i = 0; i < sorted.size(); ++i) {
        ostr << "#" << *(sorted[i].second) << " (" << sorted[i].first << ") ";
    }
    ostr << std::endl;
}

void ROOMS::DisplayState(const STATE& state, std::ostream& ostr) const
{
    const ROOMS_STATE& rooms_state = safe_cast<const ROOMS_STATE&>(state);

    ostr << "Y" << endl;
    for (int y = mGrid->GetYSize() - 1; y >= 0; --y) {
        for (int x = 0; x < mGrid->GetXSize(); ++x) {
            char cell = mGrid->operator()(x, y);

            if (rooms_state.AgentPos == COORD(x, y)) {
                ostr << "@";
            }
            else if (cell != 'x') {
                ostr << ".";
            }
            else {
                ostr << cell;
            }
        }
        if (y == 0) {
            ostr << "X" << endl;
        }
        else {
            ostr << endl;
        }
    }
    ostr << endl;
}

void ROOMS::DisplayObservation(const STATE& , int observation, std::ostream& ostr) const
{
  if (mStateAbstraction)
    ostr << "Observation: " << "Room" << observation << endl;
  else
    ostr << "Observation: " << "Coord" << mGrid->Coord(observation) << endl;
}

void ROOMS::DisplayAction(int action, std::ostream& ostr) const
{
    ostr << coord::CompassString[action] << endl;
}
