#ifndef TASK_UTILS_SUCCESSOR_GENERATOR_H
#define TASK_UTILS_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>

namespace successor_generator {
    class GeneratorBase;

    class NaiveSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{
        std::unique_ptr<GeneratorBase> root;

    public:
        NaiveSuccessorGenerator();
        /*
          We cannot use the default destructor (implicitly or explicitly)
          here because GeneratorBase is a forward declaration and the
          incomplete type cannot be destroyed.
        */
        virtual ~NaiveSuccessorGenerator();

        virtual void initialize(const TaskProxy &task_proxy) override;

        virtual void generate_applicable_ops(
                const State &state, std::vector<OperatorID> &applicable_ops) const override ;
        // Transitional method, used until the search is switched to the new task interface.
        virtual void generate_applicable_ops(
                const GlobalState &state, std::vector<OperatorID> &applicable_ops) const override;
    };
}

#endif
