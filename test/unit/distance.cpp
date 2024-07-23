//
// Created by Rakesh on 20/07/2024.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <iostream>
#include "../../src/lib/geocode/geocode.h"

SCENARIO( "Distance computation test suite", "[distance]" )
{
  GIVEN( "A set of points with known distance" )
  {
    const auto p1 = spt::geocode::Point{ .latitude = 51.752021, .longitude = -1.257726 };
    const auto p2 = spt::geocode::Point{ .latitude = 51.507351, .longitude = -0.127758 };
    const auto [d, a] = spt::geocode::distance( p1, p2 );
    CHECK_THAT( d, Catch::Matchers::WithinAbs(  82841.8915, 0.001 ) );
    std::cout << "Azimuth distance " << a << '\n';
  }

  GIVEN( "A set of points with a large distance" )
  {
    const auto p1 = spt::geocode::Point{ .latitude = 63.8066559, .longitude = -83.6791916 };
    const auto p2 = spt::geocode::Point{ .latitude = 41.943203, .longitude = -87.7009201 };
    const auto [d, a] = spt::geocode::distance( p1, p2 );
    CHECK_THAT( d, Catch::Matchers::WithinAbs( 2446844.3375, 0.001 ) );
    std::cout << "Azimuth distance " << a << '\n';
  }

  GIVEN( "A set of points with a centre location" )
  {
    const auto p1 = spt::geocode::Point{ .latitude = 35.9127, .longitude = -90.0995 };
    const auto p2 = spt::geocode::Point{ .latitude = 0.0, .longitude = 0.0 };
    const auto [d, a] = spt::geocode::distance( p1, p2 );
    CHECK_THAT( d, Catch::Matchers::WithinAbs( 10019330.3042, 0.0001 ) );
    std::cout << "Azimuth distance " << a << '\n';
  }
}