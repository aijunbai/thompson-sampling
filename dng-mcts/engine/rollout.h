/*
 *  Copyright (C) 2011 Universidad Simon Bolivar
 * 
 *  Permission is hereby granted to distribute this software for
 *  non-commercial research purposes, provided that this copyright
 *  notice is included with any such distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 *  EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
 *  SOFTWARE IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU
 *  ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 *  
 *  Blai Bonet, bonet@ldc.usb.ve
 *
 */

#ifndef ROLLOUT_H
#define ROLLOUT_H

#include "policy.h"
#include "dot_graph.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <limits>
#include <vector>

//#define DEBUG
//#define DUMP_TREE


namespace Online {

namespace Policy {

namespace Rollout {

#ifdef DUMP_TREE
dot::Graph search_tree; //global
int depth = 0;
#endif

// nested rollout policy
template<typename T> class rollout_t : public improvement_t<T> {
  using policy_t<T>::problem;

  protected:
    unsigned width_;
    unsigned depth_;
    unsigned nesting_;

  public:
    rollout_t(const policy_t<T> &base_policy,
              unsigned width,
              unsigned depth,
              unsigned nesting)
      : improvement_t<T>(base_policy),
        width_(width), depth_(depth), nesting_(nesting) {
        std::stringstream name_stream;
        name_stream << "rollout("
                    << "width=" << width_
                    << ",depth=" << depth_
                    << ",nesting=" << nesting_
                    << ")";
        policy_t<T>::set_name(name_stream.str());
    }
    virtual ~rollout_t() { }

    virtual const policy_t<T>* clone() const {
        return new rollout_t(improvement_t<T>::base_policy_, width_, depth_, nesting_);
    }

    virtual Problem::action_t operator()(const T &s) const {
        ++policy_t<T>::decisions_;
        Problem::action_t best_action = Problem::noop;
        float best_value = std::numeric_limits<float>::max();

        for( Problem::action_t a = 0; a < problem().number_actions(s); ++a ) { //遍历每一个动作
            if( problem().applicable(s, a) ) {
                float value = 0;
                for( unsigned trial = 0; trial < width_; ++trial ) {
                    std::pair<T, bool> p = problem().sample(s, a);

#ifdef DUMP_TREE
                    std::stringstream state_node;
                    state_node << s << " @ " << depth;

                    std::stringstream action_node;
                    action_node << a << " | " << s << " @ " << depth;

                    std::stringstream new_state_node;
                    new_state_node << p.first << " @ " << depth + 1;

                    search_tree.addEdge(state_node.str(), action_node.str(), "red");
                    search_tree.addEdge(action_node.str(), new_state_node.str(), "green");

                    depth += 1;
#endif

                    value += problem().cost(s, a) + problem().discount() * evaluate(p.first);

#ifdef DUMP_TREE
                    depth -= 1;
#endif
                }
                value /= width_;
                if( value < best_value ) {
                    best_value = value;
                    best_action = a;
                }
            }
        }

#ifdef DUMP_TREE
        if (depth == 0) {
        	std::stringstream ss;
        	ss << s;
        	dump_search_tree(ss.str());
        }
#endif

        assert(best_action != Problem::noop);
        return best_action;
    }

    virtual void print_stats(std::ostream &os) const {
        os << "stats: policy=" << policy_t<T>::name() << std::endl;
        os << "stats: decisions=" << policy_t<T>::decisions_ << std::endl;
        improvement_t<T>::base_policy_.print_stats(os);
    }

#ifdef DUMP_TREE
    void dump_search_tree(const std::string &str) const {
    	std::stringstream ss;

    	ss << "search_" << str << ".dot";
        std::ofstream fout(ss.str().c_str());

        if (!fout.good()) {
            perror("fopen");
            exit(1);
        }

        fout << search_tree;
        fout.close();
    }
#endif

    float evaluate(const T &s) const {
        return Evaluation::evaluation(improvement_t<T>::base_policy_, s, 1, depth_); //使用基础策略进行 rollout
    }
};

}; // namespace Rollout

template<typename T>
inline const policy_t<T>* make_nested_rollout(const policy_t<T> &base_policy,
                                              unsigned width,
                                              unsigned depth,
                                              unsigned nesting = 1) {
    std::vector<const policy_t<T>*> nested_policies;

    nested_policies.reserve(1 + nesting);
    nested_policies.push_back(&base_policy);

    for( unsigned level = 0; level < nesting; ++level ) {
        const policy_t<T> *policy = nested_policies.back();

        policy_t<T> *rollout = new Rollout::rollout_t<T>(*policy, width, depth, 1+level); //使用下一层次的 rollout 策略做为本层次的基础策略

        nested_policies.push_back(rollout);
    }

    return nested_policies.back();
}

}; // namespace Policy

}; // namespace Online

#undef DEBUG

#endif

