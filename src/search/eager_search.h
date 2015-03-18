#ifndef EAGER_SEARCH_H
#define EAGER_SEARCH_H

#include <vector>

#include "global_state.h"
#include "search_engine.h"
#include "search_space.h"
#include "timer.h"

#include "open_lists/open_list.h"

class GlobalOperator;
class Heuristic;
class Options;
class ScalarEvaluator;

class EagerSearch : public SearchEngine {
    // Search Behavior parameters
    bool reopen_closed_nodes; // whether to reopen closed nodes upon finding lower g paths
    bool use_multi_path_dependence;

    OpenList<StateID> *open_list;
    ScalarEvaluator *f_evaluator;

    EvaluationContext evaluate_state(
        const GlobalState &state, int g, bool preferred);
    EvaluationContext evaluate_state_for_preferred_ops(
        const GlobalState &state, int g, bool preferred);
protected:
    SearchStatus step();
    std::pair<SearchNode, bool> fetch_next_node();
    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(const SearchNode &node);
    void reward_progress();

    std::vector<Heuristic *> heuristics;
    std::vector<Heuristic *> preferred_operator_heuristics;
    // TODO: in the long term this
    // should disappear into the open list

    virtual void initialize();

    void print_best_h_line(int g) const;
public:
    explicit EagerSearch(const Options &opts);
    virtual ~EagerSearch() = default;

    virtual void print_statistics() const override;

    void dump_search_space();
};

#endif
