#include <iostream>
#include <iomanip>
#include <strings.h>
#include <algorithm>
#include <vector>

#define DISCOUNT 0.95

typedef int state_t;

class problem_t : public Problem::problem_t<state_t> {
	int num_states_;
	int num_actions_;
	int branching_factor_;
	state_t init_state_;
	state_t terminal_state_;

	std::vector<std::pair<state_t,float> > **transition_;
	float **cost_;
  public:
    problem_t(int num_states, int num_actions, int branching_factor):
    	Problem::problem_t<state_t>(DISCOUNT),
    	num_states_(num_states),
    	num_actions_(num_actions),
    	branching_factor_(branching_factor),
    	init_state_(state_t(0)),
    	terminal_state_(state_t(num_states - 1))
    	{

    	std::cout << "mdp=(s=" << num_states_ << ", a=" << num_actions << ", b=" << branching_factor_ << ")" << std::endl;

    	transition_ = new std::vector<std::pair<state_t,float> >*[num_states_];
    	for (int i = 0; i < num_states_; ++i) {
    		transition_[i] = new std::vector<std::pair<state_t,float> >[num_actions_];
    		for (int j = 0; j < num_actions_; ++j) {
    			std::vector<float> tmp;
    			for (int k = 0; k < branching_factor_ - 1; ++k) {
    				tmp.push_back(Random::real());
    		 	}
    			tmp.push_back(0.0);
    			tmp.push_back(1.0);

    			std::sort(tmp.begin(), tmp.end());

    			for (int k = 0; k < branching_factor_; ++k) {
    				double prob = tmp[k+1] - tmp[k];

    				int s;
    				bool found;
    				do {
    					s = Random::uniform(num_states_);
    					found = false;

    					for (auto it = transition_[i][j].begin(); it != transition_[i][j].end(); ++it) {
    						if (it->first == s) {
    							found = true;
    							break;
    						}
    					}
    				} while (found);

    				transition_[i][j].push_back(std::make_pair(s, prob));
    			}
    		}
    	}

    	cost_ = new float*[num_states_];
    	for (int i = 0; i < num_states_; ++i) {
    		cost_[i] = new float[num_actions_];

    		for (int j = 0; j < num_actions_; ++j) {
    			cost_[i][j] = Random::uniform(0, 10);
    		}
    	}

    	for (int a = 0; a < num_actions_; ++a) {
    		transition_[terminal_state_][a].clear();
    		transition_[terminal_state_][a].push_back(std::make_pair(terminal_state_, 1.0));
    		cost_[terminal_state_][a] = 0;
    	}
    }
    virtual ~problem_t() {
    	for (int i = 0; i < num_states_; ++i) {
    		delete[] transition_[i];
    	}
    	delete transition_;

    	for (int i = 0; i < num_states_; ++i) {
    		delete[] cost_[i];
    	}
    	delete[] cost_;
    }

    virtual Problem::action_t number_actions(const state_t &s) const { return num_actions_; }
    virtual const state_t& init() const { return init_state_; }
    virtual bool terminal(const state_t &s) const { return s == terminal_state_; }
    virtual bool dead_end(const state_t &s) const { return false; }
    virtual bool applicable(const state_t &s, ::Problem::action_t a) const { return true; }
    virtual float cost(const state_t &s, Problem::action_t a) const {
    	return cost_[s][a];
    }
    virtual void next(const state_t &s, Problem::action_t a, std::vector<std::pair<state_t,float> > &outcomes) const {
        ++expansions_;

        outcomes = transition_[s][a];
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


