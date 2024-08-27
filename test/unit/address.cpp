//
// Created by Rakesh on 20/07/2024.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../../src/lib/geocode/geocode.hpp"

SCENARIO( "Position Stack api test", "[positionstack]" )
{
  GIVEN( "POSITION_STACK_KEY environment variable set" )
  {
    const auto key = std::getenv( "POSITION_STACK_KEY" );
    REQUIRE( key != nullptr );

    THEN( "Looking up address by geo-coordinate" )
    {
      const auto address = spt::geocode::address( 41.9215927, -87.6953278, key );
      REQUIRE( address.has_value() );
      CHECK_FALSE( address->city.empty() );
      CHECK_FALSE( address->state.empty() );
      CHECK_FALSE( address->country.empty() );
    }

    AND_THEN( "Looking up geo-coordinate by address" )
    {
      const auto point = spt::geocode::fromAddress( "565 5 Ave, Manhattan, New York, NY, USA", key );
      REQUIRE( point.has_value() );
      CHECK_THAT( point->latitude, Catch::Matchers::WithinAbs( 40.755884, 0.001 ) );
      CHECK_THAT( point->longitude, Catch::Matchers::WithinAbs( -73.978504, 0.001 ) );
    }
  }
}