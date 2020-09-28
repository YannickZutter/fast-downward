//
// Created by yannick on 08.07.20.
//

#include "psvn_split_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../utils/logging.h"
#include "psvn_split_factory.h"

using namespace std;

namespace successor_generator {
    PSVNSplitSuccessorGenerator::PSVNSplitSuccessorGenerator() = default;

    PSVNSplitSuccessorGenerator::~PSVNSplitSuccessorGenerator() = default;

    void PSVNSplitSuccessorGenerator::initialize(const TaskProxy &task_proxy){
        utils::Timer init_timer;

        vertex_lists = PSVNSplitFactory::PSVNSplitFactory(task_proxy, list_maximum).create();

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }

        int size = 0;
        for(int i = 0; i < int(vertex_lists.size()); i++){
            size += vertex_lists[i].size();
        }

        cout << "vertex list size: " << size;

        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void PSVNSplitSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        for(int i = 0; i < int(vertex_lists.size()); i++){
            iterate_through_DAG(vertex_lists[i][0], i, state, applicable_ops);
        }

        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSplitSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        for(int i = 0; i < int(vertex_lists.size()); i++){
            iterate_through_DAG(vertex_lists[i][0], i, state.unpack(), applicable_ops);
        }

        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSplitSuccessorGenerator::iterate_through_DAG(const Vertex &v, int list_nr, const State &state, vector<OperatorID> &applicable_ops) {

        for(int i : v.satisfied_operators){
            applicable_ops.push_back(OperatorID(i));
        }

        if(!v.children.empty()){
            iterate_through_DAG(vertex_lists[list_nr][v.children[state[v.choice].get_value()]], list_nr, state, applicable_ops);
        }

    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<PSVNSplitSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("psvn_split", _parse);
}
