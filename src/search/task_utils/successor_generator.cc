#include "successor_generator.h"

#include "successor_generator_factory.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"

#include "../abstract_task.h"
#include "../global_state.h"
#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace successor_generator {
SuccessorGenerator::SuccessorGenerator()
    {
}

SuccessorGenerator::~SuccessorGenerator() = default;

void SuccessorGenerator::initialize(const TaskProxy &task_proxy){
    root = SuccessorGeneratorFactory(task_proxy).create();
}

void SuccessorGenerator::generate_applicable_ops(
    const State &state, vector<OperatorID> &applicable_ops) const {
    root->generate_applicable_ops(state, applicable_ops);
}

void SuccessorGenerator::generate_applicable_ops(
    const GlobalState &state, vector<OperatorID> &applicable_ops) const {
    root->generate_applicable_ops(state, applicable_ops);
}

    static shared_ptr<SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<SuccessorGenerator>();
    }

    static Plugin<SuccessorGeneratorBase> _plugin("default_successor_generator", _parse);

}
