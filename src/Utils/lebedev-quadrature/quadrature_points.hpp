#ifndef QUADRATURE_POINTS_HPP
#define QUADRATURE_POINTS_HPP

#include "preprocessor.hpp"
#include "quadrature_order.hpp"

#include <vector>
#include <functional>

/**
 * \namespace lebedev
 * \brief Global namespace for the lebedev-quadrature library
 */
namespace lebedev {

/**
 * \brief Contains Lebedev quadrature points and has functions to evaluate
 * numerical integrals on the unit sphere.
 */
class QuadraturePoints 
{
public:
    /** \brief vector of doubles */
    using vec = std::vector<double>;
    /** \brief scalar_function */
    using scalar_function = std::function<double(double, double, double)>;
    /** \brief vector of doubles */
    using vector_function = std::function<vec(const vec&, const vec&, const vec&)>;

    /** \brief Calculates set of quadrature points based on integration order */
    QuadraturePoints(QuadratureOrder quad_order);

    /** \brief Calculates spherical integral given a function object.
     *
     * The function object `integrand_at_point` takes three doubles `x`, `y`, `z`
     * corresponding to the coordinates of the evaluation point.
     * It should return a double corresponding to the integrand evaluated at that point.
     */
    double 
    evaluate_spherical_integral(const scalar_function& integrand_at_point) const;

    /** \brief Calculates spherical integral given a function object.
     *
     * The function object `integrand_at_points` takes three references to
     * vectors `x`, `y`, `z` which contain the coordinates of all of the points
     * at which the integrand will be evaluated.
     * It should return a vector of doubles corresponding to the integrand 
     * evaluated at all of the quadrature points.
     */
    double 
    evaluate_spherical_integral(const vector_function& integrand_at_points) const;

    /** \brief Returns const reference to vector of x-coordinates of quadrature points */
    const vec& get_x() const;
    /** \brief Returns const reference to vector of y-coordinates of quadrature points */
    const vec& get_y() const;
    /** \brief Returns const reference to vector of z-coordinates of quadrature points */
    const vec& get_z() const;
    /** \brief Returns const reference to vector of weights of quadrature points */
    const vec& get_weights() const;

private:

    /** \brief x-coordinates of quadrature points */
    vec x;
    /** \brief y-coordinates of quadrature points */
    vec y;
    /** \brief z-coordinates of quadrature points */
    vec z;
    /** \brief weights of quadrature points */
    vec weights;
};

} // namespace lebedev

#endif