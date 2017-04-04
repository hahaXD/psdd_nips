//
// Created by Jason Shen on 4/1/17.
//

#ifndef PSDD_VTREE_HPP
#define PSDD_VTREE_HPP

#include <cstddef>
class Vtree {
public:
    Vtree(size_t index, Vtree* left, Vtree* right, size_t var_index);
    void set_parent(Vtree* parent);
    Vtree* get_parent() const;
    size_t get_index() const;
    size_t get_var_index()const;
    size_t get_var_cnt()const;
    size_t get_inorder_index() const;
    void set_inorder_index(size_t index);
    Vtree* get_left() const;
    Vtree* get_right() const;
    size_t get_depth() const;
    void set_var_index(size_t var_index);
    void set_left(Vtree* left);
    void set_right(Vtree* right);
    bool left_rotatable() const;
    bool right_rotatable() const;
    bool swapable()const;
private:
    size_t m_index;
    size_t m_inorder_index;
    size_t m_var_index;
    Vtree* m_left;
    Vtree* m_right;
    Vtree* m_parent;
    size_t m_var_cnt;
};


#endif //PSDD_VTREE_HPP
