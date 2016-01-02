#include "abstraction.h"

#include "abstract_state.h"
#include "utils.h"

#include "../globals.h"
#include "../option_parser.h"
#include "../task_tools.h"

#include "../utils/logging.h"
#include "../utils/memory.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>

using namespace std;

namespace CEGAR {
static const int STATES_LOG_STEP = 1000;

struct Flaw {
    const State concrete_state;
    AbstractState *current_abstract_state;
    const AbstractState desired_abstract_state;

    Flaw(State &&conc_state, AbstractState *abs_state, AbstractState &&desired_abs_state)
        : concrete_state(move(conc_state)),
          current_abstract_state(abs_state),
          desired_abstract_state(move(desired_abs_state)) {
    }

    vector<Split> get_possible_splits() const {
        vector<Split> splits;
        for (FactProxy wanted_fact : concrete_state) {
            if (!current_abstract_state->contains(wanted_fact) ||
                !desired_abstract_state.contains(wanted_fact)) {
                VariableProxy var = wanted_fact.get_variable();
                vector<int> wanted;
                for (int value = 0; value < var.get_domain_size(); ++value) {
                    FactProxy fact = var.get_fact(value);
                    if (current_abstract_state->contains(fact) && desired_abstract_state.contains(fact)) {
                        wanted.push_back(value);
                    }
                }
                splits.emplace_back(var.get_id(), move(wanted));
            }
        }
        assert(!splits.empty());
        return splits;
    }
};

Abstraction::Abstraction(const Options &opts)
    : task_proxy(*opts.get<shared_ptr<AbstractTask>>("transform")),
      do_separate_unreachable_facts(opts.get<bool>("separate_unreachable_facts")),
      max_states(opts.get<int>("max_states")),
      abstract_search(opts),
      split_selector(
          opts.get<shared_ptr<AbstractTask>>("transform"),
          PickSplit(opts.get<int>("pick"))),
      timer(opts.get<double>("max_time")),
      init(nullptr),
      deviations(0),
      unmet_preconditions(0),
      unmet_goals(0) {
    g_log << "Start building abstraction." << endl;
    build();
    g_log << "Done building abstraction." << endl;

    /* Even if we found a concrete solution, we might have refined in the
       last iteration, so we should update the h values. */
    update_h_values();

    print_statistics();
}

Abstraction::~Abstraction() {
    for (AbstractState *state : states)
        delete state;
}

bool Abstraction::is_goal(AbstractState *state) const {
    return goals.count(state) == 1;
}

void Abstraction::separate_unreachable_facts() {
    assert(states.size() == 1);
    assert(task_proxy.get_goals().size() == 1);
    FactProxy goal = task_proxy.get_goals()[0];
    unordered_set<FactProxy> reachable_facts = get_relaxed_possible_before(
        task_proxy, goal);
    for (VariableProxy var : task_proxy.get_variables()) {
        int var_id = var.get_id();
        vector<int> unreachable_values;
        for (int value = 0; value < var.get_domain_size(); ++value) {
            FactProxy fact = var.get_fact(value);
            if (reachable_facts.count(fact) == 0)
                unreachable_values.push_back(value);
        }
        if (!unreachable_values.empty())
            refine(init, var_id, unreachable_values);
    }
    goals.insert(states.begin(), states.end());
}

void Abstraction::create_trivial_abstraction() {
    init = AbstractState::get_trivial_abstract_state(task_proxy, split_tree.get_root());
    goals.insert(init);
    for (OperatorProxy op : task_proxy.get_operators()) {
        init->add_loop(op);
    }
    states.insert(init);
    if (do_separate_unreachable_facts)
        separate_unreachable_facts();
}

bool Abstraction::may_keep_refining() const {
    return Utils::extra_memory_padding_is_reserved() &&
           get_num_states() < max_states &&
           !timer.is_expired();
}

void Abstraction::build() {
    create_trivial_abstraction();
    bool found_conc_solution = false;
    while (may_keep_refining()) {
        bool found_abs_solution = abstract_search.find_solution(init, goals);
        if (!found_abs_solution) {
            cout << "Abstract problem is unsolvable!" << endl;
            break;
        }
        unique_ptr<Flaw> flaw = find_flaw(abstract_search.get_solution());
        if (!flaw) {
            found_conc_solution = true;
            break;
        }
        AbstractState *abs_state = flaw->current_abstract_state;
        vector<Split> splits = flaw->get_possible_splits();
        const Split &split = split_selector.pick_split(*abs_state, splits);
        refine(abs_state, split.var_id, split.values);
    }
    cout << "Concrete solution found: " << found_conc_solution << endl;
}

void Abstraction::refine(AbstractState *state, int var, const vector<int> &wanted) {
    if (DEBUG)
        cout << "Refine " << *state << " for " << var << "=" << wanted << endl;
    pair<AbstractState *, AbstractState *> new_states = state->split(var, wanted);
    AbstractState *v1 = new_states.first;
    AbstractState *v2 = new_states.second;

    states.erase(state);
    states.insert(v1);
    states.insert(v2);

    /* Since the search is always started from the abstract initial state, v2
       is never the new initial state and v1 is never a goal state. */
    if (state == init) {
        assert(v1->includes(task_proxy.get_initial_state()));
        assert(!v2->includes(task_proxy.get_initial_state()));
        init = v1;
        if (DEBUG)
            cout << "New init state: " << *init << endl;
    }
    if (is_goal(state)) {
        goals.erase(state);
        goals.insert(v2);
        if (DEBUG)
            cout << "New/additional goal state: " << *v2 << endl;
    }

    int num_states = get_num_states();
    if (num_states % STATES_LOG_STEP == 0)
        g_log << "Abstract states: " << num_states << "/" << max_states << endl;

    delete state;
}

unique_ptr<Flaw> Abstraction::find_flaw(const Solution &solution) {
    if (DEBUG)
        cout << "Check solution:" << endl;

    AbstractState *abs_state = init;
    State conc_state = task_proxy.get_initial_state();
    assert(abs_state->includes(conc_state));

    if (DEBUG)
        cout << "  Initial abstract state: " << *abs_state << endl;

    for (auto &step : solution) {
        if (!Utils::extra_memory_padding_is_reserved())
            break;
        const OperatorProxy op = step.first;
        AbstractState *next_abs_state = step.second;
        if (is_applicable(op, conc_state)) {
            if (DEBUG)
                cout << "  Move to " << *next_abs_state << " with "
                     << op.get_name() << endl;
            State next_conc_state = move(conc_state.get_successor(op));
            if (!next_abs_state->includes(next_conc_state)) {
                if (DEBUG)
                    cout << "  Paths deviate." << endl;
                ++deviations;
                return Utils::make_unique_ptr<Flaw>(
                    move(conc_state),
                    abs_state,
                    next_abs_state->regress(op));
            }
            abs_state = next_abs_state;
            conc_state = move(next_conc_state);
        } else {
            if (DEBUG)
                cout << "  Operator not applicable: " << op.get_name() << endl;
            ++unmet_preconditions;
            return Utils::make_unique_ptr<Flaw>(
                move(conc_state),
                abs_state,
                AbstractState::get_abstract_state(
                    task_proxy, op.get_preconditions()));
        }
    }
    assert(is_goal(abs_state));
    if (is_goal_state(task_proxy, conc_state)) {
        // We found a concrete solution.
        return nullptr;
    } else {
        if (DEBUG)
            cout << "  Goal test failed." << endl;
        ++unmet_goals;
        return Utils::make_unique_ptr<Flaw>(
            move(conc_state),
            abs_state,
            AbstractState::get_abstract_state(
                task_proxy, task_proxy.get_goals()));
    }
}

void Abstraction::update_h_values() {
    abstract_search.backwards_dijkstra(goals);
    for (AbstractState *state : states) {
        state->set_h_value(abstract_search.get_g_value(state));
    }
}

int Abstraction::get_h_value_of_initial_state() const {
    return init->get_h_value();
}

vector<int> Abstraction::get_needed_costs() {
    return abstract_search.get_needed_costs(init, task_proxy.get_operators().size());
}

void Abstraction::print_statistics() {
    int total_incoming_arcs = 0;
    int total_outgoing_arcs = 0;
    int total_loops = 0;
    int dead_ends = 0;
    for (AbstractState *state : states) {
        if (state->get_h_value() == INF)
            ++dead_ends;
        const Arcs &incoming_arcs = state->get_incoming_arcs();
        const Arcs &outgoing_arcs = state->get_outgoing_arcs();
        const Loops &loops = state->get_loops();
        total_incoming_arcs += incoming_arcs.size();
        total_outgoing_arcs += outgoing_arcs.size();
        total_loops += loops.size();
    }
    assert(total_outgoing_arcs == total_incoming_arcs);

    int total_cost = 0;
    for (OperatorProxy op : task_proxy.get_operators())
        total_cost += op.get_cost();

    cout << "Time for building abstraction: " << timer << endl;
    cout << "Total operator cost: " << total_cost << endl;
    cout << "States: " << get_num_states() << endl;
    cout << "Dead ends: " << dead_ends << endl;
    cout << "Init h: " << get_h_value_of_initial_state() << endl;

    cout << "Transitions: " << total_incoming_arcs << endl;
    cout << "Self-loops: " << total_loops << endl;

    cout << "Deviations: " << deviations << endl;
    cout << "Unmet preconditions: " << unmet_preconditions << endl;
    cout << "Unmet goals: " << unmet_goals << endl;
}
}
