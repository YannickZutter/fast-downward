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

    enum class GroupOperatorsBy {
        VAR,
        VALUE
    };

    class OperatorGrouper {
        const vector<OperatorInfo> &operator_infos;
        const int depth;
        const GroupOperatorsBy group_by;
        OperatorRange range;

        const OperatorInfo &get_current_op_info() const {
            assert(!range.empty());
            return operator_infos[range.begin];
        }

        int get_current_group_key() const {
            const OperatorInfo &op_info = get_current_op_info();
            if (group_by == GroupOperatorsBy::VAR) {
                return op_info.get_var(depth);
            } else {
                assert(group_by == GroupOperatorsBy::VALUE);
                return op_info.get_value(depth);
            }
        }
    public:
        explicit OperatorGrouper(
                const vector<OperatorInfo> &operator_infos,
                int depth,
                GroupOperatorsBy group_by,
                OperatorRange range)
                : operator_infos(operator_infos),
                  depth(depth),
                  group_by(group_by),
                  range(range) {
        }

        bool done() const {
            return range.empty();
        }

        pair<int, OperatorRange> next() {
            assert(!range.empty());
            int key = get_current_group_key();
            int group_begin = range.begin;
            do {
                ++range.begin;
            } while (!range.empty() && get_current_group_key() == key);
            OperatorRange group_range(group_begin, range.begin);
            return make_pair(key, group_range);
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

        OperatorGrouper grouper_by_var(operator_infos, depth, GroupOperatorsBy::VAR, range);
        while (!grouper_by_var.done()){
            auto var_group = grouper_by_var.next();
            int var = var_group.first;
            OperatorRange var_range = var_group.second;

            if (var == -1){
                nodes.push_back(construct_leaf(var_range));
            } else{
                ValuesAndGenerators values_and_generators;
                OperatorGrouper grouper_by_value(operator_infos, depth, GroupOperatorsBy::VALUE, range);
                while (!grouper_by_value.done()){
                    auto value_group = grouper_by_value.next();
                    int value = value_group.first;
                    OperatorRange value_range = value_group.second;

                    values_and_generators.emplace_back(value, construct_recursive(depth+1, value_range));
                }
                nodes.push_back(construct_switch(var, move(values_and_generators)));

            }
        }
        return construct_fork(move(nodes));
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
        for (OperatorProxy op : operators) {
            operator_infos.emplace_back(
                    OperatorID(op.get_id()), build_sorted_precondition(op));
        }

        OperatorRange full_range(0,operator_infos.size());
        GeneratorPtr root;

        vector<GeneratorPtr> nodes;
        OperatorGrouper grouper_by_var(operator_infos, 0, GroupOperatorsBy::VAR, full_range);
        while (!grouper_by_var.done()){
            auto var_group = grouper_by_var.next();
            int var = var_group.first;
            OperatorRange var_range =var_group.second;
            if(var==-1){
                nodes.push_back(construct_leaf(var_range));
            } else{
                ValuesAndGenerators values_and_generators;
                OperatorGrouper grouper_by_value(operator_infos, 0, GroupOperatorsBy::VALUE, var_range);
                while(!grouper_by_value.done()){
                    auto value_group = grouper_by_value.next();
                    int value = value_group.first;
                    OperatorRange value_range = value_group.second;

                    nodes.push_back(construct_leaf(value_range));

                }
            }

        }
        root = construct_fork(nodes);

        return root;

    }

}