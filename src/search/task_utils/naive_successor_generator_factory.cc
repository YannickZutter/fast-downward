//
// Created by yannick on 09.07.20.
//

#include "naive_successor_generator_factory.h"

#include "successor_generator_internals.h"

#include "../task_proxy.h"

#include "../utils/collections.h"
#include "../utils/memory.h"

#include <algorithm>
#include <cassert>

using namespace std;

namespace successor_generator{

    struct OperatorRange {
        int begin;
        int end;

        OperatorRange(int begin, int end)
                : begin(begin), end(end) {
        }

        bool empty() const {
            return begin == end;
        }

        int span() const {
            return end - begin;
        }
    };

class OperatorInfo {

    OperatorID op;
    vector<FactPair> precondition;
public:
    OperatorInfo(OperatorID op, vector<FactPair> precondition)
            : op(op),
              precondition(move(precondition)) {
    }

    bool operator<(const OperatorInfo &other) const {
        return precondition < other.precondition;
    }

    OperatorID get_op() const {
        return op;
    }

    // Returns -1 as a past-the-end sentinel.
    int get_var(int depth) const {
        if (depth == static_cast<int>(precondition.size())) {
            return -1;
        } else {
            return precondition[depth].var;
        }
    }

    int get_value(int depth) const {
        return precondition[depth].value;
    }
};

successor_generator::NaiveSuccessorGeneratorFactory::NaiveSuccessorGeneratorFactory(
        const TaskProxy &task_proxy)
        : task_proxy(task_proxy) {
}

successor_generator::NaiveSuccessorGeneratorFactory::~NaiveSuccessorGeneratorFactory() = default;

successor_generator::GeneratorPtr successor_generator::NaiveSuccessorGeneratorFactory::construct_fork(vector<GeneratorPtr> nodes) const {

    //do stuff construct fork
    return NULL;
}

successor_generator::GeneratorPtr successor_generator::NaiveSuccessorGeneratorFactory::construct_leaf(OperatorRange range) const {

    //do stuff construct leaf
    return NULL;
}

successor_generator::GeneratorPtr successor_generator::NaiveSuccessorGeneratorFactory::construct_switch(int switch_var_id, ValuesAndGenerators values_and_generators) const {

    //do stuff construct switch
    return NULL;
}

successor_generator::GeneratorPtr successor_generator::NaiveSuccessorGeneratorFactory::construct_recursive(int depth, OperatorRange range) const {

    //do stuff construct recursive
    return NULL;
}



}
