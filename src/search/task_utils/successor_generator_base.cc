
#include "successor_generator_base.h"
#include "../option_parser.h"
#include "../plugin.h"

namespace successor_generator{

    SuccessorGeneratorBase::SuccessorGeneratorBase(){

    }

    SuccessorGeneratorBase::~SuccessorGeneratorBase(){

    }


    static PluginTypePlugin<SuccessorGeneratorBase> _type_plugin(
            "SuccessorGeneratorBase",
            // TODO: Replace empty string by synopsis for the wiki page.
            "");

}