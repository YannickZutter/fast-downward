
//
// Created by yannick on 20.07.20.
//

#include "marked_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "task_properties.h"
#include <vector>

using namespace std;

namespace successor_generator{


    MarkedSuccessorGenerator::MarkedSuccessorGenerator() = default;

    MarkedSuccessorGenerator::~MarkedSuccessorGenerator() = default;

    void MarkedSuccessorGenerator::initialize(const TaskProxy &task_proxy) {

        precondition_of.resize(task_properties::get_num_facts(task_proxy));
        counter.resize(task_proxy.get_operators().size());
        num_of_preconditions.resize(counter.size());
        first_visit.resize(counter.size());

        int temp_offset = 0;

        for(VariableProxy var : task_proxy.get_variables()){
            offset.push_back(temp_offset);
            temp_offset += var.get_domain_size();
        }

        for(VariableProxy var : task_proxy.get_variables()){
            for(int i = 0; i < var.get_domain_size(); i++){
                FactProxy val = var.get_fact(i);

                for(OperatorProxy op : task_proxy.get_operators()){
                    for(int j = 0; j < int(op.get_preconditions().size()); j++){
                        if(op.get_preconditions()[j].get_pair().var == var.get_id()){
                            if(op.get_preconditions()[j].get_pair().value == val.get_value()){
                                precondition_of[get_fact_id(var, val)].push_back(OperatorID(op.get_id()));
                            }
                        }
                    }
                    num_of_preconditions[op.get_id()] = op.get_preconditions().size();
                }
            }
        }

    }

    void MarkedSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops){

        fill(first_visit.begin(), first_visit.end(), true);

        for(FactProxy fact : state){

            for(OperatorID op_id : precondition_of[get_fact_id(fact)]){
                if(first_visit[op_id.get_index()]){
                    counter[op_id.get_index()] = num_of_preconditions[op_id.get_index()];
                    first_visit[op_id.get_index()] = false;
                }
                counter[op_id.get_index()]--;
            }
        }
        for(int i = 0; i < int(counter.size()); i++){
            if(counter[i] == 0 && !first_visit[i]){
                applicable_ops.push_back(OperatorID(i));
            }
        }
    }

    void MarkedSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops){

        generate_applicable_ops(state.unpack(), applicable_ops);
    }


    int MarkedSuccessorGenerator::get_fact_id(int var, int value) const {
        return offset[var] + value;
    }

    int MarkedSuccessorGenerator::get_fact_id(const FactProxy &fact) const {
        return get_fact_id(fact.get_variable().get_id(), fact.get_value());
    }

    int MarkedSuccessorGenerator::get_fact_id(VariableProxy var, FactProxy value) const {
        return get_fact_id(var.get_id(), value.get_value());
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<MarkedSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("marked", _parse);

}