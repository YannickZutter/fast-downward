//
// Created by yannick on 05.09.20.
//

#ifndef FAST_DOWNWARD_WATCHED_LITERAL_SUCCESSOR_GENERATOR_H
#define FAST_DOWNWARD_WATCHED_LITERAL_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>
bool factproxy_comparer(FactProxy a, FactProxy b){
    return a.get_pair().var < b.get_pair().var;
}
struct OwnOps{
    int op_id;
    std::vector<FactProxy> preconditions;
     OwnOps(int id, std::vector<FactProxy> precons){
         op_id = id;
         preconditions = precons;
         std::sort(preconditions.begin(), preconditions.end(), factproxy_comparer);
     }
     OwnOps(int id, PreconditionsProxy precons){
         op_id = id;
         for(FactProxy p : precons){
             preconditions.push_back(p);
         }
         std::sort(preconditions.begin(), preconditions.end(), factproxy_comparer);
     }

};


namespace successor_generator{
    class GeneratorBase;

    class WatchedLiteralSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{
        std::vector<OwnOps> operators;

    public:
        WatchedLiteralSuccessorGenerator();
        virtual ~WatchedLiteralSuccessorGenerator();

        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) override;
        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) override;

    };
}

#endif //FAST_DOWNWARD_WATCHED_LITERAL_SUCCESSOR_GENERATOR_H
