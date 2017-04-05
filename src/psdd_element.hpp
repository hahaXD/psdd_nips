//
// Created by Jason Shen on 4/4/17.
//

#ifndef PSDD_PSDD_ELEMENT_HPP
#define PSDD_PSDD_ELEMENT_HPP


#include "psdd_parameter.hpp"
class PsddNode;
class PsddElement {
public:
    PsddElement(PsddNode* prime, PsddNode* sub, PsddParameter param);
    PsddNode* get_prime() const;
    PsddNode* get_sub() const;
    PsddParameter get_parameter() const;
    size_t get_hash_value() const;
    bool operator==(const PsddElement& other) const;
private:
    PsddNode* m_prime;
    PsddNode* m_sub;
    PsddParameter m_param;
};


#endif //PSDD_PSDD_ELEMENT_HPP
