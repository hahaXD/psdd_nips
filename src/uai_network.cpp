//
// Created by Jason Shen on 4/2/17.
//

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include "uai_network.hpp"

UaiNetwork::UaiNetwork(){}

void UaiNetwork::read_file(const char *uai_file){
    std::ifstream uaifs (uai_file,std::ifstream::in);
    if (!uaifs){
        std::cerr << "factor file " << uai_file << " cannot be open."<< std::endl;
    }
    std::string string_buf = "";
    getline(uaifs,string_buf);
    std::string network_type = "";
    if (string_buf.find("BAYES") != std::string::npos){
        m_network_type = BAYESIAN_NETWORK_TYPE;
    }else{
        m_network_type = MARKOV_NETWORK_TYPE;
    }
    getline(uaifs, string_buf);
    size_t var_num = (size_t)stoi(string_buf);
    m_var_size = var_num;
    getline(uaifs, string_buf);
    getline(uaifs, string_buf);
    int factor_num = stoi(string_buf);
    m_factor_size = (size_t) factor_num;
    for (int i = 0; i < m_factor_size; i++){
        std::vector<size_t> order_list;
        getline(uaifs, string_buf);
        std::istringstream iss(string_buf);
        int var_index=0;
        int size;
        iss >> size;
        while (iss >> var_index){
            order_list.push_back((size_t)(var_index+1));
        }
        m_ordered_clusters.push_back(order_list);
        m_clusters.push_back(std::unordered_set<size_t>(order_list.begin(), order_list.end()));
    }
    for (int i = 0; i < m_factor_size; i++){
        getline(uaifs, string_buf);
        while (string_buf == ""){
            getline(uaifs, string_buf);
        }
        size_t entry_cnt;
        std::istringstream iss (string_buf);
        iss >> entry_cnt;
        double cur_entry;
        std::vector<PsddParameter> param_list;
        param_list.reserve(entry_cnt);
        while (iss >> cur_entry){
            param_list.push_back(PsddParameter::get_from_regular(cur_entry));
        }
        while (param_list.size() < entry_cnt){
            getline(uaifs, string_buf);
            iss.str(string_buf);
            iss.clear();
            while (iss >> cur_entry){
                param_list.push_back(PsddParameter::get_from_regular(cur_entry));
            }
        }
        assert(param_list.size() == entry_cnt);
        assert(entry_cnt != 0);
        m_params.push_back(param_list);
    }
}


size_t UaiNetwork::get_var_size(){
    return m_var_size;
}

size_t UaiNetwork::get_factor_size(){
    return m_factor_size;
}

int UaiNetwork::get_network_type() const {
    return m_network_type;
}

const std::vector<std::vector<size_t> >& UaiNetwork::get_ordered_clusters() const {
    return m_ordered_clusters;
}

const std::vector<std::unordered_set<size_t> >& UaiNetwork::get_clusters() const {
    return m_clusters;
}

const std::vector<std::vector<PsddParameter> >& UaiNetwork::get_params() const{
    return m_params;
}

const std::vector<size_t> &UaiNetwork::get_ordered_cluster(size_t cluster_index) const {
    return m_ordered_clusters[cluster_index];
}

const std::vector<PsddParameter> &UaiNetwork::get_parameter(size_t cluster_index) const {
    return m_params[cluster_index];
}

