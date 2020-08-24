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

        vertex_list = PSVNFactory::PSVNFactory(task_proxy).create();

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
            op_ids.push_back(-1);
        }

        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        //iterate_through_DAG(vertex_list[0], state, applicable_ops);

        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        generate_applicable_ops(state.unpack(), applicable_ops);
    }

    void PSVNSuccessorGenerator::iterate_through_DAG(Vertex v, State state, vector<OperatorID> &applicable_ops) {

        /**
        for(OperatorProxy op : v.satisfied_rules){
            if(op_ids[op.get_id()] == -1){

                applicable_ops.push_back(OperatorID(op.get_id()));
                op_ids[op.get_id()] = op.get_id();
            }
        }
        if(!v.rules.empty()) {
            if (state[v.choice].get_value() == 0) {
                for (int child_pos : v.children) {
                    iterate_through_DAG(vertex_list[child_pos], state, applicable_ops);
                }
            } else {
                int a = state[v.choice].get_value();
                iterate_through_DAG(vertex_list[v.children[a]], state, applicable_ops);
            }
        }
         **/

    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<PSVNSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("psvn", _parse);

}
