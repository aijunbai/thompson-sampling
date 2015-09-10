#include <iostream>
#include <fstream>
#include <vector>

#include "race.h"
#include <dispatcher.h>

using namespace std;

void usage(ostream &os) {
    os << "usage: race [-a <n>] [-b <n>] [-e <f>] [-f] [-g <f>] [-h <n>] [-p <f>] [-s <n>] <file>"
       << endl << endl
       << "  -a <n>    Algorithm bitmask: 1=vi, 2=slrtdp, 4=ulrtdp, 8=blrtdp, 16=ilao, 32=plain-check, 64=elrtdp, 128=hdp-i, 256=hdp, 512=ldfs+, 1024=ldfs."
       << endl
       << "  -b <n>    Visits bound for blrtdp. Default: inf."
       << endl
       << "  -e <f>    Epsilon. Default: 0."
       << endl
       << "  -f        Formatted output."
       << endl
       << "  -g <f>    Parameter for epsilon-greedy. Default: 0."
       << endl
       << "  -h <n>    Heuristics: 0=zero, 1=minmin. Default: 0."
       << endl
#if 0
       << "  -k <n>    Kappa consistency level. Default: 0."
       << endl
       << "  -K <f>    Used to define kappa measures. Default: 2."
       << endl
#endif
       << "  -p <f>    Parameter p in [0,1]. Default: 1."
       << endl
       << "  -s <n>    Random seed. Default: 0."
       << endl
       << "  <file>    Racetrack file."
       << endl << endl;
}

int main(int argc, const char **argv) {
    FILE *is = 0;
    float p = 1.0;
    unsigned bitmap = 0;
    int h = 0;
    bool formatted = false;
    float dead_end_value = 1e3;
    float divisor = 1.0;

    string base_name;
    string policy_type;
    Online::Evaluation::parameters_t eval_pars;

    cout << fixed;
    Algorithm::parameters_t alg_pars;

    cout << "Arguments:";
    for( int i = 0; i < argc; ++i ) {
        cout << " " << argv[i];
    }
    cout << endl;

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
                alg_pars.rtdp.bound_ = strtol(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            case 'd':
                dead_end_value = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 'D':
                divisor = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 'e':
                alg_pars.epsilon_ = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 'f':
                formatted = true;
                ++argv;
                --argc;
                break;
            case 'g':
                alg_pars.rtdp.epsilon_greedy_ = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 'h':
                h = strtol(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            case 'l':
                eval_pars.labeling_ = true;
                ++argv;
                --argc;
                break;
            case 'p':
                p = strtod(argv[1], 0);
                argv += 2;
                argc -= 2;
                break;
            case 's':
                alg_pars.seed_ = strtoul(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            case 't':
                eval_pars.evaluation_trials_ = strtoul(argv[1], 0, 0);
                argv += 2;
                argc -= 2;
                break;
            default:
                usage(cout);
                exit(-1);
        }
    }

    if( argc >= 3 ) {
        std::cerr << argv[0] << std::endl;
        is = fopen(argv[0], "r");
        base_name = argv[1];
        policy_type = argv[2];
        if( argc >= 4 ) eval_pars.width_ = strtoul(argv[3], 0, 0);
        if( argc >= 5 ) eval_pars.depth_ = strtoul(argv[4], 0, 0);
        if( argc >= 6 ) eval_pars.par1_ = strtod(argv[5], 0);
        if( argc >= 7 ) eval_pars.par2_ = strtoul(argv[6], 0, 0);
    } else {
        usage(cout);
        exit(-1);
    }

    // build problem instances
    cout << "seed=" << alg_pars.seed_ << endl;
    Random::set_seed(alg_pars.seed_);
    grid_t grid;
    grid.parse(cout, is);
    problem_t problem(grid, p);
    fclose(is);

    // create heuristic
    vector<pair<const Heuristic::heuristic_t<state_t>*, string> > heuristics;
    heuristics.push_back(make_pair(new Heuristic::zero_heuristic_t<state_t>, "zero"));
    heuristics.push_back(make_pair(new Heuristic::min_min_heuristic_t<state_t>(problem, divisor), "min-min"));

    Heuristic::heuristic_t<state_t> *heuristic = 0;
    if( h == 0 ) {
        heuristic = new Heuristic::zero_heuristic_t<state_t>;
    } else if( h == 1 ) {
        heuristic = new Heuristic::min_min_heuristic_t<state_t>(problem, divisor);
    }

    // solve problem with algorithms
    vector<Dispatcher::result_t<state_t> > results;
    Dispatcher::solve(problem, heuristic, problem.init(), bitmap, alg_pars, results);

    // print results
    if( !results.empty() ) {
        if( formatted ) Dispatcher::print_result<state_t>(cout, 0);
        for( unsigned i = 0; i < results.size(); ++i ) {
            Dispatcher::print_result(cout, &results[i]);
        }
    }

    // evaluate policies
    vector<pair<const Online::Policy::policy_t<state_t>*, string> > base_policies;

    // fill base policies
    const Problem::hash_t<state_t> *hash = results.empty() ? 0 : results[0].hash_;
    if( hash != 0 ) {
        Online::Policy::hash_policy_t<state_t> optimal(*hash);
        base_policies.push_back(make_pair(optimal.clone(), "optimal"));
    }
    if( heuristic != 0 ) {
        Online::Policy::greedy_t<state_t> greedy(problem, *heuristic);
        base_policies.push_back(make_pair(greedy.clone(), "greedy"));
    }
    Online::Policy::random_t<state_t> random(problem);
    base_policies.push_back(make_pair(&random, "random"));

    // evaluate
    pair<const Online::Policy::policy_t<state_t>*, string> policy =
      Online::Evaluation::select_policy(problem, base_name, policy_type, base_policies, heuristics, eval_pars);
    if( policy.first != 0 ) {
        pair<pair<float, float>, float> eval = Online::Evaluation::evaluate_policy(*policy.first, eval_pars, false);
        cout << policy.second
             << "= " << setprecision(5) << eval.first.first
             << " " << eval.first.second << setprecision(2)
             << " ( " << eval.second << " secs "
             << policy.first->decisions() << " decisions"
            << ", " << 1000 * eval.second / double(policy.first->decisions()) << " ms/decision)"
            << endl;
        policy.first->print_stats(cout);
    } else {
        cout << "error: " << policy.second << endl;
    }

    // free resources
    delete policy.first;
    for( unsigned i = 0; i < results.size(); ++i ) {
        delete results[i].hash_;
    }
    delete heuristic;

    exit(0);
}

