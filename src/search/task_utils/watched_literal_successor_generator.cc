//
// Created by yannick on 05.09.20.
//

#include "watched_literal_successor_generator.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../utils/logging.h"
#include "task_properties.h"

using namespace std;

namespace successor_generator{
    WatchedLiteralSuccessorGenerator::WatchedLiteralSuccessorGenerator() {
    }

    WatchedLiteralSuccessorGenerator::~WatchedLiteralSuccessorGenerator() = default;

    void WatchedLiteralSuccessorGenerator::initialize(const TaskProxy &task_proxy) {
        utils::Timer init_timer;
        for(OperatorProxy op: task_proxy.get_operators()){
            operators.push_back(op);
        }
        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void WatchedLiteralSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        vector<OperatorProxy> plausible_ops = operators;
        vector<bool> unsat_ops(operators.size(), false);
        vector<bool> sat_ops(operators.size(), false);
        vector<vector<FactProxy>> watcher;

        for(int i = 0; i < plausible_ops.size(); i++){
            watcher[i].push_back(plausible_ops[i].get_preconditions()[0]);
        }
        for(FactProxy fact : state){
            for(int j = 0; j < watcher.size(); j++){
                if(!unsat_ops[j] && !sat_ops[j]){
                    FactProxy watched = watcher[j][watcher[j].size()-1];
                    if(watched.get_variable() == fact.get_variable()){
                        if(watched.get_value() != fact.get_value()){
                            unsat_ops[j] = true;
                        }else{
                            //do the other stuff
                        }
                    }
                }
            }
        }

        gao_timer.stop();
        total_duration += gao_timer();
        num_of_calls++;
    }

    void WatchedLiteralSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        gao_timer.stop();
        total_duration += gao_timer();
        num_of_calls++;
    }

    FactProxy WatchedLiteralSuccessorGenerator::get_next_watched_fact(OperatorID op_id, vector<FactProxy> watch_list) {
        OperatorProxy op = operators
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser){
        Options opts = parser.parse();
        return make_shared<WatchedLiteralSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("watched", _parse);
}
