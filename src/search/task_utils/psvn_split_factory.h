//
// Created by yannick on 07.08.20.
//

#ifndef FAST_DOWNWARD_PSVN_SPLIT_FACTORY_H
#define FAST_DOWNWARD_PSVN_SPLIT_FACTORY_H

#include <utility>
#include <vector>
#include "../task_proxy.h"
#include "../utils/hash.h"
#include "psvn_factory.h"

using namespace std;

namespace PSVNSplitFactory{

    class PSVNSplitFactory{

        const TaskProxy &task_proxy;
        vector<vector<Vertex>> vertex_lists;
        vector<OperatorProxy> operators;
        utils::HashMap<int, int> map;
        bool dag_too_big = true;
        bool stop_all_recursion = false;
        int max_dag_val = 500000; // 500'000
        int top;
        int bot;

    public:
        explicit PSVNSplitFactory(const TaskProxy &task_proxy);
        virtual ~PSVNSplitFactory();

        vector<vector<Vertex>> create();
        void create_DAG_recursive(int list_nr, int pos);
        void split_and_simplify(vector<int> &rules, vector<int>& tests, vector<int> &sat_rules);
        //void divide_tree(int factor);
    };
}

#endif //FAST_DOWNWARD_PSVN_SPLIT_FACTORY_H
