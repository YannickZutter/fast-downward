//
// Created by yannick on 07.08.20.
//

#ifndef FAST_DOWNWARD_PSVN_FACTORY_H
#define FAST_DOWNWARD_PSVN_FACTORY_H

#include <utility>
#include <vector>
#include "../task_proxy.h"
#include "../utils/hash.h"

using namespace std;

struct Vertex {

    //TODO: change everything to nested ints, so we can change to hash comparison
    vector<OperatorProxy> rules;
    vector<FactPair>  test_results;
    vector<FactPair> already_satisfied_tests;
    vector<OperatorProxy> satisfied_rules;
    // children only gibes reference to where in vertex_list the child is saved
    vector<int> children;
    int choice{};


    Vertex(vector<OperatorProxy> rules, vector<FactPair> test_results){
        this->rules = move(rules);
        this->test_results = move(test_results);

    }
    vector<FactPair> get_test_results() const{
        return test_results;
    }

    void set_satisfied_rules(vector<OperatorProxy> rules){
        satisfied_rules = rules;
    }

    void choose_test(vector<VariableProxy> variables){
        for(VariableProxy var : variables){
            for(FactPair fct : this->test_results){
                if(fct.var == var.get_id()){
                    this->choice = var.get_id();
                }
            }
        }
    }

    bool operator==(Vertex &b) const{
        bool check_a = this->rules == b.rules;
        bool check_b = this->test_results == b.test_results;
        bool check_c = this->already_satisfied_tests == b.already_satisfied_tests;
        return check_a && check_b && check_c;
    }


};

namespace PSVNFactory{


    class PSVNFactory{

        const TaskProxy &task_proxy;
        vector<VariableProxy> variables;
            vector<Vertex> vertex_list;

    public:
        explicit PSVNFactory(const TaskProxy &task_proxy);
        virtual ~PSVNFactory();

        void create();

        void create_DAG_recursive(Vertex vertex);

        vector<VariableProxy> get_variables(){
            return variables;
        }

        static void split_and_simplify(vector<OperatorProxy> &rules, vector<FactPair>& tests, vector<OperatorProxy> &sat_rules);
        int check_existence(const Vertex& vertex);

    };


}


#endif //FAST_DOWNWARD_PSVN_FACTORY_H
