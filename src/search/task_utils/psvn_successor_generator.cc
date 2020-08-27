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

namespace successor_generator {
    PSVNSuccessorGenerator::PSVNSuccessorGenerator() = default;

    PSVNSuccessorGenerator::~PSVNSuccessorGenerator() = default;

    void PSVNSuccessorGenerator::initialize(const TaskProxy &task_proxy){
        utils::Timer init_timer;

        vertex_list = PSVNFactory::PSVNFactory(task_proxy).create();
        //cout << "\nmade vertex list";
        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
            op_ids.push_back(-1);
        }
        //cout << "\nmade operators and ids";

        //cout<<"\nstart outputting whole vertex list";
/**
        for(int i = 0; i < vertex_list.size(); i++){
            Vertex v = vertex_list[i];
            cout << "\n" << i << " "<<v.choice<<" rule ";
            for(int j : v.rules){
                cout<<j<<" ";
            }
            cout <<"sat ";
            for(int j : v.satisfied_rules){
                cout << j <<" ";
            }
            cout << "test ";
            for(int j : v.test_results){
                cout << j<<" ";
            }
            cout << "children ";
            for(int j : v.children){
                cout << j <<" ";
            }
        }
**/
        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        vector<bool> taken_ops(vertex_list[0].satisfied_rules.size(), false);
        iterate_through_DAG(vertex_list[0], state, applicable_ops, taken_ops);
        for(int i = 0; i < int(taken_ops.size()); i++){
            if(taken_ops[i]){
                applicable_ops.push_back(OperatorID(i));
            }
        }/**
        cout << "\napplicable ops are: ";
        for(OperatorID i : applicable_ops){
            cout<<i.get_index()<<", ";
        }
        **/
        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        generate_applicable_ops(state.unpack(), applicable_ops);
    }

    void PSVNSuccessorGenerator::iterate_through_DAG(const Vertex &v,const State &state, vector<OperatorID> &applicable_ops, vector<bool> &taken_ops) {

        for(int i : v.satisfied_rules){
            if(i != -1){
                taken_ops[i] = true;
            }
        }

        if(v.children.size()>0){
            int var = v.choice;
            int val = state[var].get_value();
            int index =v.children[val];
            iterate_through_DAG(vertex_list[index], state, applicable_ops, taken_ops);
        }
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<PSVNSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("psvn", _parse);
}
