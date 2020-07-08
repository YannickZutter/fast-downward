//
// Created by yannick on 08.07.20.
//

#include "naive_successor_generator.h"
#include "successor_generator_factory.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"

#include "../abstract_task.h"
#include "../global_state.h"
#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace successor_generator {
    NaiveSuccessorGenerator::NaiveSuccessorGenerator()
    {
    }

    NaiveSuccessorGenerator::~NaiveSuccessorGenerator() = default;

    void NaiveSuccessorGenerator::initialize(const TaskProxy &task_proxy){
        root = successor_generator::SuccessorGeneratorFactory(task_proxy).create();
    }

    //TODO change to naive version instead of default onegi
    void NaiveSuccessorGenerator::generate_applicable_ops(
            const State &state, vector<OperatorID> &applicable_ops) const {
        root->generate_applicable_ops(state, applicable_ops);
    }

    void NaiveSuccessorGenerator::generate_applicable_ops(
            const GlobalState &state, vector<OperatorID> &applicable_ops) const {
        root->generate_applicable_ops(state, applicable_ops);
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<NaiveSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("naive", _parse);

}
