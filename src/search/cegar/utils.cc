#include "utils.h"

#include <cassert>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>

#include "../globals.h"
#include "../operator.h"
#include "../state.h"

using namespace std;

namespace cegar_heuristic {
bool DEBUG = false;

Operator create_op(const string desc) {
    istringstream iss("begin_operator\n" + desc + "\nend_operator");
    return Operator(iss, false);
}

Operator create_op(const string name, vector<string> prevail, vector<string> pre_post, int cost) {
    ostringstream oss;
    // Create operator description.
    oss << name << endl << prevail.size() << endl;
    for (int i = 0; i < prevail.size(); ++i)
        oss << prevail[i] << endl;
    oss << pre_post.size() << endl;
    for (int i = 0; i < pre_post.size(); ++i)
        oss << pre_post[i] << endl;
    oss << cost;
    return create_op(oss.str());
}

State *create_state(const string desc) {
    string full_desc = "begin_state\n" + desc + "\nend_state";
    istringstream iss(full_desc);
    return new State(iss);
}

int get_pre(const Operator &op, int var) {
    for (int i = 0; i < op.get_prevail().size(); i++) {
        const Prevail &prevail = op.get_prevail()[i];
        if (prevail.var == var)
            return prevail.prev;
    }
    for (int i = 0; i < op.get_pre_post().size(); i++) {
        const PrePost &pre_post = op.get_pre_post()[i];
        if (pre_post.var == var)
            return pre_post.pre;
    }
    return UNDEFINED;
}

int get_eff(const Operator &op, int var) {
    for (int i = 0; i < op.get_prevail().size(); ++i) {
        const Prevail &prevail = op.get_prevail()[i];
        if (prevail.var == var)
            return prevail.prev;
    }
    for (int i = 0; i < op.get_pre_post().size(); ++i) {
        const PrePost &pre_post = op.get_pre_post()[i];
        if (pre_post.var == var)
            return pre_post.post;
    }
    return UNDEFINED;
}

void get_unmet_preconditions(const Operator &op, const State &state,
                             vector<pair<int, int> > *cond) {
    assert(cond->empty());
    for (int i = 0; i < op.get_prevail().size(); ++i) {
        const Prevail &prevail = op.get_prevail()[i];
        if (state[prevail.var] != prevail.prev)
            cond->push_back(pair<int,int>(prevail.var, prevail.prev));
    }
    for (int i = 0; i < op.get_pre_post().size(); ++i) {
        const PrePost &pre_post = op.get_pre_post()[i];
        if ((pre_post.pre != -1) && (state[pre_post.var] != pre_post.pre))
            cond->push_back(pair<int,int>(pre_post.var, pre_post.pre));
    }
    assert(cond->empty() == op.is_applicable(state));
}

void get_unmet_goal_conditions(const State &state,
                               vector<pair<int, int> > *unmet_conditions) {
    for (int i = 0; i < g_goal.size(); i++) {
        int var = g_goal[i].first;
        int value = g_goal[i].second;
        if (state[var] != value) {
            unmet_conditions->push_back(pair<int, int>(var, value));
        }
    }
}

bool goal_var(int var) {
    for (int i = 0; i < g_goal.size(); i++) {
        if (var == g_goal[i].first)
            return true;
    }
    return false;
}

void partial_ordering(const CausalGraph &causal_graph, vector<int> *order) {
    assert(order->empty());
    // Set of variables that still have to be ordered.
    set<int> vars;
    set<int>::iterator it;
    for (int i = 0; i < g_variable_domain.size(); ++i) {
        vars.insert(vars.end(), i);
    }
    // For each variable, maintain sets of predecessor and successor variables
    // that haven't been ordered yet.
    vector<set<int> > predecessors;
    vector<set<int> > successors;
    predecessors.resize(g_variable_domain.size());
    successors.resize(g_variable_domain.size());
    for (int var = 0; var < g_variable_domain.size(); ++var) {
        const vector<int> &pre = causal_graph.get_predecessors(var);
        for (int i = 0; i < pre.size(); ++i) {
            predecessors[var].insert(pre[i]);
        }
        const vector<int> &succ = causal_graph.get_successors(var);
        for (int i = 0; i < succ.size(); ++i) {
            successors[var].insert(succ[i]);
        }
    }
    while (!vars.empty()) {
        int min_pre = g_variable_domain.size() + 1;
        int max_succ = -1;
        int var = -1;
        for (it = vars.begin(); it != vars.end(); ++it) {
            set<int> &pre = predecessors[*it];
            set<int> &succ = successors[*it];
            assert(pre.size() <= g_variable_domain.size());
            assert(succ.size() <= g_variable_domain.size());
            if (DEBUG) {
                cout << "pre(" << *it << "): ";
                for (set<int>::iterator p = pre.begin(); p != pre.end(); ++p)
                    cout << *p << " ";
                cout << "(" << succ.size() << " succ)" << endl;
            }
            if ((pre.size() < min_pre) || ((pre.size() == min_pre) && (succ.size() > max_succ))) {
                var = *it;
                min_pre = pre.size();
                max_succ = succ.size();
            }
        }
        assert(var >= 0);
        if (DEBUG)
            cout << "Choose " << var << endl << endl;
        order->push_back(var);
        vars.erase(var);
        // For all unsorted variables, delete var from their predecessor and
        // successor lists.
        for (it = vars.begin(); it != vars.end(); ++it) {
            set<int> &pre = predecessors[*it];
            set<int>::iterator pos = find(pre.begin(), pre.end(), var);
            if (pos != pre.end())
                pre.erase(pos);
            set<int> &succ = successors[*it];
            pos = find(succ.begin(), succ.end(), var);
            if (pos != succ.end())
                succ.erase(pos);
        }
    }
    assert(order->size() == g_variable_domain.size());
}

void write_causal_graph(const CausalGraph &causal_graph) {
    ofstream dotfile("causal-graph.dot");
    if (!dotfile.is_open()) {
        cout << "dot file for causal graph could not be opened" << endl;
        exit(1);
    }
    dotfile << "digraph cg {" << endl;
    for (int var = 0; var < g_variable_domain.size(); ++var) {
        const vector<int> &successors = causal_graph.get_successors(var);
        for (int i = 0; i < successors.size(); ++i) {
            dotfile << "  " << var << " -> " << successors[i] << ";" << endl;
        }
    }
    for (int i = 0; i < g_goal.size(); i++) {
        int var = g_goal[i].first;
        dotfile << var << " [color=red];" << endl;
    }
    dotfile << "}" << endl;
    dotfile.close();
}

void pick_condition_for_each_var(vector<pair<int, int> > *conditions) {
    set<int> used_vars;
    vector<Condition> picked_conditions;
    for (int cond = 0; cond < conditions->size(); ++cond) {
        int &var = (*conditions)[cond].first;
        int &value = (*conditions)[cond].second;
        if (used_vars.count(var) == 0) {
            picked_conditions.push_back(Condition(var, value));
            used_vars.insert(var);
        }
    }
    conditions->swap(picked_conditions);
}

void print_conditions(const vector<pair<int, int> > &conditions) {
    string separator = "";
    for (int cond = 0; cond < conditions.size(); ++cond) {
        int var = conditions[cond].first;
        int value = conditions[cond].second;
        cout << separator << var << "=" << value;
        separator = ", ";
    }
    cout << endl;
}

int get_memory_in_kb(const string& type) {
    // On error, produces a warning on cerr and returns -1.
    int memory_in_kb = -1;

#ifdef __APPLE__
    // Based on http://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
    task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (task_info(mach_task_self(), TASK_BASIC_INFO,
                  reinterpret_cast<task_info_t>(&t_info),
                  &t_info_count) == KERN_SUCCESS)
        memory_in_kb = t_info.virtual_size / 1024;
#else
    ostringstream filename_stream;
    filename_stream << "/proc/" << getpid() << "/status";
    const char *filename = filename_stream.str().c_str();

    ifstream procfile(filename);
    string word;
    while (procfile.good()) {
        procfile >> word;
        if (word == (type + ":")) {
            procfile >> memory_in_kb;
            break;
        }
        // Skip to end of line.
        procfile.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    if (procfile.fail())
        memory_in_kb = -1;
#endif

    if (memory_in_kb == -1)
        cerr << "warning: could not determine peak memory" << endl;
    return memory_in_kb;
}

#include <sstream>

string to_string(int i) {
    stringstream out;
    out << i;
    return out.str();
}

}
