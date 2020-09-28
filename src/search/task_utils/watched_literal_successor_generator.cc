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
        precondition_tracker = vector<int>(task_proxy.get_operators().size(), 0);
        watcher_list.resize(task_properties::get_num_facts(task_proxy));
        int temp_offset = 0;
        for(VariableProxy var : task_proxy.get_variables()){
            offset.push_back(temp_offset);
            temp_offset += var.get_domain_size();
        }

        for(OperatorProxy op: task_proxy.get_operators()){

            operators.push_back(op);
            int factID = get_fact_id(op.get_preconditions()[0]);
            watcher_list[factID].push_back(op.get_id());
        }

        init_timer.stop();
        utils::g_log << "time to initialize successor generator: " << init_timer() << endl;

    }

    void WatchedLiteralSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        for(FactProxy fact : state){
            process_watch_list(fact, state, applicable_ops);
        }

        gao_timer.stop();
        total_duration += gao_timer();
        num_of_calls++;
    }

    void WatchedLiteralSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) {
        utils::Timer gao_timer;

        for(FactProxy fact : state.unpack()){
            process_watch_list(fact, state.unpack(), applicable_ops);
        }

        gao_timer.stop();
        total_duration += gao_timer();
        num_of_calls++;
    }

    void WatchedLiteralSuccessorGenerator::process_watch_list(FactProxy fact, const State& state, vector<OperatorID> &applicable_ops) {

        int list_size = watcher_list[get_fact_id(fact)].size();

        for(int i = list_size-1; i >= 0; i--){
            int id = watcher_list[get_fact_id(fact)][i];
            int precondition_counter = 0;

            for(int j = 0; j < int(operators[id].get_preconditions().size()); j++){

                FactProxy precon = operators[id].get_preconditions()[precondition_tracker[id]];

                if(precon == state[precon.get_variable()]){
                    precondition_counter++;
                    precondition_tracker[id] = (precondition_tracker[id]+1)%operators[id].get_preconditions().size();
                }else{
                    watcher_list[get_fact_id(precon)].push_back(id);
                    watcher_list[get_fact_id(fact)].erase(watcher_list[get_fact_id(fact)].begin()+i);
                    break;
                }
            }
            if(precondition_counter == int(operators[id].get_preconditions().size())){
                applicable_ops.push_back(OperatorID(id));
            }
        }
    }

    int WatchedLiteralSuccessorGenerator::get_fact_id(int var, int value) const {
        return offset[var] + value;
    }

    int WatchedLiteralSuccessorGenerator::get_fact_id(const FactProxy &fact) const {
        return get_fact_id(fact.get_variable().get_id(), fact.get_value());
    }

    int WatchedLiteralSuccessorGenerator::get_fact_id(VariableProxy var, FactProxy value) const {
        return get_fact_id(var.get_id(), value.get_value());
    }



    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser){
        Options opts = parser.parse();
        return make_shared<WatchedLiteralSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("watched", _parse);
}
