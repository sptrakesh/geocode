//
// Created by Rakesh on 22/07/2024.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../../src/lib/geocode/geocode.hpp"

SCENARIO( "Geo-coordinate centroid tests", "[centroid]" )
{
  GIVEN( "A variety of points" )
  {
    WHEN( "Computing centroid of empty span" )
    {
      const auto points = std::array<spt::geocode::Point, 0>{};
      const auto centroid = spt::geocode::centroid( std::span<const spt::geocode::Point>{ points } );
      CHECK( centroid.latitude == 0.0 );
      CHECK( centroid.longitude == 0.0 );
    }

    AND_WHEN( "Computing centroid of single element span" )
    {
      const auto points = std::array{ spt::geocode::Point{ .latitude = 43.1234, .longitude = -87.876 } };
      const auto centroid = spt::geocode::centroid( std::span<const spt::geocode::Point>{ points } );
      CHECK( centroid.latitude == points[0].latitude );
      CHECK( centroid.longitude == points[0].longitude );
    }

    AND_WHEN( "Computing centroid of 2 points" )
    {
      const auto points = std::array{
        spt::geocode::Point{ .latitude = 63.8066559, .longitude = -83.6791916 },
        spt::geocode::Point{ .latitude = 60.244442, .longitude = -149.6915436 },
      };
      const auto centroid = spt::geocode::centroid( std::span<const spt::geocode::Point>{ points } );
      for ( const auto& p : points )
      {
        CHECK_FALSE( centroid.latitude == p.latitude );
        CHECK_FALSE( centroid.longitude == p.longitude );
      }
    }

    AND_WHEN( "Computing centroid of multiple points" )
    {
      const auto points = std::vector<spt::geocode::Point>{
        {41.9461021, -87.6977005},
        {41.9215927, -87.6953278},
        {41.9121971, -87.6807251},
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
      const auto centroid = spt::geocode::centroid( std::span<const spt::geocode::Point>{ points } );
      for ( const auto& p : points )
      {
        CHECK_FALSE( centroid.latitude == p.latitude );
        CHECK_FALSE( centroid.longitude == p.longitude );
      }
    }
  }
}