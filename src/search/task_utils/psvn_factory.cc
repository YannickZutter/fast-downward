//
// Created by yannick on 07.08.20.
//

#include "psvn_factory.h"
#include <math.h>

namespace PSVNFactory{

    PSVNFactory::PSVNFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNFactory::~PSVNFactory() = default;

    vector<vector<Vertex>> PSVNFactory::create() {

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }

        int split_factor = 1;
        bool status = true;
        status = split_and_create(split_factor);

        while (!status){
            split_factor++;
            status = split_and_create(split_factor);
        }

        return list_of_lists;
    }

    bool PSVNFactory::split_and_create(int split_factor){
        cout << "\nsplit factor of: "<<split_factor;
/**
        for(int i = 0; i < split_factor; i++){
            vector<Vertex> a;
            list_of_lists.push_back(a);
            utils::HashMap<int, int> m;
            maps.push_back(m);

            vector<int> test_set(task_proxy.get_variables().size(), -1);
            vector<int> plausible_rules;
            int bot = floor(i*operators.size()/split_factor);
            int top = floor((i+1)*operators.size()/split_factor);
            for(int j = bot; j < top; j++){
                plausible_rules.push_back(operators[j].get_id());
            }

            Vertex vertex(plausible_rules, test_set);
            list_of_lists[i].push_back(vertex);
            maps[i].insert(make_pair(vertex.hash, list_of_lists[i].size()-1));

            bool status = true;
            status = create_DAG_recursive(0, i);

            if(!status){
                list_of_lists.clear();
                maps.clear();
                return false;

            }
        }
        **/
        return true;
    }

    bool PSVNFactory::create_DAG_recursive(int pos, int list_pos) {

        if(list_of_lists[list_pos].size() > 5000000){
            return false;
        }

        if (list_of_lists[pos][list_pos].rules.empty() > 0) {

            if (list_of_lists[pos][list_pos].choose_test(operators)) {

                for (int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[list_of_lists[pos][list_pos].choice].get_domain_size(); domain_iterator++) {
                    vector<int> temp_tests = list_of_lists[pos][list_pos].test_results;
                    temp_tests[list_of_lists[pos][list_pos].choice] = domain_iterator;

                    vector<int> temp_rules = list_of_lists[pos][list_pos].rules;
                    vector<int> temp_sat_rules = list_of_lists[pos][list_pos].satisfied_rules;

                    split_and_simplify(temp_rules, temp_tests, temp_sat_rules);

                    Vertex v(temp_rules, temp_tests, temp_sat_rules);

                    if(maps[list_pos].find(v.hash) == maps[list_pos].end()){ // not in hashmap
                        list_of_lists[list_pos].push_back(v);
                        list_of_lists[pos][list_pos].add_child(int(list_of_lists[list_pos].size())-1);
                        maps[list_pos].insert(make_pair(v.hash, list_of_lists[list_pos].size()-1));
                        create_DAG_recursive(int(list_of_lists[list_pos].size())-1, list_pos);
                    }else{
                        list_of_lists[pos][list_pos].add_child(maps[list_pos].find(v.hash)->second);
                    }
                }
            }
        }
    }

    void PSVNFactory::split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules) {

        vector<bool> visited_vars(tests.size(), false);
        vector<int> new_rules;

        for(int rule_id : rules){
            int precondition_counter = 0;
            bool unsatisfied = false;

            for(FactProxy fact : operators[rule_id].get_preconditions()){
                visited_vars[fact.get_variable().get_id()] = true;

                if(tests[fact.get_variable().get_id()] == fact.get_value()){ // if the precon is satisfied
                    precondition_counter++;
                }else if(tests[fact.get_variable().get_id()] != -1){ // if not satisfied and var has been set
                    unsatisfied = true;
                    break;
                }
            }

            if(!unsatisfied){
                if(precondition_counter == int(operators[rule_id].get_preconditions().size())){
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