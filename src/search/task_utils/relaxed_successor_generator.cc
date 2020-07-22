//
// Created by yannick on 20.07.20.
//

#include "relaxed_successor_generator.h"
#include "successor_generator_internals.h"
#include "successor_generator_base.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "task_properties.h"
#include "../utils/collections.h"
#include "../utils/logging.h"
#include "../utils/timer.h"




using namespace std;

namespace successor_generator{

    Fact::Fact()
            : cost(-1),
              reached_by(NO_OP),
              is_goal(false),
              marked(false),
              num_precondition_occurences(-1) {
    }
    RelaxedSuccessorGenerator::RelaxedSuccessorGenerator() {

    }

    RelaxedSuccessorGenerator::~RelaxedSuccessorGenerator() = default;

    void RelaxedSuccessorGenerator::initialize(const TaskProxy &task_proxy) {

        //setting propositions
        facts.resize(task_properties::get_num_facts(task_proxy));
        // setting offset
        VariablesProxy variables = task_proxy.get_variables();
        fact_offsets.reserve(variables.size());
        PropID offset = 0;
        for(VariableProxy var : variables){
            fact_offsets.push_back(offset);
            offset += var.get_domain_size();
        }
        assert(offset == static_cast<int>(facts.size()));

        //setting unary operators for operators and axioms
        unary_operators.reserve(task_properties::get_num_total_effects(task_proxy));
        for(OperatorProxy op : task_proxy.get_operators()){
            build_unary_operators(op);
        }
        for(OperatorProxy axiom : task_proxy.get_axioms()){
            build_unary_operators(axiom);
        }

        //simplify unary operators
        utils::Timer simplify_timer;
        simplify();
        utils::g_log << "time to simplify: " << simplify_timer << endl;

        //cross-reference unary operators

        vector<vector<OpID>> precondition_of_vectors(facts.size());

        int num_unary_ops = unary_operators.size();

        for(OpID op_id = 0; op_id < num_unary_ops; op_id++){
            for(PropID precond : get_preconditions(op_id)){
                precondition_of_vectors[precond].push_back(op_id);
            }
        }

        int num_facts = facts.size();

        //counter.reserve(num_facts);
        for(PropID prop_id = 0; prop_id < num_facts; prop_id++){
            auto precondition_of_vec = move(precondition_of_vectors[prop_id]);
            facts[prop_id].precondition_of = precondition_of_pool.append(precondition_of_vec);
            facts[prop_id].num_precondition_occurences = precondition_of_vec.size();
            //counter[prop_id] = precondition_of_vec.size();
        }

    }

    void RelaxedSuccessorGenerator::reset_counter(vector<int> &cnt){

        int s = cnt.size();
        assert(s == facts.size());


        for(int i = 0; i < s; i++){
            cnt[i] = facts[i].num_precondition_occurences;
        }
    }
    void RelaxedSuccessorGenerator::generate_applicable_ops(const State &state, vector<OperatorID> &applicable_ops) const{

        vector<int> counter;
        int facts_size = facts.size();
        int counter_size = counter.size();
        counter.reserve(facts_size);
        for(int i = 0; i < facts_size; i++){
            counter[i] = facts[i].num_precondition_occurences;
        }

        for(FactProxy fact : state){
            PropID prop = get_prop_id(fact);
            counter[prop]--;
        }
        for(int i = 0; i < counter_size; i++){
            if(counter[i] == 0){
                applicable_ops.push_back(OperatorID(i));
            }
        }

    }

    void RelaxedSuccessorGenerator::generate_applicable_ops(const GlobalState &state, vector<OperatorID> &applicable_ops) const{

        generate_applicable_ops(state.unpack(), applicable_ops);

    }

    void RelaxedSuccessorGenerator::build_unary_operators(const OperatorProxy &op) {
        int op_no = op.is_axiom() ? -1 : op.get_id();
        int base_cost = op.get_cost();
        vector<PropID> precondition_props;
        PreconditionsProxy preconditions = op.get_preconditions();
        precondition_props.reserve(preconditions.size());
        for (FactProxy precondition : preconditions) {
            precondition_props.push_back(get_prop_id(precondition));
        }
        for (EffectProxy effect : op.get_effects()) {
            PropID effect_prop = get_prop_id(effect.get_fact());
            EffectConditionsProxy eff_conds = effect.get_conditions();
            precondition_props.reserve(preconditions.size() + eff_conds.size());
            for (FactProxy eff_cond : eff_conds) {
                precondition_props.push_back(get_prop_id(eff_cond));
            }

            // The sort-unique can eventually go away. See issue497.
            vector<PropID> preconditions_copy(precondition_props);
            utils::sort_unique(preconditions_copy);
            array_pool::ArrayPoolIndex precond_index =
                    preconditions_pool.append(preconditions_copy);
            unary_operators.emplace_back(
                    preconditions_copy.size(), precond_index, effect_prop,
                    op_no, base_cost);
            precondition_props.erase(precondition_props.end() - eff_conds.size(), precondition_props.end());
        }
    }

    void RelaxedSuccessorGenerator::simplify() {

#ifndef NDEBUG
        int num_ops = unary_operators.size();
        for (OpID op_id = 0; op_id < num_ops; ++op_id)
            assert(utils::is_sorted_unique(get_preconditions_vector(op_id)));
#endif

        const int MAX_PRECONDITIONS_TO_TEST = 5;

        utils::g_log << "Simplifying " << unary_operators.size() << " unary operators..." << flush;

        using Key = pair<vector<PropID>, PropID>;
        using Value = pair<int, OpID>;
        using Map = utils::HashMap<Key, Value>;
        Map unary_operator_index;
        unary_operator_index.reserve(unary_operators.size());

        for (size_t op_no = 0; op_no < unary_operators.size(); ++op_no) {
            const UnaryOperator &op = unary_operators[op_no];

            Key key(get_preconditions_vector(op_no), op.effect);
            Value value(op.base_cost, op_no);
            auto inserted = unary_operator_index.insert(
                    make_pair(move(key), value));
            if (!inserted.second) {
                // We already had an element with this key; check its cost.
                Map::iterator iter = inserted.first;
                Value old_value = iter->second;
                if (value < old_value)
                    iter->second = value;
            }
        }

        Key dominating_key;

        auto is_dominated = [&](const UnaryOperator &op) {

            OpID op_id = get_op_id(op);
            int cost = op.base_cost;

            const vector<PropID> precondition = get_preconditions_vector(op_id);

            if (unary_operator_index[make_pair(precondition, op.effect)].second != op_id)
                return true;

            if (op.num_preconditions > MAX_PRECONDITIONS_TO_TEST) {

                return false;
            }

            vector<PropID> &dominating_precondition = dominating_key.first;
            dominating_key.second = op.effect;

            int powerset_size = (1 << precondition.size()) - 1;
            for (int mask = 0; mask < powerset_size; ++mask) {
                dominating_precondition.clear();
                for (size_t i = 0; i < precondition.size(); ++i)
                    if (mask & (1 << i))
                        dominating_precondition.push_back(precondition[i]);
                Map::iterator found = unary_operator_index.find(dominating_key);
                if (found != unary_operator_index.end()) {
                    Value dominator_value = found->second;
                    int dominator_cost = dominator_value.first;
                    if (dominator_cost <= cost)
                        return true;
                }
            }
            return false;
        };

        unary_operators.erase(
                remove_if(
                        unary_operators.begin(),
                        unary_operators.end(),
                        is_dominated),
                unary_operators.end());

        utils::g_log << " done! [" << unary_operators.size() << " unary operators]" << endl;
    }

    PropID RelaxedSuccessorGenerator::get_prop_id(int var, int value) const {
        return fact_offsets[var] + value;
    }

    PropID RelaxedSuccessorGenerator::get_prop_id(const FactProxy &fact) const {
        return get_prop_id(fact.get_variable().get_id(), fact.get_value());
    }

    const Fact *RelaxedSuccessorGenerator::get_proposition(
            int var, int value) const {
        return &facts[get_prop_id(var, value)];
    }

    Fact *RelaxedSuccessorGenerator::get_proposition(int var, int value) {
        return &facts[get_prop_id(var, value)];
    }

    Fact *RelaxedSuccessorGenerator::get_proposition(const FactProxy &fact) {
        return get_proposition(fact.get_variable().get_id(), fact.get_value());
    }

    static shared_ptr<successor_generator::SuccessorGeneratorBase> _parse(OptionParser &parser) {

        Options opts = parser.parse();

        return make_shared<RelaxedSuccessorGenerator>();
    }

    static Plugin<successor_generator::SuccessorGeneratorBase> _plugin("relaxed", _parse);

    UnaryOperator::UnaryOperator(
            int num_preconditions, array_pool::ArrayPoolIndex preconditions,
            PropID effect, int operator_no, int base_cost)
            : effect(effect),
              base_cost(base_cost),
              num_preconditions(num_preconditions),
              preconditions(preconditions),
              operator_no(operator_no) {
    }
}