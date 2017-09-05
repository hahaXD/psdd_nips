//
// Created by Jason Shen on 4/1/17.
//

#include <cmath>
#include <limits>
#include "psdd_parameter.hpp"
#define APPX_LEVEL 12


PsddParameter PsddParameter::get_from_regular(double num) {
    return PsddParameter(std::log(num));
}

PsddParameter PsddParameter::get_from_log(double num) {
    return PsddParameter(num);
}

PsddParameter PsddParameter::operator+(const PsddParameter &other) const {
    if (m_param == -std::numeric_limits<double>::infinity()){
        // if this is zero
        return other;
    }else{
        return PsddParameter(m_param + std::log(1+std::exp(other.m_param-m_param)));
    }
}

PsddParameter PsddParameter::operator/(const PsddParameter &other) const {
    return PsddParameter(m_param-other.m_param);
}

PsddParameter PsddParameter::operator*(const PsddParameter &other) const {
    return PsddParameter(m_param + other.m_param);
}

double PsddParameter::get_parameter_value() const {
    return m_param;
}

PsddParameter::PsddParameter(double num):m_param (num){
    m_hash_value = (size_t) (m_param*std::pow(10,APPX_LEVEL));
}

size_t PsddParameter::get_hash_value() const {
    return m_hash_value;
}

bool PsddParameter::operator==(const PsddParameter &other) const {
    return m_hash_value == other.m_hash_value;
}

bool PsddParameter::operator!=(const PsddParameter &other) const {
    return m_hash_value != other.m_hash_value;
}

double PsddParameter::get_regular_value() const {
    return std::exp(m_param);
}

PsddParameter::PsddParameter():m_param(0){}


