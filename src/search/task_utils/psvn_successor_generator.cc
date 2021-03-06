//
// Created by yannick on 08.07.20.
//

#include "psvn_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../utils/logging.h"
#include "psvn_factory.h"

using namespace std;

bool comparer(OperatorID a, OperatorID b){
    return a.get_index() < b.get_index();
}

namespace successor_generator {
    PSVNSuccessorGenerator::PSVNSuccessorGenerator() = default;

    PSVNSuccessorGenerator::~PSVNSuccessorGenerator() = default;

    void PSVNSuccessorGenerator::initialize(const TaskProxy &task_proxy){
        utils::Timer init_timer;

        vertex_list = PSVNFactory::PSVNFactory(task_proxy).create();

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }

        cout << "\nvertex list size: " <<vertex_list.size();
        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        iterate_through_DAG(vertex_list[0], state, applicable_ops);

        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        iterate_through_DAG(vertex_list[0], state, applicable_ops);

        total_duration += gao_timer();
        num_of_calls++;
    }
    void PSVNSuccessorGenerator::iterate_through_DAG(const Vertex &v,const GlobalState &state, vector<OperatorID> &applicable_ops) {

        for(int i : v.satisfied_operators){
            applicable_ops.push_back(OperatorID(i));
        }
        if(!v.children.empty()){
            iterate_through_DAG(vertex_list[v.children[state[v.choice]]], state, applicable_ops);
        }
    }

    void PSVNSuccessorGenerator::iterate_through_DAG(const Vertex &v,const State &state, vector<OperatorID> &applicable_ops) {

        for(int i : v.satisfied_operators){
            applicable_ops.push_back(OperatorID(i));
        }
        if(!v.children.empty()){
            iterate_through_DAG(vertex_list[v.children[state[v.choice].get_value()]], state, applicable_ops);
        }
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<PSVNSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("psvn", _parse);
}
