//
// Created by yannick on 09.07.20.
//

#include "naive_successor_generator_factory.h"

#include "successor_generator_internals.h"

#include "../task_proxy.h"

#include "../utils/collections.h"
#include "../utils/memory.h"

#include <algorithm>
#include <cassert>

using namespace std;

namespace successor_generator {

    struct OperatorRange {
        int begin;
        int end;

        OperatorRange(int begin, int end)
                : begin(begin), end(end) {
        }

        bool empty() const {
            return begin == end;
        }

        int span() const {
            return end - begin;
        }
    };

    class OperatorInfo {

        OperatorID op;
        vector<FactPair> precondition;
    public:
        OperatorInfo(OperatorID op, vector<FactPair> precondition)
                : op(op),
                  precondition(move(precondition)) {
        }

        bool operator<(const OperatorInfo &other) const {
            return precondition < other.precondition;
        }

        OperatorID get_op() const {
            return op;
        }

        // Returns -1 as a past-the-end sentinel.
        int get_var(int depth) const {
            if (depth == static_cast<int>(precondition.size())) {
                return -1;
            } else {
                return precondition[depth].var;
            }
        }

        int get_value(int depth) const {
            return precondition[depth].value;
        }
    };

    successor_generator::NaiveSuccessorGeneratorFactory::NaiveSuccessorGeneratorFactory(
            const TaskProxy &task_proxy)
            : task_proxy(task_proxy) {
    }

    successor_generator::NaiveSuccessorGeneratorFactory::~NaiveSuccessorGeneratorFactory() = default;

    successor_generator::GeneratorPtr successor_generator::NaiveSuccessorGeneratorFactory::construct_fork(
            vector<GeneratorPtr> nodes) const {

        int size = nodes.size();
        if(size == 1){
            return move(nodes.at(0));
        }else if(size == 2){
            return utils::make_unique_ptr<GeneratorForkBinary>(move(nodes.at(0)), move(nodes.at(1)));
        } else {
            return utils::make_unique_ptr<GeneratorForkMulti>(move(nodes));
        }

    }

    successor_generator::GeneratorPtr successor_generator::NaiveSuccessorGeneratorFactory::construct_leaf(
            OperatorRange range) const {

        assert(!range.empty());
        vector<OperatorID> operators;
        operators.reserve(range.span());
        while(range.begin != range.end){
            operators.emplace_back(operator_infos[range.begin].get_op());
            ++range.begin;
        }
        if (operators.size() == 1) {
            return utils::make_unique_ptr<GeneratorLeafSingle>(operators.front());
        } else {
            return utils::make_unique_ptr<GeneratorLeafVector>(move(operators));
        }
    }

    successor_generator::GeneratorPtr successor_generator::NaiveSuccessorGeneratorFactory::construct_switch(
            int switch_var_id, ValuesAndGenerators values_and_generators) const {

            VariablesProxy variables = task_proxy.get_variables();
            int var_domain = variables[switch_var_id].get_domain_size();
            int num_children = values_and_generators.size();

            assert(num_children > 0);
            if(num_children == 1){
                int value = values_and_generators[0].first;
                GeneratorPtr generator = move(values_and_generators[0].second);
                return utils::make_unique_ptr<GeneratorSwitchSingle>(switch_var_id, value, move(generator));
            }

            int vector_bytes = utils::estimate_vector_bytes<GeneratorPtr>(var_domain);
            int hash_bytes = utils::estimate_unordered_map_bytes<int, GeneratorPtr>(num_children);
            if(hash_bytes < vector_bytes){
                unordered_map<int, GeneratorPtr> generator_by_value;
                for(auto &item : values_and_generators){
                    generator_by_value[item.first] = move(item.second);
                }
                return utils::make_unique_ptr<GeneratorSwitchHash>(switch_var_id, move(generator_by_value));
            } else{
                vector<GeneratorPtr> generator_by_value(var_domain);
                for(auto &item : values_and_generators){
                    generator_by_value[item.first] = move(item.second);
                }
                return utils::make_unique_ptr<GeneratorSwitchVector>(switch_var_id, move(generator_by_value));
            }
;
    }

    successor_generator::GeneratorPtr
    successor_generator::NaiveSuccessorGeneratorFactory::construct_recursive(int depth, OperatorRange range) const {

        vector<GeneratorPtr> nodes;


        //TODO stuff construct recursive
        return NULL;
    }

    static vector<FactPair> build_sorted_precondition(const OperatorProxy &op) {
        vector<FactPair> precond;
        precond.reserve(op.get_preconditions().size());
        for (FactProxy pre : op.get_preconditions())
            precond.emplace_back(pre.get_pair());
        // Preconditions must be sorted by variable.
        sort(precond.begin(), precond.end());
        return precond;
    }

    successor_generator::GeneratorPtr NaiveSuccessorGeneratorFactory::create() {
        OperatorsProxy operators = task_proxy.get_operators();
        operator_infos.reserve(operators.size());

        for(OperatorProxy op : operators){
            operator_infos.emplace_back(OperatorID(op.get_id()), build_sorted_precondition(op));
        }

        stable_sort(operator_infos.begin(), operator_infos.end());
        OperatorRange full_range(0, operator_infos.size());
        GeneratorPtr root = construct_recursive(0, full_range);
        operator_infos.clear();
        return root;

    }

}