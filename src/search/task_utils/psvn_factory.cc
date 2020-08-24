//
// Created by yannick on 07.08.20.
//

#include "psvn_factory.h"

namespace PSVNFactory{

    PSVNFactory::PSVNFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNFactory::~PSVNFactory() = default;

    vector<Vertex> PSVNFactory::create() {

        vector<int> test_set(task_proxy.get_operators().size(), -1);
        cout << "\nsize of taskproxy variables: "<< task_proxy.get_variables().size() <<"\n";
        vector<int> plausible_rules;
        //vector<int> plausible_rules(task_proxy.get_variables().size());

        for(OperatorProxy op : task_proxy.get_operators()){
            operators.push_back(op);
            //plausible_rules[op.get_id()] = op.get_id();
        }

        Vertex vertex(plausible_rules, test_set);
        create_DAG_recursive(vertex);

        return vertex_list;



    }

    void PSVNFactory::create_DAG_recursive(Vertex vertex) {

        //vertex_list.push_back(vertex);
        int rule_counter = 0;
        for(int id : vertex.rules){
            if(vertex.rules[id] != -1){
                rule_counter++;
            }
        }
        if(rule_counter != 0){

            vertex.choose_test(task_proxy.get_variables());
/**
            for(int i = 0; i < task_proxy.get_variables()[vertex.choice].get_domain_size(); i++){
                vector<int> temp_tests;
                temp_tests = vertex.test_results;
                temp_tests[vertex.choice] = i;

                vector<int> temp_rules = vertex.rules;

                vector<int> temp_sat_rules = vertex.satisfied_rules;

                split_and_simplify(temp_rules, temp_tests, temp_sat_rules);

                Vertex v(temp_rules, temp_tests, temp_sat_rules);

                int existence = check_existence(v);
                if(existence != -1){
                    vertex.children.push_back(existence);
                }else{
                    vertex.children.push_back(vertex_list.size()+1);
                    create_DAG_recursive(v);
                }
            }
**/
        }

    }

    void PSVNFactory::split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules) {

        /**

        vector<int> temp_rules(rules.size(), -1);
        vector<int> temp_tests(tests.size(), -1);
        vector<bool> visited_tests(tests.size(), false);

        for(int rule_id : rules){
            int precondition_counter = 0;
            bool unsatisfied = false;
            OperatorProxy op = operators[rule_id];
            int dom_size = op.get_preconditions().size();
            for(int precondition_iterator = 0; precondition_iterator < dom_size; precondition_iterator++){

                FactPair pair = op.get_preconditions()[precondition_iterator].get_pair();
                if(!visited_tests[pair.var]){
                    visited_tests[pair.var] = true;
                }
                if(tests[pair.var] == pair.value){
                    precondition_counter++;
                }else{
                    unsatisfied = true;
                    break;
                }
            }
            // TODO: check if really correct
            if(!unsatisfied){
                if(precondition_counter == int(op.get_preconditions().size())){
                    sat_rules[rule_id] = rule_id;
                }else{
                    temp_rules[rule_id] = rule_id;
                }
            }
        }

        for(int test_iterator = 0; test_iterator < int(visited_tests.size()); test_iterator++){
            if(visited_tests[test_iterator]){
                temp_tests[test_iterator] = tests[test_iterator];
            }
        }

        tests = temp_tests;
        rules = temp_rules;
        **/

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