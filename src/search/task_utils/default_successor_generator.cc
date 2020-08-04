#include "default_successor_generator.h"

#include "successor_generator_factory.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../utils/logging.h"
#include "../utils/timer.h"

using namespace std;

namespace successor_generator {
DefaultSuccessorGenerator::DefaultSuccessorGenerator()
    {
}

DefaultSuccessorGenerator::~DefaultSuccessorGenerator() = default;

void DefaultSuccessorGenerator::initialize(const TaskProxy &task_proxy){
    utils::Timer timer;
    root = SuccessorGeneratorFactory(task_proxy).create();
    double time = timer();
    utils::g_log << "time to initialize successor generator: " << time << endl;
}

void DefaultSuccessorGenerator::generate_applicable_ops(
    const State &state, vector<OperatorID> &applicable_ops) {
    utils::Timer timer;
    root->generate_applicable_ops(state, applicable_ops);
    double time = timer();
    total_duration += time;
    num_of_calls++;
}

void DefaultSuccessorGenerator::generate_applicable_ops(
    const GlobalState &state, vector<OperatorID> &applicable_ops) {
    utils::Timer timer;
    root->generate_applicable_ops(state, applicable_ops);
    double time = timer();
    total_duration += time;
    num_of_calls++;
}

    static shared_ptr<DefaultSuccessorGenerator> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<DefaultSuccessorGenerator>();
    }

    static Plugin<SuccessorGeneratorBase> _plugin("default", _parse);

}
