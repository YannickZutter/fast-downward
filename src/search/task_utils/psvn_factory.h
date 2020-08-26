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
    vector<int> satisfied_rules;
    // children only gives reference to where in vertex_list the child is saved
    vector<int> children;
    int choice;


    Vertex(vector<int> rls, vector<int> tst){
        satisfied_rules = vector<int>(rls.size(), -1);
        rules = move(rls);
        test_results = move(tst);


    }
    Vertex(vector<int> rls, vector<int> tst, vector<int> sat){
        rules = move(rls);
        test_results = move(tst);
        satisfied_rules = move(sat);
    }


    void set_satisfied_rules(vector<int> rules){
        satisfied_rules = rules;
    }

    bool choose_test(const vector<OperatorProxy> &operators){

        for(int op_id : rules){
            if(op_id != -1){
                OperatorProxy op = operators[op_id];
                for(FactProxy fact : op.get_preconditions()){
                    if(test_results[fact.get_pair().var] == -1){
                        choice = fact.get_pair().var;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool operator==(Vertex &b) const{
        if(this->rules.size() != b.rules.size() || this->test_results.size() != b.test_results.size() || this->satisfied_rules.size() != b.satisfied_rules.size()){
            return false;
        }
        for(int i = 0; i < int(this->rules.size()); i++){
            if(this->rules[i] != b.rules[i]){
                return false;
            }
        }
        for(int i = 0; i < int(this->test_results.size()); i++){
            if(this->test_results[i] != b.test_results[i]){
                return false;
            }
        }
        for(int i = 0; i < int(this->satisfied_rules.size()); i++){
            if(this->satisfied_rules[i] != b.satisfied_rules[i]){
                return false;
            }
        }
        return true;
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

        void create_DAG_recursive(int pos);

        void split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules);
        int check_existence(const Vertex& vertex);

    };


}


#endif //FAST_DOWNWARD_PSVN_FACTORY_H
