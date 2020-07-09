//
// Created by yannick on 09.07.20.
//

#ifndef FAST_DOWNWARD_NAIVE_SUCCESSOR_GENERATOR_FACTORY_H
#define FAST_DOWNWARD_NAIVE_SUCCESSOR_GENERATOR_FACTORY_H

#include <memory>
#include <vector>

using namespace std;

class TaskProxy;


namespace successor_generator {
    class GeneratorBase;

    using GeneratorPtr = unique_ptr<GeneratorBase>;

    struct OperatorRange;
    class OperatorInfo;


    class NaiveSuccessorGeneratorFactory {
        using ValuesAndGenerators = vector<pair<int, GeneratorPtr>>;

        const TaskProxy &task_proxy;
        vector<OperatorInfo> operator_infos;

        GeneratorPtr construct_fork(vector<GeneratorPtr> nodes) const;
        GeneratorPtr construct_leaf(OperatorRange range) const;
        GeneratorPtr construct_switch(
                int switch_var_id, ValuesAndGenerators values_and_generators) const;
        GeneratorPtr construct_recursive(int depth, OperatorRange range) const;
    public:
        explicit NaiveSuccessorGeneratorFactory(const TaskProxy &task_proxy);
        // Destructor cannot be implicit because OperatorInfo is forward-declared.
        ~NaiveSuccessorGeneratorFactory();
        GeneratorPtr create();
    };
}


#endif //FAST_DOWNWARD_NAIVE_SUCCESSOR_GENERATOR_FACTORY_H
