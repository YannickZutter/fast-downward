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
        std::vector<int> precondition_tracker;
        std::vector<int> offset;
        std::vector<std::vector<int>> watcher_list;

    public:
        WatchedLiteralSuccessorGenerator();
        virtual ~WatchedLiteralSuccessorGenerator();

        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) override;
        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) override;

        void process_watch_list(FactProxy fact, const State& state, std::vector<OperatorID> &applicable_ops);

        int get_fact_id(int var, int value) const;
        int get_fact_id(const FactProxy &fact) const;
        int get_fact_id(VariableProxy var, FactProxy value) const;
    };
}

#endif //FAST_DOWNWARD_WATCHED_LITERAL_SUCCESSOR_GENERATOR_H
