//
// Created by Jason Shen on 4/2/17.
//

#include <cassert>
#include <stack>
#include <iostream>
#include <ios>
#include <sstream>
#include <fstream>
#include <queue>
#include "psdd_manager.hpp"

void PsddManager::inc_ref(PsddNode *node) {
    if (m_node_ref_cnt.find((size_t) node) != m_node_ref_cnt.end()){
        m_node_ref_cnt[(size_t)node] +=1;
    }else{
        m_node_ref_cnt[(size_t)node] = 1;
    }
}

void PsddManager::dec_ref(PsddNode *node) {
    auto ref_it = m_node_ref_cnt.find((size_t)node);
    assert(ref_it != m_node_ref_cnt.end());
    ref_it->second -=1;
    if (ref_it->second == 0){
        m_node_ref_cnt.erase(ref_it);
    }
}

PsddNode *
PsddManager::create_unique_decn_node(Vtree *v, std::vector<PsddElement>& elements) {
    return m_put.create_decn_node(v, elements);
}

PsddNode *PsddManager::create_unique_literal_node(Vtree *v, size_t var_index, bool lit_sign) {
    return m_put.create_literal_node(v, var_index, lit_sign);
}

PsddNode *
PsddManager::create_unique_simple_node(Vtree *v, size_t var_index, PsddParameter pos_param, PsddParameter neg_param) {
    return m_put.create_simple_node(v, var_index, pos_param, neg_param);
}

VtreeManager *PsddManager::get_vtree_manager() const {
    return m_vm;
}

void PsddManager::init_by_vtree_manager(VtreeManager *vm) {
    m_vm = new VtreeManager(*vm);
    m_put.init(vm->get_vtree_size());
}

void PsddManager::init_by_vtree_file(const char *vtree_file) {
    m_vm = new VtreeManager();
    m_vm->read_vtree_file(vtree_file);
    m_put.init(m_vm->get_vtree_size());
}

void PsddManager::gc_manual() {
    std::unordered_set<size_t> refed_nodes;
    for (auto i = m_node_ref_cnt.begin(); i != m_node_ref_cnt.end(); i ++){
        std::stack<PsddNode*> open_list;
        if (refed_nodes.find(i->first) != refed_nodes.end()){
            continue;
        }
        refed_nodes.insert(i->first);
        open_list.push((PsddNode*)i->first);
        while (!open_list.empty()){
            PsddNode* top = open_list.top();
            open_list.pop();
            if (top->get_type() == PSDD_LITERAL_TYPE || top->get_type() == PSDD_SIMPLE_TYPE){
                continue;
            }else{
                assert(top->get_type() == PSDD_DECN_TYPE);
                const std::vector<PsddElement>& children = top->get_elements();
                for (auto j = children.begin(); j != children.end(); j++){
                    PsddNode* prime = j->get_prime();
                    PsddNode* sub = j->get_sub();
                    if (refed_nodes.find((size_t)prime) == refed_nodes.end()){
                        refed_nodes.insert((size_t)prime);
                        open_list.push(prime);
                    }
                    if (refed_nodes.find((size_t)sub) == refed_nodes.end()){
                        refed_nodes.insert((size_t) sub);
                        open_list.push(sub);
                    }
                }
            }
        }
    }
    // try to iterate over the unique table.
    std::vector<std::unordered_set<PsddNode*, psdd_unique_util, psdd_unique_util> >& unique_table =
            m_put.get_unique_table();
    for (auto i = unique_table.begin(); i != unique_table.end(); i++){
        std::unordered_set<PsddNode*, psdd_unique_util, psdd_unique_util>& unique_table_at_vtree = *i;
        auto j = unique_table_at_vtree.begin();
        while (j != unique_table_at_vtree.end()){
            if (refed_nodes.find((size_t)*j) == refed_nodes.end()){
                j = unique_table_at_vtree.erase(j);
            }else{
                j ++;
            }
        }
    }
}
struct psdd_ptr_pair_util{
    bool operator()(const std::pair<PsddNode*, PsddNode*>& first, const std::pair<PsddNode*, PsddNode*>& second) const{
        if (first.first == second.first){
            return first.second == second.second;
        }else if (first.second == second.first){
            return first.first == second.second;
        }else{
            return false;
        }
    }
    size_t operator()(const std::pair<PsddNode*, PsddNode* >& pair) const{
        return std::hash<size_t>()((size_t) pair.first) ^ std::hash<size_t>()((size_t) pair.second + sizeof(PsddNode*));
    }
};

std::pair<PsddNode *, PsddParameter> multiply_helper(PsddNode* node1, PsddNode* node2, PsddManager* pm, Vtree* parent,
                                                     std::unordered_map<std::pair<PsddNode*, PsddNode*>,
                                                             std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util>& mult_cache);

std::pair<PsddNode*, PsddParameter> multiply_independent_node(PsddNode* node1, PsddNode* node2, Vtree* common_ans,
                                                              PsddManager* pm){
    std::vector<PsddElement> elements;
    if (common_ans->get_inorder_index() > node1->get_vtree()->get_inorder_index()){
        // node1 is left child
        elements.push_back({PsddElement(node1, node2, PsddParameter::get_from_regular(1))});
    }else{
        elements.push_back({PsddElement(node2, node1, PsddParameter::get_from_regular(1))});
    }
    PsddNode* result = pm->create_unique_decn_node(common_ans,elements);
    return {result, PsddParameter::get_from_regular(1)};
}


std::pair<PsddNode*, PsddParameter> multiply_lit_lit(PsddNode* lit1, PsddNode* lit2, PsddManager* pm ,Vtree* parent,
                                                     std::unordered_map<std::pair<PsddNode*, PsddNode*>,
                                                             std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util>& mult_cache){
    VtreeManager* vm = pm->get_vtree_manager();
    if (lit1->get_vtree() == lit2->get_vtree()){
        if (lit1->get_literal_sign() == lit2->get_literal_sign()){
            return {lit1, PsddParameter::get_from_regular(1)};
        }else{
            return {nullptr, PsddParameter::get_from_regular(0)};
        }
    }else{
        Vtree* common_ans = vm->get_lca(lit1->get_vtree(), lit2->get_vtree(), parent);
        return multiply_independent_node(lit1, lit2, common_ans, pm);
    }
}

std::pair<PsddNode*, PsddParameter> multiply_lit_simple(PsddNode* lit, PsddNode* simple, PsddManager* pm, Vtree* parent,
                                                        std::unordered_map<std::pair<PsddNode*, PsddNode*>,
                                                                std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util>& mult_cache){
    VtreeManager* vm = pm->get_vtree_manager();
    if (lit->get_vtree() == simple->get_vtree()){
        if (lit->get_literal_sign()){
            return {lit, simple->get_pos_param()};
        }else{
            return {lit, simple->get_neg_param()};
        }
    }else{
        Vtree* common_ans = vm->get_lca(lit->get_vtree(), simple->get_vtree(), parent);
        return multiply_independent_node(lit, simple, common_ans, pm);
    }
}

std::pair<PsddNode*, PsddParameter> multiply_term_decn(PsddNode* term, PsddNode* decn, PsddManager* pm, Vtree* parent,
                                                      std::unordered_map<std::pair<PsddNode*, PsddNode*>,
                                                              std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util >& mult_cache){
    VtreeManager* vm = pm->get_vtree_manager();
    Vtree* common_ans = vm->get_lca(term->get_vtree(), decn->get_vtree(), parent);
    if (common_ans == decn->get_vtree()){
        const std::vector<PsddElement>& elements = decn->get_elements();
        std::vector<PsddElement> next_elements;
        std::vector<PsddNode*> next_prime_nodes;
        std::vector<PsddNode*> next_sub_nodes;
        std::vector<PsddParameter> next_params;
        PsddParameter next_norm = PsddParameter::get_from_regular(0);
        if (common_ans->get_inorder_index() > term->get_vtree()->get_inorder_index()){
            // lit is multiplied with the prime
            for (auto i = elements.begin(); i != elements.end(); i++){
                auto result = multiply_helper(term, i->get_prime(), pm, common_ans, mult_cache);
                if (result.first != nullptr){
                    next_prime_nodes.push_back(result.first);
                    next_sub_nodes.push_back(i->get_sub());
                    next_params.push_back(i->get_parameter()*result.second);
                    next_norm = next_norm + next_params.back();
                }
            }
        }else{
            // lit is multiplied with the sub
            for (auto i = elements.begin(); i != elements.end(); i++){
                auto result = multiply_helper(term, i->get_sub(), pm, common_ans, mult_cache);
                if (result.first != nullptr){
                    next_prime_nodes.push_back(i->get_prime());
                    next_sub_nodes.push_back(result.first);
                    next_params.push_back(i->get_parameter()*result.second);
                    next_norm = next_norm + next_params.back();
                }
            }
        }
        size_t next_elements_size = next_prime_nodes.size();
        for (auto i = 0; i < next_elements_size; i++){
            next_elements.push_back(PsddElement(next_prime_nodes[i], next_sub_nodes[i], next_params[i]/next_norm));
        }
        if (next_elements.size() != 0){
            return {pm->create_unique_decn_node(common_ans, next_elements), next_norm};
        }else{
            return {nullptr, PsddParameter::get_from_regular(0)};
        }
    }else{
        return multiply_independent_node(term, decn, common_ans, pm);
    }
}


std::pair<PsddNode*, PsddParameter> multiply_simple_simple(PsddNode* simple_1, PsddNode* simple_2, PsddManager* pm, Vtree* parent,
                                                           std::unordered_map<std::pair<PsddNode*, PsddNode*>,
                                                                   std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util>& mult_cache){
    if (simple_1->get_vtree() == simple_2->get_vtree()){
        PsddParameter pos_param_prod = simple_1->get_pos_param() * simple_2->get_pos_param();
        PsddParameter neg_param_prod = simple_1->get_neg_param() * simple_2->get_neg_param();
        PsddParameter next_norm = pos_param_prod + neg_param_prod;
        PsddNode* next_node = pm->create_unique_simple_node(simple_1->get_vtree(), simple_1->get_var_index(),
                                                            pos_param_prod/next_norm, neg_param_prod/next_norm);
        return {next_node, next_norm};
    }else{
        Vtree* common_ans = pm->get_vtree_manager()->get_lca(simple_1->get_vtree(), simple_2->get_vtree(), parent);
        return multiply_independent_node(simple_1, simple_2, common_ans, pm);
    }
}


std::pair<PsddNode*, PsddParameter> multiply_decn_decn(PsddNode* decn_1, PsddNode* decn_2, PsddManager* pm, Vtree* parent,
                                                       std::unordered_map<std::pair<PsddNode*, PsddNode*>,
                                                               std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util>& mult_cache){
    Vtree* common_ans = pm->get_vtree_manager()->get_lca(decn_1->get_vtree(), decn_2->get_vtree(), parent);
    if (common_ans == decn_1->get_vtree() || common_ans == decn_2->get_vtree()){
        std::vector<PsddNode*> next_primes;
        std::vector<PsddNode*> next_subs;
        std::vector<PsddParameter> next_params;
        PsddParameter next_norm = PsddParameter::get_from_regular(0);
        if (decn_1->get_vtree() == decn_2->get_vtree()){
            const std::vector<PsddElement>& decn1_elements = decn_1->get_elements();
            const std::vector<PsddElement>& decn2_elements = decn_2->get_elements();
            for (auto i = decn1_elements.begin(); i != decn1_elements.end(); i++){
                PsddNode* prime1= i->get_prime();
                PsddNode* sub1= i->get_sub();
                PsddParameter parami = i->get_parameter();
                for (auto j = decn2_elements.begin(); j != decn2_elements.end(); j++){
                    auto prime_res = multiply_helper(prime1, j->get_prime(), pm, decn_1->get_vtree(),mult_cache);
                    auto sub_res = multiply_helper(sub1, j->get_sub(), pm, decn_1->get_vtree(), mult_cache);
                    if (prime_res.first == nullptr || sub_res.first == nullptr){
                        continue;
                    }else{
                        next_primes.push_back(prime_res.first);
                        next_subs.push_back(sub_res.first);
                        PsddParameter cur_param = parami * j->get_parameter() * prime_res.second * sub_res.second;
                        next_norm = next_norm + cur_param;
                        next_params.push_back(cur_param);
                    }
                }
            }
        }else if (decn_1->get_vtree() == common_ans){
            const std::vector<PsddElement>& decn1_elements = decn_1->get_elements();
            if (decn_2->get_vtree()->get_inorder_index() < common_ans->get_inorder_index()){
                // decn2 is on the prime side;
                for (auto i = decn1_elements.begin(); i != decn1_elements.end(); i++){
                    PsddNode* prime = i->get_prime();
                    auto prime_res = multiply_helper(prime, decn_2, pm, common_ans, mult_cache);
                    if (prime_res.first != nullptr){
                        next_primes.push_back(prime_res.first);
                        next_subs.push_back(i->get_sub());
                        next_params.push_back(i->get_parameter() * prime_res.second);
                        next_norm = next_norm + next_params.back();
                    }
                }
            }else{
                for (auto i = decn1_elements.begin(); i != decn1_elements.end(); i++){
                    PsddNode* sub = i->get_sub();
                    auto sub_res = multiply_helper(sub, decn_2, pm,common_ans, mult_cache);
                    if (sub_res.first != nullptr){
                        next_primes.push_back(i->get_prime());
                        next_subs.push_back(sub_res.first);
                        next_params.push_back(i->get_parameter() * sub_res.second);
                        next_norm = next_norm + next_params.back();
                    }
                }
            }
        }else{
            assert(decn_2->get_vtree() == common_ans);
            const std::vector<PsddElement>& decn2_elements = decn_2->get_elements();
            if (decn_1->get_vtree()->get_inorder_index() < common_ans->get_inorder_index()){
                // decn1 is on the prime side;
                for (auto i = decn2_elements.begin(); i != decn2_elements.end(); i++){
                    PsddNode* prime = i->get_prime();
                    auto prime_res = multiply_helper(prime, decn_1, pm, common_ans, mult_cache);
                    if (prime_res.first != nullptr){
                        next_primes.push_back(prime_res.first);
                        next_subs.push_back(i->get_sub());
                        next_params.push_back(i->get_parameter() * prime_res.second);
                        next_norm = next_norm + next_params.back();
                    }
                }
            }else{
                for (auto i = decn2_elements.begin(); i != decn2_elements.end(); i++){
                    PsddNode* sub = i->get_sub();
                    auto sub_res = multiply_helper(sub, decn_1, pm,common_ans, mult_cache);
                    if (sub_res.first != nullptr){
                        next_primes.push_back(i->get_prime());
                        next_subs.push_back(sub_res.first);
                        next_params.push_back(i->get_parameter() * sub_res.second);
                        next_norm = next_norm + next_params.back();
                    }
                }
            }
        }
        std::vector<PsddElement> next_elements;
        size_t element_size = next_primes.size();
        if (element_size == 0){
            return {nullptr, PsddParameter::get_from_regular(0)};
        }else{
            for (auto i = 0; i< element_size; i ++){
                next_elements.push_back(PsddElement(next_primes[i],next_subs[i],next_params[i]/next_norm));
            }
            PsddNode* next_node = pm->create_unique_decn_node(common_ans, next_elements);
            return {next_node, next_norm};
        }
    }else{
        return multiply_independent_node(decn_1, decn_2, common_ans, pm);
    }
}

std::pair<PsddNode *, PsddParameter> multiply_helper(PsddNode* node1, PsddNode* node2, PsddManager* pm, Vtree* parent,
                                                     std::unordered_map<std::pair<PsddNode*, PsddNode*>,
                                                             std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util>& mult_cache){
    if (mult_cache.find({node1, node2}) != mult_cache.end()){
        return mult_cache[{node1, node2}];
    }
    std::pair<PsddNode*, PsddParameter> result;
    if (node1->get_type() == PSDD_LITERAL_TYPE){
        if (node2->get_type() == PSDD_LITERAL_TYPE){
            result = multiply_lit_lit(node1,node2,pm, parent, mult_cache);
        }else if (node2->get_type() == PSDD_SIMPLE_TYPE){
            result = multiply_lit_simple(node1, node2, pm, parent, mult_cache);
        }else{
            result = multiply_term_decn(node1, node2, pm, parent, mult_cache);
        }
    }else if (node1->get_type() == PSDD_SIMPLE_TYPE){
        if (node2->get_type() == PSDD_LITERAL_TYPE){
            result = multiply_lit_simple(node2,node1,pm, parent, mult_cache);
        }else if (node2->get_type() == PSDD_SIMPLE_TYPE){
            result = multiply_simple_simple(node1, node2, pm, parent, mult_cache);
        }else{
            result = multiply_term_decn(node1, node2, pm, parent, mult_cache);
        }
    }else{
        assert(node1->get_type() == PSDD_DECN_TYPE);
        if (node2->get_type() == PSDD_LITERAL_TYPE){
            result = multiply_term_decn(node2, node1,pm, parent, mult_cache);
        }else if (node2->get_type() == PSDD_SIMPLE_TYPE){
            result = multiply_term_decn(node2, node1, pm, parent, mult_cache);
        }else{
            result = multiply_decn_decn(node1, node2, pm, parent, mult_cache);
        }
    }
    mult_cache[{node1, node2}] = result;
    assert(!isnan(result.second.get_parameter_value()));
    return result;
}

std::pair<PsddNode *, PsddParameter> PsddManager::multiply(PsddNode *node1, PsddNode *node2) {
    std::unordered_map<std::pair<PsddNode*, PsddNode* >, std::pair<PsddNode*, PsddParameter>, psdd_ptr_pair_util, psdd_ptr_pair_util> mult_cache;
    return multiply_helper(node1, node2, this, m_vm->get_root(), mult_cache);
}

PsddNode* PsddManager::read_psdd_file(const char *psdd_file) {
    assert(m_vm != nullptr);
    std::vector<Vtree* > v_nodes = m_vm->serialize();
    std::vector<Vtree* > vnodes_map(m_vm->get_vtree_size());
    for (auto i = v_nodes.begin(); i != v_nodes.end(); i++){
        vnodes_map[(*i)->get_inorder_index()] = *i;
    }
    std::ifstream psddfs (psdd_file,std::ifstream::in);
    if (!psddfs){
        std::cerr << "psdd file " << psdd_file << " cannot be open."<<std::endl;
        exit(1);
    }
    std::string string_buf = "";
    std::getline(psddfs, string_buf);
    while (string_buf[0] == 'c'){
        std::getline(psddfs, string_buf);
    }
    std::istringstream iss(string_buf);
    std::string tok;
    iss >> tok;
    std::size_t psdd_size;
    int var_size;
    iss >> var_size >> psdd_size;
    std::vector<PsddNode*> node_cache(psdd_size,nullptr);
    std::size_t root_index = 0;
    for (auto i = 0; i< psdd_size; i++){
        std::getline(psddfs,string_buf);
        if (string_buf[0] == 'L'){
            iss.str(string_buf.substr(1,std::string::npos));
            iss.clear();
            std::size_t node_index;
            int vtree_index;
            int lit_var;
            iss >> node_index >> vtree_index >> lit_var;
            if (lit_var < 0){
                int var_index = -lit_var;
                node_cache[node_index] = create_unique_literal_node(vnodes_map[vtree_index], (size_t) var_index, false);
            }else{
                int var_index = lit_var;
                node_cache[node_index] = create_unique_literal_node(vnodes_map[vtree_index], (size_t) var_index, true);
            }
            root_index = node_index;
        }else if (string_buf[0] == 'T'){
            iss.str(string_buf.substr(1,std::string::npos));
            iss.clear();
            std::size_t node_index;
            int vtree_index;
            int var_index;
            double pos_weight_in_log;
            iss >> node_index >> vtree_index >> var_index >> pos_weight_in_log;
            double neg_weight_in_log = log(1-exp(pos_weight_in_log));
            node_cache[node_index] = create_unique_simple_node(vnodes_map[vtree_index], (size_t)var_index,
                                                               PsddParameter::get_from_log(pos_weight_in_log),
                                                               PsddParameter::get_from_log(neg_weight_in_log));
            root_index = node_index;
        }else{
            if (string_buf[0] != 'D'){
                std::cerr<< "unrecognized node type " << string_buf[0] << std::endl;
                exit(1);
            }
            assert(string_buf[0] == 'D');
            iss.str(string_buf.substr(1,std::string::npos));
            iss.clear();
            std::size_t node_index;
            int vtree_index;
            std::size_t element_size;
            iss >> node_index >> vtree_index >> element_size;
            std::vector<PsddElement> elements;
            for (auto j = 0; j < element_size; j++){
                std::size_t prime_index;
                std::size_t sub_index;
                double weight_in_log;
                iss >> prime_index >> sub_index >> weight_in_log;
                PsddNode* prime_node = node_cache[prime_index];
                PsddNode* sub_node = node_cache[sub_index];
                if (prime_node == nullptr || sub_node == nullptr){
                    std::cerr<< "children of node " << node_index << "  have children not defined before it in the psdd file " << std::endl;
                    exit(1);
                }
                elements.push_back(PsddElement(prime_node, sub_node, PsddParameter::get_from_log(weight_in_log)));
            }
            node_cache[node_index] = create_unique_decn_node(vnodes_map[vtree_index], elements);
            root_index = node_index;
        }
    }
    psddfs.close();
    return node_cache[root_index];
}

void PsddManager::write_psdd_file(PsddNode *root, size_t var_size, const char *psdd_file) {
    std::ofstream output_file;
    output_file.open(psdd_file);
    output_file << "c ids of psdd nodes start at 0\n";
    output_file << "c psdd nodes appear bottom-up, children before parents\n";
    output_file << "c file syntax:\n";
    output_file << "c psdd count-of-sdd-nodes\n";
    output_file << "c L id-of-literal-sdd-node id-of-vtree literal\n";
    output_file << "c T id-of-trueNode-sdd-node id-of-vtree variable log(litProb)\n";
    output_file << "c D id-of-decomposition-sdd-node id-of-vtree number-of-elements {id-of-prime id-of-sub log(elementProb)}\n";
    auto psdd_serialized = PsddManager::serialize(root);
    size_t index = 0;
    std::unordered_map<size_t,size_t > psdd_index_translation; // key is the node_index, and value is the index used in the file;
    output_file << "psdd " << var_size<<" " << psdd_serialized.size()<< "\n";
    for (auto i = psdd_serialized.rbegin(); i != psdd_serialized.rend(); i++ ){
        PsddNode* cur = *i;
        psdd_index_translation[cur->get_index()] = index++;
        if (cur->get_type() == PSDD_DECN_TYPE){
            const std::vector<PsddElement> & elements = cur->get_elements();
            output_file << "D " << psdd_index_translation[cur->get_index()] << " " << cur->get_vtree()->get_index()
                        << " " << elements.size();
            for (auto j = elements.begin(); j != elements.end(); j++){
                output_file<< " " << psdd_index_translation[j->get_prime()->get_index()]<< " "
                           << psdd_index_translation[j->get_sub()->get_index()]<<" "
                           << j->get_parameter().get_parameter_value();
            }
            output_file << "\n";
        }else if (cur->get_type() == PSDD_SIMPLE_TYPE){
            output_file << "T "<< psdd_index_translation[cur->get_index()]<< " " << cur->get_vtree()->get_index()<< " "
                        << cur->get_var_index() << " "<< cur->get_pos_param().get_parameter_value()<<"\n";
        }else {
            if (cur->get_literal_sign()){
                output_file << "L " <<  psdd_index_translation[cur->get_index()]<<" "<<cur->get_vtree()->get_index()
                            << " "<< cur->get_var_index()<< "\n";
            }else{
                output_file<<"L " << psdd_index_translation[cur->get_index()] << " "<<cur->get_vtree()->get_index()
                           <<" -"<<cur->get_var_index()<<"\n";
            }
        }
    }
    output_file.close();
}

std::vector<PsddNode *> PsddManager::serialize(PsddNode *root) {
    std::unordered_set<size_t> ptr_cache;
    std::vector<PsddNode*> result;
    ptr_cache.insert((size_t) root);
    result.push_back(root);
    size_t read_it = 0;
    while (read_it != result.size()){
        PsddNode* cur = result[read_it];
        if (cur->get_type() == PSDD_DECN_TYPE){
            auto elements = cur->get_elements();
            for (auto j = elements.begin(); j != elements.cend(); j++){
                PsddNode* prime = j->get_prime();
                PsddNode* sub = j->get_sub();
                if (ptr_cache.find((size_t)prime) == ptr_cache.end()){
                    ptr_cache.insert((size_t) prime);
                    result.push_back(prime);
                }
                if (ptr_cache.find((size_t)sub) == ptr_cache.end()){
                    ptr_cache.insert((size_t) sub);
                    result.push_back(sub);
                }
            }
        }
        read_it++;
    }
    return result;
}

