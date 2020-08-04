//
// Created by yannick on 20.07.20.
//

#include "timestamps_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "task_properties.h"
#include <vector>
#include "../utils/logging.h"

using namespace std;

namespace successor_generator{


    TimestampsSuccessorGenerator::TimestampsSuccessorGenerator() = default;

    TimestampsSuccessorGenerator::~TimestampsSuccessorGenerator() = default;

    void TimestampsSuccessorGenerator::initialize(const TaskProxy &task_proxy) {

        utils::Timer timer;
        facts.resize(task_properties::get_num_facts(task_proxy));
        counter.resize(task_proxy.get_operators().size());
        num_of_preconditions.resize(counter.size());
        timestamps.resize(counter.size());

        int offset = 0;
        for(VariableProxy var : task_proxy.get_variables()){
            fact_offsets.push_back(offset);
            offset += var.get_domain_size();
        }
        for(VariableProxy var : task_proxy.get_variables()){
            for(int i = 0; i < var.get_domain_size(); i++){

                FactProxy val = var.get_fact(i);

                for(OperatorProxy op : task_proxy.get_operators()){
                    PreconditionsProxy precons = op.get_preconditions();

                    for(int j = 0; j < int(precons.size()); j++){
                        if(precons[j].get_pair().var == var.get_id()){
                            if(precons[j].get_pair().value == val.get_value()){
                                facts[get_fact_id(var, val)].emplace_back(OperatorID(op.get_id()));
                            }
                        }

                    }
                    num_of_preconditions[op.get_id()] = op.get_preconditions().size();
                }
            }
        }
        double time = timer();
        utils::g_log << "time to initialize successor generator: " << time << endl;

    }

    void TimestampsSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops){

        utils::Timer timer;
        fill(timestamps.begin(), timestamps.end(), false);
        current_timestamp++;
        for(FactProxy fact : state){

            for(OperatorID op_id : facts[get_fact_id(fact.get_pair().var, fact.get_pair().value)]){

                if(timestamps[op_id.get_index()] < current_timestamp){
                    counter[op_id.get_index()] = num_of_preconditions[op_id.get_index()];
                    timestamps[op_id.get_index()] = current_timestamp;
                }
                counter[op_id.get_index()]--;
            }
        }

        for(int i = 0; i < int(counter.size()); i++){
            if(counter[i] == 0 && timestamps[i] == current_timestamp){
                applicable_ops.emplace_back(i);
            }
        }
        double time = timer();
        total_duration += time;
        num_of_calls++;
    }

    void TimestampsSuccessorGenerator::generate_applicable_ops(const GlobalState &state_, vector<OperatorID> &applicable_ops){

        utils::Timer timer;
        fill(timestamps.begin(), timestamps.end(), false);
        current_timestamp++;
        State state = state_.unpack();
        for(FactProxy fact : state){

            for(OperatorID op_id : facts[get_fact_id(fact.get_pair().var, fact.get_pair().value)]){

                if(timestamps[op_id.get_index()] < current_timestamp){
                    counter[op_id.get_index()] = num_of_preconditions[op_id.get_index()];
                    timestamps[op_id.get_index()] = current_timestamp;
                }
                counter[op_id.get_index()]--;
            }
        }

        for(int i = 0; i < int(counter.size()); i++){
            if(counter[i] == 0 && timestamps[i] == current_timestamp){
                applicable_ops.emplace_back(i);
            }
        }
        double time = timer();
        total_duration += time;
        num_of_calls++;
    }


    int TimestampsSuccessorGenerator::get_fact_id(int var, int value) const {
        return fact_offsets[var] + value;
    }

    int TimestampsSuccessorGenerator::get_fact_id(const FactProxy &fact) const {
        return get_fact_id(fact.get_variable().get_id(), fact.get_value());
    }

    int TimestampsSuccessorGenerator::get_fact_id(VariableProxy var, FactProxy value) const {
        return get_fact_id(var.get_id(), value.get_value());
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<TimestampsSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("timestamps", _parse);

}