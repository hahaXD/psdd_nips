//
// Created by Jason Shen on 4/3/17.
//

#include <cassert>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "psdd_compiler.hpp"

PsddCompiler::PsddCompiler():m_network(nullptr),m_pm(nullptr) {}

bool comp_vtree_nodes(Vtree* a, Vtree* b){
        return a->get_inorder_index() > b->get_inorder_index();
}

std::pair<PsddNode*, PsddParameter> PsddCompiler::compile_cluster(size_t cluster_index) {
    auto ordered_cluster_variables = m_network->get_ordered_cluster(cluster_index);
    auto cluster_param  = m_network->get_parameter(cluster_index);
    VtreeManager* cur_vm = m_pm->get_vtree_manager();
    // LSB is the first variable in ordered_cluster_variables now
    std::reverse(ordered_cluster_variables.begin(), ordered_cluster_variables.end());
    std::vector<Vtree*> v_nodes;
    for (auto i = ordered_cluster_variables.begin(); i != ordered_cluster_variables.end(); i++){
        v_nodes.push_back(cur_vm->get_leaf_node(*i));
    }

    std::sort(v_nodes.begin(), v_nodes.end(), comp_vtree_nodes);
    std::vector<size_t> var_location_in_cluster;
    for (auto i = v_nodes.begin(); i != v_nodes.end(); i++){
        size_t var_index = (*i)->get_var_index();
        for (size_t j = 0; j < ordered_cluster_variables.size(); j++){
            if (ordered_cluster_variables[j] == var_index){
                var_location_in_cluster.push_back(j);
                break;
            }
        }
    }
    assert(var_location_in_cluster.size() == ordered_cluster_variables.size());
    std::unordered_map<std::size_t, PsddNode*>* context_node_cache =
            new std::unordered_map<std::size_t, PsddNode*> ();
    std::unordered_map<std::size_t, PsddParameter>* context_norm_cache =
            new std::unordered_map<std::size_t, PsddParameter>();
    size_t term_var_loc = var_location_in_cluster[0];
    size_t mask =(size_t) ((~0x0) ^ (1<<term_var_loc));
    PsddParameter zero_param = PsddParameter::get_from_regular(0);
    for (auto k = cluster_param.begin(); k != cluster_param.end(); k++){
        size_t param_index = (size_t) (k - cluster_param.begin());
        size_t context = param_index & mask;
        if (context_node_cache->find(context) != context_node_cache->end()){
            continue;
        }else{
            size_t neg_index = context;
            size_t pos_index = context | (1 << term_var_loc);
            PsddParameter neg_weight = cluster_param[neg_index];
            PsddParameter pos_weight = cluster_param[pos_index];
            PsddParameter new_norm = pos_weight + neg_weight;
            if (new_norm == zero_param){
                context_node_cache->insert({context, nullptr});
                context_norm_cache->insert({context,zero_param});
            }else if (pos_weight == zero_param){
                context_node_cache->insert({context, m_pm->create_unique_literal_node(v_nodes[0],v_nodes[0]->get_var_index(),false)});
                context_norm_cache->insert({context, new_norm});
            }else if (neg_weight == zero_param){
                context_node_cache->insert({context, m_pm->create_unique_literal_node(v_nodes[0], v_nodes[0]->get_var_index(),true)});
                context_norm_cache->insert({context, new_norm});
            }else{
                PsddParameter pos_weight_renorm = pos_weight/new_norm;
                PsddParameter neg_weight_renorm = neg_weight/new_norm;
                context_node_cache->insert({context, m_pm->create_unique_simple_node(v_nodes[0], v_nodes[0]->get_var_index(), pos_weight_renorm, neg_weight_renorm)});
                context_norm_cache->insert({context, new_norm});
            }
        }
    }
    for (auto k = v_nodes.begin()+1; k != v_nodes.end(); k ++){
        std::unordered_map<std::size_t, PsddNode*>* context_node_cache_out = new std::unordered_map<std::size_t, PsddNode*> ();
        std::unordered_map<std::size_t, PsddParameter>* context_norm_cache_out = new std::unordered_map<std::size_t, PsddParameter>();
        size_t cond_var_location = var_location_in_cluster[k-v_nodes.begin()];
        mask = (size_t) ((~0x0) ^(1 << cond_var_location));
        for (auto p = context_node_cache->begin(); p != context_node_cache->end(); p++){
            size_t last_context = p->first;
            size_t next_context = last_context & mask;
            if (context_node_cache_out->find(next_context) != context_node_cache_out->end()){
                continue;
            }
            PsddParameter neg_param = context_norm_cache->at(next_context);
            PsddParameter pos_param = context_norm_cache->at(next_context|(1<<cond_var_location));
            PsddParameter new_norm = pos_param + neg_param;
            if (new_norm == zero_param){
                context_node_cache_out->insert({next_context, nullptr});
                context_norm_cache_out->insert({next_context, zero_param});
            }else if (pos_param == zero_param){
                PsddNode* neg_lit = m_pm->create_unique_literal_node(*k,(*k)->get_var_index(), false);
                std::vector<PsddElement> elements;
                elements.push_back(PsddElement(neg_lit, context_node_cache->at(next_context), PsddParameter::get_from_regular(1)));
                PsddNode* next_node = m_pm->create_unique_decn_node((*k)->get_parent(),elements);
                context_node_cache_out->insert({next_context, next_node});
                context_norm_cache_out->insert({next_context, neg_param});
            }else if (neg_param == zero_param){
                PsddNode* pos_lit = m_pm->create_unique_literal_node(*k,(*k)->get_var_index(), true);
                std::vector<PsddElement> elements;
                elements.push_back(PsddElement(pos_lit, context_node_cache->at(next_context | (1<<cond_var_location)),
                                               PsddParameter::get_from_regular(1)));
                PsddNode* next_node = m_pm->create_unique_decn_node((*k)->get_parent(),elements);
                context_node_cache_out->insert({next_context, next_node});
                context_norm_cache_out->insert({next_context, pos_param});
            }else{
                PsddNode* pos_lit = m_pm -> create_unique_literal_node((*k), (*k)->get_var_index(), true);
                PsddNode* neg_lit = m_pm -> create_unique_literal_node((*k), (*k)->get_var_index(), false);
                std::vector<PsddElement> elements;
                elements.push_back(PsddElement(pos_lit, context_node_cache->at(next_context| (1<<cond_var_location)),
                                               pos_param/new_norm));
                elements.push_back(PsddElement(neg_lit,context_node_cache->at(next_context),neg_param/new_norm));
                PsddNode* next_node = m_pm -> create_unique_decn_node((*k)->get_parent(), elements);
                context_node_cache_out->insert({next_context, next_node});
                context_norm_cache_out->insert({next_context, new_norm});
            }
        }
        delete context_node_cache;
        delete context_norm_cache;
        context_node_cache = context_node_cache_out;
        context_norm_cache = context_norm_cache_out;
    }

    assert(context_node_cache->size() == 1);
    assert(context_node_cache->find(0) != context_node_cache->end());
    assert(context_norm_cache->size() == 1);
    assert(context_norm_cache->find(0) != context_norm_cache->end());
    PsddNode* result = context_node_cache->begin()->second;
    PsddParameter result_norm = context_norm_cache->begin()->second;
    delete context_node_cache;
    delete context_norm_cache;
    return {result, result_norm};
}

void PsddCompiler::read_uai_file(const char *uai_file) {
    assert(m_network == nullptr);
    m_network = new UaiNetwork();
    m_network->read_file(uai_file);
}


void PsddCompiler::init_psdd_manager(char mode) {
    std::string pid_affix = std::to_string(getpid());
    char cnf_name[100];
    char vtree_name[100];
    char cmd[1000];
    cnf_name[0] = '\0';
    vtree_name[0] = '\0';
    cmd[0] = '\0';
    sprintf(cnf_name, "/tmp/uai_%s.cnf", pid_affix.c_str());
    sprintf(vtree_name, "/tmp/uai_%s.vtree", pid_affix.c_str());
    sprintf(cmd, "./miniC2D -c %s -m %d -o %s > /dev/null 2>&1", cnf_name, mode, vtree_name);

    std::string content = "";
    content +=
            "p cnf " + std::to_string(m_network->get_var_size()) + " " + std::to_string(m_network->get_factor_size()) +
            "\n";
    auto ordered_clusters = m_network->get_clusters();
    for (auto k = ordered_clusters.begin(); k != ordered_clusters.end(); k++) {
        for (auto p = k->begin(); p != k->end(); p++) {
            content += std::to_string(*p) + " ";
        }
        content += "0\n";
    }
    std::ofstream cnf_file;
    cnf_file.open(cnf_name);
    cnf_file << content;
    cnf_file.close();
    system(cmd);
    m_pm = new PsddManager();
    m_pm->init_by_vtree_file(vtree_name);
    std::remove(vtree_name);
    std::remove(cnf_name);
}

void PsddCompiler::init_psdd_manager_using_vtree_manager(VtreeManager *v) {
    m_pm = new PsddManager();
    m_pm->init_by_vtree_manager(v);
}

bool compare_psdd_node_by_vtree_inorder(const PsddNode* p1, const PsddNode* p2){
    return (p1->get_vtree()->get_inorder_index() > p2->get_vtree()->get_inorder_index());
}

std::pair<PsddNode *, PsddParameter> PsddCompiler::compile_network(size_t gc_freq) {
    std::vector<PsddNode*> nodes;
    PsddParameter z = PsddParameter::get_from_regular(1);
    auto factor_size = m_network->get_factor_size();
    for (auto i = 0; i < factor_size; i++){
        auto compiled_cluster = compile_cluster(i);
        nodes.push_back(compiled_cluster.first);
        m_pm->inc_ref(compiled_cluster.first);
        z = z * compiled_cluster.second;
    }
    std::vector<std::vector<PsddNode* > > mult_pool (m_pm->get_vtree_manager()->get_vtree_size());
    for (auto i = nodes.begin(); i != nodes.end(); i++){
        mult_pool[(*i)->get_vtree()->get_index()].push_back(*i);
    }
    std::vector<Vtree* > v_nodes_serialized = m_pm->get_vtree_manager()->serialize();
    size_t mult_acc = 1;
    PsddNode* final_result = nullptr;
    for (auto i = v_nodes_serialized.begin(); i != v_nodes_serialized.end(); i++){
        Vtree* cur = *i;
        const std::vector<PsddNode*>& to_mult = mult_pool[cur->get_index()];
        if (to_mult.size() == 0){
            continue;
        }else{
            PsddNode* result = to_mult[0];
            for (auto j = to_mult.begin()+1; j != to_mult.end(); j++){
                auto mult_result = m_pm->multiply(result, (*j));
                mult_acc +=1;
                m_pm->dec_ref(result);
                m_pm->dec_ref(*j);
                m_pm->inc_ref(mult_result.first);
                z = z* mult_result.second;
                result = mult_result.first;
                if ((mult_acc%gc_freq)== 0){
                    m_pm->gc_manual();
                }
            }
            if (cur->get_parent() == nullptr){
                // cur is the root;
                final_result = result;
            }else{
                mult_pool[cur->get_parent()->get_index()].push_back(result);
            }
        }
    }
    return {final_result, z};
}

PsddManager *PsddCompiler::get_psdd_manager() const {
    return m_pm;
}

