#include "preprocessor.hpp"
#include "quadrature_order.hpp"

#include <stdexcept>

namespace lebedev
{

constexpr unsigned int number_of_rules = 65;

constexpr unsigned int n_orders = 65;
constexpr std::array<bool, n_orders> rule_availability_table
    = {true,    true,    true,    true,    true,    true,    true,    true,    true,    true,
       true,    true,    true,    true,    true,    false,   true,    false,   false,   true, 
       false,   false,   true,    false,   false,   true,    false,   false,   true,    false,
       false,   true,    false,   false,   true,    false,   false,   true,    false,   false,
       true,    false,   false,   true,    false,   false,   true,    false,   false,   true,
       false,   false,   true,    false,   false,   true,    false,   false,   true,    false,
       false,   true,    false,   false,   true };



constexpr std::array<unsigned int, n_orders> rule_precision_table
    = {   3,   5,   7,   9,  11,  13,  15,  17,  19,  21,
         23,  25,  27,  29,  31,  33,  35,  37,  39,  41,
         43,  45,  47,  49,  51,  53,  55,  57,  59,  61,
         63,  65,  67,  69,  71,  73,  75,  77,  79,  81,
         83,  85,  87,  89,  91,  93,  95,  97,  99, 101,
        103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 
        123, 125, 127, 129, 131 };



constexpr std::array<QuadratureOrder, n_orders> order_enum_table
    = { QuadratureOrder::order_6,
        QuadratureOrder::order_14,
        QuadratureOrder::order_26,
        QuadratureOrder::order_38,
        QuadratureOrder::order_50,
        QuadratureOrder::order_74,
        QuadratureOrder::order_86,
        QuadratureOrder::order_110,
        QuadratureOrder::order_146,
        QuadratureOrder::order_170,
        QuadratureOrder::order_194,
        QuadratureOrder::order_230,
        QuadratureOrder::order_266,
        QuadratureOrder::order_302,
        QuadratureOrder::order_350,
        QuadratureOrder::order_386,
        QuadratureOrder::order_434,
        QuadratureOrder::order_482,
        QuadratureOrder::order_530,
        QuadratureOrder::order_590,
        QuadratureOrder::order_650,
        QuadratureOrder::order_698,
        QuadratureOrder::order_770,
        QuadratureOrder::order_830,
        QuadratureOrder::order_890,
        QuadratureOrder::order_974,
        QuadratureOrder::order_1046,
        QuadratureOrder::order_1118,
        QuadratureOrder::order_1202,
        QuadratureOrder::order_1274,
        QuadratureOrder::order_1358,
        QuadratureOrder::order_1454,
        QuadratureOrder::order_1538,
        QuadratureOrder::order_1622,
        QuadratureOrder::order_1730,
        QuadratureOrder::order_1814,
        QuadratureOrder::order_1910,
        QuadratureOrder::order_2030,
        QuadratureOrder::order_2126,
        QuadratureOrder::order_2222,
        QuadratureOrder::order_2354,
        QuadratureOrder::order_2450,
        QuadratureOrder::order_2558,
        QuadratureOrder::order_2702,
        QuadratureOrder::order_2810,
        QuadratureOrder::order_2930,
        QuadratureOrder::order_3074,
        QuadratureOrder::order_3182,
        QuadratureOrder::order_3314,
        QuadratureOrder::order_3470,
        QuadratureOrder::order_3590,
        QuadratureOrder::order_3722,
        QuadratureOrder::order_3890,
        QuadratureOrder::order_4010,
        QuadratureOrder::order_4154,
        QuadratureOrder::order_4334,
        QuadratureOrder::order_4466,
        QuadratureOrder::order_4610,
        QuadratureOrder::order_4802,
        QuadratureOrder::order_4934,
        QuadratureOrder::order_5090,
        QuadratureOrder::order_5294,
        QuadratureOrder::order_5438,
        QuadratureOrder::order_5606,
        QuadratureOrder::order_5810 
};



LEBEDEV_EXTERNAL_LINKAGE
QuadratureOrder get_rule_order(unsigned int rule_number)
{
    if (rule_number >= number_of_rules)
        throw std::invalid_argument("Requested order of a rule which exceeds number of rules");

    return order_enum_table[rule_number];
}



LEBEDEV_EXTERNAL_LINKAGE
bool get_rule_availability(unsigned int rule_number)
{
    if (rule_number >= number_of_rules)
        throw std::invalid_argument("Requested availability of a rule which exceeds number of rules");

    return rule_availability_table[rule_number];
}



LEBEDEV_EXTERNAL_LINKAGE
unsigned int get_rule_precision(unsigned int rule_number)
{
    if (rule_number >= number_of_rules)
        throw std::invalid_argument("Requested precision of a rule which exceeds number of rules");
    
    return rule_precision_table[rule_number];
}



LEBEDEV_EXTERNAL_LINKAGE
QuadratureOrder get_order_enum(unsigned int quadrature_order)
{
    if (quadrature_order == 6)
        return QuadratureOrder::order_6; 
    else if (quadrature_order == 14)
        return QuadratureOrder::order_14; 
    else if (quadrature_order == 26)
        return QuadratureOrder::order_26; 
    else if (quadrature_order == 38)
        return QuadratureOrder::order_38; 
    else if (quadrature_order == 50)
        return QuadratureOrder::order_50; 
    else if (quadrature_order == 74)
        return QuadratureOrder::order_74; 
    else if (quadrature_order == 86)
        return QuadratureOrder::order_86; 
    else if (quadrature_order == 110)
        return QuadratureOrder::order_110; 
    else if (quadrature_order == 146)
        return QuadratureOrder::order_146; 
    else if (quadrature_order == 170)
        return QuadratureOrder::order_170; 
    else if (quadrature_order == 194)
        return QuadratureOrder::order_194; 
    else if (quadrature_order == 230)
        return QuadratureOrder::order_230; 
    else if (quadrature_order == 266)
        return QuadratureOrder::order_266; 
    else if (quadrature_order == 302)
        return QuadratureOrder::order_302; 
    else if (quadrature_order == 350)
        return QuadratureOrder::order_350; 
    else if (quadrature_order == 386)
        return QuadratureOrder::order_386; 
    else if (quadrature_order == 434)
        return QuadratureOrder::order_434; 
    else if (quadrature_order == 482)
        return QuadratureOrder::order_482; 
    else if (quadrature_order == 530)
        return QuadratureOrder::order_530; 
    else if (quadrature_order == 590)
        return QuadratureOrder::order_590; 
    else if (quadrature_order == 650)
        return QuadratureOrder::order_650; 
    else if (quadrature_order == 698)
        return QuadratureOrder::order_698; 
    else if (quadrature_order == 770)
        return QuadratureOrder::order_770; 
    else if (quadrature_order == 830)
        return QuadratureOrder::order_830; 
    else if (quadrature_order == 890)
        return QuadratureOrder::order_890; 
    else if (quadrature_order == 974)
        return QuadratureOrder::order_974; 
    else if (quadrature_order == 1046)
        return QuadratureOrder::order_1046; 
    else if (quadrature_order == 1118)
        return QuadratureOrder::order_1118; 
    else if (quadrature_order == 1202)
        return QuadratureOrder::order_1202; 
    else if (quadrature_order == 1274)
        return QuadratureOrder::order_1274; 
    else if (quadrature_order == 1358)
        return QuadratureOrder::order_1358; 
    else if (quadrature_order == 1454)
        return QuadratureOrder::order_1454; 
    else if (quadrature_order == 1538)
        return QuadratureOrder::order_1538; 
    else if (quadrature_order == 1622)
        return QuadratureOrder::order_1622; 
    else if (quadrature_order == 1730)
        return QuadratureOrder::order_1730; 
    else if (quadrature_order == 1814)
        return QuadratureOrder::order_1814; 
    else if (quadrature_order == 1910)
        return QuadratureOrder::order_1910; 
    else if (quadrature_order == 2030)
        return QuadratureOrder::order_2030; 
    else if (quadrature_order == 2126)
        return QuadratureOrder::order_2126; 
    else if (quadrature_order == 2222)
        return QuadratureOrder::order_2222; 
    else if (quadrature_order == 2354)
        return QuadratureOrder::order_2354; 
    else if (quadrature_order == 2450)
        return QuadratureOrder::order_2450; 
    else if (quadrature_order == 2558)
        return QuadratureOrder::order_2558; 
    else if (quadrature_order == 2702)
        return QuadratureOrder::order_2702; 
    else if (quadrature_order == 2810)
        return QuadratureOrder::order_2810; 
    else if (quadrature_order == 2930)
        return QuadratureOrder::order_2930; 
    else if (quadrature_order == 3074)
        return QuadratureOrder::order_3074; 
    else if (quadrature_order == 3182)
        return QuadratureOrder::order_3182; 
    else if (quadrature_order == 3314)
        return QuadratureOrder::order_3314; 
    else if (quadrature_order == 3470)
        return QuadratureOrder::order_3470; 
    else if (quadrature_order == 3590)
        return QuadratureOrder::order_3590; 
    else if (quadrature_order == 3722)
        return QuadratureOrder::order_3722; 
    else if (quadrature_order == 3890)
        return QuadratureOrder::order_3890; 
    else if (quadrature_order == 4010)
        return QuadratureOrder::order_4010; 
    else if (quadrature_order == 4154)
        return QuadratureOrder::order_4154; 
    else if (quadrature_order == 4334)
        return QuadratureOrder::order_4334; 
    else if (quadrature_order == 4466)
        return QuadratureOrder::order_4466; 
    else if (quadrature_order == 4610)
        return QuadratureOrder::order_4610; 
    else if (quadrature_order == 4802)
        return QuadratureOrder::order_4802; 
    else if (quadrature_order == 4934)
        return QuadratureOrder::order_4934; 
    else if (quadrature_order == 5090)
        return QuadratureOrder::order_5090; 
    else if (quadrature_order == 5294)
        return QuadratureOrder::order_5294; 
    else if (quadrature_order == 5438)
        return QuadratureOrder::order_5438; 
    else if (quadrature_order == 5606)
        return QuadratureOrder::order_5606; 
    else if (quadrature_order == 5810)
        return QuadratureOrder::order_5810;
    else
        throw std::invalid_argument("Requested enum for lebedev order which does not exist");
}

} // namespace lebedev