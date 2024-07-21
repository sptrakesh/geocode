//
// Created by Rakesh on 21/07/2024.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../../src/lib/geocode/geocode.h"

namespace
{
  namespace itest
  {
    struct Point
    {
      double latitude{ 0.0 };
      double longitude{ 0.0 };
      std::string text;
    };
  }
}

SCENARIO( "Clustering test suite", "[cluster]" )
{
  GIVEN( "A set of geo coordinates" )
  {
    const auto points = std::vector<spt::geocode::Point>{
      {63.8066559, -83.6791916},
      {41.9461021, -87.6977005},
      {41.9215927, -87.6953278},
      {41.9121971, -87.6807251},
      {60.244442, -149.6915436},
      {41.8827209, -87.6352386},
      {41.8839951, -87.6347198},
      {41.8830872, -87.6359787},
      {41.883255, -87.6354523},
      {41.8830147, -87.6354752},
      {41.881218, -87.6351395},
      {41.8841934, -87.6364594},
      {41.8837547, -87.6352844},
      {41.8826141, -87.6353912},
      {41.8827934, -87.6357727},
      {41.8830872, -87.6352005},
      {41.8839989, -87.632843},
      {41.8855286, -87.6347198},
      {41.8848267, -87.6368179},
      {41.943203, -87.7009201}
    };

    const auto clustered = spt::geocode::cluster( points, 32, 3 );
    REQUIRE( clustered.size() > 1 );
    CHECK( clustered[0].points.size() > 2 );
  }

  GIVEN( "A list of coordinates with custom struct" )
  {
    const auto points = std::vector<itest::Point>{
      {63.8066559, -83.6791916, "Far"},
      {41.9461021, -87.6977005, "Dense"},
      {41.9215927, -87.6953278, "Dense"},
      {41.9121971, -87.6807251, "Dense"},
      {60.244442, -149.6915436, "Far"},
      {41.8827209, -87.6352386, "Dense"},
      {41.8839951, -87.6347198, "Dense"},
      {41.8830872, -87.6359787, "Dense"},
      {41.883255, -87.6354523, "Dense"},
      {41.8830147, -87.6354752, "Dense"},
      {41.881218, -87.6351395, "Dense"},
      {41.8841934, -87.6364594, "Dense"},
      {41.8837547, -87.6352844, "Dense"},
      {41.8826141, -87.6353912, "Dense"},
      {41.8827934, -87.6357727, "Dense"},
      {41.8830872, -87.6352005, "Dense"},
      {41.8839989, -87.632843, "Dense"},
      {41.8855286, -87.6347198, "Dense"},
      {41.8848267, -87.6368179, "Dense"},
      {41.943203, -87.7009201, "Dense"}
    };

    const auto clustered = spt::geocode::cluster( points, 32, 3 );
    REQUIRE( clustered.size() > 1 );
    CHECK( clustered[0].points.size() > 2 );
    for ( const auto& p : clustered[0].points ) CHECK( p->text == "Dense" );
    for ( const auto& p : clustered.back().points ) CHECK( p->text == "Far" );
  }
}
