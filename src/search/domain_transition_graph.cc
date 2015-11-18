#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <unordered_map>

using namespace std;

#include "domain_transition_graph.h"
#include "global_operator.h"
#include "globals.h"
#include "utilities.h"
#include "utilities_hash.h"

DTGFactory::DTGFactory(const TaskProxy& task_proxy,
    bool collect_transition_side_effects,
    const function<bool(int, int)>& pruning_condition)
    : task_proxy(task_proxy),
        collect_transition_side_effects(collect_transition_side_effects),
        pruning_condition(pruning_condition) {
    }

void DTGFactory::build_dtgs(vector<DomainTransitionGraph *>& location) {
    assert(location.empty()); // TODO use local location and move

    cout << "Building DTGs..." << flush;
    allocate_graphs_and_nodes(location);
    transition_index.clear();
    transition_index.resize(location.size());
    global_to_local_var.clear();
    global_to_local_var.resize(location.size());
    for (OperatorProxy op : task_proxy.get_operators())
        for (EffectProxy eff : op.get_effects())
            process_effect(eff, op, location);

    cout << "Simplifying transitions..." << flush;
    for (auto *dtg : location) {
        vector<ValueNode> &nodes = dtg->nodes;
        for (size_t value = 0; value < nodes.size(); ++value)
            for (size_t i = 0; i < nodes[value].transitions.size(); ++i)
                nodes[value].transitions[i].simplify(task_proxy);
    }

    if (collect_transition_side_effects) {
        cout << "Collecting transition side effects..." << flush;
        for (auto *dtg : location) {
            vector<ValueNode> &nodes = dtg->nodes;
            for (size_t value = 0; value < nodes.size(); ++value)
                for (size_t i = 0; i < nodes[value].transitions.size(); ++i)
                    collect_side_effects(dtg, nodes[value].transitions[i].labels);
        }
    }
}

void DTGFactory::allocate_graphs_and_nodes(vector<DomainTransitionGraph *>& location) {
    VariablesProxy variables = task_proxy.get_variables();
    location.resize(variables.size());
    for (VariableProxy var : variables) {
        int var_id = var.get_id();
        int range = var.get_domain_size();
        location[var_id] = new DomainTransitionGraph(var_id, range);
    }
}

void DTGFactory::process_effect(const EffectProxy& eff, const OperatorProxy& op,
    vector<DomainTransitionGraph *>& dtgs) {
    FactProxy fact = eff.get_fact();
    int var_id = fact.get_variable().get_id();
    DomainTransitionGraph *dtg = dtgs[var_id];
    int origin = -1;
    int target = fact.get_value();
    vector<LocalAssignment> transition_condition;
    vector<LocalAssignment> side_effect;
    unsigned int first_new_local_var = dtg->local_to_global_child.size();
    for (FactProxy fact : op.get_preconditions()) {
        if (fact.get_variable() == eff.get_fact().get_variable())
            origin = fact.get_value();
        else {
            update_transition_condition(fact, dtg, transition_condition);
        }
    }
    for (FactProxy fact : eff.get_conditions()) {
        if (fact.get_variable() == eff.get_fact().get_variable()) {
            if (origin != -1 && fact.get_value() != origin) {
                revert_new_local_vars(dtg, first_new_local_var);
                return; // conflicting condition on effect variable
            }
            origin = fact.get_value();
        } else {
            update_transition_condition(fact, dtg, transition_condition);
        }
    }
    if (origin != -1 && target == origin) {
        revert_new_local_vars(dtg, first_new_local_var);
        return;
    }
    if (origin != -1) {
        ValueTransition *trans = get_transition(origin, target, dtg);
        trans->labels.push_back(
            ValueTransitionLabel(op.get_id(), transition_condition, side_effect));
    } else {
        int domain_size = fact.get_variable().get_domain_size();
        for (int origin = 0; origin < domain_size; ++origin) {
            if (origin == target)
                continue;
            ValueTransition *trans = get_transition(origin, target, dtg);
            trans->labels.push_back(
                ValueTransitionLabel(op.get_id(), transition_condition, side_effect));
        }
    }
}

void DTGFactory::update_transition_condition(const FactProxy& fact,
    DomainTransitionGraph* dtg, vector<LocalAssignment>& condition) {
    int fact_var = fact.get_variable().get_id();
    if (!pruning_condition(dtg->var, fact_var)) {
        extend_global_to_local_mapping_if_necessary(dtg, fact_var);
        int local_var = global_to_local_var[dtg->var][fact_var];
        condition.push_back(LocalAssignment(local_var, fact.get_value()));
    }
}

void DTGFactory::extend_global_to_local_mapping_if_necessary(
    DomainTransitionGraph *dtg, int global_var) {
    if (!global_to_local_var[dtg->var].count(global_var)) {
        global_to_local_var[dtg->var][global_var] = dtg->local_to_global_child.size();
        dtg->local_to_global_child.push_back(global_var);
    }
}

void DTGFactory::revert_new_local_vars(DomainTransitionGraph* dtg,
    unsigned int first_local_var) {
    vector<int> &loc_to_glob = dtg->local_to_global_child;
    for (unsigned int l = first_local_var; l < loc_to_glob.size(); ++l)
        global_to_local_var[dtg->var].erase(loc_to_glob[l]);
    if (loc_to_glob.size() > first_local_var)
        loc_to_glob.erase(loc_to_glob.begin() + first_local_var,
                          loc_to_glob.end());
}

ValueTransition* DTGFactory::get_transition(int origin, int target,
    DomainTransitionGraph* dtg) {
    map<pair<int,int>, int>& trans_map = transition_index[dtg->var];
    pair<int,int> arc = make_pair(origin, target);
    ValueNode& origin_node = dtg->nodes[origin];
    // create new transition if necessary
    if (!trans_map.count(arc)) {
        trans_map[arc] = origin_node.transitions.size();
        ValueNode& target_node = dtg->nodes[target];
        origin_node.transitions.push_back(ValueTransition(&target_node));
    }
    return &origin_node.transitions[trans_map[arc]];
}

void DTGFactory::collect_side_effects(DomainTransitionGraph *dtg,
    vector<ValueTransitionLabel> &labels) {
    const vector<int> &loc_to_glob = dtg->local_to_global_child;
    const map<int, int> &glob_to_loc = global_to_local_var[dtg->var];
    
    vector<ValueTransitionLabel> old_labels;
    old_labels.swap(labels);
    for (auto &label : old_labels) {
        // create global condition for label
        vector<pair<int, int>> precond_pairs;
        for (auto &assignment : label.precond) {
            int var = loc_to_glob[assignment.local_var];
            precond_pairs.push_back(make_pair(var, assignment.value));
        }
        sort(precond_pairs.begin(), precond_pairs.end());

        // collect operator precondition
        OperatorProxy op = task_proxy.get_operators()[label.op_id];
        unordered_map<int, int> pre_map;
        for (FactProxy pre : op.get_preconditions())
            pre_map[pre.get_variable().get_id()] = pre.get_value();

        // collect side effect from each operator effect
        vector<LocalAssignment> side_effects;
        for (EffectProxy eff : op.get_effects()) {
            int var_no = eff.get_fact().get_variable().get_id();
            if (var_no == dtg->var || !glob_to_loc.count(var_no)) {
                // This is either an effect on the variable we're
                // building the DTG for, or an effect on a variable we
                // don't need to track because it doesn't appear in
                // conditions of this DTG. Ignore it.
                continue;
            }

            int pre = -1;
            auto pre_it = pre_map.find(var_no);
            if (pre_it != pre_map.end())
                pre = pre_it->second;
            int post = eff.get_fact().get_value();

            vector<pair<int, int>> triggercond_pairs;
            if (pre != -1)
                triggercond_pairs.push_back(make_pair(var_no, pre));

            for (FactProxy condition : eff.get_conditions()) {
                int c_var_id = condition.get_variable().get_id();
                int c_val = condition.get_value();
                triggercond_pairs.push_back(make_pair(c_var_id, c_val));
            }
            sort(triggercond_pairs.begin(), triggercond_pairs.end());

            if (includes(precond_pairs.begin(), precond_pairs.end(),
                         triggercond_pairs.begin(), triggercond_pairs.end())) {
                int local_var = glob_to_loc.at(var_no);
                side_effects.push_back(LocalAssignment(local_var, post));
            }
        }
        // TODO remove once cea heuristic uses task interface
        const GlobalOperator *g_op = op.get_global_operator();
        // TODO Can we modify the existing labels instead?
        labels.push_back(
            ValueTransitionLabel(g_op, label.precond, side_effects));
    }
}


DomainTransitionGraph::DomainTransitionGraph(int var_index, int node_count) {
    is_axiom = g_axiom_layers[var_index] != -1;
    var = var_index;
    nodes.reserve(node_count);
    for (int value = 0; value < node_count; ++value)
        nodes.push_back(ValueNode(this, value));
    last_helpful_transition_extraction_time = -1;
}

// TODO after the read_all method has gone, this should maybe belong to the
// factory (the current flow of the task proxy is somewhat awkward)
void ValueTransition::simplify(const TaskProxy &task_proxy) {
    // Remove labels with duplicate or dominated conditions.

    /*
      Algorithm: Put all transitions into an unordered_map
      (key: condition, value: index in label list).
      This already gets rid of transitions with identical conditions.

      Then go through the unordered_map, checking for each element if
      none of the subset conditions are part of the unordered_map.
      Put the element into the new labels list iff this is the case.
     */

    typedef vector<pair<int, int>> HashKey;
    typedef unordered_map<HashKey, int> HashMap;
    HashMap label_index;
    label_index.reserve(labels.size());
    OperatorsProxy operators = task_proxy.get_operators();

    for (size_t i = 0; i < labels.size(); ++i) {
        HashKey key;
        for (LocalAssignment &assign : labels[i].precond)
            key.push_back(make_pair(assign.local_var, assign.value));
        sort(key.begin(), key.end());
        label_index[key] = i;
    }

    vector<ValueTransitionLabel> old_labels;
    old_labels.swap(labels);

    for (auto &entry : label_index) {
        const HashKey &key = entry.first;
        int label_no = entry.second;
        int powerset_size = (1 << key.size()) - 1; // -1: only consider proper subsets
        bool match = false;
        if (powerset_size <= 31) { // HACK! Don't spend too much time here...
            int label_op_id = old_labels[label_no].op_id;
            int label_cost = operators[label_op_id].get_cost();
            for (int mask = 0; mask < powerset_size; ++mask) {
                HashKey subset;
                for (size_t i = 0; i < key.size(); ++i)
                    if (mask & (1 << i))
                        subset.push_back(key[i]);
                HashMap::iterator found = label_index.find(subset);
                if (found != label_index.end()) {
                    const ValueTransitionLabel &f_label = old_labels[found->second];
                    if (label_cost >= operators[f_label.op_id].get_cost()) {
                        /* TODO: Depending on how clever we want to
                           be, we could prune based on the *adjusted*
                           cost for the respective heuristic instead.
                           This would potentially allow us more pruning
                           when using unit costs as adjusted costs.
                           Seems a minor optimization though.
                        */
                        match = true;
                        break;
                    }
                }
            }
        }
        if (!match)
            labels.push_back(old_labels[label_no]);
    }
}
