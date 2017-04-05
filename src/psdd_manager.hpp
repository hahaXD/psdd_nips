//
// Created by Jason Shen on 4/2/17.
//

#ifndef PSDD_PSDD_MANAGER_HPP
#define PSDD_PSDD_MANAGER_HPP


#include "psdd_unique_table.hpp"
#include "vtree_manager.hpp"

class PsddManager{
public:
    void init_by_vtree_manager(VtreeManager* vm);
    void init_by_vtree_file(const char* vtree_file);
    void inc_ref(PsddNode* node);
    void dec_ref(PsddNode* node);
    PsddNode* read_psdd_file(const char* psdd_file);
    static void write_psdd_file(PsddNode* root, size_t var_size, const char* psdd_file);
    static std::vector<PsddNode*> serialize(PsddNode* root);
    PsddNode* create_unique_decn_node(Vtree* v, std::vector<PsddElement>& elements);
    PsddNode* create_unique_literal_node(Vtree* v, size_t var_index, bool lit_sign);
    PsddNode* create_unique_simple_node(Vtree* v, size_t var_index, PsddParameter pos_param, PsddParameter neg_param);
    std::pair<PsddNode*,PsddParameter> multiply(PsddNode* node1, PsddNode* node2);
    void gc_manual();
    VtreeManager* get_vtree_manager() const;
private:
    PsddUniqueTable m_put;
    VtreeManager* m_vm;
    std::unordered_map<size_t, size_t> m_node_ref_cnt;
};


#endif //PSDD_PSDD_MANAGER_HPP
