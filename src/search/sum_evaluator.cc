#include "sum_evaluator.h"

#include <limits>
#include <cassert>

#include "option_parser.h"
#include "plugin.h"

SumEvaluator::SumEvaluator(const Options &opts)
    : CombiningEvaluator(opts.get_list<ScalarEvaluator *>("evals")) {
}

SumEvaluator::SumEvaluator(const std::vector<ScalarEvaluator *> &evals)
    : CombiningEvaluators(evals) {
}

SumEvaluator::~SumEvaluator() {
}

int SumEvaluator::combine_values(const vector<int> &values) {
    int result = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        assert(values[i] >= 0);
        result += values[i];
        assert(result >= 0); // Check against overflow.
    }
    return result;
}



static ScalarEvaluator *_parse(OptionParser &parser) {
    parser.add_list_option<ScalarEvaluator *>("evals");
    Options opts = parser.parse();
    if (parser.help_mode())
        return 0;

    if (opts.get_list<ScalarEvaluator *>("evals").empty())
        parser.error("expected non-empty list of scalar evaluators");
    if (parser.dry_run())
        return 0;
    else
        return new SumEvaluator(opts);
}

static ScalarEvaluatorPlugin _plugin("sum", _parse);
