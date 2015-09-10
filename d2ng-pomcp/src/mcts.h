//D2NG-POMCP algorithm
//Created by Aijun Bai, aijunbai@gmail.com

#ifndef MCTS_H
#define MCTS_H

#include "simulator.h"
#include "node.h"
#include "statistic.h"

class MCTS
{
public:

    struct PARAMS
    {
        PARAMS();

        int Verbose;
        int MaxDepth;
        int NumSimulations;
        int NumStartStates;
        bool UseTransforms;
        bool UseParticleFilter;
        int NumTransforms;
        int MaxAttempts;
/*        int ExpandCount;*/
        double RaveDiscount;
        double RaveConstant;
        bool ReuseTree;
        int ReuseDepth;
        double TimeOutPerAction;
    };

    MCTS(const SIMULATOR& simulator, const PARAMS& params);
    ~MCTS();

    int SelectAction();
    bool Update(int action, int observation, double reward);

    void Search();
    void SearchImp();
    double Rollout(STATE& state);

    const BELIEF_STATE& BeliefState() const { return Root->Beliefs(); }
    const HISTORY& GetHistory() const { return History; }
    const SIMULATOR::STATUS& GetStatus() const { return Status; }
    void ClearStatistics();
    void DisplayStatistics(std::ostream& ostr) const;
    void DisplayValue(int depth, std::ostream& ostr) const;
    void DisplayPolicy(int depth, std::ostream& ostr) const;

    static void UnitTest();
    static void InitFastUCB(double exploration);

private:

    const SIMULATOR& Simulator;
    PARAMS Params;
    VNODE* Root;
    HISTORY History;
    SIMULATOR::STATUS Status; //标记树搜索的阶段和样本状态
    int TreeDepth, PeakTreeDepth;

    STATISTIC StatBeliefSize; //统计每次 UCTSearch 开始时信念大小
    STATISTIC StatNumSimulation; //统计 Any time 模式下每次 simulation 次数

    STATISTIC StatBeginTreeSize; //统计每次 UCTSearch 开始时树的大小
    STATISTIC StatEndTreeSize; //统计每次 UCTSearch 结束时树的大小
    STATISTIC StatIncTreeSize; //统计每次 UCTSearch 结束时树的大小变化

    STATISTIC StatTreeDepth;
    STATISTIC StatRolloutDepth;
    STATISTIC StatTotalReward;

    int ThompsonSampling(VNODE* vnode, bool sampling) const;
    int SelectRandom() const;
    std::vector<double> SimulateV(STATE& state, VNODE* vnode);
    std::vector<double> SimulateQ(STATE& state, QNODE& qnode, int action);
    VNODE* ExpandNode(const STATE* state);
    void AddSample(VNODE* node, const STATE& state);
    void ParticleFilter(BELIEF_STATE& beliefs);
    void AddTransforms(BELIEF_STATE& beliefs);
    STATE* CreateTransform() const;
    void Resample(BELIEF_STATE& beliefs);

    double QValue(QNODE& qnode, bool sampling) const; //XXX state?
    double HValue(VNODE* vnode, bool sampling) const;

    static void UnitTestGreedy();
    static void UnitTestUCB();
    static void UnitTestRollout();
    static void UnitTestSearch(int depth);

    static const int Rollouts = MULTI_ROLLOUTS;
};

#endif // MCTS_H
