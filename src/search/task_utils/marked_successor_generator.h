#ifndef FAST_DOWNWARD_MARKED_SUCCESSOR_GENERATOR_H
#define FAST_DOWNWARD_MARKED_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>
#include "../heuristics/array_pool.h"

namespace successor_generator{
    class GeneratorBase;

    using namespace std;

    class MarkedSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{

        vector<int> offset;
        vector<vector<OperatorID>> precondition_of;
        vector<int> counter;
        vector<int> num_of_preconditions;
        vector<bool> first_visit;

    public:
        MarkedSuccessorGenerator();

        virtual ~MarkedSuccessorGenerator();


        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) override;

        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) override;

        int get_fact_id(int var, int value) const;

        int get_fact_id(const FactProxy &fact) const;

        int get_fact_id(VariableProxy var, FactProxy value) const;

    };
}


#endif //FAST_DOWNWARD_MARKED_SUCCESSOR_GENERATOR_H
