//
// Created by yannick on 07.08.20.
//

#include "psvn_factory.h"

namespace PSVNFactory{

    PSVNFactory::PSVNFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNFactory::~PSVNFactory() = default;

    vector<Vertex> PSVNFactory::create() {

        vector<int> test_set(task_proxy.get_variables().size(), -1);
        vector<Operator> plausible_rules;

        for(OperatorProxy op : task_proxy.get_operators()){
            plausible_rules.push_back(Operator(op.get_id(), op.get_preconditions()));
        }

        Vertex vertex(plausible_rules, test_set, vector<int>());
        vertex_list.push_back(vertex);

        //map.insert(make_pair( vertex.hash, vertex_list.size()-1));
        create_DAG_recursive(0);
        return vertex_list;
    }

    void PSVNFactory::create_DAG_recursive(int pos) {


        if (!vertex_list[pos].plausible_operators.empty()) {

            vertex_list[pos].choose_test(task_proxy.get_operators());
            //assert(vertex_list[pos].choice != -1);
            for(int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[vertex_list[pos].choice].get_domain_size(); domain_iterator++){

                vector<int> sat_rules;
                vector<Operator> plausible_rules;
                vector<int> vars = vertex_list[pos].test_results;
                vars[vertex_list[pos].choice] = domain_iterator;

                split_and_simplify(vertex_list[pos], plausible_rules, vertex_list[pos].choice, domain_iterator, sat_rules);

                Vertex v(plausible_rules, vars, sat_rules);

                vertex_list.push_back(v);
                vertex_list[pos].children.push_back(vertex_list.size()-1);
                create_DAG_recursive(vertex_list.size()-1);
            }

        }

    }

    void PSVNFactory::split_and_simplify(const Vertex &v, vector<Operator> &rules, int test_var, int test_val, vector<int> &sat_rules) {

        for(Operator o : v.plausible_operators){
            vector<int> p;
            bool unsat = false;
            for(int pre : o.precons){
                if(test_var == task_proxy.get_operators()[o.id].get_preconditions()[pre].get_variable().get_id()){
                    if(test_val != task_proxy.get_operators()[o.id].get_preconditions()[pre].get_value()){
                        unsat = true;
                        break;
                    }
                }else{
                    p.push_back(pre);
                }

            }
            if(!unsat){
                if(p.empty()){
                    sat_rules.push_back(o.id);
                }else{
                    rules.push_back(Operator(o.id, p));
                }
            }
        }
    }
}