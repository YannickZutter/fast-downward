//
// Created by yannick on 08.07.20.
//

#include "psvn_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"
#include "task_properties.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../utils/logging.h"
#include "psvn_factory.h"

using namespace std;

namespace successor_generator {
    PSVNSuccessorGenerator::PSVNSuccessorGenerator()
    {
    }

    PSVNSuccessorGenerator::~PSVNSuccessorGenerator() = default;

    void PSVNSuccessorGenerator::initialize(const TaskProxy &task_proxy){
        utils::Timer init_timer;

        PSVNFactory::PSVNFactory factory(task_proxy);

        factory.create();

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }

        // only for testing purposes, can be deleted afterwards!!!
        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }

        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        for(OperatorProxy op : operators){
            if(task_properties::is_applicable(op, state)){
                applicable_ops.push_back(OperatorID(op.get_id()));
            }
        }

        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;


        for(OperatorProxy op : operators){
            if(task_properties::is_applicable(op, state)){
                applicable_ops.push_back(OperatorID(op.get_id()));
            }
        }

        gao_timer.stop();
        total_duration += gao_timer();
        num_of_calls++;
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<PSVNSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("psvn", _parse);

}
