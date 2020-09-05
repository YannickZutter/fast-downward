//
// Created by yannick on 05.09.20.
//

#ifndef FAST_DOWNWARD_WATCHED_LITERAL_SUCCESSOR_GENERATOR_H
#define FAST_DOWNWARD_WATCHED_LITERAL_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>

namespace successor_generator{
    class GeneratorBase;

    class WatchedLiteralSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{
        std::vector<OperatorProxy> operators;

    public:
        WatchedLiteralSuccessorGenerator();
        virtual ~WatchedLiteralSuccessorGenerator();

        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) override;
        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) override;

        FactProxy get_next_watched_fact(OperatorID op_id, std::vector<FactProxy> watch_list);
    };
}

#endif //FAST_DOWNWARD_WATCHED_LITERAL_SUCCESSOR_GENERATOR_H
