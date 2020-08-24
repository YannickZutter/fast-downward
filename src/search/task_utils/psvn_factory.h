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
    vector<int> rules;
    vector<int>  test_results;
    vector<int> already_satisfied_tests;
    vector<int> satisfied_rules;
    // children only gibes reference to where in vertex_list the child is saved
    vector<int> children;
    int choice;


    Vertex(vector<int> rls, vector<int> tst){
        this->rules = move(rls);
        this->test_results = move(tst);
        this->test_results = vector<int>(tst.size(), -1);
        this->already_satisfied_tests = vector<int>(tst.size(), -1);

    }
    Vertex(vector<int> rls, vector<int> tst, vector<int> sat){
        this->rules = move(rls);
        this->test_results = move(tst);
        this->satisfied_rules = move(sat);
        this->test_results = vector<int>(tst.size(), -1);
        this->already_satisfied_tests = vector<int>(tst.size(), -1);
    }


    void set_satisfied_rules(vector<int> rules){
        satisfied_rules = rules;
    }

    void choose_test(VariablesProxy variables){
        bool chosen = false;

        for(VariableProxy var : variables){

            if(test_results[var.get_id()] == -1){
                this->choice = var.get_id();
                chosen = true;
            }

        }
        if(!chosen){
            cout << "problems finding correct test variable!";
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
        vector<Vertex> vertex_list;
        vector<OperatorProxy> operators;

    public:
        explicit PSVNFactory(const TaskProxy &task_proxy);
        virtual ~PSVNFactory();

        vector<Vertex> create();

        void create_DAG_recursive(Vertex vertex);

        void split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules);
        int check_existence(const Vertex& vertex);

    };


}


#endif //FAST_DOWNWARD_PSVN_FACTORY_H
