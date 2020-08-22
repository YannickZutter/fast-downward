//
// Created by yannick on 07.08.20.
//

#include "psvn_factory.h"

namespace PSVNFactory{

    PSVNFactory::PSVNFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNFactory::~PSVNFactory() = default;

    void PSVNFactory::create_DAG_recursive(Vertex vertex) {

        if(!vertex.rules.empty()){
            vertex_list.push_back(vertex);

            vertex.choose_test(variables);

            // generate children
            for(int i = 0; i < variables[vertex.choice].get_domain_size(); i++){
                vector<FactPair> temp_tests;
                temp_tests.insert(temp_tests.end(), vertex.get_test_results().begin(), vertex.get_test_results().end());
                FactPair pair(vertex.choice, i);
                temp_tests.push_back(pair);
                vector<OperatorProxy> temp_rules = vertex.rules;
                vector<OperatorProxy> temp_sat_rules;
                split_and_simplify(temp_rules, temp_tests, temp_sat_rules);
                Vertex v(temp_rules, temp_tests);
                v.set_satisfied_rules(temp_sat_rules);

                int existence = check_existence(v);
                if(existence != -1){
                    vertex.children.push_back(existence);
                } else{
                    create_DAG_recursive(v);
                    vertex.children.push_back(vertex_list.size());
                }
            }
        }
    }

    void PSVNFactory::create() {

        for(VariableProxy var : task_proxy.get_variables()){
            variables.push_back(var);
        }

        vector<FactPair> test_set;

        vector<OperatorProxy> plausible_rules;

        for(OperatorProxy op : task_proxy.get_operators()){
            plausible_rules.push_back(op);
        }
        Vertex vertex(plausible_rules, test_set);
        create_DAG_recursive(vertex);
        Vertex root = vertex_list.at(0);

    }

    void PSVNFactory::split_and_simplify(vector<OperatorProxy> &rules, vector<FactPair>& tests, vector<OperatorProxy> &sat_rules) {
        vector<OperatorProxy> temp_rules = rules;
        for(auto & rule : rules){
            // the idea is to increase this counter everytime a precondition is satisfied, then at the end if it matches
            // the number of preconditions in the operator, then the operator or rule is satisfied.
            int precondition_counter = 0;
            // set this to true if the operator is unsatisfied. check at end of all preconditions and remove(do not
            // insert in temp_rules if necessary.
            bool unsatisfied = false;
            for(int precondition_iterator = 0; precondition_iterator < int(rule.get_preconditions().size()); precondition_iterator++){

                for(auto & test : tests){
                    // check if correct var
                    if(test.var == rule.get_id()){

                        // now check if val is same in the var
                        if(test.value == rule.get_preconditions()[precondition_iterator].get_value()){
                            precondition_counter++;
                        } else if (test.value != rule.get_preconditions()[precondition_iterator].get_value()){
                            unsatisfied = true;
                            break;
                        }
                    }
                }
            }
            if(!unsatisfied){
                if(precondition_counter == int(rule.get_preconditions().size())){
                    sat_rules.push_back(rule);
                }
                else{
                    temp_rules.push_back(rule);
                }
            }
        }
        vector<FactPair> temp_tests = tests;
        //now simplify test set
        for(auto & test : tests){
            bool visited = false;

            for(auto & temp_rule : temp_rules){
                for(int precondition_iterator = 0; precondition_iterator < int(temp_rule.get_preconditions().size()); precondition_iterator++){
                    if(test.var == temp_rule.get_preconditions()[precondition_iterator].get_variable().get_id()){
                        visited = true;
                    }
                }
            }
            if(visited){
               temp_tests.push_back(test);
            }
        }

        rules.clear();
        for(OperatorProxy op : temp_rules){
            rules.push_back(op);
        }
        tests.clear();
        for(FactPair pair : temp_tests){
            tests.push_back(pair);
        }

    }

    /**
     * return position in vertex_list if vertex already exists, or -1 if not
     * @param vertex
     * @return position in vertex_list or -1 else
     */
    int PSVNFactory::check_existence(const Vertex& vertex) {

        for(int i = 0; i < int(vertex_list.size()); i++){
            if(vertex == vertex_list[i]){
                return i;
            }
        }

        return -1;
    }
}