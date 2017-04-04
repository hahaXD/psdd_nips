//
// Created by Jason Shen on 4/1/17.
//

#include "vtree.hpp"


Vtree::Vtree(size_t index, Vtree *left, Vtree *right, size_t var_index):m_index(index), m_var_index(var_index),
                                                                        m_left(left), m_right(right),
                                                                        m_inorder_index(0), m_parent(nullptr){
    if (left == nullptr){
        m_var_cnt = 1;
    }else{
        m_var_cnt = left->get_var_cnt() + right->get_var_cnt();
    }
}

size_t Vtree::get_index() const {
    return m_index;
}

size_t Vtree::get_inorder_index() const {
    return m_inorder_index;
}

Vtree *Vtree::get_left() const {
    return m_left;
}

Vtree *Vtree::get_right() const {
    return m_right;
}

void Vtree::set_inorder_index(size_t index) {
    m_inorder_index = index;
}

size_t Vtree::get_var_index() const {
    return m_var_index;
}

void Vtree::set_parent(Vtree *parent) {
    m_parent = parent;
}

Vtree *Vtree::get_parent() const {
    return m_parent;
}

size_t Vtree::get_var_cnt() const {
    return m_var_cnt;
}

size_t Vtree::get_depth() const {
    if (m_left == nullptr){
        return 1;
    }else{
        size_t left_depth = m_left->get_depth();
        size_t right_depth = m_right->get_depth();
        if (left_depth<right_depth){
            return right_depth+1;
        }else{
            return left_depth +1;
        }
    }
}

void Vtree::set_var_index(size_t var_index) {
    m_var_index = var_index;
}

bool Vtree::left_rotatable() const {
    return (m_left != nullptr) && m_parent != nullptr && m_parent->get_right() == this;
}

bool Vtree::right_rotatable() const {
    return (m_left != nullptr) && m_left->get_left() != nullptr;
}

void Vtree::set_left(Vtree *left) {
    m_left = left;
    m_var_cnt = m_left->get_var_cnt() + m_right->get_var_cnt();
}

void Vtree::set_right(Vtree *right) {
    m_right = right;
    m_var_cnt = m_left->get_var_cnt() + m_right->get_var_cnt();
}

bool Vtree::swapable() const {
    return m_right != nullptr;
}


