//
// Created by Jason Shen on 4/1/17.
//
#include <functional>
#include "psdd_node.hpp"

PsddNode::PsddNode(size_t node_index, Vtree *v, char node_type):m_index(node_index), m_vtree(v), m_type(node_type),
                                                                m_hash_val(0),m_elements(),
                                                                m_pos_param(PsddParameter::get_from_log(0)),
                                                                m_neg_param(PsddParameter::get_from_log(0)),
                                                                m_var_lit(false){}

size_t PsddNode::get_index() const {
    return m_index;
}

Vtree *PsddNode::get_vtree() const {
    return m_vtree;
}

char PsddNode::get_type() const {
    return m_type;
}

const std::vector<PsddElement>& PsddNode::get_elements() const {
    return m_elements;
}

size_t PsddNode::get_size() const {
    return m_elements.size();
}


size_t PsddNode::get_var_index() const {
    return m_var_index;
}

PsddParameter PsddNode::get_pos_param() const {
    return m_pos_param;
}

PsddParameter PsddNode::get_neg_param() const {
    return m_neg_param;
}

PsddNode::~PsddNode() {}

size_t PsddNode::get_hash_value() const {
    return m_hash_val;
}

bool PsddNode::operator==(const PsddNode &other) const{
    if (other.m_vtree != m_vtree) {
        return false;
    }
    if (other.m_type != m_type){
        return false;
    }
    if (m_type == 3){
        // literal node
        return (m_var_lit == other.m_var_lit);
    }else if (m_type == 2) {
        // simple node
        return (m_pos_param == other.m_pos_param);
    }else{
        // decn node
        return m_elements == other.m_elements;
    }
}

void PsddNode::set_simple_node(size_t var_index, PsddParameter pos_param, PsddParameter neg_param){
    m_var_index = var_index;
    m_pos_param = pos_param;
    m_neg_param = neg_param;
    m_hash_val = pos_param.get_hash_value();
}

void PsddNode::set_literal_node(size_t var_index, bool lit_sign) {
    m_var_index = var_index;
    if (lit_sign){
        m_hash_val = std::hash<size_t>()(1000);
    }else{
        m_hash_val = std::hash<size_t>()(1001);
    }
    m_var_lit = lit_sign;
}

bool PsddNode::get_literal_sign() const {
    return m_var_lit;
}

void PsddNode::set_decn_node(const std::vector<PsddElement> &elements) {
    m_elements = elements;
    for (auto i = m_elements.begin(); i != m_elements.end(); i++){
        m_hash_val^= i->get_hash_value();
    }
}


