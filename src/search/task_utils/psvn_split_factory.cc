//
// Created by yannick on 07.08.20.
//

#include "psvn_split_factory.h"
#include <math.h>

namespace PSVNSplitFactory{

    PSVNSplitFactory::PSVNSplitFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNSplitFactory::~PSVNSplitFactory() = default;

    vector<vector<Vertex>> PSVNSplitFactory::create() {

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
        }
        int split_factor = 1;
        cout << "\n go in while loop";

        do {
            vertex_lists.resize(split_factor);

            for(int i = 0; i < split_factor; i++){

                bot = floor(i*operators.size()/split_factor);
                top = floor((i+1)*operators.size()/split_factor-1);

                cout <<"\nfor " << i << " and operator size "<<operators.size()<< ", bot is: " << bot << " and top is: " << top;

                vector<int> test_set(task_proxy.get_variables().size(), -1);
                vector<int> plausible_rules;

                for(int j = bot; j < top; j++){
                    plausible_rules.push_back(operators[j].get_id());
                }

                Vertex vertex(plausible_rules, test_set);
                vertex_lists[i].push_back(vertex);

                map.insert(make_pair(vertex.hash, vertex_lists[i].size()-1));

                dag_too_big = create_DAG_recursive(i, 0);

            }

            split_factor *= 2;

        }while (dag_too_big);


        return vertex_lists;

    }

    bool PSVNSplitFactory::create_DAG_recursive(int list_nr, int pos) {




        if(!vertex_lists[list_nr][pos].rules.empty()){

            if(vertex_lists[list_nr][pos].choose_test(vector<OperatorProxy>(&operators[bot], &operators[top]))){

                for (int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[vertex_lists[list_nr][pos].choice].get_domain_size(); domain_iterator++) {
                    vector<int> temp_tests = vertex_lists[list_nr][pos].test_results;
                    temp_tests[vertex_lists[list_nr][pos].choice] = domain_iterator;

                    vector<int> temp_rules = vertex_lists[list_nr][pos].rules;
                    vector<int> temp_sat_rules = vertex_lists[list_nr][pos].satisfied_rules;

                    split_and_simplify(temp_rules, temp_tests, temp_sat_rules);

                    Vertex v(temp_rules, temp_tests, temp_sat_rules);

                    if(map.find(v.hash) == map.end()){ // not in hashmap
                        vertex_lists[list_nr].push_back(v);
                        vertex_lists[list_nr][pos].add_child(int(vertex_lists[list_nr].size())-1);
                        map.insert(make_pair(v.hash, vertex_lists[list_nr].size()-1));
                        create_DAG_recursive(list_nr, int(vertex_lists[list_nr].size())-1);
                    }else{
                        vertex_lists[list_nr][pos].add_child(map.find(v.hash)->second);
                    }
                }
            }
        }

        return false;
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