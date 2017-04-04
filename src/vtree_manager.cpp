//
// Created by Jason Shen on 4/1/17.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <stack>
#include <random>
#include <algorithm>
#include "vtree_manager.hpp"

VtreeManager::VtreeManager():m_root(nullptr),m_vtree_size(0) {}

Vtree *VtreeManager::get_root() {
    return m_root;
}

Vtree *VtreeManager::get_leaf_node(size_t var_index) const{
    return m_leaves[var_index];
}

Vtree *VtreeManager::get_lca(Vtree *a, Vtree *b, Vtree *root) {
    if (root == a || root == b){
        return root;
    }else{
        size_t a_inorder = a->get_inorder_index();
        size_t b_inorder = b->get_inorder_index();
        size_t r_inorder = root->get_inorder_index();
        if (a_inorder < r_inorder){
            if (r_inorder < b_inorder){
                return root;
            }else{
                return get_lca(a, b, root->get_left());
            }
        }else{
            if (r_inorder > b_inorder){
                return root;
            } else{
                return get_lca(a, b, root->get_right());
            }
        }
    }
}

void set_inorder_index(Vtree* root){
    std::stack<Vtree*> open_stack;
    Vtree* cur = root;
    while (cur != nullptr){
        open_stack.push(cur);
        cur = cur->get_left();
    }
    size_t start_index = 0;
    while (!open_stack.empty()){
        Vtree* top = open_stack.top();
        open_stack.pop();
        top->set_inorder_index(start_index);
        start_index+=1;
        cur = top->get_right();
        while (cur != nullptr){
            open_stack.push(cur);
            cur = cur->get_left();
        }
    }
}

void VtreeManager::read_vtree_file(const char *vtree_file) {
    std::ifstream vtreefs (vtree_file,std::ifstream::in);
    if (!vtreefs){
        std::cerr << "vtree file " << vtree_file << " cannot be open."<<std::endl;
        exit(1);
    }
    std::string string_buf = "";
    getline(vtreefs, string_buf);
    while (string_buf[0] == 'c'){
        getline(vtreefs, string_buf);
    }
    std::istringstream iss(string_buf);
    std::string tok;
    iss >> tok;
    iss >> m_vtree_size;
    m_var_size = (m_vtree_size +1)/2;
    m_leaves.resize(m_var_size+1, nullptr);
    std::vector<Vtree*> node_cache(m_vtree_size, nullptr);
    for (int i = 0;i < m_vtree_size;++i){
        getline(vtreefs, string_buf);
        iss.str(string_buf);
        iss.clear();
        iss >> tok;
        if (tok == "L"){
            size_t vtree_index;
            iss >> vtree_index;
            size_t var_index;
            iss >> var_index;
            Vtree* new_node = new Vtree(vtree_index, nullptr, nullptr, var_index);
            m_leaves[var_index] = new_node;
            assert(vtree_index < m_vtree_size);
            node_cache[vtree_index] = new_node;
        }else{
            size_t vtree_index;
            size_t left_child;
            size_t right_child;
            iss >> vtree_index >> left_child >> right_child;
            Vtree* left_node = node_cache[left_child];
            Vtree* right_node = node_cache[right_child];
            Vtree* new_node = new Vtree(vtree_index, left_node, right_node, 0);
            left_node->set_parent(new_node);
            right_node->set_parent(new_node);
            assert(vtree_index < m_vtree_size);
            node_cache[vtree_index] = new_node;
            m_root = new_node;
        }
    }
    set_inorder_index(m_root);
}

size_t VtreeManager::get_var_size() const {
    return m_var_size;
}

size_t VtreeManager::get_vtree_size() const {
    return m_vtree_size;
}

bool VtreeManager::check_decision_path(const std::vector<size_t> &var_order) const {
    size_t order_size = var_order.size();
    Vtree* cur = m_root;
    for (auto i = order_size-2; i< order_size; i--){
        Vtree* lca_node = VtreeManager::get_lca(get_leaf_node(var_order[(i)]), get_leaf_node(var_order[i+1]), cur);
        if (lca_node->get_left() == get_leaf_node(var_order[i+1])){
            cur = lca_node;
        }else{
            return false;
        }
    }
    return true;
}

void set_vtree_parent(Vtree* root){
    if (root->get_left() != nullptr){
        root->get_left()->set_parent(root);
        set_vtree_parent(root->get_left());
        root->get_right()->set_parent(root);
        set_vtree_parent(root->get_right());
    }
}

void VtreeManager::generate_rl_vtree(const std::vector<int> &var_order) {
    m_var_size = var_order.size();
    m_vtree_size = 2*m_var_size-1;
    m_leaves.resize(m_var_size+1, nullptr);
    size_t vtree_index = 0;
    for (int i = 0; i < m_var_size; i++){
        m_leaves[i+1] = new Vtree(vtree_index++, nullptr, nullptr, (size_t)(i+1));
    }
    if (m_var_size == 1){
        m_root = m_leaves[0];
        m_root->set_inorder_index(0);
    }else{
        Vtree* cur = new Vtree(vtree_index++, m_leaves[var_order[1]], m_leaves[var_order[0]],0);
        for (int i = 2; i < m_var_size; i++){
            cur = new Vtree(vtree_index++, m_leaves[var_order[i]], cur,0);
        }
        m_root = cur;
    }
    set_vtree_parent(m_root);
    set_inorder_index(m_root);
}

void VtreeManager::generate_random_vtree(size_t var_size, unsigned int seed) {
    m_var_size = var_size;
    m_vtree_size = 2*var_size -1;
    m_leaves.resize(m_var_size +1, nullptr);
    size_t vtree_index  = 0;
    std::vector<Vtree*> random_pool;
    for (int i = 0; i < m_var_size; i++){
        m_leaves[i+1] = new Vtree(vtree_index++, nullptr, nullptr, (size_t)(i+1));
        random_pool.push_back(m_leaves[i+1]);
    }
    while (random_pool.size() >1){
        std::shuffle(random_pool.begin(), random_pool.end(), std::default_random_engine(seed));
        Vtree* v_last = random_pool[random_pool.size()-1];
        Vtree* v_s_last = random_pool[random_pool.size()-2];
        Vtree* new_v = new Vtree(vtree_index++, v_last, v_s_last,0);
        random_pool.pop_back();
        random_pool.pop_back();
        random_pool.push_back(new_v);
    }
    m_root = random_pool[0];
    set_vtree_parent(m_root);
    set_inorder_index(m_root);
}

std::vector<Vtree *> VtreeManager::serialize() const {
    std::vector<Vtree*> return_list;
    size_t read_it = 0;
    return_list.push_back(m_root);
    while (read_it < return_list.size()){
        Vtree* v = return_list[read_it];
        read_it++;
        if (v->get_left() != nullptr){
            return_list.push_back(v->get_left());
            return_list.push_back(v->get_right());
        }
    }
    std::reverse(return_list.begin(), return_list.end());
    return return_list;
}

void VtreeManager::write_dot_file(const char *dot_file) {
    std::ofstream dot_fd;
    dot_fd.open(dot_file);
    dot_fd << "graph g {\n  node [shape=plaintext];\n";
    std::vector<Vtree*> serialized_vtree = serialize();
    for (auto i = serialized_vtree.rbegin(); i != serialized_vtree.rend(); i ++){
        Vtree * cur = *i;
        if (cur->get_left() == nullptr){
            dot_fd << "node" << cur->get_index() << " [label=\""<< cur->get_var_index() <<"\"];\n";
        }else{
            dot_fd << "node" << cur->get_index() << " [label=<&#9679;>];\n";
        }
    }
    for (auto i = serialized_vtree.rbegin(); i != serialized_vtree.rend(); i ++){
        Vtree * cur = *i;
        if (cur->get_left() != nullptr){
            Vtree* left = cur->get_left();
            Vtree* right = cur->get_right();
            dot_fd << "node" << cur->get_index() << " -- node" << left->get_index()<<";\n";
            dot_fd << "node" << cur->get_index() << " -- node" << right->get_index()<<";\n";
        }
    }
    dot_fd <<"}\n";
    dot_fd.close();
}

size_t VtreeManager::get_depth() const {
    return m_root->get_depth();
}

void VtreeManager::swap_leaves(size_t var_a, size_t var_b) {
    Vtree* old_a = m_leaves[var_a];
    Vtree* old_b = m_leaves[var_b];
    old_a->set_var_index(var_b);
    old_b->set_var_index(var_a);
    Vtree* new_a = old_b;
    Vtree* new_b = old_a;
    m_leaves[var_a] = new_a;
    m_leaves[var_b] = new_b;
}

// (a v=(b c)) -> v= ((a b) c)
void VtreeManager::left_rotate(Vtree *v) {
    assert(v->left_rotatable());
    Vtree* b = v->get_left();
    Vtree* parent = v->get_parent();
    parent->set_right(b);
    v->set_left(parent);
    // check root;
    if (m_root == parent){
        m_root = v;
    }
    // check parent
    Vtree* old_parent_parent = parent->get_parent();
    if (old_parent_parent != nullptr){
        if (old_parent_parent->get_left() == parent){
            old_parent_parent->set_left(v);
        }else{
            old_parent_parent->set_right(v);
        }
    }
    v->set_parent(old_parent_parent);
    b->set_parent(parent);
    parent->set_parent(v);
}

// v=(w=(a b) c) -> w= (a v=(b c))
void VtreeManager::right_rotate(Vtree *v) {
    assert(v->right_rotatable());
    Vtree* w = v->get_left();
    Vtree* b = w->get_right();
    w->set_right(v);
    v->set_left(b);
    //check root, and parent
    if (m_root == v){
        m_root = w;
    }
    //check parent
    Vtree* old_parent_parent = v->get_parent();
    if (old_parent_parent != nullptr){
        if (old_parent_parent->get_left() == v){
            old_parent_parent->set_left(w);
        }else{
            old_parent_parent->set_right(w);
        }
    }
    w->set_parent(old_parent_parent);
    v->set_parent(w);
    b->set_parent(v);
}

VtreeManager::~VtreeManager() {
    auto vtree_list = serialize();
    for (auto k = vtree_list.begin(); k != vtree_list.end(); k++){
        delete(*k);
    }
}

VtreeManager::VtreeManager(const VtreeManager &other):m_leaves(other.get_var_size()+1) {
    auto vtree_list = other.serialize();
    std::vector<Vtree*> vtree_node_map(vtree_list.size());
    m_var_size = other.get_var_size();
    m_vtree_size = other.get_vtree_size();
    for (auto i = vtree_list.begin(); i != vtree_list.end(); i++){
        Vtree* cur = (*i);
        Vtree* left_node = cur->get_left();
        Vtree* right_node = cur->get_right();
        Vtree* new_node = nullptr;
        if(left_node == nullptr){
            new_node = new Vtree(cur->get_index(), nullptr, nullptr, cur->get_var_index());
            m_leaves[cur->get_var_index()] = new_node;
        }else{
            new_node = new Vtree(cur->get_index(), vtree_node_map[left_node->get_index()],
                                 vtree_node_map[right_node->get_index()],0);
        }
        new_node->set_inorder_index(cur->get_inorder_index());
        vtree_node_map[cur->get_index()] = new_node;
    }
    size_t root_index = vtree_list.back()->get_index();
    m_root = vtree_node_map[root_index];
    set_vtree_parent(m_root);
}

VtreeManager::VtreeManager(Vtree *root) {
    m_root = root;
    set_vtree_parent(m_root);
    set_inorder_index(m_root);
    auto vnodes = serialize();
    m_var_size = (vnodes.size()+1)/2;
    m_vtree_size = vnodes.size();
    m_leaves.resize(m_var_size+1, nullptr);
    for (auto i = vnodes.begin(); i != vnodes.end(); i++){
        if ((*i)->get_left() == nullptr){
            m_leaves[(*i)->get_var_index()] = (*i);
        }
    }
}

void VtreeManager::swap(Vtree *v) {
    Vtree* new_left = v->get_right();
    Vtree* new_right = v->get_left();
    v->set_left(new_left);
    v->set_right(new_right);
    set_inorder_index(m_root);
}



