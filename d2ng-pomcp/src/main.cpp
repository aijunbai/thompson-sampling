#include "battleship.h"
#include "mcts.h"
#include "network.h"
#include "pocman.h"
#include "rocksample.h"
#include "rooms.h"
#include "tag.h"
#include "experiment.h"
#include "statistic.h"
#include "utils.h"
#include "distribution.h"
#include <boost/program_options.hpp>

using namespace std;
using namespace boost::program_options;

double NormalGammaInfo::ALPHA = 0.01;
double NormalGammaInfo::BETA = 10.0;

double BetaInfo::MIN = 0.0;
double BetaInfo::MAX = 1.0;

double BetaInfo::ALPHA = 0.5;
double BetaInfo::BETA = 0.5;

void UnitTests()
{
    cout << "Testing UTILS" << endl;
    UTILS::UnitTest();
    cout << "Testing COORD" << endl;
    COORD::UnitTest();
    cout << "Testing MCTS" << endl;
    MCTS::UnitTest();
}

void disableBufferedIO(void)
{
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    setbuf(stderr, NULL);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

int main(int argc, char* argv[])
{
    MCTS::PARAMS searchParams;
    EXPERIMENT::PARAMS expParams;
    SIMULATOR::KNOWLEDGE knowledge;
    string problem, outputfile;
    int size = 0, number = 0;
    bool seeding = false;

    options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("test", "run unit tests")
        ("problem", value<string>(&problem), "problem to run")
        ("outputfile", value<string>(&outputfile)->default_value("output.txt"), "summary output file")
        ("size", value<int>(&size), "size of problem (problem specific)")
        ("number", value<int>(&number), "number of elements in problem (problem specific)")
        ("timeout", value<double>(&expParams.TimeOut), "timeout (seconds)")
        ("mindoubles", value<int>(&expParams.MinDoubles), "minimum power of two simulations")
        ("maxdoubles", value<int>(&expParams.MaxDoubles), "maximum power of two simulations")
        ("runs", value<int>(&expParams.NumRuns), "number of runs")
        ("accuracy", value<double>(&expParams.Accuracy), "accuracy level used to determine horizon")
        ("horizon", value<int>(&expParams.UndiscountedHorizon), "horizon to use when not discounting")
        ("num steps", value<int>(&expParams.NumSteps), "number of steps to run when using average reward")
        ("verbose", value<int>(&searchParams.Verbose), "verbosity level")
        ("usetransforms", value<bool>(&searchParams.UseTransforms), "Use transforms")
        ("useparticlefilter", value<bool>(&searchParams.UseParticleFilter), "Use particle fileter")
        ("transformdoubles", value<int>(&expParams.TransformDoubles), "Relative power of two for transforms compared to simulations")
        ("transformattempts", value<int>(&expParams.TransformAttempts), "Number of attempts for each transform")
        ("ravediscount", value<double>(&searchParams.RaveDiscount), "RAVE discount factor")
        ("raveconstant", value<double>(&searchParams.RaveConstant), "RAVE bias constant")
        ("treeknowledge", value<int>(&knowledge.TreeLevel), "Knowledge level in tree (0=Pure, 1=Legal, 2=Smart)")
        ("rolloutknowledge", value<int>(&knowledge.RolloutLevel), "Knowledge level in rollouts (0=Pure, 1=Legal, 2=Smart)")
        ("smarttreecount", value<int>(&knowledge.SmartTreeCount), "Prior count for preferred actions during smart tree search")
        ("smarttreevalue", value<double>(&knowledge.SmartTreeValue), "Prior value for preferred actions during smart tree search")
        ("reusetree", value<bool>(&searchParams.ReuseTree), "Reuse tree generated during previous search")
        ("reusedepth", value<int>(&searchParams.ReuseDepth), "The maximal depth for tree reusing")
        ("seeding", value<bool>(&seeding), "Use pid as random seed")
        ("timeoutperaction", value<double>(&searchParams.TimeOutPerAction), "timeout per action (seconds)")
        ;

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help"))
    {
        cout << desc << "\n";
        return 1;
    }

    if (vm.count("test"))
    {
        cout << "Running unit tests" << endl;
        UnitTests();
        return 0;
    }

    if (vm.count("problem") == 0)
    {
        cout << "No problem specified" << endl;
        return 1;
    }

    if (seeding) {
      SimpleRNG::ins().RandomSeed(getpid());
    }

    SIMULATOR* real = 0; //真实环境
    SIMULATOR* simulator = 0; //模拟器

    if (problem == "battleship")
    {
        real = new BATTLESHIP(size, size, number);
        simulator = new BATTLESHIP(size, size, number);
    }
    else if (problem == "pocman")
    {
        switch (size) {
        case 0:
            real = new MICRO_POCMAN;
            simulator = new MICRO_POCMAN;
            break;
        case 1:
            real = new MINI_POCMAN;
            simulator = new MINI_POCMAN;
            break;
        case 2:
            real = new FULL_POCMAN;
            simulator = new FULL_POCMAN;
            break;
        default:
            cout << "PocMan size 0|1|2" << endl;
            return 1;
        }
    }
    else if (problem == "network")
    {
        real = new NETWORK(size, number);
        simulator = new NETWORK(size, number);
    }
    else if (problem == "rocksample")
    {
        real = new ROCKSAMPLE(size, number);
        simulator = new ROCKSAMPLE(size, number);
    }
    else if (problem == "fieldvisionrocksample")
    {
        real = new FieldVisionRockSample(size, number);
        simulator = new FieldVisionRockSample(size, number);
    }
    else if (problem == "tag")
    {
        real = new TAG(number);
        simulator = new TAG(number);
    }
    else if (problem == "rooms")
    {
        real = new ROOMS("rooms.map");
        simulator = new ROOMS("rooms.map", true);
    }
    else
    {
        cout << "Unknown problem" << endl;
        exit(1);
    }

    if (searchParams.TimeOutPerAction > 0.0) {
        expParams.MinDoubles = 0;
        expParams.MaxDoubles = 0;
        searchParams.NumSimulations = 0;
        searchParams.NumStartStates = pow(2, 10);
    }

    if (seeding) {
      SimpleRNG::ins().RandomSeed(getpid());
    }

    simulator->SetKnowledge(knowledge);
    EXPERIMENT experiment(*real, *simulator, outputfile, expParams, searchParams);
    experiment.DiscountedReturn();

    delete real;
    delete simulator;
    return 0;
}
