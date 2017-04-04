//
// Created by Jason Shen on 4/2/17.
//

#ifndef PSDD_UAI_NETWORK_HPP
#define PSDD_UAI_NETWORK_HPP

#include <vector>
#include <unordered_set>
#include "psdd_parameter.hpp"

#define MARKOV_NETWORK_TYPE 1
#define BAYESIAN_NETWORK_TYPE 2

class UaiNetwork {
public:
    UaiNetwork();
    void read_file (const char* uai_file); // uai file, variables appearing in the same factor, the last varaible will be the LSB, where mod/2 corresponding to its value.
    size_t get_var_size();
    size_t get_factor_size();
    int get_network_type() const;
    const std::vector<size_t>& get_ordered_cluster(size_t cluster_index) const;
    const std::vector<PsddParameter>& get_parameter(size_t cluster_index) const;
    const std::vector<std::vector<size_t> >& get_ordered_clusters() const;
    const std::vector<std::unordered_set<size_t> >& get_clusters() const;
    const std::vector<std::vector<PsddParameter> >& get_params() const;
private:
    size_t m_var_size;
    size_t m_factor_size;
    int m_network_type; // 1 is Mark, 2 is BN
    std::vector<std::vector<size_t> > m_ordered_clusters;
    std::vector<std::unordered_set<size_t> > m_clusters;
    std::vector<std::vector<PsddParameter> > m_params;
};


#endif //PSDD_UAI_NETWORK_HPP
