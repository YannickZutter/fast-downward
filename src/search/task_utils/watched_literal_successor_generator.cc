//
// Created by yannick on 05.09.20.
//

#include "watched_literal_successor_generator.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../utils/logging.h"

using namespace std;

namespace successor_generator{
    WatchedLiteralSuccessorGenerator::WatchedLiteralSuccessorGenerator() {
    }

    WatchedLiteralSuccessorGenerator::~WatchedLiteralSuccessorGenerator() = default;

    void WatchedLiteralSuccessorGenerator::initialize(const TaskProxy &task_proxy) {
        utils::Timer init_timer;
        for(OperatorProxy op: task_proxy.get_operators()){

            operators.push_back(OwnOps(op.get_id(), op.get_preconditions()));

        }

        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void WatchedLiteralSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        vector<int> plausible_ops;
        for(int i = 0; i < operators.size(); i++){
            plausible_ops.push_back(operators[i].op_id);
        }
        vector<int> temp;
        vector<vector<FactProxy>> watcher_list(operators.size());

        for(int op_id : plausible_ops){
            watcher_list[op_id].push_back(operators[op_id].preconditions[0]);
        }

        for(FactProxy fact : state){

            for (const int id : plausible_ops) {
                FactProxy watched = watcher_list[id][watcher_list[id].size()-1];

                if(watched.get_variable() == fact.get_variable()){

                    if(watched.get_value() == fact.get_value()){// if is same

                        //check if it is last precon
                        if(watcher_list[id].size() == operators[id].preconditions.size()){
                            //do not put back, dont need to watch again, already applicable
                            applicable_ops.push_back(OperatorID(id));
                        }else{//if list is still smaller

                            watcher_list[id].push_back(operators[id].preconditions[watcher_list[id].size()]);
                            temp.push_back(id);
                        }
                    }
                }else{ // operator not precon on this, put it back in list
                    temp.push_back(id);
                }

            }
            plausible_ops.clear();
            plausible_ops = temp;
            temp.clear();
        }

        gao_timer.stop();
        total_duration += gao_timer();
        num_of_calls++;
    }

    void WatchedLiteralSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        vector<int> plausible_ops;
        for(int i = 0; i < operators.size(); i++){
            plausible_ops.push_back(operators[i].op_id);
        }
        vector<int> temp;
        vector<vector<FactProxy>> watcher_list(operators.size());

        for(int op_id : plausible_ops){
            watcher_list[op_id].push_back(operators[op_id].preconditions[0]);
        }

        for(FactProxy fact : state.unpack()){

            for (const int id : plausible_ops) {
                FactProxy watched = watcher_list[id][watcher_list[id].size()-1];

                if(watched.get_variable() == fact.get_variable()){

                    if(watched.get_value() == fact.get_value()){// if is same

                        //check if it is last precon
                        if(watcher_list[id].size() == operators[id].preconditions.size()){
                            //do not put back, dont need to watch again, already applicable
                            applicable_ops.push_back(OperatorID(id));
                        }else{//if list is still smaller

                            watcher_list[id].push_back(operators[id].preconditions[watcher_list[id].size()]);
                            temp.push_back(id);
                        }
                    }
                }else{ // operator not precon on this, put it back in list
                    temp.push_back(id);
                }

            }
            plausible_ops.clear();
            plausible_ops = temp;
            temp.clear();
        }

        gao_timer.stop();
        total_duration += gao_timer();
        num_of_calls++;
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser){
        Options opts = parser.parse();
        return make_shared<WatchedLiteralSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("watched", _parse);
}
