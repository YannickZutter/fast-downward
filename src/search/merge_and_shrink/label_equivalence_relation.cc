#include "label_equivalence_relation.h"

#include "labels.h"

#include <cassert>


using namespace std;

namespace MergeAndShrink {
LabelEquivalenceRelation::LabelEquivalenceRelation(const Labels &labels)
    : labels(labels) {
    grouped_labels.reserve(labels.get_max_size());
    label_to_positions.resize(labels.get_max_size());
}

void LabelEquivalenceRelation::add_label_to_group(int group_id,
                                                  int label_no) {
    LabelIter label_it = grouped_labels[group_id].insert(label_no);
    label_to_positions[label_no] = make_pair(group_id, label_it);

    int label_cost = labels.get_label_cost(label_no);
    if (label_cost < grouped_labels[group_id].get_cost())
        grouped_labels[group_id].set_cost(label_cost);
}

void LabelEquivalenceRelation::recompute_group_costs() {
    for (LabelGroup &label_group : grouped_labels) {
        // Setting cost to infinity for empty groups does not hurt.
        label_group.set_cost(INF);
        for (int label_no : label_group) {
            int cost = labels.get_label_cost(label_no);
            if (cost < label_group.get_cost()) {
                label_group.set_cost(cost);
            }
        }
    }
}

void LabelEquivalenceRelation::apply_label_mapping(
    const vector<pair<int, vector<int>>> &label_mapping,
    bool from_same_group) {
    for (const pair<int, vector<int>> &mapping : label_mapping) {
        int new_label_no = mapping.first;
        const vector<int> &old_label_nos = mapping.second;

        // Add new label to group
        int canonical_group_id = get_group_id(old_label_nos.front());
        if (from_same_group) {
            add_label_to_group(canonical_group_id, new_label_no);
        } else {
            add_label_group({new_label_no});
        }

        // Remove old labels from group
        for (int old_label_no : old_label_nos) {
            if (from_same_group) {
                assert(canonical_group_id == get_group_id(old_label_no));
            }
            LabelIter label_it = label_to_positions[old_label_no].second;
            grouped_labels[canonical_group_id].erase(label_it);
        }
    }

    if (!from_same_group) {
        // Recompute the cost of all label groups.
        // TODO: collect group ids for which recomputation is required?
        recompute_group_costs();
    }
}

void LabelEquivalenceRelation::move_group_into_group(
    int from_group_id, int to_group_id) {
    assert(!is_empty_group(from_group_id));
    assert(!is_empty_group(to_group_id));
    LabelGroup &from_group = grouped_labels[from_group_id];
    for (int label_no : from_group) {
        add_label_to_group(to_group_id, label_no);
    }
    from_group.clear();
}

int LabelEquivalenceRelation::add_label_group(const vector<int> &new_labels) {
    int new_group_id = grouped_labels.size();
    grouped_labels.push_back(LabelGroup());
    for (int label_no : new_labels) {
        add_label_to_group(new_group_id, label_no);
    }
    return new_group_id;
}
}
