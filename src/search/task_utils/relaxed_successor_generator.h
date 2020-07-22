//
// Created by yannick on 20.07.20.
//

#ifndef FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H
#define FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>

namespace successor_generator{
    class GeneratorBase;

    class RelaxedSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{


    public:
        RelaxedSuccessorGenerator();

        virtual ~RelaxedSuccessorGenerator();

        void initialize();

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) const;

        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) const;
    };
}


#endif //FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H
