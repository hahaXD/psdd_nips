#include <iostream>
#include "psdd_compiler.hpp"

//
// Created by Jason Shen on 4/4/17.
//
int main(int argc, const char* argv[]){
    const char* uai_file = argv[1];
    PsddCompiler pc;
    pc.read_uai_file(uai_file);
    pc.init_psdd_manager(VTREE_METHOD_HYPER_RANDOM_BF);
    //auto result = pc.compile_cluster(0);
    auto result = pc.compile_network(100);
    PsddManager::write_psdd_file(result.first, pc.get_psdd_manager()->get_vtree_manager()->get_var_size(),"test.psdd");
    pc.get_psdd_manager()->get_vtree_manager()->write_vtree_file("test.vtree");
    PsddManager* new_pm = new PsddManager();
    new_pm->init_by_vtree_file("test.vtree");
    PsddNode* new_node = new_pm->read_psdd_file("test.psdd");
}
