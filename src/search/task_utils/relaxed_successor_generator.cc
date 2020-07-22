//
// Created by yannick on 20.07.20.
//

#include "relaxed_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"
#include "task_properties.h"

#include "../option_parser.h"
#include "../plugin.h"


using namespace std;

namespace successor_generator{
    RelaxedSuccessorGenerator::RelaxedSuccessorGenerator() {

    }

    RelaxedSuccessorGenerator::~SuccessorGeneratorBase() = default;

    void RelaxedSuccessorGenerator::initialize() {

    }

    void RelaxedSuccessorGenerator::generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) const {

    }

    void RelaxedSuccessorGenerator::generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) const {

    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<RelaxedSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("relaxed", _parse);
}