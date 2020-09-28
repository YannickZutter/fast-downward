
#include "successor_generator_base.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "../utils/logging.h"

using namespace std;

namespace successor_generator{

    SuccessorGeneratorBase::SuccessorGeneratorBase(){

    }

    SuccessorGeneratorBase::~SuccessorGeneratorBase(){

    }

    void SuccessorGeneratorBase::statistics() {
        utils::g_log << "number of get_applicable_ops calls: " << num_of_calls << endl;
        utils::g_log << "average duration of get_applicable_ops calls: " << total_duration*1000/num_of_calls << endl;
        utils::g_log << "total duration of get_applicable_ops calls: " << total_duration << endl;
    }

    static PluginTypePlugin<SuccessorGeneratorBase> _type_plugin(
            "SuccessorGeneratorBase",
            // TODO: Replace empty string by synopsis for the wiki page.
            "");

}