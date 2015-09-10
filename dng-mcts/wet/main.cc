#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "wet.h"

using namespace std;

void usage(ostream &os) {
    os << "usage: wet [-a <n>] [-b <n>] [-e <f>] [-g <f>] [-h <n>] [-p <f>] [-s <n>] [-X] [-Y|-Z] <size>"
       << endl;
}

int main(int argc, const char **argv) {
    size_t size = 0;

    float p = 0.0;
    unsigned bitmap = 0;
    int h = 0;
    bool formatted = false;

    string base_name;
    string policy_type;
    Online::Evaluation::parameters_t par;

    cout << fixed;
    Algorithm::parameters_t parameters;

    // parse arguments
    ++argv;
    --argc;
    while( argc > 1 ) {
        if( **argv != '-' ) break;
        switch( (*argv)[1] ) {
            case 'a':
                bitmap = strtoul(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            case 'b':
                parameters.rtdp.bound_ = strtol(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            case 'e':
                parameters.epsilon_ = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 'f':
                formatted = true;
                ++argv;
                --argc;
                break;
            case 'g':
                parameters.rtdp.epsilon_greedy_ = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 'h':
                h = strtol(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            case 'p':
                p = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 's':
                parameters.seed_ = strtoul(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            case 'X':
            case 'Y':
            case 'Z':
                version += 1 << (int)((*argv)[1] - 'X');
                ++argv;
                --argc;
                if( ZVER && YVER ) {
                    cout << "Y and Z cannot be used simultaneously." << endl;
                    exit(-1);
                }
                break;
            default:
                usage(cout);
                exit(-1);
        }
    }

    if( argc >= 3 ) {
        size = strtoul(argv[0], 0, 0);
        base_name = argv[1];
        policy_type = argv[2];
        if( argc >= 4 ) par.width_ = strtoul(argv[3], 0, 0);
        if( argc >= 5 ) par.depth_ = strtoul(argv[4], 0, 0);
        if( argc >= 6 ) par.par1_ = strtod(argv[5], 0);
        if( argc >= 7 ) par.par2_ = strtoul(argv[6], 0, 0);
    } else {
        usage(cout);
        exit(-1);
    }

    // build problem instances
    cout << "seed=" << parameters.seed_ << endl;
    Random::set_seed(parameters.seed_);
    state_t init(Random::uniform(size), Random::uniform(size));
    state_t goal(Random::uniform(size), Random::uniform(size));
    problem_t problem(size, p, init, goal);
    //problem.print(cout);

    // create heuristic
    vector<pair<const Heuristic::heuristic_t<state_t>*, string> > heuristics;
    Heuristic::heuristic_t<state_t> *heuristic = 0;
    if( h == 1 ) {
        heuristic = new Heuristic::min_min_heuristic_t<state_t>(problem);
    } else if( h == 2 ) {
        //heuristic = new Heuristic::hdp_heuristic_t<state_t>(problem, eps, 0);
    }

    // solve problem with algorithms
    vector<Dispatcher::result_t<state_t> > results;
    Dispatcher::solve(problem, heuristic, problem.init(), bitmap, parameters, results);

    // print results
    if( !results.empty() ) {
        if( formatted ) Dispatcher::print_result<state_t>(cout, 0);
        for( unsigned i = 0; i < results.size(); ++i ) {
            Dispatcher::print_result(cout, &results[i]);
        }
    }

    // evaluate policies
    vector<pair<const Online::Policy::policy_t<state_t>*, string> > bases;

    // fill base policies
    const Problem::hash_t<state_t> *hash = results.empty() ? 0 : results[0].hash_;
    Online::Policy::hash_policy_t<state_t> optimal(*hash);
    bases.push_back(make_pair(&optimal, "optimal"));
    Online::Policy::greedy_t<state_t> greedy(problem, *heuristic);
    bases.push_back(make_pair(&greedy, "greedy"));
    Online::Policy::random_t<state_t> random(problem);
    bases.push_back(make_pair(&random, "random"));

    // evaluate
    pair<const Online::Policy::policy_t<state_t>*, std::string> policy =
      Online::Evaluation::select_policy(problem, base_name, policy_type, bases, heuristics, par);
    cout << policy.second << "= " << flush;
    pair<pair<float, float>, float> eval = Online::Evaluation::evaluate_policy(*policy.first, par);
    cout << setprecision(5) << eval.first.first << " " << eval.first.second << setprecision(2) << " ( " << eval.second << " secs)" << endl;

    // free resources
    delete policy.first;
    for( unsigned i = 0; i < results.size(); ++i ) {
        delete results[i].hash_;
    }
    delete heuristic;

    exit(0);
}

