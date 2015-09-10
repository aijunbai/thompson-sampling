#include <iostream>
#include <iomanip>
#include <strings.h>
#include <algorithm>
#include <vector>

#include "model.h"

#define DISCOUNT 1.0

typedef State state_t;

class problem_t : public Problem::problem_t<state_t> {
  mutable TaxiEnv env_;

  public:
    problem_t(int size):
    	Problem::problem_t<state_t>(DISCOUNT)
    {
      TaxiEnv::SIZE = size;
      TaxiEnv::model = new TaxiEnv::EnvModel();
    }
    virtual ~problem_t() {
    }

    virtual Problem::action_t number_actions(const state_t &s) const {
      return Action::ActionSize;
    }

    virtual const state_t& init() const {
      static state_t init_state = env_.get_init_state();

      return init_state;
    }

    virtual const state_t& random_init() const {
      static state_t init_state;
      init_state = env_.get_init_state();

      return init_state;
    }

    virtual bool terminal(const state_t &s) const {
      return s.absorbing();
    }

    virtual bool dead_end(const state_t &s) const { return false; }

    virtual bool applicable(const state_t &s, ::Problem::action_t a) const { return true; }

    virtual float cost(const state_t &s, Problem::action_t a) const {
      return -env_.Reward(s, Action(a));
    }

    virtual void next(const state_t &s, Problem::action_t a, std::vector<std::pair<state_t,float> > &outcomes) const {
        ++expansions_;

        outcomes = env_.Transition(s, Action(a));
    }

    virtual void print(std::ostream &os) const { }
};

inline std::ostream& operator<<(std::ostream &os, const problem_t &p) {
    p.print(os);
    return os;
}

class scaled_heuristic_t : public Heuristic::heuristic_t<state_t> {
    const Heuristic::heuristic_t<state_t> *h_;
    float multiplier_;
  public:
    scaled_heuristic_t(const Heuristic::heuristic_t<state_t> *h, float multiplier = 1.0)
      : h_(h), multiplier_(multiplier) { }
    virtual ~scaled_heuristic_t() { }
    virtual float value(const state_t &s) const {
        return h_->value(s) * multiplier_;
    }
    virtual void reset_stats() const { }
    virtual float setup_time() const { return 0; }
    virtual float eval_time() const { return 0; }
    virtual size_t size() const { return 0; }
    virtual void dump(std::ostream &os) const { }
    float operator()(const state_t &s) const { return value(s); }
};

class zero_heuristic_t: public Heuristic::heuristic_t<state_t> {
  public:
    zero_heuristic_t() { }
    virtual ~zero_heuristic_t() { }
    virtual float value(const state_t &s) const { return 0; }
    virtual void reset_stats() const { }
    virtual float setup_time() const { return 0; }
    virtual float eval_time() const { return 0; }
    virtual size_t size() const { return 0; }
    virtual void dump(std::ostream &os) const { }
    float operator()(const state_t &s) const { return value(s); }
};


