//
// Created by yannick on 06.07.20.
//
#ifndef SUCCESSOR_GENERATOR_BASE_H
#define SUCCESSOR_GENERATOR_BASE_H
#include <vector>

class GlobalState;
class OperatorID;
class State;
class TaskProxy;

namespace successor_generator {

    class SuccessorGeneratorBase {

    public:
        SuccessorGeneratorBase();

        virtual ~SuccessorGeneratorBase();

        virtual void initialize(const TaskProxy &task_proxy) = 0;

        virtual void generate_applicable_ops(
                const State &state, std::vector<OperatorID> &applicable_ops) = 0;

        virtual void generate_applicable_ops(
                const GlobalState &state, std::vector<OperatorID> &applicable_ops) = 0;


    };
}
#endif