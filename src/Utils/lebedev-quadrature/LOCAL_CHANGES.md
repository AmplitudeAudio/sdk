# Local changes to the vendored lebedev-quadrature

- The explicit specialisations in `generator_point.inl` and `quadrature_points.inl` carry `LEBEDEV_EXTERNAL_LINKAGE`
  (`inline` in the header-only mode that `lebedev_quadrature.hpp` forces), so the header can be included from several
  translation units without duplicate symbols.
- Keep this when refreshing from upstream.
