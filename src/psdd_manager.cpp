//
// Created by Jason Shen on 4/2/17.
//

#include <cassert>
#include "psdd_manager.hpp"

void PsddManager::inc_ref(PsddNode *node) {
    if (m_node_ref_cnt.find((size_t) node) != m_node_ref_cnt.end()){
        m_node_ref_cnt[(size_t)node] +=1;
    }else{
        m_node_ref_cnt[(size_t)node] = 1;
    }
}

void PsddManager::dec_ref(PsddNode *node) {
    auto ref_it = m_node_ref_cnt.find((size_t)node);
    assert(ref_it != m_node_ref_cnt.end());
    ref_it->second -=1;
    if (ref_it->second == 0){
        m_node_ref_cnt.erase(ref_it);
    }
}

PsddManager::PsddManager(VtreeManager *vm):m_put(vm->get_vtree_size()) {
    m_vm = new VtreeManager(*vm);
}

PsddNode *
PsddManager::create_unique_decn_node(Vtree *v, size_t element_size, PsddNode **elements, PsddParameter *param) {
    return m_put.create_decn_node(v, element_size, elements, param);
}

PsddNode *PsddManager::create_unique_literal_node(Vtree *v, size_t var_index, bool lit_sign) {
    return m_put.create_literal_node(v, var_index, lit_sign);
}

PsddNode *
PsddManager::create_unique_simple_node(Vtree *v, size_t var_index, PsddParameter pos_param, PsddParameter neg_param) {
    return m_put.create_simple_node(v, var_index, pos_param, neg_param);
}

VtreeManager *PsddManager::get_vtree_manager() const {
    return m_vm;
}
