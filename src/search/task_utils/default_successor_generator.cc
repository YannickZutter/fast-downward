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
    utils::Timer init_timer;
    root = SuccessorGeneratorFactory(task_proxy).create();
    init_timer.stop();
    utils::g_log << "time to initialize successor generator: " << init_timer() << endl;
}

void DefaultSuccessorGenerator::generate_applicable_ops(
    const State &state, vector<OperatorID> &applicable_ops) {
    utils::Timer gao_timer;
    root->generate_applicable_ops(state, applicable_ops);
    gao_timer.stop();
    total_duration += gao_timer();
    num_of_calls++;
}

void DefaultSuccessorGenerator::generate_applicable_ops(
    const GlobalState &state, vector<OperatorID> &applicable_ops) {
    utils::Timer gao_timer;
    root->generate_applicable_ops(state, applicable_ops);
    gao_timer.stop();
    total_duration += gao_timer();
    num_of_calls++;
}

    static shared_ptr<DefaultSuccessorGenerator> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<DefaultSuccessorGenerator>();
    }

    static Plugin<SuccessorGeneratorBase> _plugin("default", _parse);

}
