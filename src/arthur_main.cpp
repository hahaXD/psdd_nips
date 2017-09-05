//
// Created by Yujia Shen on 9/4/17.
//

#include <iostream>
#include <time.h>
#include <tuple>
#include <stdio.h>
#include <assert.h>
#include <string>
#include "optionparser.h"
#include "psdd_compiler.hpp"
#include <stack>

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


enum  optionIndex {UNKNOWN, HELP, VTREE_INPUT, PSDD_OUTPUT};//,PSDD_INPUT};

const option::Descriptor usage[] =
{
    {HELP,    0,"" , "help",option::Arg::None, "--help  \tPrint usage and exit." },
    {VTREE_INPUT, 0, "", "vtree_intput", Arg::Required, "--vtree_input the vtree_file which will be used for the manuel vtree method"},
    //{PSDD_INPUT,0, "","psdd_input", Arg::Required, "--psdd_input input path of the psdd file for evaluation"},
    {PSDD_OUTPUT,0, "","psdd_output", Arg::Required, "--psdd_output output path of the psdd file for evaluation"},
    {UNKNOWN, 0,"" ,  ""   ,option::Arg::None, "\nExamples:\n./compile asia.uai --evid_file=asia.evid -m3 --vtree_input=asia.vtree --psdd_out=asia.psdd"},
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
    std::vector<const char*> psdd_file_names;
    for (int i = 0; i < parse.nonOptionsCount(); ++i)
        psdd_file_names.push_back(parse.nonOption(i));
    PsddManager pm;
    pm.init_by_vtree_file(options[VTREE_INPUT].arg);
    std::stack<PsddNode *> loaded_cpts;
    for (const auto& psdd_cpt_name : psdd_file_names){
        auto loaded_psdd = pm.read_psdd_file(psdd_cpt_name);
        pm.inc_ref(loaded_psdd);
        loaded_cpts.push(loaded_psdd);
    }
    if (loaded_cpts.size() == 0){
        std::cerr <<"Requires at least one PSDD file "<< std::endl;
        option::printUsage(std::cout, usage);
    }
    PsddParameter norm = PsddParameter::get_from_regular(1);
    while (loaded_cpts.size()>1){
        PsddNode* first = loaded_cpts.top();
        loaded_cpts.pop();
        PsddNode* second = loaded_cpts.top();
        loaded_cpts.pop();
        auto cur_res = pm.multiply(first, second);
        norm = norm * cur_res.second;
        pm.inc_ref(cur_res.first);
        pm.dec_ref(first);
        pm.dec_ref(second);
        loaded_cpts.push(cur_res.first);
    }
    std::cout << "Finished multiply. The norm of the PSDD is "<< norm.get_regular_value()<<" and the normalized PSDD is saving to PSDD file "<< options[PSDD_OUTPUT].arg << std::endl;
    PsddNode* result = loaded_cpts.top();
    PsddManager::write_psdd_file(result, pm.get_vtree_manager()->get_var_size(), options[PSDD_OUTPUT].arg);
    /*
    const char* inf_result_file = options[INF_OUTPUT].arg;
    // compilation
    PsddCompiler pc;
    if (!options[UAI_FILE]){
        std::cerr << "Must provide UAI file for compilation. For evaluation, please provide psdd_input file. See help" << std::endl;
        return 1;
    }
    pc.read_uai_file(options[UAI_FILE].arg);
    if (options[VTREE_INPUT]){
        VtreeManager* vm = new VtreeManager();
        vm->read_vtree_file(options[VTREE_INPUT].arg);
        pc.init_psdd_manager_using_vtree_manager(vm);
        // pc will keep its own copy of vm
        delete(vm);
    }else{
        if (!options[VTREE_METHOD]){
            std::cerr<< "requires either vtree input file or vtree method. see help" << std::endl;
            return 1;
        }
        pc.init_psdd_manager((char)atoi(options[VTREE_METHOD].arg));
    }
    auto result = pc.compile_network(100);
    std::cout << "norm "<< result.second.get_parameter_value() << std::endl;
    if (options[PSDD_OUTPUT]){
        PsddManager::write_psdd_file(result.first, pc.get_psdd_manager()->get_vtree_manager()->get_var_size(),
                                     options[PSDD_OUTPUT].arg);
    }
     */
}

