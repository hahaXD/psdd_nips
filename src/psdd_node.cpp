//
// Created by Jason Shen on 4/1/17.
//
#include <functional>
#include "psdd_node.hpp"

PsddNode::PsddNode(size_t node_index, Vtree *v, char node_type):m_index(node_index), m_vtree(v), m_type(node_type),
                                                                m_hash_val(0),m_elements(nullptr), m_params(nullptr),
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

void PsddNode::set_decn_node(size_t element_size, PsddNode **node_element, PsddParameter *params){
    m_elements = node_element;
    m_size = element_size;
    m_params = params;
    m_hash_val = 0;
    for (auto i = 0 ; i < element_size; i++){
        m_hash_val = (std::hash<size_t>()((size_t)node_element[2*i]) >> i);
        m_hash_val ^= (std::hash<size_t>()((size_t)node_element[2*i+1]) >>i);
        m_hash_val ^=  node_element[i]->get_hash_value();
    }
}

PsddNode **PsddNode::get_elements() const {
    return m_elements;
}

PsddParameter *PsddNode::get_params() const {
    return m_params;
}

size_t PsddNode::get_size() const {
    return m_size;
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

PsddNode::~PsddNode() {
    if (m_elements){
        delete m_elements;
    }
    if (m_params){
        delete m_params;
    }
}

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
        if (other.m_size != m_size){
            return false;
        }
        for (size_t i = 0; i< m_size; i++){
            if (m_elements[2*i] != other.m_elements[2*i]){
                return false;
            }
            if (m_elements[2*i+1] != other.m_elements[2*i+1]){
                return false;
            }
            if (m_params[i] != other.m_params[i]){
                return false;
            }
        }
        return true;
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


