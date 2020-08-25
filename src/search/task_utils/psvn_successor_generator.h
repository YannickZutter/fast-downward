#ifndef TASK_UTILS_PSVN_SUCCESSOR_GENERATOR_H
#define TASK_UTILS_PSVN_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>
#include "psvn_factory.h"

namespace successor_generator {
    class GeneratorBase;

    class PSVNSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{
        std::vector<OperatorProxy> operators;
        std::vector<int> op_ids;
        std::vector<Vertex> vertex_list;


    public:
        PSVNSuccessorGenerator();

        virtual ~PSVNSuccessorGenerator();

        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) override;

        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) override;

        void iterate_through_DAG(const Vertex &v,const State &state, std::vector<OperatorID> &applicable_ops, std::vector<bool> &taken_ops);
    };
}

#endif
