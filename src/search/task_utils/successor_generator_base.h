//
// Created by yannick on 06.07.20.
//
#ifndef SUCCESSOR_GENERATOR_BASE_H
#define SUCCESSOR_GENERATOR_BASE_H
#include <vector>
#include "../utils/timer.h"

class GlobalState;
class OperatorID;
class State;
class TaskProxy;

namespace successor_generator {

    class SuccessorGeneratorBase {

    public:

        double total_duration = 0;
        int num_of_calls = 0;

        SuccessorGeneratorBase();

        virtual ~SuccessorGeneratorBase();

        virtual void initialize(const TaskProxy &task_proxy) = 0;

        virtual void generate_applicable_ops(
                const State &state, std::vector<OperatorID> &applicable_ops) = 0;

        virtual void generate_applicable_ops(
                const GlobalState &state, std::vector<OperatorID> &applicable_ops) = 0;

        void statistics();


    };
}
#endif