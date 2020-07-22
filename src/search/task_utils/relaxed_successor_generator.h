#ifndef FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H
#define FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H

#include "../per_task_information.h"
#include "successor_generator_base.h"
#include <memory>
#include "../heuristics/array_pool.h"

namespace successor_generator{
    class GeneratorBase;

    using namespace std;
    using PropID = int;
    using OpID = int;

    const OpID NO_OP = -1;

    struct Fact {
        Fact();
        int cost; // used for h^max cost or h^add cost
        // TODO: Make sure in constructor that reached_by does not overflow.
        OpID reached_by : 30;
        /* The following two variables are conceptually bools, but Visual C++ does
           not support packing ints and bools together in a bitfield. */
        unsigned int is_goal : 1;
        unsigned int marked : 1; // used for preferred operators of h^add and h^FF
        int num_precondition_occurences;
        array_pool::ArrayPoolIndex precondition_of;
    };

    static_assert(sizeof(Fact) == 16, "Fact has wrong size");

    struct UnaryOperator {
        UnaryOperator(int num_preconditions, array_pool::ArrayPoolIndex preconditions, PropID effect,int operator_no, int base_cost);
        int cost; // Used for h^max cost or h^add cost;
        // includes operator cost (base_cost)
        int unsatisfied_preconditions;
        PropID effect;
        int base_cost;
        int num_preconditions;
        array_pool::ArrayPoolIndex preconditions;
        int operator_no; // -1 for axioms; index into the task's operators otherwise
    };


    class RelaxedSuccessorGenerator : public successor_generator::SuccessorGeneratorBase{

        vector<PropID> fact_offsets;
        vector<Fact> facts;
        //vector<int> counter;
        vector<UnaryOperator> unary_operators;

        array_pool::ArrayPool preconditions_pool;
        array_pool::ArrayPool precondition_of_pool;

        void build_unary_operators(const OperatorProxy &op);
        void simplify();


        array_pool::ArrayPoolSlice get_preconditions(OpID op_id) const {
            const UnaryOperator &op = unary_operators[op_id];
            return preconditions_pool.get_slice(op.preconditions, op.num_preconditions);
        }

        vector<PropID> get_preconditions_vector(OpID op_id) const {
            auto view = get_preconditions(op_id);
            return std::vector<PropID>(view.begin(), view.end());
        }

        PropID get_prop_id(const Fact &prop) const {
            PropID prop_id = &prop - facts.data();
            assert(utils::in_bounds(prop_id, facts));
            return prop_id;
        }

        OpID get_op_id(const UnaryOperator &op) const {
            OpID op_id = &op - unary_operators.data();
            assert(utils::in_bounds(op_id, unary_operators));
            return op_id;
        }

        PropID get_prop_id(int var, int value) const;
        PropID get_prop_id(const FactProxy &fact) const;

        Fact *get_proposition(PropID prop_id) {
            return &facts[prop_id];
        }
        UnaryOperator *get_operator(OpID op_id) {
            return &unary_operators[op_id];
        }

        const Fact *get_proposition(int var, int value) const;
        Fact *get_proposition(int var, int value);
        Fact *get_proposition(const FactProxy &fact);



    public:
        RelaxedSuccessorGenerator();

        virtual ~RelaxedSuccessorGenerator();

        void initialize(const TaskProxy &task_proxy) override;

        void generate_applicable_ops(const State &state, std::vector<OperatorID> &applicable_ops) const override ;

        void generate_applicable_ops(const GlobalState &state, std::vector<OperatorID> &applicable_ops) const override ;

        void reset_counter(vector<int> &cnt);



    };
}


#endif //FAST_DOWNWARD_RELAXED_SUCCESSOR_GENERATOR_H
