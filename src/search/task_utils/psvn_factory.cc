//
// Created by yannick on 07.08.20.
//

#include "psvn_factory.h"

namespace PSVNFactory{

    PSVNFactory::PSVNFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNFactory::~PSVNFactory() = default;

    vector<Vertex> PSVNFactory::create() {

        vector<int> test_set(task_proxy.get_variables().size(), -1);
        vector<int> plausible_rules;

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
            plausible_rules.push_back(op.get_id());

        }

        Vertex vertex(plausible_rules, test_set);

        vertex_list.push_back(vertex);
        create_DAG_recursive(0);
        cout << "\nvertex list size: " << vertex_list.size();
        return vertex_list;
    }

    void PSVNFactory::create_DAG_recursive(int pos) {

        int rule_counter = 0;
        for (int id : vertex_list[pos].rules) {
            if (id != -1) {
                rule_counter++;
            }
        }

        if (rule_counter != 0) {

            if (vertex_list[pos].choose_test(operators)) {

                for (int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[vertex_list[pos].choice].get_domain_size(); domain_iterator++) {
                    vector<int> temp_tests = vertex_list[pos].test_results;
                    temp_tests[vertex_list[pos].choice] = domain_iterator;

                    vector<int> temp_rules = vertex_list[pos].rules;
                    vector<int> temp_sat_rules = vertex_list[pos].satisfied_rules;

                    split_and_simplify(temp_rules, temp_tests, temp_sat_rules);

                    Vertex v(temp_rules, temp_tests, temp_sat_rules);

                    int existence = check_existence(v);


                    if (existence != -1) {
                        vertex_list[pos].children.push_back(existence);

                    } else {

                        vertex_list.push_back(v);
                        vertex_list[pos].children.push_back(vertex_list.size()-1);

                        create_DAG_recursive(int(vertex_list.size())-1);
                    }
                }
            } else{
                cout << "\nerror while finding choice";
            }
        }else{
            cout <<"error, counter is 0";
        }
    }

    void PSVNFactory::split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules) {

        vector<bool> visited_vars(tests.size(), false);
        for(int rule_id : rules){
            if(rule_id != -1){
                int precon_counter = 0;

                bool unsat = false;

                for(FactProxy fact : operators[rule_id].get_preconditions()){
                    visited_vars[fact.get_pair().var] = true;
                    if(fact.get_value() == tests[fact.get_pair().var]){
                        precon_counter++;
                    }else if(tests[fact.get_variable().get_id() != -1]){
                        unsat = true;
                    }
                }
                if(unsat){
                   rules[rule_id] = -1;
                } else if(precon_counter == int(operators[rule_id].get_preconditions().size())){
                    sat_rules[rule_id] = rule_id;

                    rules[rule_id] = -1;
                }
            }
        }

        for(int test_iterator = 0; test_iterator < int(tests.size()); test_iterator++){
            if(!visited_vars[test_iterator]){
                tests[test_iterator] = -2;

            }
        }

        cout <<"\n split and simplify: rule:";
        for(int i : rules){
            cout << i << ", ";
        }
        cout << "\ntests: ";
        for(int i : tests){
            cout << i << ", ";
        }
    }

    /**
     * return position in vertex_list if vertex already exists, or -1 if not
     * @param vertex
     * @return position in vertex_list or -1 else
     */
    // TODO: change this to hash comparison
    int PSVNFactory::check_existence(const Vertex& vertex) {

        for(int i = 0; i < int(vertex_list.size()); i++){
            if(vertex == vertex_list[i]){
                return i;
            }
        }
        return -1;
    }

}