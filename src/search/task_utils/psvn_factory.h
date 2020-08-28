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

    vector<int> rules;
    vector<int> test_results;
    vector<int> satisfied_rules;
    vector<int> children;
    int choice;
    int hash;

    Vertex(vector<int> rls, vector<int> tst){
        //satisfied_rules = vector<int>(rls.size(), -1);
        rules = move(rls);
        test_results = move(tst);
        choice = -1;
        utils::HashState temp;
        utils::feed(temp, rules);
        utils::feed(temp, test_results);
        utils::feed(temp, satisfied_rules);
        hash = temp.get_hash64();
    }

    Vertex(vector<int> rls, vector<int> tst, vector<int> sat){
        rules = move(rls);
        test_results = move(tst);
        satisfied_rules = move(sat);
        choice = -1;
        utils::HashState temp;
        utils::feed(temp, rules);
        utils::feed(temp, test_results);
        utils::feed(temp, satisfied_rules);
        hash = temp.get_hash64();
    }


    void set_satisfied_rules(vector<int> rls){
        satisfied_rules = move(rls);
    }

    bool choose_test(const vector<OperatorProxy> &operators){

        // first check all operator precondition to satisfy one after another
        for(int rule_id : rules){
            if(rule_id != -1){
                OperatorProxy op = operators[rule_id];
                for(FactProxy fact : op.get_preconditions()){
                    if(test_results[fact.get_pair().var] == -1){
                        choice = fact.get_pair().var;
                        return true;
                    }
                }
            }
        }
        // then go for not yet set variables
        for(int i = 0; i < int(test_results.size()); i++){
            if(test_results[i] == -1){
                this->choice = i;
                return true;
            }
        }
        return false;
    }
    void add_child(int index){
        children.push_back(index);
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
        utils::HashMap<int, int> map;

    public:
        explicit PSVNFactory(const TaskProxy &task_proxy);
        virtual ~PSVNFactory();

        vector<Vertex> create();
        void create_DAG_recursive(int pos);
        void split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules);
    };
}

#endif //FAST_DOWNWARD_PSVN_FACTORY_H
