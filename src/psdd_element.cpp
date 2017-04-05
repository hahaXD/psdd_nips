//
// Created by Jason Shen on 4/4/17.
//

#include <functional>
#include "psdd_element.hpp"
#include "psdd_node.hpp"

PsddElement::PsddElement(PsddNode *prime, PsddNode *sub, PsddParameter param):m_prime(prime),m_sub(sub),m_param(param){}

PsddNode *PsddElement::get_prime() const {
    return m_prime;
}

PsddNode *PsddElement::get_sub() const {
    return m_sub;
}

PsddParameter PsddElement::get_parameter() const {
    return m_param;
}

size_t PsddElement::get_hash_value() const {
    return std::hash<size_t>()((size_t)m_prime)^ std::hash<size_t>()((size_t)m_sub+sizeof(PsddNode*))
           ^m_param.get_hash_value();
}

bool PsddElement::operator==(const PsddElement &other) const {
    return m_prime == other.m_prime && m_sub == other.m_sub && m_param == other.m_param;
}

