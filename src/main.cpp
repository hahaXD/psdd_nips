//
//  main.cpp
//  psdd
//
//  Created by Jason Shen on 10/25/16.
//  Copyright © 2016 Jason Shen. All rights reserved.
//

#include <iostream>
#include <time.h>
#include <tuple>
#include <stdio.h>
#include <assert.h>
#include "optionparser.h"
#include "psdd_compiler.hpp"

struct Arg: public option::Arg
{
    static void printError(const char* msg1, const option::Option& opt, const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, (size_t)opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }
    
    static option::ArgStatus Required(const option::Option& option, bool msg)
    {
        if (option.arg != 0)
            return option::ARG_OK;
        
        if (msg) printError("Option '", option, "' requires an argument\n");
        return option::ARG_ILLEGAL;
    }
    
    static option::ArgStatus Numeric(const option::Option& option, bool msg)
    {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
        if (endptr != option.arg && *endptr == 0)
            return option::ARG_OK;
        
        if (msg) printError("Option '", option, "' requires a numeric argument\n");
        return option::ARG_ILLEGAL;
    }
};


enum  optionIndex {UNKNOWN, HELP, VTREE_METHOD, VTREE_OUTPUT, PSDD_OUTPUT,UAI_FILE, EVID_FILE, PSDD_INPUT, INF_OUTPUT};

const option::Descriptor usage[] =
{
    {UNKNOWN, 0,"" , ""    ,option::Arg::None, "USAGE: example [options]\n\n \tOptions:"},
    {HELP,    0,"" , "help",option::Arg::None, "--help  \tPrint usage and exit." },
    {VTREE_METHOD, 0,"m", "vtree_method",Arg::Numeric, "--vtree_method, -m  Set vtree method.\n\t0 hyper vtree with random branching factor,\n\t1 hyper vtree generated using fixed branching factor, \n\t4 vtree with minfill." },
    {VTREE_OUTPUT, 0, "", "vtree_output", Arg::Optional, "--vtree_output output file path for the vtree used for the compilation."},
    {PSDD_OUTPUT, 0, "", "psdd_output", Arg::Optional, "--psdd_output output path of the psdd file for the compiled psdd"},
    {PSDD_INPUT,0, "","psdd_input", Arg::Optional, "--psdd_input input path of the psdd file for evaluation"},
    {UAI_FILE, 0, "", "uai_file", Arg::Optional, "--uai_file uai file path"},
    {EVID_FILE,0, "", "evid_file", Arg::Optional, "--evid_file evid file path"},
    {INF_OUTPUT,0, "", "inf_output", Arg::Required, "--inf_output output file path for inference result"},
    {UNKNOWN, 0,"" ,  ""   ,option::Arg::None, "\nExamples:\n./psdd --uai_file=asia.uai --evid_file=asia.evid -m3 --vtree_output=asia.vtree --psdd_out=asia.psdd"},
    {0,0,0,0,0,0}
};
    
int main(int argc, char* argv[]){
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);
    if (parse.error())
        return 1;
    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        return 0;
    }
    const char* inf_result_file = options[INF_OUTPUT].arg;
    // compilation
    PsddCompiler pc;
    if (!options[UAI_FILE]){
        std::cerr << "Must provide UAI file for compilation. For evaluation, please provide psdd_input file. See help" << std::endl;
        return 1;
    }
    pc.read_uai_file(options[UAI_FILE].arg);
    if (!options[VTREE_METHOD]){
      std::cerr<< "requires vtree method. see help" << std::endl;
      return 1;
    }
    pc.init_psdd_manager((char)atoi(options[VTREE_METHOD].arg));
    auto result = pc.compile_network(100);
    std::cout << "norm "<< result.second.get_parameter_value() << std::endl;
    if (options[PSDD_OUTPUT]){
        PsddManager::write_psdd_file(result.first, pc.get_psdd_manager()->get_vtree_manager()->get_var_size(),
                                     options[PSDD_OUTPUT].arg);
    }
    if (options[VTREE_OUTPUT]){
      pc.get_psdd_manager()->get_vtree_manager()->write_vtree_file(options[VTREE_OUTPUT].arg);
    }
    /*
    auto result = buce.Compile(uai, evid);
    auto inf_result = PsddNode::Evaluate(std::get<0>(result), evid, uai->get_var_size());
    inf_result.set_pr(std::get<1>(result));
    inf_result.write_to_file(inf_result_file);
    if (options[PSDD_OUTPUT]){
        PsddNode::write_to_file(std::get<0>(result), uai->get_var_size(), options[PSDD_OUTPUT].arg);
    }*/
}
