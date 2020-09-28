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
struct Operator{
    int id;
    vector<int> precons;
    Operator(int i, vector<int> pre){
        id = i;
        precons = pre;
    }
    Operator(int i, PreconditionsProxy pre){
        id = i;
        for(int i = 0; i < pre.size(); i++){
            precons.push_back(i);
        }
    }
};
struct Vertex {

    vector<Operator> plausible_operators;
    vector<int> test_results;
    vector<int> satisfied_operators;
    vector<int> children;
    int choice;
    int hash;

    Vertex(vector<Operator> rls, vector<int> tst, vector<int> sat){
        plausible_operators = move(rls);
        test_results = move(tst);
        satisfied_operators = move(sat);
        choice = -1;
        /**
        utils::HashState temp;
        utils::feed(temp, plausible_operators);
        utils::feed(temp, test_results);
        utils::feed(temp, satisfied_operators);
        hash = temp.get_hash64();
         **/
    }

    void choose_test(const OperatorsProxy &operators) {
        for(Operator op : plausible_operators){
            for( int precon_id : op.precons){
                int var = operators[op.id].get_preconditions()[precon_id].get_variable().get_id();
                if( test_results[var] == -1){
                    choice = var;
                }
            }
        }
    }

};

namespace PSVNFactory{

    class PSVNFactory{

        const TaskProxy &task_proxy;
        vector<Vertex> vertex_list;
        utils::HashMap<int, int> map;

    public:
        explicit PSVNFactory(const TaskProxy &task_proxy);
        virtual ~PSVNFactory();
        vector<Vertex> create();
        void create_DAG_recursive(int pos);
        void split_and_simplify(const Vertex &v, vector<Operator> &rules, int test_var, int test_val, vector<int> &sat_rules);
    };
}

#endif //FAST_DOWNWARD_PSVN_FACTORY_H
