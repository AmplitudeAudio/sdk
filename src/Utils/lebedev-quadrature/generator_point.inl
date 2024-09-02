#include "preprocessor.hpp"
#include "generator_point.hpp"

namespace lebedev {

using OhPointGen = OctahedralPointGeneration;

template <OhPointGen>
LEBEDEV_INTERNAL_LINKAGE
void generate_oh_symmetric_points
(double a, double b, double c,vec &x, vec &y, vec &z);



template <>
void generate_oh_symmetric_points<OhPointGen::points_6>
(double a, double b, double c, vec &x, vec &y, vec &z) 
{

    assert((a == 1) && (b == 0) && (c == 0) 
            && "6 point symmetry must have one nonzero component");

    x.insert(x.end(), { a, -a,  0,  0,  0,  0});
    y.insert(y.end(), { 0,  0,  a, -a,  0,  0});
    z.insert(z.end(), { 0,  0,  0,  0,  a, -a});
}



template <>
void generate_oh_symmetric_points<OhPointGen::points_12>
(double a, double b, double c, vec &x, vec &y, vec &z)
{

    assert((b == 0) && (c == 0)
           && "12 point symmetry has two components which are the same");

    x.insert(x.end(), { 0,  0,  0,  0,  a, -a,  a, -a,  a, -a,  a, -a});
    y.insert(y.end(), { a, -a,  a, -a,  0,  0,  0,  0,  a,  a, -a, -a});
    z.insert(z.end(), { a,  a, -a, -a,  a,  a, -a, -a,  0,  0,  0,  0});
}



template <>
void generate_oh_symmetric_points<OhPointGen::points_8>
(double a, double b, double c, vec &x, vec &y, vec &z)
{
    assert((b == 0) && (c == 0)
            && "8 point symmetry has all three components which are the same");

    x.insert(x.end(), { a, -a,  a, -a,  a, -a,  a, -a});
    y.insert(y.end(), { a,  a, -a, -a,  a,  a, -a, -a});
    z.insert(z.end(), { a,  a,  a,  a, -a, -a, -a, -a});
}



template <>
void generate_oh_symmetric_points<OhPointGen::points_24>
(double a, double b, double c, vec &x, vec &y, vec &z)
{
    assert((c == 0)
            && "24 points symmetry has form (a, a, b), so c = 0");
    
    x.insert(x.end(), { a, -a,  a, -a,  a, -a,  a, -a,  a, -a,  a, -a});
    y.insert(y.end(), { a,  a, -a, -a,  a,  a, -a, -a,  b,  b, -b, -b});
    z.insert(z.end(), { b,  b,  b,  b, -b, -b, -b, -b,  a,  a,  a,  a});

    x.insert(x.end(), { a, -a,  a, -a,  b, -b,  b, -b,  b, -b,  b, -b});
    y.insert(y.end(), { b,  b, -b, -b,  a,  a, -a, -a,  a,  a, -a, -a});
    z.insert(z.end(), {-a, -a, -a, -a,  a,  a,  a,  a, -a, -a, -a, -a});
}



template <>
void generate_oh_symmetric_points<OhPointGen::points_24_axis>
(double a, double b, double c, vec &x, vec &y, vec &z)
{
    assert((c == 0)
            && "24 points symmetry (axis) has form (a, b, 0), so c = 0");

    x.insert(x.end(), { a, -a,  a, -a,  b, -b,  b, -b,  a, -a,  a, -a});
    y.insert(y.end(), { b,  b, -b, -b,  a,  a, -a, -a,  0,  0,  0,  0});
    z.insert(z.end(), { 0,  0,  0,  0,  0,  0,  0,  0,  b,  b, -b, -b});

    x.insert(x.end(), { b, -b,  b, -b,  0,  0,  0,  0,  0,  0,  0,  0});
    y.insert(y.end(), { 0,  0,  0,  0,  a, -a,  a, -a,  b, -b,  b, -b});
    z.insert(z.end(), { a,  a, -a, -a,  b,  b, -b, -b,  a,  a, -a, -a});
}



template <>
void generate_oh_symmetric_points<OhPointGen::points_48>
(double a, double b, double c, vec &x, vec &y, vec &z)
{
    x.insert(x.end(), { a, -a,  a, -a,  a, -a,  a, -a,  a, -a,  a, -a});
    y.insert(y.end(), { b,  b, -b, -b,  b,  b, -b, -b,  c,  c, -c, -c});
    z.insert(z.end(), { c,  c,  c,  c, -c, -c, -c, -c,  b,  b,  b,  b});

    x.insert(x.end(), { a, -a,  a, -a,  b, -b,  b, -b,  b, -b,  b, -b});
    y.insert(y.end(), { c,  c, -c, -c,  a,  a, -a, -a,  a,  a, -a, -a});
    z.insert(z.end(), {-b, -b, -b, -b,  c,  c,  c,  c, -c, -c, -c, -c});

    x.insert(x.end(), { b, -b,  b, -b,  b, -b,  b, -b,  c, -c,  c, -c});
    y.insert(y.end(), { c,  c, -c, -c,  c,  c, -c, -c,  a,  a, -a, -a});
    z.insert(z.end(), { a,  a,  a,  a, -a, -a, -a, -a,  b,  b,  b,  b});

    x.insert(x.end(), { c, -c,  c, -c,  c, -c,  c, -c,  c, -c,  c, -c});
    y.insert(y.end(), { a,  a, -a, -a,  b,  b, -b, -b,  b,  b, -b, -b});
    z.insert(z.end(), {-b, -b, -b, -b,  a,  a,  a,  a, -a, -a, -a, -a});
}



LEBEDEV_EXTERNAL_LINKAGE
void GeneratorPoint::generate_quadrature_points(vec &x, vec &y, vec &z, vec &w) const
{
    unsigned int n_points = 0;
    if (generating_rule == OhPointGen::points_6)
    {
        generate_oh_symmetric_points<OhPointGen::points_6> (a, b, c, x, y, z);
        n_points = 6;
    } else if (generating_rule == OhPointGen::points_12)
    {
        generate_oh_symmetric_points<OhPointGen::points_12> (a, b, c, x, y, z);
        n_points = 12;
    } else if (generating_rule == OhPointGen::points_8)
    {
        generate_oh_symmetric_points<OhPointGen::points_8> (a, b, c, x, y, z);
        n_points = 8;
    } else if (generating_rule == OhPointGen::points_24)
    {
        generate_oh_symmetric_points<OhPointGen::points_24> (a, b, c, x, y, z);
        n_points = 24;
    } else if (generating_rule == OhPointGen::points_24_axis)
    {
        generate_oh_symmetric_points<OhPointGen::points_24_axis> (a, b, c, x, y, z);
        n_points = 24;
    } else if (generating_rule == OhPointGen::points_48)
    {
        generate_oh_symmetric_points<OhPointGen::points_48> (a, b, c, x, y, z);
        n_points = 48;
    } else 
    {
        throw std::invalid_argument("Not a valid octahedral generating rule");
    }
    w.insert(w.end(), n_points, weight);
}

} // namespace lebedev