//
// Created by Jason Shen on 4/3/17.
//

#ifndef PSDD_PSDD_COMPILER_HPP
#define PSDD_PSDD_COMPILER_HPP


#include "uai_network.hpp"
#include "psdd_manager.hpp"
#define VTREE_METHOD_MINFILL 4
#define VTREE_METHOD_HYPER_RANDOM_BF 0
#define VTREE_METHOD_HYPER_FIXED_BF 1
class PsddCompiler{
public:
    PsddCompiler();
    void init_psdd_manager(char mode);
    void init_psdd_manager_using_vtree_manager(VtreeManager* v);
    void read_uai_file(const char* uai_file);
    PsddManager* get_psdd_manager() const;
    std::pair<PsddNode*, PsddParameter> compile_cluster(size_t cluster_index);
    std::pair<PsddNode*, PsddParameter> compile_network(size_t gc_freq);
private:
    UaiNetwork* m_network;
    PsddManager* m_pm;
};


#endif //PSDD_PSDD_COMPILER_HPP
