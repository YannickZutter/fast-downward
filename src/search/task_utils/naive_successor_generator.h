#ifndef TASK_UTILS_SUCCESSOR_GENERATOR_H
#define TASK_UTILS_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>

namespace successor_generator {
    class GeneratorBase;

    class NaiveSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{
        std::vector<OperatorProxy> operators;

    public:
        NaiveSuccessorGenerator();

        virtual ~NaiveSuccessorGenerator();

        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) override;

        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) override;
    };
}

#endif
