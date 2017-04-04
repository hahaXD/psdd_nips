//
// Created by Jason Shen on 4/1/17.
//

#ifndef PSDD_PSDD_PARAMETER_HPP
#define PSDD_PSDD_PARAMETER_HPP


class PsddParameter {
public:
    PsddParameter();
    static PsddParameter get_from_regular(double num);
    static PsddParameter get_from_log(double num);
    size_t get_hash_value() const;
    bool operator==(const PsddParameter& other) const;
    bool operator!= (const PsddParameter& other) const;
    PsddParameter operator+(const PsddParameter& other) const;
    PsddParameter operator/(const PsddParameter& other) const;
    PsddParameter operator*(const PsddParameter& other) const;
    double get_parameter_value() const;
private:
    PsddParameter(double num);
    double m_param;
    size_t m_hash_value;
};


#endif //PSDD_PSDD_PARAMETER_HPP
