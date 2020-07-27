//
// Created by yannick on 08.07.20.
//

#include "naive_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"
#include "task_properties.h"

#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace successor_generator {
    NaiveSuccessorGenerator::NaiveSuccessorGenerator()
    {
    }

    NaiveSuccessorGenerator::~NaiveSuccessorGenerator() = default;

    void NaiveSuccessorGenerator::initialize(const TaskProxy &task_proxy){
        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }

    }

    void NaiveSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        for(OperatorProxy op : operators){
            if(task_properties::is_applicable(op, state)){
                applicable_ops.push_back(OperatorID(op.get_id()));
            }
        }
    }

    void NaiveSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        for(OperatorProxy op : operators){
            if(task_properties::is_applicable(op, state)){
                applicable_ops.push_back(OperatorID(op.get_id()));
            }
        }
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<NaiveSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("naive", _parse);

}
