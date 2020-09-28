//
// Created by yannick on 07.08.20.
//

#include "psvn_split_factory.h"
#include <cmath>

namespace PSVNSplitFactory{

    PSVNSplitFactory::PSVNSplitFactory(const TaskProxy &task_proxy, int lim) :  task_proxy(task_proxy) {
        list_limit = lim;
    }
    PSVNSplitFactory::~PSVNSplitFactory() = default;

    vector<vector<Vertex>> PSVNSplitFactory::create() {

        int split_factor = 1;
        dag_too_big = false;
        do {
            vertex_lists.resize(split_factor);
            dag_too_big = false;

            for (int i = 0; i < split_factor; i++) {
                map.clear();
                vertex_lists[i].clear();
                stop_all_recursion = false;
                bot = floor(i * task_proxy.get_operators().size() / split_factor);
                top = floor((i + 1) * task_proxy.get_operators().size() / split_factor) - 1;

                vector<int> test_set(task_proxy.get_variables().size(), -1);
                vector<Operator> plausible_rules;

                for (int j = bot; j < top; j++) {
                    OperatorProxy o = task_proxy.get_operators()[j];
                    plausible_rules.push_back(Operator(o.get_id(), o.get_preconditions()));
                }

                //Vertex v(plausible_rules, test_set, vector<int>());
                //vertex_lists[i].push_back(v);
                //map.insert(make_pair(v.hash, 0));
                create_DAG_recursive(i, 0);

                if (vertex_lists[i].size() > list_limit) {
                    stop_all_recursion = true;
                    dag_too_big = true;
                    break;
                }
                plausible_rules.clear();
                test_set.clear();
            }

            if (!dag_too_big) {
                for (int i = 0; i < int(vertex_lists.size()); i++) {
                    if (int(vertex_lists[i].size()) > list_limit) {
                        dag_too_big = true;
                    }
                }
            }
            split_factor *= 2;

        }while (dag_too_big);
        return vertex_lists;
    }

    void PSVNSplitFactory::create_DAG_recursive(int list_nr, int pos) {

        if (!(vertex_lists[list_nr].size() < list_limit) && !stop_all_recursion) {

            if (!vertex_lists[list_nr][pos].plausible_operators.empty()) {

                vertex_lists[list_nr][pos].choose_test(task_proxy.get_operators());
                assert(vertex_lists[list_nr][pos].choice != -1);

                for (int domain_iterator = 0; domain_iterator <task_proxy.get_variables()[vertex_lists[list_nr][pos].choice].get_domain_size(); domain_iterator++) {

                    vector<int> sat_rules;
                    vector<Operator> plausible_rules;
                    vector<int> vars = vertex_lists[list_nr][pos].test_results;
                    vars[vertex_lists[list_nr][pos].choice] = domain_iterator;

                    split_and_simplify(vertex_lists[list_nr][pos], plausible_rules, vertex_lists[list_nr][pos].choice, domain_iterator, sat_rules);

                    //Vertex v(plausible_rules, vars, sat_rules);

                    //if(map.find(v.hash) == map.end()){
                    //vertex_lists[list_nr].push_back(v);
                    vertex_lists[list_nr][pos].children.push_back(vertex_lists[list_nr].size() - 1);
                    //map.insert(make_pair(v.hash, vertex_lists[list_nr].size() - 1));
                    create_DAG_recursive(list_nr, vertex_lists[list_nr].size() - 1);
                    //}else{
                    //    vertex_list[pos].children.push_back(map.find(v.hash)->second);
                    //}

                }

            } else {
                stop_all_recursion = true;
            }
        }
    }

    void PSVNSplitFactory::split_and_simplify(const Vertex &v, vector<Operator> &rules,const int test_var,const int test_val, vector<int> &sat_rules) {

        for(Operator o : v.plausible_operators){
            vector<int> p;
            bool unsat = false;
            for(int pre : o.precons){
                if(test_var == task_proxy.get_operators()[o.id].get_preconditions()[pre].get_variable().get_id()){ // if same variable

                    if(test_val != task_proxy.get_operators()[o.id].get_preconditions()[pre].get_value()){ // if not same value
                        unsat = true;
                        break;
                    }
                }else{ // if not same vairable
                    p.push_back(pre);
                }
            }
            if(!unsat){ // only if still plausible or better
                if(p.empty()){ // if satisfied
                    sat_rules.push_back(o.id);
                }else{
                    rules.push_back(Operator(o.id, p));
                }
            }
        }
    }
}