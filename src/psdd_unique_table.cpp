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

void PsddUniqueTable::init(size_t vtree_size) {
    m_unique_table = std::vector<std::unordered_set<PsddNode*, psdd_unique_util, psdd_unique_util>>(vtree_size);
    m_node_index_acc = 0;
}

bool compare_psdd_element_by_prime_index(const PsddElement& e1, const PsddElement& e2){
    return e1.get_prime()->get_index() < e2.get_prime()->get_index();
}

PsddNode *PsddUniqueTable::create_decn_node(Vtree *v, std::vector<PsddElement> &elements) {
    if (elements.size() == 2 && elements[0].get_sub() == elements[1].get_sub()&&
            elements[0].get_prime()->get_type()== PSDD_LITERAL_TYPE){
        std::vector<PsddElement> new_element;
        PsddParameter pos_param;
        PsddParameter neg_param;
        if (elements[0].get_prime()->get_literal_sign()){
            pos_param = elements[0].get_parameter();
            neg_param = elements[1].get_parameter();
        }else{
            pos_param = elements[1].get_parameter();
            neg_param = elements[0].get_parameter();
        }
        PsddNode* new_prime = create_simple_node(elements[0].get_prime()->get_vtree(),
                                                 elements[0].get_prime()->get_var_index(), pos_param, neg_param);
        new_element.push_back(PsddElement(new_prime, elements[0].get_sub(),PsddParameter::get_from_regular(1)));
        PsddNode* new_node = new PsddNode(m_node_index_acc, v, PSDD_DECN_TYPE);
        new_node->set_decn_node(new_element);
        return create_node_helper(new_node);
    }else{
        std::sort(elements.begin(), elements.end(), compare_psdd_element_by_prime_index);
        PsddNode* new_node = new PsddNode(m_node_index_acc, v, PSDD_DECN_TYPE);
        new_node->set_decn_node(elements);
        return create_node_helper(new_node);
    }
}

std::vector<std::unordered_set<PsddNode *, psdd_unique_util, psdd_unique_util> > &PsddUniqueTable::get_unique_table() {
    return m_unique_table;
}





