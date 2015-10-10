#include "battleship.h"
#include "beliefstate.h"
#include "utils.h"
#include "distribution.h"
#include <math.h>
#include <iomanip>

using namespace std;
using namespace UTILS;

BATTLESHIP::BATTLESHIP(int xsize, int ysize, int maxlength)
:   XSize(xsize),
    YSize(ysize),
    MaxLength(maxlength+1)
{
    NumActions = XSize * YSize;
    NumObservations = 2;
    Discount = 1;
    TotalRemaining = MaxLength - 1;
    mName << "battleship_" << XSize << "_" << YSize << "_" << MaxLength;
}

STATE* BATTLESHIP::Copy(const STATE& state) const
{
    assert(state.IsAllocated());
    const BATTLESHIP_STATE& oldstate = safe_cast<const BATTLESHIP_STATE&>(state);
    BATTLESHIP_STATE* newstate = MemoryPool.Allocate();
    *newstate = oldstate;
    return newstate;
}

void BATTLESHIP::Validate(const STATE& state) const
{
    const BATTLESHIP_STATE& bsstate = safe_cast<const BATTLESHIP_STATE&>(state);
    for (int i = 0; i < XSize * YSize; ++i)
    {
        if (bsstate.Cells(i).Diagonal && bsstate.Cells(i).Occupied)
        {
            DisplayState(bsstate, cout);
            assert(false);
        }
    }
}

STATE* BATTLESHIP::CreateStartState() const
{
    BATTLESHIP_STATE* bsstate = MemoryPool.Allocate();
    bsstate->Cells.Resize(XSize, YSize);
    for (int i = 0; i < XSize * YSize; ++i)
    {
        CELL& cell = bsstate->Cells(i);
        cell.Occupied = false;
        cell.Visited = false;
        cell.Diagonal = false;
    }
    bsstate->NumRemaining = 0;

/*    bool found;*/
    bsstate->Ships.clear();
    for (int length = MaxLength; length >= 2; --length)
    {
        int numShips = 1;
        for (int shipIndex = 0; shipIndex < numShips; ++shipIndex)
        {
            SHIP ship;
            do
            {
                ship.Direction = SimpleRNG::ins().Random(4);
                ship.Position = COORD(SimpleRNG::ins().Random(XSize), SimpleRNG::ins().Random(YSize));
                ship.Length = length;
            }
            while (Collision(*bsstate, ship));

            MarkShip(*bsstate, ship);
            bsstate->Ships.push_back(ship);
        }
    }
    return bsstate;
}

void BATTLESHIP::FreeState(STATE* state) const
{
    BATTLESHIP_STATE* bsstate = safe_cast<BATTLESHIP_STATE*>(state);
    MemoryPool.Free(bsstate);
}

bool BATTLESHIP::Step(STATE& state, int action,
    int& observation, double& reward) const
{
    BATTLESHIP_STATE& bsstate = safe_cast<BATTLESHIP_STATE&>(state);

    COORD actionPos = bsstate.Cells.Coord(action);

    CELL& cell = bsstate.Cells(actionPos);
    if (cell.Visited)
    {
        reward = -10;
        observation = 0;
        DisplayState(state, cout);
        DisplayAction(action, cout);
        assert(false);
    }
    else
    {
        if (cell.Occupied) // hit
        {
            reward = -1;
            observation = 1;
            bsstate.NumRemaining--;

            // Mark four diagonals, not possible for ships to be here
            for (int d = 4; d < 8; ++d)
                if (bsstate.Cells.Inside(actionPos + coord::Compass[d]))
                    bsstate.Cells(actionPos + coord::Compass[d]).Diagonal = true;
        }
        else // miss
        {
            reward = -1;
            observation = 0;
        }
        cell.Visited = true;
    }

    if (bsstate.NumRemaining == 0)
    {
        reward += XSize * YSize;
        return true;
    }
    else
    {
        return false;
    }
}

bool BATTLESHIP::LocalMove(STATE& state, const HISTORY& history,
    int /*stepObs*/, const STATUS& ) const
{
    BATTLESHIP_STATE& bsstate = safe_cast<BATTLESHIP_STATE&>(state);
    bool refreshDiagonals = history.Size() &&
        bsstate.Cells(history.Back().Action).Occupied != history.Back().Observation;

    int mode = SimpleRNG::ins().Random(3);
    bool success = false;
    switch (mode)
    {
        case 0:
            success = MoveShips(bsstate);
            break;
        case 1:
            success = SwitchTwoShips(bsstate);
            break;
        case 2:
            success = SwitchThreeShips(bsstate);
            break;
    }
    if (!success)
        return false;

    if (refreshDiagonals)
        for (int i = 0; i < XSize * YSize; ++i)
            bsstate.Cells(i).Diagonal = false;

    for (int t = 0; t < history.Size(); ++t)
    {
        // Ensure that ships are consistent with observation history
        int a = history[t].Action;
        COORD pos = bsstate.Cells.Coord(a);
        const CELL& cell = bsstate.Cells(a);
        assert(cell.Visited);
        if (cell.Occupied != history[t].Observation)
            return false;

        if (refreshDiagonals && cell.Occupied)
            for (int d = 4; d < 8; ++d)
                if (bsstate.Cells.Inside(pos + coord::Compass[d]))
                    bsstate.Cells(pos + coord::Compass[d]).Diagonal = true;
    }

    return true;
}

bool BATTLESHIP::MoveShips(BATTLESHIP_STATE& bsstate) const
{
    // Number of ships to move
    int numMoves = SimpleRNG::ins().Random(1, 4);
    static vector<int> shipIndices;
    shipIndices.clear();

    for (int move = 0; move < numMoves; ++move)
    {
        int shipIndex = SimpleRNG::ins().Random(bsstate.Ships.size());
        if (Contains(shipIndices, shipIndex))
            return false;
        shipIndices.push_back(shipIndex);
        UnmarkShip(bsstate, bsstate.Ships[shipIndex]);
    }

    for (int move = 0; move < numMoves; ++move)
    {
        SHIP& ship = bsstate.Ships[shipIndices[move]];
        ship.Direction = SimpleRNG::ins().Random(4);
        ship.Position = COORD(SimpleRNG::ins().Random(XSize), SimpleRNG::ins().Random(YSize));
        if (Collision(bsstate, ship))
            return false;
        MarkShip(bsstate, ship);
    }

    return true;
}

bool BATTLESHIP::SwitchTwoShips(BATTLESHIP_STATE& bsstate) const
{
    int longShipIndex = SimpleRNG::ins().Random(bsstate.Ships.size());
    int shortShipIndex = SimpleRNG::ins().Random(bsstate.Ships.size());
    SHIP& longShip = bsstate.Ships[longShipIndex];
    SHIP& shortShip = bsstate.Ships[shortShipIndex];

    int sizeDiff = longShip.Length - shortShip.Length;
    if (sizeDiff <= 0)
        return false;

    int longOffset = SimpleRNG::ins().Random(0, sizeDiff + 1);
    int shortOffset = SimpleRNG::ins().Random(0, sizeDiff + 1);

    SHIP oldShortShip = shortShip;
    SHIP oldLongShip = longShip;
    longShip.Direction = oldShortShip.Direction;
    longShip.Position = oldShortShip.Position
        + coord::Compass[coord::Opposite(oldShortShip.Direction)] * longOffset;
    shortShip.Direction = oldLongShip.Direction;
    shortShip.Position = oldLongShip.Position
        + coord::Compass[oldLongShip.Direction] * shortOffset;

    UnmarkShip(bsstate, oldLongShip);
    UnmarkShip(bsstate, oldShortShip);

    if (Collision(bsstate, longShip))
        return false;
    MarkShip(bsstate, longShip);
    if (Collision(bsstate, shortShip))
        return false;
    MarkShip(bsstate, shortShip);

    return true;
}

bool BATTLESHIP::SwitchThreeShips(BATTLESHIP_STATE& bsstate) const
{
    int longShipIndex = SimpleRNG::ins().Random(bsstate.Ships.size());
    int shortShipIndex1 = SimpleRNG::ins().Random(bsstate.Ships.size());
    int shortShipIndex2 = SimpleRNG::ins().Random(bsstate.Ships.size());
    SHIP& longShip = bsstate.Ships[longShipIndex];
    SHIP& shortShip1 = bsstate.Ships[shortShipIndex1];
    SHIP& shortShip2 = bsstate.Ships[shortShipIndex2];

    int sizeDiff = longShip.Length - shortShip1.Length - shortShip2.Length;
    if (sizeDiff <= 0 || shortShipIndex1 == shortShipIndex2)
        return false;

    int longOffset = SimpleRNG::ins().Random(0, longShip.Length - shortShip1.Length + 1);
    int shortOffset1 = SimpleRNG::ins().Random(0, sizeDiff);
    int shortOffset2 = SimpleRNG::ins().Random(shortOffset1 + 2, longShip.Length - shortShip2.Length + 1);

    SHIP oldShortShip1 = shortShip1;
    SHIP oldShortShip2 = shortShip2;
    SHIP oldLongShip = longShip;
    longShip.Direction = oldShortShip1.Direction;
    longShip.Position = oldShortShip1.Position
        + coord::Compass[coord::Opposite(oldShortShip1.Direction)] * longOffset;
    shortShip1.Direction = oldLongShip.Direction;
    shortShip1.Position = oldLongShip.Position
        + coord::Compass[oldLongShip.Direction] * shortOffset1;
    shortShip2.Direction = oldLongShip.Direction;
    shortShip2.Position = oldLongShip.Position
        + coord::Compass[oldLongShip.Direction] * shortOffset2;

    UnmarkShip(bsstate, oldLongShip);
    UnmarkShip(bsstate, oldShortShip1);
    UnmarkShip(bsstate, oldShortShip2);

    if (Collision(bsstate, longShip))
        return false;
    MarkShip(bsstate, longShip);
    if (Collision(bsstate, shortShip1))
        return false;
    MarkShip(bsstate, shortShip1);
    if (Collision(bsstate, shortShip2))
        return false;
    MarkShip(bsstate, shortShip2);

    return true;
}

void BATTLESHIP::GenerateLegal(const STATE& state, /*const HISTORY& ,*/
    vector<int>& legal, const STATUS& status) const
{
    const BATTLESHIP_STATE& bsstate = safe_cast<const BATTLESHIP_STATE&>(state);
    bool diagonals = Knowledge.Level(status.Phase) == KNOWLEDGE::SMART;
    if (diagonals)
    {
        for (int a = 0; a < NumActions; ++a)
            if (!bsstate.Cells(a).Visited && !bsstate.Cells(a).Diagonal)
                legal.push_back(a);
    }
    else
    {
        for (int a = 0; a < NumActions; ++a)
            if (!bsstate.Cells(a).Visited)
                legal.push_back(a);
    }
}

bool BATTLESHIP::Collision(const BATTLESHIP_STATE& bsstate,
    const SHIP& ship) const
{
    COORD pos = ship.Position;
    for (int i = 0; i < ship.Length; ++i)
    {
        if (!bsstate.Cells.Inside(pos))
            return true;
        const CELL& cell = bsstate.Cells(pos);
        if (cell.Occupied)
            return true;
        for (int adj = 0; adj < 8; ++adj)
            if (bsstate.Cells.Inside(pos + coord::Compass[adj]) &&
                bsstate.Cells(pos + coord::Compass[adj]).Occupied)
                return true;
        pos += coord::Compass[ship.Direction];
    }
    return false;
}

void BATTLESHIP::MarkShip(BATTLESHIP_STATE& bsstate, const SHIP& ship) const
{
    COORD pos = ship.Position;
    for (int i = 0; i < ship.Length; ++i)
    {
        CELL& cell = bsstate.Cells(pos);
        assert(!cell.Occupied);
        cell.Occupied = true;
        if (!cell.Visited)
            bsstate.NumRemaining++;
        pos += coord::Compass[ship.Direction];
    }
}

void BATTLESHIP::UnmarkShip(BATTLESHIP_STATE& bsstate, const SHIP& ship) const
{
    COORD pos = ship.Position;
    for (int i = 0; i < ship.Length; ++i)
    {
        CELL& cell = bsstate.Cells(pos);
        assert(cell.Occupied);
        if (!cell.Visited)
            bsstate.NumRemaining--;
        cell.Occupied = false;
        pos += coord::Compass[ship.Direction];
    }
}

void BATTLESHIP::DisplayBeliefs(const BELIEF_STATE& beliefState,
    ostream& ostr) const
{
    GRID<int> counts(XSize, YSize);
    counts.SetAllValues(0);

    for (int i = 0; i < beliefState.GetNumSamples(); i++)
    {
        const BATTLESHIP_STATE* bsstate =
            safe_cast<const BATTLESHIP_STATE*>(
                beliefState.GetSample(i));

        for (int x = 0; x < XSize; ++x)
            for (int y = 0; y < YSize; ++y)
                counts(x, y) += bsstate->Cells(x, y).Occupied;
    }

    for (int y = YSize - 1; y >= 0; y--)
    {
        for (int x = 0; x < XSize; x++)
        {
            ostr.width(6);
            ostr.precision(2);
            ostr << fixed << (double) counts(x, y) / beliefState.GetNumSamples();
        }
        ostr << endl;
    }
}

void BATTLESHIP::DisplayState(const STATE& state, ostream& ostr) const
{
    const BATTLESHIP_STATE& bsstate = safe_cast<const BATTLESHIP_STATE&>(state);
    ostr << endl << "  ";
    for (int x = 0; x < XSize; x++)
        ostr << setw(1) << x << ' ';
    ostr << "  " << endl;
    for (int y = YSize - 1; y >= 0; y--)
    {
        ostr << setw(1) << y << ' ';
        for (int x = 0; x < XSize; x++)
        {
            const CELL& cell = bsstate.Cells(x, y);
            char c = '.';
            if (cell.Occupied && cell.Visited)
                c = '@';
            else if (cell.Occupied && !cell.Visited)
                c = '*';
            else if (!cell.Occupied && cell.Visited)
                c = 'X';
            else if (!cell.Occupied && cell.Diagonal)
                c = '/';
            ostr << c << ' ';
        }
        ostr << setw(1) << y << endl;
    }
    ostr << "  ";
    for (int x = 0; x < XSize; x++)
        ostr << setw(1) << x << ' ';
    ostr << "  " << endl;
    ostr << "NumRemaining = " << bsstate.NumRemaining << endl;
}

void BATTLESHIP::DisplayObservation(const STATE& , int observation, ostream& ostr) const
{
    if (observation)
        ostr << "o=Hit\n";
    else
        ostr << "o=Miss\n";
}

void BATTLESHIP::DisplayAction(int action, ostream& ostr) const
{
    COORD actionPos = COORD(action % XSize, action / XSize);

    ostr << endl << "  ";
    for (int x = 0; x < XSize; x++)
        ostr << setw(1) << x << ' ';
    ostr << "  " << endl;
    for (int y = YSize - 1; y >= 0; y--)
    {
        ostr << setw(1) << y << ' ';
        for (int x = 0; x < XSize; x++)
        {
            char c = ' ';
            if (actionPos == COORD(x, y))
                c = '@';
            ostr << c << ' ';
        }
        ostr << setw(1) << y << endl;
    }
    ostr << "  ";
    for (int x = 0; x < XSize; x++)
        ostr << setw(1) << x << ' ';
    ostr << "  " << endl;
}
