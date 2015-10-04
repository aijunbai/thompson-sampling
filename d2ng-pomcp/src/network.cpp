#include "network.h"
#include "distribution.h"
#include "utils.h"

using namespace std;
using namespace UTILS;

NETWORK::NETWORK(int numMachines, int ntype)
:   NumMachines(numMachines),
    FailureProb1(0.1),
    FailureProb2(0.333),
    ObsProb(0.95)
{
    NumActions = NumMachines * 2 + 1;
    NumObservations = 3;
    Discount = 0.95;

    switch (ntype)
    {
    case E_CYCLE:
        MakeRingNeighbours();
        break;
    case E_3LEGS:
        Make3LegsNeighbours();
        break;
    }
}

void NETWORK::MakeRingNeighbours()
{
    Neighbours.resize(NumMachines);
    for (int i = 0; i < NumMachines; ++i)
    {
        Neighbours[i].push_back((i + 1) % NumMachines);
        Neighbours[i].push_back((i + NumMachines - 1) % NumMachines);
    }
}

void NETWORK::Make3LegsNeighbours()
{
    assert(NumMachines >= 4 && NumMachines % 3 == 1);
    Neighbours.resize(NumMachines);
    Neighbours[0].push_back(1);
    Neighbours[0].push_back(2);
    Neighbours[0].push_back(3);
    for (int i = 1; i < NumMachines; ++i)
    {
        if (i < NumMachines - 3)
            Neighbours[i].push_back(i + 3);
        if (i <= 4)
            Neighbours[i].push_back(0);
        else
            Neighbours[i].push_back(i - 3);
    }
}

STATE* NETWORK::Copy(const STATE& state) const
{
    const NETWORK_STATE& nstate = safe_cast<const NETWORK_STATE&>(state);
    NETWORK_STATE* newstate = MemoryPool.Allocate();
    *newstate = nstate;
    return newstate;
}

void NETWORK::Validate(const STATE& ) const
{
//    const NETWORK_STATE& nstate = safe_cast<const NETWORK_STATE&>(state);
    // what to validate?
}

STATE* NETWORK::CreateStartState() const
{
    NETWORK_STATE* nstate = MemoryPool.Allocate();
    for (int i = 0; i < NumMachines; i++)
        nstate->Machines.push_back(true);
    return nstate;
}

void NETWORK::FreeState(STATE* state) const
{
    NETWORK_STATE* nstate = safe_cast<NETWORK_STATE*>(state);
    MemoryPool.Free(nstate);
}

bool NETWORK::Step(STATE& state, int action,
    int& observation, double& reward) const
{
    NETWORK_STATE& nstate = safe_cast<NETWORK_STATE&>(state);
    reward = 0;
    observation = 2;

    vector<bool> neighbourFailure(NumMachines, false);
    for (int i = 0; i < NumMachines; i++)
        for (uint j = 0; j < Neighbours[i].size(); ++j)
            if (!nstate.Machines[Neighbours[i][j]])
                neighbourFailure[i] = true;

    for (int i = 0; i < NumMachines; i++)
    {
        if (!neighbourFailure[i])
            nstate.Machines[i] = !SimpleRNG::ins().Bernoulli(FailureProb1);
        else
            nstate.Machines[i] = !SimpleRNG::ins().Bernoulli(FailureProb2);
    }

    for (int i = 0; i < NumMachines; i++)
    {
        if (nstate.Machines[i])
        {
            if (Neighbours[i].size() > 2) // server
                reward += 2;
            else
                reward += 1;
        }
    }

    if (action < NumMachines * 2)
    {
        int machine = action / 2;
        bool reboot = action % 2;

        if (reboot)
        {
            reward -= 2.5;
            nstate.Machines[machine] = true;
            observation = SimpleRNG::ins().Bernoulli(ObsProb);
        }
        else // ping
        {
            reward -= 0.1;
            if (SimpleRNG::ins().Bernoulli(ObsProb))
                observation = nstate.Machines[machine];
            else
                observation = !nstate.Machines[machine];
        }
    }

    return false;
}

void NETWORK::DisplayBeliefs(const BELIEF_STATE& ,
    std::ostream& ) const
{
}

void NETWORK::DisplayState(const STATE& state, std::ostream& ostr) const
{
    const NETWORK_STATE& nstate = safe_cast<const NETWORK_STATE&>(state);
    for (int i = 0; i < NumMachines; i++)
        ostr << i << ": " << (nstate.Machines[i] ? "operational" : "failed") << endl;
}

void NETWORK::DisplayObservation(const STATE& , int observation, std::ostream& ostr) const
{
    switch (observation)
    {
    case 0:
        ostr << "Machine failed" << endl;
        break;
    case 1:
        ostr << "Machine operational" << endl;
        break;
    case 2:
        // Don't say anything
        break;
    }
}

void NETWORK::DisplayAction(int action, std::ostream& ostr) const
{
    switch (action)
    {
        if (action == NumMachines)
            ostr << "No action" << endl;
        int machine = action / 2;
        int reboot = action % 2;
        ostr << (reboot ? "Reboot" : "Ping") << " machine " << machine << endl;
    }
}
