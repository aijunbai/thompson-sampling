//DNG-MCTS algorithm
//Created by Aijun Bai, aijunbai@gmail.com

#ifndef DNG_H
#define DNG_H

#include "policy.h"
#include "statistic.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <limits>
#include <vector>
#include <math.h>
#include <set>
#include <algorithm>

#define DISPLAY_VALUES 0
#define DIRICHLET_INIT 0
#define REUSE_TREE 0

namespace Online {

namespace Policy {

namespace DNG {

////////////////////////////////////////////////
//
// Tree
//

//template<typename T> struct node_t {
//    mutable std::vector<unsigned> counts_;
//    mutable std::vector<float> values_;
//    node_t(int num_actions)
//      : counts_(1+num_actions, 0),
//        values_(1+num_actions, 0) {
//    }
//    ~node_t() { }
//};

////////////////////////////////////////////////
//
// Hash Table
//

template<typename T> struct map_functions_t {
  size_t operator()(const std::pair<unsigned, T> &p) const {
    return p.second.hash();
  }
};

template<> struct map_functions_t <int> {
  size_t operator()(const std::pair<unsigned, int> &p) const {
    return p.second;
  }
};

template <class T>
struct data_t {
  mutable NormalGammaInfo cumulative_reward_;
  mutable std::vector<int> counts_;

  data_t(int num_actions): counts_(num_actions, 0)  {
  }

  ~data_t() { }
};

template<typename T> class hash_t :
    public Hash::generic_hash_map_t<std::pair<unsigned, T>, //使用深度和状态共同索引
    data_t<T>,
    map_functions_t<T> > {
    public:
  typedef typename Hash::generic_hash_map_t<std::pair<unsigned, T>,
      data_t<T>,
      map_functions_t<T> >
  base_type;
  typedef typename base_type::const_iterator const_iterator;
  const_iterator begin() const { return base_type::begin(); }
  const_iterator end() const { return base_type::end(); }

  typedef typename base_type::iterator iterator;
  iterator begin() { return base_type::begin(); }
  iterator end() { return base_type::end(); }

    public:
  hash_t() { }
  virtual ~hash_t() { }
  void print(std::ostream &os) const {
    for( const_iterator it = begin(); it != end(); ++it ) {
      os << "(" << it->first.first << "," << it->first.second << ")" << std::endl;
    }
  }

  void inc() {
    hash_t<T> backup;

    for( iterator it = begin(); it != end(); ++it ) {
      if (it->first.first > 0) {
        backup.insert(std::make_pair(std::make_pair(it->first.first - 1, it->first.second), it->second));
      }
    }

    base_type::swap(backup);
  }
};

////////////////////////////////////////////////
//
// Policy
//

template<typename T> class dng_t : public improvement_t<T> {
public:
  using policy_t<T>::problem;

  const Heuristic::heuristic_t<T> &heuristic_;
  const unsigned width_;
  const unsigned horizon_;
  const unsigned rollouts_;
  const unsigned hvalue_option_;
  const bool random_ties_;
  mutable hash_t<T> table_;
  mutable Hash::generic_hash_map_t<T, std::vector<DirichletInfo<T, Hash::generic_hash_map_t<T, double> > > > transition_;

public:
  dng_t(const policy_t<T> &base_policy,
        const Heuristic::heuristic_t<T> &heuristic,
        unsigned width,
        unsigned horizon,
        unsigned rollouts,
        unsigned hoption,
        bool random_ties)
: improvement_t<T>(base_policy),
  heuristic_(heuristic),
  width_(width),
  horizon_(horizon),
  rollouts_(rollouts > 1? rollouts: 1),
  hvalue_option_(hoption),
  random_ties_(random_ties) {
    std::stringstream name_stream;
    name_stream << "dng("
                        << "heuristic=" << heuristic_.name()
                        << ",width=" << width_
                        << ",horizon=" << horizon_
                        << ",rollouts=" << rollouts_
                        << ",hvalue_option=" << hvalue_option_
                        << ",random-ties=" << (random_ties_ ? "true" : "false")
                        << ")";
    policy_t<T>::set_name(name_stream.str());
  }
  virtual ~dng_t() { }

  virtual Problem::action_t operator()(const T &s) const { //action selection
    ++policy_t<T>::decisions_;
    //        int h = Hash::hash_function_t<T>()(s);
    //        std::cout << "s.hash=" << h << std::endl;

#if REUSE_TREE
    table_.inc();
#else
    table_.clear();
    transition_.clear(); //转移矩阵应该保存比较好
#endif
    for( unsigned i = 0; i < width_; ++i ) {
      search(s, 0, rollouts_);
    }

    typename hash_t<T>::iterator it = table_.find(std::make_pair(0, s));
    assert(it != table_.end());
    Problem::action_t action = greedy_selection(s, it->second, 0, random_ties_).first;
    assert(problem().applicable(s, action));

#if DISPLAY_VALUES
    const int nactions = problem().number_actions(s);

    it->second.cumulative_reward_.Print("state", std::cout);

    for( Problem::action_t a = 0; a < nactions; ++a ) {
      if( problem().applicable(s, a) ) {
        std::cout << "#action " << a
            << " n=" << it->second.counts_[a]
                                           << " q=" << QValue(s, it->second, a, 0, false);
        transition_[s][a].Print("", std::cout);
      }
    }

    std::cout << "#besta=" << action << std::endl;
    std::cout << std::endl;
#endif

    return action;
  }

  virtual const policy_t<T>* clone() const {
    return new dng_t(improvement_t<T>::base_policy_, heuristic_, width_, horizon_, rollouts_, hvalue_option_, random_ties_);
  }

  virtual void print_stats(std::ostream &os) const {
    os << "stats: policy=" << policy_t<T>::name() << std::endl;
    os << "stats: decisions=" << policy_t<T>::decisions_ << std::endl;
    improvement_t<T>::base_policy_.print_stats(os);
  }

  float value(const T &s, Problem::action_t a) const {
    typename hash_t<T>::const_iterator it = table_.find(std::make_pair(0, s));
    assert(it != table_.end());
    return it->second.values_[1+a];
  }
  unsigned count(const T &s, Problem::action_t a) const {
    typename hash_t<T>::const_iterator it = table_.find(std::make_pair(0, s));
    assert(it != table_.end());
    return it->second.counts_[a];
  }
  size_t size() const { return table_.size(); }
  void print_table(std::ostream &os) const {
    table_.print(os);
  }

  std::vector<double> search(const T &s, unsigned depth, int rollouts) const { //DNG-MCTS
    if( (depth == horizon_) || problem().terminal(s) ) { //搜索超过深度或结束状态
      return std::vector<double>(rollouts, 0.0);
    }

    if( problem().dead_end(s) ) { //吸收状态
      return std::vector<double>(rollouts, problem().dead_end_value());
    }

    typename hash_t<T>::iterator it = table_.find(std::make_pair(depth, s));

    if( it == table_.end() ) { //新节点
      table_.insert(std::make_pair(std::make_pair(depth, s), data_t<T>( problem().number_actions(s)))); //插入

      std::vector<double> values;

      for (int i = 0; i < rollouts; ++i) {
        float value = evaluate(s, depth); //使用基本策略进行 rollout
        values.push_back(value);
      }

      it = table_.find(std::make_pair(depth, s));
      assert(it != table_.end());
      it->second.cumulative_reward_.Add(values);

      if (transition_[s].empty()) {
        transition_[s].resize(problem().number_actions(s));

#if DIRICHLET_INIT
        for (int a = 0; a < problem().number_actions(s); ++a) {
          if (problem().applicable(s, a)) {
            std::vector<std::pair<T, float> > outcomes;
            problem().next(s, a, outcomes);

            for (auto it = outcomes.begin(); it != outcomes.end(); ++it) {
              transition_[s][a].Initial(it->first); //对转移矩阵进行初始化，初始化为 1, 1, 1...
            }
          }
        }
#endif
      }

      return values; //交给上层处理
    }
    else { //当前深度下该状态已经存在于树上节点
      Problem::action_t a = thompson_sampling(s, it->second, depth); // select action for this node and increase counts

      // sample next state
      std::pair<const T, bool> p = problem().sample(s, a);
      float cost = problem().cost(s, a);

      // do recursion and update value
      std::vector<double> values = search(p.first, 1 + depth, rollouts); //递归调用
      assert(int(values.size()) == rollouts);

      for (uint i = 0; i < values.size(); ++i) {
        values[i] = cost + problem().discount() * values[i];
      }

      transition_[s][a].Add(p.first);
      it->second.cumulative_reward_.Add(values);
      it->second.counts_[a] += 1;

      return values;
    }
  }

  Problem::action_t thompson_sampling(const T &state,
                                      const data_t<T> &data, int depth ) const {
    int nactions = problem().number_actions(state);
    std::vector<Problem::action_t> unexplored_actions;

    for( Problem::action_t a = 0; a < nactions; ++a ) {
      if( problem().applicable(state, a) ) {
        if( data.counts_[a] < 0.1 ) { // if this action has never been taken in this node, select it
          unexplored_actions.push_back(a); //乐观估计
        }
      }
    }

    if (!unexplored_actions.empty()) {
      return unexplored_actions[Random::uniform(unexplored_actions.size())];
    }

    Problem::action_t best_action = -1;
    float best_value = std::numeric_limits<float>::max();

    for( Problem::action_t a = 0; a < nactions; ++a ) {
      if( problem().applicable(state, a) ) {
        float q = QValue(state, a, depth, true);

        if (q < best_value) { //XXX
          best_value = q;
          best_action = a;
        }
      }
    }

    assert(best_action != -1);
    return best_action;
  }

  std::pair<Problem::action_t, float> greedy_selection(const T &state,
                                                       const data_t<T> &data, //状态节点存储的数据，需要区分不同深度
                                                       int depth,
                                                       bool random_ties) const {
    const int nactions = problem().number_actions(state);

    std::vector<Problem::action_t> best_actions;
    float best_value = std::numeric_limits<float>::max();

    best_actions.reserve(random_ties ? nactions : 1);
    for( Problem::action_t a = 0; a < nactions; ++a ) {
      if( problem().applicable(state, a) ) {
        if( data.counts_[a] < 0.1 ) { // if this action has never been taken in this node, select it
          return std::make_pair(a, 0); //乐观估计
        }

        float value = QValue(state, a, depth, false); //depth=0

        // update best action so far
        if( value <= best_value ) {
          if( value < best_value ) {
            best_value = value;
            best_actions.clear();
          }
          if( random_ties || best_actions.empty() )
            best_actions.push_back(a);
        }
      }
    }

    assert(!best_actions.empty());

    return std::make_pair(best_actions[Random::uniform(best_actions.size())], best_value);
  }

  float HValue(const T &s, unsigned depth, bool sampling) const {
    float hvalue = 0;
    auto it = table_.find(std::make_pair(depth, s)); //找到树上的节点

    if (it == table_.end()) { //未访问过的节点
      if( (depth == horizon_) || problem().terminal(s) ) { //搜索超过深度或结束状态
        hvalue = 0.0;
      }
      else if( problem().dead_end(s) ) {
        hvalue = problem().dead_end_value();
      }
      else {
        if (hvalue_option_ == 0) {
          static const NormalGammaInfo ng;

          hvalue = ng.ThompsonSampling(sampling); //随机值
        }
        else if (hvalue_option_ == 1) {
          hvalue = heuristic_.value(s); //启发值
        }
        else if (hvalue_option_ == 2) {
          search(s, depth, rollouts_);

          it = table_.find(std::make_pair(depth, s)); //找到树上的节点
          assert(it != table_.end());
          hvalue = it->second.cumulative_reward_.ThompsonSampling(sampling); //扩展一次以后的随机值
        }
        else {
          assert(0);
        }
      }
    }
    else {
      hvalue = it->second.cumulative_reward_.ThompsonSampling(sampling);
    }

    return hvalue;
  }

  float QValue(const T &s, Problem::action_t a, unsigned depth, bool sampling) const { //cost(s, a) + \gamma \sum_{s'} V(s')
    float qvalue = 0;

    const auto &outcomes = transition_[s][a].ThompsonSampling(sampling);
    for (auto it = outcomes.begin(); it != outcomes.end(); ++it) {
      qvalue += it->second * HValue(it->first, depth + 1, sampling);
    }

    qvalue *= problem().discount();
    qvalue += problem().cost(s, a);

    return qvalue;
  }

  float evaluate(const T &s, unsigned depth) const {
    return Evaluation::evaluation(improvement_t<T>::base_policy_, s, 1, horizon_ - depth); //使用基本策略进行 rollout
  }
};

}; // namespace UCT

template<typename T>
inline const policy_t<T>* make_dng(const policy_t<T> &base_policy,
                                   const Heuristic::heuristic_t<T> &heuristic,
                                   unsigned width,
                                   unsigned horizon,
                                   unsigned rollouts,
                                   unsigned option,
                                   bool random_ties) {
  return new DNG::dng_t<T>(base_policy, heuristic, width, horizon, rollouts, option, random_ties);
}

}; // namespace Policy

}; // namespace Online

#endif

