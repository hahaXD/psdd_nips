//
// Created by Jason Shen on 4/3/17.
//

#include "psdd_compiler.hpp"

int main(int argc, const char* argv[]){
    const char* uai_file = argv[1];
    PsddCompiler pc;
    pc.read_uai_file(uai_file);
}