#ifndef FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H
#define FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>
#include "../heuristics/array_pool.h"

namespace successor_generator{
    class GeneratorBase;

    using namespace std;

    class RelaxedSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{

        vector<int> fact_offsets;
        vector<vector<OperatorID>> facts;
        vector<OperatorProxy> operators;
        vector<int> counter;


    public:
        RelaxedSuccessorGenerator();

        virtual ~RelaxedSuccessorGenerator();


        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) override ;

        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) override ;

        int get_fact_id(int var, int value) const;

        int get_fact_id(const FactProxy &fact) const;

        int get_fact_id(VariableProxy var, FactProxy value) const;

    };
}


#endif //FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H
