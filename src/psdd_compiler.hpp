//
// Created by Jason Shen on 4/3/17.
//

#ifndef PSDD_PSDD_COMPILER_HPP
#define PSDD_PSDD_COMPILER_HPP


#include "uai_network.hpp"
#include "psdd_manager.hpp"

class PsddCompiler{
public:
    PsddCompiler();
    void read_uai_file(const char* uai_file);
    std::pair<PsddNode*, PsddParameter> compile_cluster(size_t cluster_index);
private:
    UaiNetwork* m_network;
    PsddManager* m_pm;
};


#endif //PSDD_PSDD_COMPILER_HPP
