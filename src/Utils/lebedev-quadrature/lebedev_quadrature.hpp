#ifndef LEBEDEV_QUADRATURE_HPP
#define LEBEDEV_QUADRATURE_HPP

#ifndef LEBEDEV_HEADER_ONLY
#define LEBEDEV_HEADER_ONLY 1
#endif

#include "quadrature_points.hpp"
#include "quadrature_order.hpp"
#include "generator_point.hpp"

#if LEBEDEV_HEADER_ONLY || LEBEDEV_IMPLEMENTATION

#include "quadrature_points.inl"
#include "quadrature_order.inl"
#include "generator_point.inl"

#endif

#endif