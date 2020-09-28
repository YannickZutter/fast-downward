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
bool b(OperatorID a, OperatorID b){return a.get_index()<b.get_index();}
using namespace std;

namespace successor_generator {
    PSVNSplitSuccessorGenerator::PSVNSplitSuccessorGenerator() = default;

    PSVNSplitSuccessorGenerator::~PSVNSplitSuccessorGenerator() = default;

    void PSVNSplitSuccessorGenerator::initialize(const TaskProxy &task_proxy){
        utils::Timer init_timer;
        cout << "\nin sg limit is: " << list_maximum;

        vertex_lists = PSVNSplitFactory::PSVNSplitFactory(task_proxy, list_maximum).create();

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }
        /**
        cout << "\nprinting out all entries in tree: ";
        for(int i = 0; i < vertex_lists.size(); i++){
            cout << "\nlist nr. " <<i;
            for(Vertex v : vertex_lists[i]){
                cout << "\nchoice: "<<v.choice <<", rules: ";
                for(int i : v.rules){
                    cout << i << ", ";
                }
                cout << "tests: ";
                for(int i : v.test_results){
                    cout << i << ", ";
                }
                cout << "sat: ";
                for(int i : v.satisfied_rules){
                    cout << i << ", ";
                }
            }
        }
         **/
        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void PSVNSplitSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        for(int i = 0; i < int(vertex_lists.size()); i++){
            iterate_through_DAG(vertex_lists[i][0], i, state, applicable_ops);
        }
        /**
        std::sort(applicable_ops.begin(), applicable_ops.end(),b);
        if(num_of_calls < 100){

            cout << "\nops:";
            for(OperatorID id : applicable_ops){
                cout << id.get_index() << ", ";
            }
        }
**/
        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSplitSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        for(int i = 0; i < int(vertex_lists.size()); i++){
            iterate_through_DAG(vertex_lists[i][0], i, state.unpack(), applicable_ops);
        }
/**
        std::sort(applicable_ops.begin(), applicable_ops.end(),b);
        if(num_of_calls < 100){
            cout << "\nops:";
            for(OperatorID id : applicable_ops){
                cout << id.get_index() << ", ";
            }
        }
**/
        total_duration += gao_timer();
        num_of_calls++;
    }

    void PSVNSplitSuccessorGenerator::iterate_through_DAG(const Vertex &v, int list_nr, const State &state, vector<OperatorID> &applicable_ops) {

        if(!v.children.empty()){
            iterate_through_DAG(vertex_lists[list_nr][v.children[state[v.choice].get_value()]], list_nr, state, applicable_ops);
        }else{
            for(int rule_id : v.satisfied_rules){
                applicable_ops.push_back(OperatorID(rule_id));
            }
        }

    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<PSVNSplitSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("psvn_split", _parse);
}
