#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include "simulator.h"
#include "grid.h"
#include <list>

struct SHIP
{
    COORD Position;
    int Direction;
    int Length;
};

struct CELL
{
    bool Occupied;
    bool Visited;
    bool Diagonal;
};

inline std::size_t hash_value(const SHIP& v)
{
	using boost::hash_combine;

	// Start with a hash value of 0.
	std::size_t seed = 0;

	// Modify 'seed' by XORing and bit-shifting in
	// one member of 'Key' after the other:
	hash_combine(seed, hash_value(v.Position));
	hash_combine(seed, hash_value(v.Direction));
	hash_combine(seed, hash_value(v.Length));

	// Return the result.
	return seed;
}

inline std::size_t hash_value(const CELL &v)
{
	using boost::hash_combine;

	// Start with a hash value of 0.
	std::size_t seed = 0;

	// Modify 'seed' by XORing and bit-shifting in
	// one member of 'Key' after the other:
	hash_combine(seed, hash_value(v.Occupied));
	hash_combine(seed, hash_value(v.Visited));
	hash_combine(seed, hash_value(v.Diagonal));

	// Return the result.
	return seed;
}

class BATTLESHIP_STATE : public STATE
{
public:
    GRID<CELL> Cells;
    std::vector<SHIP> Ships;
    int NumRemaining;

    virtual size_t hash() const {
    	using boost::hash_combine;

    	// Start with a hash value of 0.
    	std::size_t seed = 0;

    	// Modify 'seed' by XORing and bit-shifting in
    	// one member of 'Key' after the other:
    	hash_combine(seed, hash_value(Cells));
    	hash_combine(seed, boost::hash_value(Ships));
    	hash_combine(seed, hash_value(NumRemaining));

    	// Return the result.
    	return seed;
    }
};

class BATTLESHIP : public SIMULATOR
{
public:

    BATTLESHIP(int xsize = 10, int ysize = 10, int maxlength = 4);

    virtual STATE* Copy(const STATE& state) const;
    virtual void Validate(const STATE& state) const;
    virtual STATE* CreateStartState() const;
    virtual void FreeState(STATE* state) const;
    virtual bool Step(STATE& state, int action, 
        int& observation, double& reward) const;
        
    void GenerateLegal(const STATE& state, /*const HISTORY& history,*/
        std::vector<int>& legal, const STATUS& status) const;
    virtual bool LocalMove(STATE& state, const HISTORY& history,
        int stepObs, const STATUS& status) const;

    virtual void DisplayBeliefs(const BELIEF_STATE& beliefState, 
        std::ostream& ostr) const;
    virtual void DisplayState(const STATE& state, std::ostream& ostr) const;
    virtual void DisplayObservation(const STATE& state, int observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;

private:

    bool Collision(const BATTLESHIP_STATE& bsstate, const SHIP& ship) const;
    void MarkShip(BATTLESHIP_STATE& bsstate, const SHIP& ship) const;
    void UnmarkShip(BATTLESHIP_STATE& bsstate, const SHIP& ship) const;
    void UpdateLegal(BATTLESHIP_STATE& bsstate) const;
    bool MoveShips(BATTLESHIP_STATE& bsstate) const;
    bool SwitchTwoShips(BATTLESHIP_STATE& bsstate) const;
    bool SwitchThreeShips(BATTLESHIP_STATE& bsstate) const;
    
    int XSize, YSize;
    int MaxLength, TotalRemaining;
   
    mutable MEMORY_POOL<BATTLESHIP_STATE> MemoryPool;
};

#endif // BATTLESHIP_H
