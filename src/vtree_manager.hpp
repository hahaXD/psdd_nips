//
// Created by Jason Shen on 4/1/17.
//

#ifndef PSDD_VTREE_MANAGER_HPP
#define PSDD_VTREE_MANAGER_HPP

#include <vector>
#include <unordered_map>
#include "vtree.hpp"

class VtreeManager{
public:
    VtreeManager();
    VtreeManager(Vtree* root);
    ~VtreeManager();
    VtreeManager(const VtreeManager& other);
    void read_vtree_file(const char* vtree_file);
    void write_vtree_file(const char* vtree_file);
    void write_dot_file(const char* dot_file);
    void generate_rl_vtree(const std::vector<int>& var_order);
    void generate_random_vtree (size_t var_size, unsigned int seed);
    Vtree* get_root();
    Vtree* get_leaf_node(size_t var_index) const;
    size_t get_var_size() const;
    size_t get_vtree_size() const;
    bool check_decision_path(const std::vector<size_t>& var_order) const;
    //serialize the vtree. The first element is the leaf, the last element is the root. children appears before parent.
    std::vector<Vtree*> serialize() const;
    static Vtree* get_lca(Vtree* a, Vtree* b, Vtree* root);
    size_t get_depth() const;
    void swap_leaves(size_t var_a, size_t b);
    void left_rotate(Vtree* v);
    void right_rotate(Vtree* v);
    void swap(Vtree* v);
private:
    Vtree* m_root;
    size_t m_var_size;
    size_t m_vtree_size;
    std::vector<Vtree*> m_leaves; // index is variable index starting from 1, value is the leaf vtree node;
};


#endif //PSDD_VTREE_MANAGER_HPP
