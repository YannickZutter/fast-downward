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
            for(FactProxy fact : operators[rule_id].get_preconditions()){
                if(test_results[fact.get_pair().var] == -1){
                    choice = fact.get_pair().var;
                    return true;
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
