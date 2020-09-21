//
// Created by yannick on 07.08.20.
//

#include "psvn_split_factory.h"
#include <cmath>

namespace PSVNSplitFactory{

    PSVNSplitFactory::PSVNSplitFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNSplitFactory::~PSVNSplitFactory() = default;

    vector<vector<Vertex>> PSVNSplitFactory::create() {

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }
        int split_factor = 1;

        while(dag_too_big){
            vertex_lists.resize(split_factor);
            dag_too_big = false;

            for(int i = 0; i < split_factor; i++){

                map.clear();
                vertex_lists[i].clear();
                stop_all_recursion = false;

                bot = floor(i*operators.size()/split_factor);
                top = floor((i+1)*operators.size()/split_factor)-1;
                cout << "\nbot/top: (" << bot << "/" << top<<") with total of "<<operators.size()<<" operators";
                vector<int> test_set(task_proxy.get_variables().size(), -1);
                vector<int> plausible_rules;

                for(int j = bot; j < top; j++){
                    plausible_rules.push_back(operators[j].get_id());
                }

                Vertex v(plausible_rules, test_set);
                vertex_lists[i].push_back(v);
                map.insert(make_pair(v.hash, vertex_lists[i].size()-1));

                create_DAG_recursive(i, 0);

                if(int(vertex_lists[i].size()) > max_dag_val){
                    cout << "\nvertex list " <<i <<" too big: " <<vertex_lists[i].size();
                    stop_all_recursion = true;
                    break;
                }
            }

            split_factor *=2;
            for(auto & vertex_list : vertex_lists){
                if(int(vertex_list.size()) > max_dag_val){
                    dag_too_big = true;
                    break;
                }
            }
        }
        return vertex_lists;
    }

    void PSVNSplitFactory::create_DAG_recursive(int list_nr, int pos) {

        if(!stop_all_recursion && int(vertex_lists[list_nr].size()) < max_dag_val) {
            if (!vertex_lists[list_nr][pos].rules.empty()) {

                if (vertex_lists[list_nr][pos].choose_test(operators)) {

                    for (int domain_iterator = 0; domain_iterator <
                                                  task_proxy.get_variables()[vertex_lists[list_nr][pos].choice].get_domain_size(); domain_iterator++) {
                        vector<int> temp_tests = vertex_lists[list_nr][pos].test_results;
                        temp_tests[vertex_lists[list_nr][pos].choice] = domain_iterator;

                        vector<int> temp_rules = vertex_lists[list_nr][pos].rules;
                        vector<int> temp_sat_rules = vertex_lists[list_nr][pos].satisfied_rules;

                        split_and_simplify(temp_rules, temp_tests, temp_sat_rules);

                        Vertex v(temp_rules, temp_tests, temp_sat_rules);

                        if (map.find(v.hash) == map.end()) { // not in hashmap
                            vertex_lists[list_nr].push_back(v);
                            vertex_lists[list_nr][pos].add_child(int(vertex_lists[list_nr].size()) - 1);
                            map.insert(make_pair(v.hash, vertex_lists[list_nr].size() - 1));
                            create_DAG_recursive(list_nr, int(vertex_lists[list_nr].size()) - 1);
                        } else {
                            vertex_lists[list_nr][pos].add_child(map.find(v.hash)->second);
                        }
                    }
                }
            }
        }




        /**
        if(!stop_all_recursion && int(vertex_lists[list_nr].size()) < max_dag_val){

            if(!vertex_lists[list_nr][pos].rules.empty()){

                if (vertex_lists[list_nr][pos].choose_test(operators)) {

                    for (int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[vertex_lists[list_nr][pos].choice].get_domain_size(); domain_iterator++) {
                        vector<int> temp_tests = vertex_lists[list_nr][pos].test_results;
                        temp_tests[vertex_lists[list_nr][pos].choice] = domain_iterator;

                        vector<int> temp_rules = vertex_lists[list_nr][pos].rules;
                        vector<int> temp_sat_rules = vertex_lists[list_nr][pos].satisfied_rules;

                        split_and_simplify(temp_rules, temp_tests, temp_sat_rules);

                        Vertex v(temp_rules, temp_tests, temp_sat_rules);

                        if (map.find(v.hash) == map.end()) { // not in hashmap
                            vertex_lists[list_nr].push_back(v);
                            vertex_lists[list_nr][pos].add_child(int(vertex_lists[list_nr].size()) - 1);
                            map.insert(make_pair(v.hash, vertex_lists[list_nr].size() - 1));
                            create_DAG_recursive(list_nr, int(vertex_lists[list_nr].size()) - 1);
                        } else {
                            vertex_lists[list_nr][pos].add_child(map.find(v.hash)->second);
                        }
                    }
                }
            }
        }else{
            stop_all_recursion = true;
        }
         **/
    }

    void PSVNSplitFactory::split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules) {

        vector<bool> visited_vars(tests.size(), false);
        vector<int> new_rules;

        for(int rule_id : rules){
            int precon_counter = 0;
            bool unsat = false;

            for(FactProxy fact : operators[rule_id].get_preconditions()){
                visited_vars[fact.get_variable().get_id()] = true;

                if(tests[fact.get_variable().get_id()] == fact.get_value()){ // if the precon is satisfied
                    precon_counter++;
                }else if(tests[fact.get_variable().get_id()] != -1){ // if not satisfied and var has been set
                    unsat = true;
                    break;
                }
            }

            if(!unsat){
                if(precon_counter == int(operators[rule_id].get_preconditions().size())){
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