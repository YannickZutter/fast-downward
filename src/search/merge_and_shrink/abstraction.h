#ifndef MERGE_AND_SHRINK_ABSTRACTION_H
#define MERGE_AND_SHRINK_ABSTRACTION_H

#include "shrink_strategy.h"

#include "../operator_cost.h"
#include "label.h"

#include <ext/slist>
#include <vector>

class State;
class Labels;

struct AbstractTransition {
    AbstractStateRef src;
    AbstractStateRef target;

    AbstractTransition(AbstractStateRef src_, AbstractStateRef target_)
        : src(src_), target(target_) {
    }

    bool operator==(const AbstractTransition &other) const {
        return src == other.src && target == other.target;
    }

    bool operator!=(const AbstractTransition &other) const {
        return !(*this == other);
    }

    bool operator<(const AbstractTransition &other) const {
        return src < other.src || (src == other.src && target < other.target);
    }
};

class Abstraction {
    friend class AtomicAbstraction;
    friend class CompositeAbstraction;

    friend class ShrinkStrategy; // for apply() -- TODO: refactor!

    static const int PRUNED_STATE = -1;
    static const int DISTANCE_UNKNOWN = -2;

    const bool is_unit_cost;
    Labels *labels;

    std::vector<const Label *> relevant_labels;
    int num_states;
    std::vector<std::vector<AbstractTransition> > transitions_by_label;

    std::vector<int> init_distances;
    std::vector<int> goal_distances;
    std::vector<bool> goal_states;
    AbstractStateRef init_state;

    int max_f;
    int max_g;
    int max_h;

    bool are_labels_reduced;

    mutable int peak_memory;

    void clear_distances();
    void compute_init_distances_unit_cost();
    void compute_goal_distances_unit_cost();
    void compute_init_distances_general_cost();
    void compute_goal_distances_general_cost();

    void apply_abstraction(std::vector<__gnu_cxx::slist<AbstractStateRef> > &collapsed_groups);

    int total_transitions() const;
    int unique_unlabeled_transitions() const;
protected:
    std::vector<int> varset;

    virtual AbstractStateRef get_abstract_state(const State &state) const = 0;
    virtual void apply_abstraction_to_lookup_table(const std::vector<
                                                       AbstractStateRef> &abstraction_mapping) = 0;
    virtual int memory_estimate() const;
public:
    Abstraction(bool is_unit_cost, Labels *labels);
    virtual ~Abstraction();

    // Two methods to identify the abstraction in output.
    // tag is a convience method that upper-cases the first letter of
    // description and appends ": ";
    virtual std::string description() const = 0;
    std::string tag() const;

    // TODO: labels has only to be passed because the method is static.
    // The method is static to get access to private members... maybe change?
    static void build_atomic_abstractions(bool is_unit_cost,
        std::vector<Abstraction *> &result, Labels *labels);
    bool is_solvable() const;

    int get_cost(const State &state) const;
    int size() const;
    void statistics(bool include_expensive_statistics) const;

    int get_peak_memory_estimate() const;
    // NOTE: This will only return something useful if the memory estimates
    //       have been computed along the way by calls to statistics().
    // TODO: Find a better way of doing this that doesn't require
    //       a mutable attribute?

    bool is_in_varset(int var) const;

    void compute_distances();
    void normalize(bool reduce_labels);
    void release_memory();

    void dump() const;

    // The following methods exist for the benefit of shrink strategies.
    int get_max_f() const;
    int get_max_g() const;
    int get_max_h() const;

    bool is_goal_state(int state) const {
        return goal_states[state];
    }

    int get_init_distance(int state) const {
        return init_distances[state];
    }

    int get_goal_distance(int state) const {
        return goal_distances[state];
    }

    // The following methods are shrink_bisimulation-exclusive
    int get_num_labels() const {
        return transitions_by_label.size();
    }
    const std::vector<AbstractTransition> &get_transitions_for_label(int label_no) const {
        return transitions_by_label[label_no];
    }
    int get_cost_for_label(int label_no) const;
};

class AtomicAbstraction : public Abstraction {
    int variable;
    std::vector<AbstractStateRef> lookup_table;
protected:
    virtual std::string description() const;
    virtual void apply_abstraction_to_lookup_table(const std::vector<
                                                       AbstractStateRef> &abstraction_mapping);
    virtual AbstractStateRef get_abstract_state(const State &state) const;
    virtual int memory_estimate() const;
public:
    AtomicAbstraction(bool is_unit_cost, Labels *labels, int variable);
    virtual ~AtomicAbstraction();
};

class CompositeAbstraction : public Abstraction {
    Abstraction *components[2];
    std::vector<std::vector<AbstractStateRef> > lookup_table;
protected:
    virtual std::string description() const;
    virtual void apply_abstraction_to_lookup_table(
        const std::vector<AbstractStateRef> &abstraction_mapping);
    virtual AbstractStateRef get_abstract_state(const State &state) const;
    virtual int memory_estimate() const;
public:
    CompositeAbstraction(bool is_unit_cost, Labels *labels,
        Abstraction *abs1, Abstraction *abs2);
    virtual ~CompositeAbstraction();
};

#endif
