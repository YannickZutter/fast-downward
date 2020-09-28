//
// Created by yannick on 07.08.20.
//

#include "psvn_factory.h"

namespace PSVNFactory{

    PSVNFactory::PSVNFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNFactory::~PSVNFactory() = default;

    vector<Vertex> PSVNFactory::create() {

        // in the test set all variables start with value -1 for undefined
        vector<int> test_set(task_proxy.get_variables().size(), -1);
        vector<int> plausible_rules;

        for(OperatorProxy op : task_proxy.get_operators()){
            plausible_rules.push_back(op.get_id()); //at beginning all operators are plausible
        }

        Vertex vertex(plausible_rules, test_set);
        vertex_list.push_back(vertex);

        map.insert(make_pair( vertex.hash, vertex_list.size()-1));
        create_DAG_recursive(0);
        return vertex_list;
    }

    void PSVNFactory::create_DAG_recursive(int pos) {

        if (!vertex_list[pos].rules.empty()) {

            if (vertex_list[pos].choose_test(task_proxy.get_operators())) {

                for (int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[vertex_list[pos].choice].get_domain_size(); domain_iterator++) {
                    vector<int> temp_tests = vertex_list[pos].test_results;
                    temp_tests[vertex_list[pos].choice] = domain_iterator;

                    vector<int> temp_rules = vertex_list[pos].rules;
                    vector<int> temp_sat_rules = vertex_list[pos].satisfied_rules;

                    split_and_simplify(temp_rules, temp_tests, temp_sat_rules);

                    Vertex v(temp_rules, temp_tests, temp_sat_rules);

                    if(map.find(v.hash) == map.end()){ // not in hashmap
                        vertex_list.push_back(v);
                        vertex_list[pos].add_child(int(vertex_list.size())-1);
                        map.insert(make_pair(v.hash, vertex_list.size()-1));
                        create_DAG_recursive(int(vertex_list.size())-1);
                    }else{
                        vertex_list[pos].add_child(map.find(v.hash)->second);
                    }
                }
            }
        }
    }

    void PSVNFactory::split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules) {

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