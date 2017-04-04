//
// Created by Jason Shen on 4/2/17.
//

#include "psdd_unique_table.hpp"

size_t psdd_unique_util::operator()(const PsddNode *node) const {
    return node->get_hash_value();
}

bool psdd_unique_util::operator()(const PsddNode *first, const PsddNode *second) const {
    return (*first) == (*second);
}

PsddUniqueTable::PsddUniqueTable(size_t vtree_size):m_unique_table(vtree_size), m_node_index_acc(0){}

PsddNode *PsddUniqueTable::create_decn_node(Vtree *v, size_t element_size, PsddNode **elements, PsddParameter *param) {
    PsddNode* new_node = new PsddNode(m_node_index_acc, v, PSDD_DECN_TYPE);
    new_node->set_decn_node(element_size, elements, param);
    return create_node_helper(new_node);
}


PsddNode *PsddUniqueTable::create_node_helper(PsddNode * new_node) {
    std::unordered_set <PsddNode*,  psdd_unique_util, psdd_unique_util>& cur_unique_table =
            m_unique_table[new_node->get_vtree()->get_index()];
    auto unique_it = cur_unique_table.find(new_node);
    if (unique_it == cur_unique_table.end()){
        cur_unique_table.insert(new_node);
        m_node_index_acc++;
        return new_node;
    }else{
        delete(new_node);
        return *unique_it;
    }
}

PsddNode *PsddUniqueTable::create_literal_node(Vtree *v, size_t var_index, bool lit_sign) {
    PsddNode* new_node = new PsddNode(m_node_index_acc, v, PSDD_LITERAL_TYPE);
    new_node->set_literal_node(var_index, lit_sign);
    return create_node_helper(new_node);
}

PsddNode *PsddUniqueTable::create_simple_node(Vtree *v, size_t var_index, PsddParameter pos_param, PsddParameter neg_param){
    PsddNode* new_node = new PsddNode(m_node_index_acc, v, PSDD_SIMPLE_TYPE);
    new_node->set_simple_node(var_index, pos_param, neg_param);
    return create_node_helper(new_node);
}





