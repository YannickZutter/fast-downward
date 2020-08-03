#include "default_successor_generator.h"

#include "successor_generator_factory.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"

#include "../abstract_task.h"
#include "../global_state.h"
#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace successor_generator {
DefaultSuccessorGenerator::DefaultSuccessorGenerator()
    {
}

DefaultSuccessorGenerator::~DefaultSuccessorGenerator() = default;

void DefaultSuccessorGenerator::initialize(const TaskProxy &task_proxy){
    root = SuccessorGeneratorFactory(task_proxy).create();
}

void DefaultSuccessorGenerator::generate_applicable_ops(
    const State &state, vector<OperatorID> &applicable_ops) {
    root->generate_applicable_ops(state, applicable_ops);
}

void DefaultSuccessorGenerator::generate_applicable_ops(
    const GlobalState &state, vector<OperatorID> &applicable_ops) {
    root->generate_applicable_ops(state, applicable_ops);
}

    static shared_ptr<DefaultSuccessorGenerator> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<DefaultSuccessorGenerator>();
    }

    static Plugin<SuccessorGeneratorBase> _plugin("default", _parse);

}
