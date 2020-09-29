//
// Created by yannick on 07.08.20.
//

#include "psvn_factory.h"

namespace PSVNFactory{

    PSVNFactory::PSVNFactory(const TaskProxy &task_proxy) :  task_proxy(task_proxy) {
    }
    PSVNFactory::~PSVNFactory() = default;

    vector<Vertex> PSVNFactory::create() {

        vector<int> &&test_set = vector<int>(task_proxy.get_variables().size(), -1);
        vector<Operator> &&plausible_rules = vector<Operator>();

        for(OperatorProxy op : task_proxy.get_operators()){
            plausible_rules.push_back(Operator(op.get_id(), op.get_preconditions()));
        }
        vector<int> &&sat = vector<int>();
        Vertex &&vertex = Vertex(move(plausible_rules), move(test_set), move(sat));
        vertex_list.push_back(move(vertex));
        map.insert(make_pair(vertex.hash, vertex_list.size()-1));
        create_DAG_recursive(0);
        return move(vertex_list);
    }

    void PSVNFactory::create_DAG_recursive(int pos) {


        if (!vertex_list[pos].plausible_operators.empty()) {

            vertex_list[pos].choose_test(task_proxy.get_operators());
            assert(vertex_list[pos].choice != -1);

            for(int domain_iterator = 0; domain_iterator < task_proxy.get_variables()[vertex_list[pos].choice].get_domain_size(); domain_iterator++){

                vector<Operator> &&plausible_ops = vector<Operator>();
                vector<int> &&sat_ops = vector<int>();
                vector<int> &&tests = vector<int>();
                tests = vertex_list[pos].test_results;
                tests[vertex_list[pos].choice] = domain_iterator;

                split_and_simplify(vertex_list[pos], plausible_ops, vertex_list[pos].choice, domain_iterator, sat_ops);



                Vertex v = Vertex(move(plausible_ops), move(tests), move(sat_ops));

                if(map.find(v.hash) == map.end()){
                    vertex_list.push_back(v);
                    vertex_list[pos].children.push_back(vertex_list.size()-1);
                    create_DAG_recursive(vertex_list.size()-1);
                }else{
                    vertex_list[pos].children.push_back(map.find(v.hash)->second);
                }


            }
        }
    }

    void PSVNFactory::split_and_simplify(const Vertex &v, vector<Operator> &rules, const int test_var, const int test_val, vector<int> &sat_rules) {

        for(const Operator& o : v.plausible_operators){
            vector<int> &&p = vector<int>();
            bool unsat = false;
            for(int pre : o.precons){
                if(test_var == task_proxy.get_operators()[o.id].get_preconditions()[pre].get_variable().get_id()){ // if same variable

                    if(test_val != task_proxy.get_operators()[o.id].get_preconditions()[pre].get_value()){ // if not same value
                        unsat = true;
                        break;
                    }
                }else{ // if not same variable
                    p.push_back(pre);
                }
            }
            if(!unsat){ // only if still plausible or better
                if(p.empty()){ // if satisfied
                    sat_rules.push_back(o.id);
                }else{
                    rules.push_back(Operator(o.id, move(p)));
                }
            }
        }
    }
}