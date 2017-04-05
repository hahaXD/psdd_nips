//
// Created by Jason Shen on 4/2/17.
//

#ifndef PSDD_PSDD_UNIQUE_TABLE_HPP
#define PSDD_PSDD_UNIQUE_TABLE_HPP


#include <cstddef>
#include <unordered_set>
#include <vector>
#include "psdd_node.hpp"

struct psdd_unique_util{
    size_t operator()(const PsddNode* node) const;
    bool operator()(const PsddNode* first, const PsddNode* second) const;
};

class PsddUniqueTable{
public:
    void init(size_t vtree_size);
    // It will steal the reference for both node_element and the params
    PsddNode* create_decn_node(Vtree* v, std::vector<PsddElement>& elements);
    PsddNode* create_literal_node(Vtree* v, size_t var_index, bool lit_sign);
    PsddNode* create_simple_node(Vtree* v, size_t var_index, PsddParameter pos_param, PsddParameter neg_param);
    std::vector<std::unordered_set<PsddNode*, psdd_unique_util, psdd_unique_util> >& get_unique_table();
private:
    PsddNode* create_node_helper(PsddNode* to_create);
    size_t m_node_index_acc;
    std::vector<std::unordered_set<PsddNode*, psdd_unique_util, psdd_unique_util> > m_unique_table;
};


#endif //PSDD_PSDD_UNIQUE_TABLE_HPP
