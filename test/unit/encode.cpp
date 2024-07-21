//
// Created by Rakesh on 20/07/2024.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../../src/lib/geocode/geocode.h"

SCENARIO( "Open Location Code suite", "[olc]" )
{
  GIVEN( "The google example geo-coordinate" )
  {
    const auto point = spt::geocode::Point{ .latitude = 47.0000625, .longitude = 8.0000625 };

    WHEN( "Encoding as open location code" )
    {
      const auto str = spt::geocode::toLocationCode( point );
      CHECK( str == "8FVC2222+22" );
    }

    AND_WHEN( "Decoding an open location code" )
    {
      const auto p = spt::geocode::fromLocationCode( "8FVC2222+22" );
      REQUIRE( p.has_value() );
      CHECK_THAT( p.value().latitude, Catch::Matchers::WithinAbs( point.latitude, 0.0001 ) );
      CHECK_THAT( p.value().longitude, Catch::Matchers::WithinAbs( point.longitude, 0.0001 ) );
    }
  }

  GIVEN( "Another geo-coordinate" )
  {
    const auto point = spt::geocode::Point{ .latitude = 63.8066559, .longitude = -83.6791916 };
    const auto str = spt::geocode::toLocationCode( point );
    const auto p = spt::geocode::fromLocationCode( str );
    REQUIRE( p.has_value() );
    CHECK_THAT( p.value().latitude, Catch::Matchers::WithinAbs( point.latitude, 0.0001 ) );
    CHECK_THAT( p.value().longitude, Catch::Matchers::WithinAbs( point.longitude, 0.0001 ) );
  }
}