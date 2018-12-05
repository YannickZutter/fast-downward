#ifndef HEURISTICS_RELAXATION_HEURISTIC_H
#define HEURISTICS_RELAXATION_HEURISTIC_H

#include "array_chain.h"

#include "../heuristic.h"

#include "../utils/collections.h"

#include <cassert>
#include <vector>

class FactProxy;
class GlobalState;
class OperatorProxy;

namespace relaxation_heuristic {
struct Proposition;
struct UnaryOperator;

using PropID = int;
using OpID = int;

const OpID NO_OP = -1;

struct Proposition {
    Proposition();
    int cost; // used for h^max cost or h^add cost
    // TODO: Make sure in constructor that reached_by does not overflow.
    PropID reached_by : 30;
    bool is_goal : 1;
    bool marked : 1; // used for preferred operators of h^add and h^FF
    array_chain::ArrayChainIndex precondition_of;
};

struct UnaryOperator {
    UnaryOperator(int num_preconditions,
                  array_chain::ArrayChainIndex preconditions,
                  PropID effect,
                  int operator_no, int base_cost);
    int cost; // Used for h^max cost or h^add cost;
              // includes operator cost (base_cost)
    int unsatisfied_preconditions;
    PropID effect;
    int base_cost;
    int num_preconditions;
    array_chain::ArrayChainIndex preconditions;
    int operator_no; // -1 for axioms; index into the task's operators otherwise
};

class RelaxationHeuristic : public Heuristic {
    void build_unary_operators(const OperatorProxy &op);
    void simplify();

    // proposition_offsets[var_no]: first PropID related to variable var_no
    std::vector<PropID> proposition_offsets;
protected:
    std::vector<UnaryOperator> unary_operators;
    std::vector<Proposition> propositions;
    std::vector<PropID> goal_propositions;

    array_chain::ArrayChain preconditions_chain;
    array_chain::ArrayChain precondition_of_chain;

    array_chain::ArrayChainView get_preconditions(OpID op_id) const {
        return preconditions_chain[unary_operators[op_id].preconditions];
    }

    // HACK!
    std::vector<PropID> get_preconditions_vector(OpID op_id) const {
        auto view = get_preconditions(op_id);
        return std::vector<PropID>(view.begin(), view.end());
    }

    /*
      TODO: Some of these protected methods are only needed for the
      CEGAR hack in the additive heuristic and should eventually go
      away.
    */
    PropID get_prop_id(const Proposition &prop) const {
        PropID prop_id = &prop - propositions.data();
        assert(utils::in_bounds(prop_id, propositions));
        return prop_id;
    }

    OpID get_op_id(const UnaryOperator &op) const {
        OpID op_id = &op - unary_operators.data();
        assert(utils::in_bounds(op_id, unary_operators));
        return op_id;
    }

    PropID get_prop_id(int var, int value) const;
    PropID get_prop_id(const FactProxy &fact) const;

    Proposition *get_proposition(PropID prop_id) {
        return &propositions[prop_id];
    }
    UnaryOperator *get_operator(OpID op_id) {
        return &unary_operators[op_id];
    }

    const Proposition *get_proposition(int var, int value) const;
    Proposition *get_proposition(int var, int value);
    Proposition *get_proposition(const FactProxy &fact);
public:
    explicit RelaxationHeuristic(const options::Options &options);

    virtual bool dead_ends_are_reliable() const override;
};
}

#endif
