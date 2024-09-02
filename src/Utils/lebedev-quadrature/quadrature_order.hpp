#ifndef QUADRATURE_ORDER_HPP
#define QUADRATURE_ORDER_HPP

#include "preprocessor.hpp"

#include <array>

namespace lebedev {

enum class QuadratureOrder : unsigned int;

/** \brief Gets order enum from rule number. See README for rule number to order translation */
QuadratureOrder get_rule_order(unsigned int rule_number);

/** \brief Returns whether a quadrature rule is available. README also contains this info */
bool get_rule_availability(unsigned int rule_number);

/** \brief Returns the highest degree of polynomial that can be integrated exactly */
unsigned int get_rule_precision(unsigned int rule_number);

/** \brief Gets `QuadratureOrder` enum value from a corresponding unsigned int */
QuadratureOrder get_order_enum(unsigned int quadrature_order);

/** \brief Enumerates all possible quadrature orders in this library */
enum class QuadratureOrder : unsigned int
{
    order_6 = 6,
    order_14 = 14,
    order_26 = 26,
    order_38 = 38,
    order_50 = 50,
    order_74 = 74,
    order_86 = 86,
    order_110 = 110,
    order_146 = 146,
    order_170 = 170,
    order_194 = 194,
    order_230 = 230,
    order_266 = 266,
    order_302 = 302,
    order_350 = 350,
    order_386 = 386,
    order_434 = 434,
    order_482 = 482,
    order_530 = 530,
    order_590 = 590,
    order_650 = 650,
    order_698 = 698,
    order_770 = 770,
    order_830 = 830,
    order_890 = 890,
    order_974 = 974,
    order_1046 = 1046,
    order_1118 = 1118,
    order_1202 = 1202,
    order_1274 = 1274,
    order_1358 = 1358,
    order_1454 = 1454,
    order_1538 = 1538,
    order_1622 = 1622,
    order_1730 = 1730,
    order_1814 = 1814,
    order_1910 = 1910,
    order_2030 = 2030,
    order_2126 = 2126,
    order_2222 = 2222,
    order_2354 = 2354,
    order_2450 = 2450,
    order_2558 = 2558,
    order_2702 = 2702,
    order_2810 = 2810,
    order_2930 = 2930,
    order_3074 = 3074,
    order_3182 = 3182,
    order_3314 = 3314,
    order_3470 = 3470,
    order_3590 = 3590,
    order_3722 = 3722,
    order_3890 = 3890,
    order_4010 = 4010,
    order_4154 = 4154,
    order_4334 = 4334,
    order_4466 = 4466,
    order_4610 = 4610,
    order_4802 = 4802,
    order_4934 = 4934,
    order_5090 = 5090,
    order_5294 = 5294,
    order_5438 = 5438,
    order_5606 = 5606,
    order_5810 = 5810 
};

} // namespace lebedev

#endif