//
// Created by yannick on 07.08.20.
//

#include "psvn_split_factory.h"
#include <cmath>

namespace PSVNSplitFactory{

    PSVNSplitFactory::PSVNSplitFactory(const TaskProxy &task_proxy, int lim) :  task_proxy(task_proxy) {
        list_limit = lim;
    }
    PSVNSplitFactory::~PSVNSplitFactory() = default;

    vector<vector<Vertex>> PSVNSplitFactory::create() {


        /**
        int split_factor = 1;
        dag_too_big = false;
        do {
            vertex_lists.resize(split_factor);
            dag_too_big = false;

            for (int i = 0; i < split_factor; i++) {
                map.clear();
                vertex_lists[i].clear();
                stop_all_recursion = false;
                bot = floor(i * task_proxy.get_operators().size() / split_factor);
                top = floor((i + 1) * task_proxy.get_operators().size() / split_factor) - 1;

                vector<int> test_set(task_proxy.get_variables().size(), -1);
                vector<int> plausible_rules;

                for (int j = bot; j < top; j++) {
                    plausible_rules.push_back(task_proxy.get_operators()[j].get_id());
                }

                Vertex v(plausible_rules, test_set, vector<int>());
                vertex_lists[i].push_back(v);
                map.insert(make_pair(v.hash, 0));
                create_DAG_recursive(i, 0);

                if (vertex_lists[i].size() > list_limit) {
                    stop_all_recursion = true;
                    dag_too_big = true;
                    break;
                }
                plausible_rules.clear();
                test_set.clear();
            }

            if (!dag_too_big) {
                for (int i = 0; i < vertex_lists.size(); i++) {
                    if (vertex_lists[i].size() > list_limit) {
                        dag_too_big = true;
                    }
                }
            }
            split_factor *= 2;

        }while (dag_too_big);
        return vertex_lists;
         **/
    }

    void PSVNSplitFactory::create_DAG_recursive(int list_nr, int pos) {
/**

        if (vertex_lists[list_nr][pos].plausible_operators.size() > 0) {

            if (vertex_lists[list_nr][pos].choose_test(task_proxy.get_operators())) {

                for (int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[vertex_lists[list_nr][pos].choice].get_domain_size(); domain_iterator++) {
                    vector<int> temp_tests = vertex_lists[list_nr][pos].test_results;
                    temp_tests[vertex_lists[list_nr][pos].choice] = domain_iterator;
                    vector<int> temp_rules = vertex_lists[list_nr][pos].plausible_operators;
                    vector<int> temp_sat_rules = vertex_lists[list_nr][pos].satisfied_operators;

                    split_and_simplify(temp_rules, temp_tests, temp_sat_rules);
                    Vertex v(temp_rules, temp_tests, temp_sat_rules);

                    if(map.find(v.hash) == map.end()){ // not in hashmap
                        vertex_lists[list_nr].push_back(v);
                        vertex_lists[list_nr][pos].children.push_back(int(vertex_lists[list_nr].size())-1);
                        map.insert(make_pair(v.hash, vertex_lists[list_nr].size()-1));
                        create_DAG_recursive(list_nr, int(vertex_lists[list_nr].size())-1);
                    }else{
                        vertex_lists[list_nr][pos].children.push_back(map.find(v.hash)->second);
                    }
                }
            }
        }
        **/
    }

    void PSVNSplitFactory::split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules) {

        vector<bool> visited_vars(tests.size(), false);
        vector<int> new_rules;

        for(int rule_id : rules){
            int precon_counter = 0;
            bool unsat = false;

            for(FactProxy fact : task_proxy.get_operators()[rule_id].get_preconditions()){
                visited_vars[fact.get_variable().get_id()] = true;

                if(tests[fact.get_variable().get_id()] == fact.get_value()){ // if the precon is satisfied
                    precon_counter++;
                }else if(tests[fact.get_variable().get_id()] != -1){ // if not satisfied and var has been set
                    unsat = true;
                    break;
                }
            }

            if(!unsat){
                if(precon_counter == int(task_proxy.get_operators()[rule_id].get_preconditions().size())){
                    sat_rules.push_back(rule_id);
                }else{
                    new_rules.push_back(rule_id);
                }
            }
        }

        for(int test_iterator = 0; test_iterator < int(tests.size()); test_iterator++){
            if(!visited_vars[test_iterator]){ // if it has not been visited
                tests[test_iterator] = -2;
            }
        }

        rules.clear();
        for(int i : new_rules){
            rules.push_back(i);
        }
    }
}