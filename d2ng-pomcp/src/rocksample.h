#ifndef ROCKSAMPLE_H
#define ROCKSAMPLE_H

#include "simulator.h"
#include "coord.h"
#include "grid.h"

struct RS_ENTRY
{
	bool Valuable;
	bool Collected;
	int Count;    				// Smart knowledge
	int Measured; 				// Smart knowledge
	double LikelihoodValuable;	// Smart knowledge
	double LikelihoodWorthless;	// Smart knowledge
	double ProbValuable;		// Smart knowledge
};

inline std::size_t hash_value(const RS_ENTRY &v)
{
	using boost::hash_combine;

	// Start with a hash value of 0.
	std::size_t seed = 0;

	// Modify 'seed' by XORing and bit-shifting in
	// one member of 'Key' after the other:
	hash_combine(seed, boost::hash_value(v.Valuable));
	hash_combine(seed, boost::hash_value(v.Collected));

	// Return the result.
	return seed;
}

class ROCKSAMPLE_STATE : public STATE
{
public:

    COORD AgentPos; //机器人位置
    std::vector<RS_ENTRY> Rocks; //每个石头的状态

    virtual size_t hash() const {
    	using boost::hash_combine;

    	// Start with a hash value of 0    .
    	std::size_t seed = 0;

    	// Modify 'seed' by XORing and bit-shifting in
    	// one member of 'Key' after the other:
    	hash_combine(seed, hash_value(AgentPos));
    	hash_combine(seed, boost::hash_value(Rocks));

    	// Return the result.
    	return seed;
    }

//    int Target; // Smart knowledge
};

class ROCKSAMPLE : public SIMULATOR
{
public:

    ROCKSAMPLE(int size, int rocks);

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

    enum
    {
        E_NONE,
        E_GOOD,
        E_BAD
    };

    enum
    {
        E_SAMPLE = 4
    };

    void InitGeneral();
    void Init_7_8();
    void Init_11_11();

    template<typename T>
    void FillField(T rocks) {
        Grid.SetAllValues(-1);

        for (int i = 0; i < NumRocks; ++i)
        {
            Grid(rocks[i]) = i;
            RockPos.push_back(rocks[i]);
        }
    }

    int GetObservation(const ROCKSAMPLE_STATE& rockstate, int rock) const;

    GRID<int> Grid;
    std::vector<COORD> RockPos;
    int Size, NumRocks;
    COORD StartPos;
    double HalfEfficiencyDistance; //控制观察函数的阈值
    double SmartMoveProb;

private:

    mutable MEMORY_POOL<ROCKSAMPLE_STATE> MemoryPool;
};

class FieldVisionRockSample: public ROCKSAMPLE {
public:
	FieldVisionRockSample(int size, int rocks);

    virtual bool Step(STATE& state, int action,
        int& observation, double& reward) const;

    virtual void GenerateLegal(const STATE& state, /*const HISTORY& history,*/
        std::vector<int>& legal, const STATUS& status) const;
    virtual void GeneratePreferred(const STATE& state, const HISTORY& history,
        std::vector<int>& legal, const STATUS& status) const;
    virtual bool LocalMove(STATE& state, const HISTORY& history,
        int stepObservation, const STATUS& status) const;

    virtual void DisplayObservation(const STATE& state, int observation, std::ostream& ostr) const;
};

#endif
