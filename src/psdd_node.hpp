//
// Created by Jason Shen on 4/1/17.
//

#ifndef PSDD_PSDD_NODE_HPP
#define PSDD_PSDD_NODE_HPP

#define PSDD_DECN_TYPE 1
#define PSDD_SIMPLE_TYPE 2
#define PSDD_LITERAL_TYPE 3
#define PSDD_CONSTANT_TYPE 4

#include <cstddef>
#include "vtree.hpp"
#include "psdd_parameter.hpp"

class PsddNode{
public:
    PsddNode(size_t node_index, Vtree* v, char node_type);
    ~PsddNode();
    bool operator==(const PsddNode& other) const;
    size_t get_index() const;
    Vtree* get_vtree() const;
    char get_type() const;
    //It will steal the node_element ptr and params_ptr;
    void set_decn_node(size_t element_size, PsddNode** node_element, PsddParameter* params);
    PsddNode** get_elements() const;
    PsddParameter* get_params() const;
    size_t get_size()const;
    void set_simple_node(size_t var_index, PsddParameter pos_param, PsddParameter neg_param);
    void set_literal_node(size_t var_index, bool lit_sign);
    size_t get_var_index()const;
    PsddParameter get_pos_param() const;
    PsddParameter get_neg_param() const;
    size_t get_hash_value() const;

private:
    size_t m_index;
    Vtree* m_vtree;
    char m_type; // 1 decn node; 2 simple_node; 3 is literal; 4 constant;
    PsddNode** m_elements;
    PsddParameter* m_params;
    size_t m_size;
    size_t m_var_index;
    bool m_var_lit;
    PsddParameter m_pos_param;
    PsddParameter m_neg_param;
    size_t m_hash_val;
};



#endif //PSDD_PSDD_NODE_HPP
