//
// Created by Rakesh on 20/07/2024.
//

#include "../geocode.h"
#include "geofence.hpp"
#include "openlocationcode.h"

#if defined __has_include
  #if __has_include("../../../log/NanoLog.h")
    #include "../../../log/NanoLog.h"
  #elif __has_include("../../src/log/NanoLog.h")
    #include "../../src/log/NanoLog.h"
  #else
    #include <log/NanoLog.h>
  #endif
#endif

#include <array>
#include <format>
#include <boost/json/parse.hpp>
#include <cpr/cpr.h>

using spt::geocode::Address;
using spt::geocode::Point;

using std::operator ""sv;

std::string spt::geocode::toLocationCode( const double latitude, const double longitude )
{
  return openlocationcode::Encode( { latitude, longitude } );
}

std::expected<Point, std::string> spt::geocode::fromLocationCode( const std::string& code )
{
  using O = std::expected<Point, std::string>;

  if ( !openlocationcode::IsValid( code ) ) return O{ std::unexpect, "Invalid code" };
  const auto area = openlocationcode::Decode( code );
  auto points = std::array{
    Point{ area.GetLatitudeLo(), area.GetLongitudeLo() },
    Point{ area.GetLatitudeHi(), area.GetLongitudeHi() }
  };
  auto cp = centroid( std::span<const Point>( points ) );
  auto out = O{ std::in_place };
  out.value().latitude = cp.latitude;
  out.value().longitude = cp.longitude;
  out.value().accuracy = static_cast<double>( area.GetCodeLength() );
  return out;
}

std::expected<Address, std::string> spt::geocode::address( const double latitude, const double longitude, const std::string& key )
{
  using O = std::expected<Address, std::string>;

  auto resp = cpr::Get( cpr::Url{ "https://api.positionstack.com/v1/reverse" },
      cpr::Parameters{ { "access_key", key },
          { "query", std::format( "{},{}", latitude, longitude ) },
          { "output", "json" }, { "limit", "1" } } );
  if ( resp.status_code != 200 )
  {
    LOG_WARN << "Error retrieving address for latitude: " << latitude <<
      "; longitude: " << longitude <<
      ".  Response status " << resp.status_line <<
      ". " << resp.text;
    return O{ std::unexpect, resp.text };
  }

  boost::system::error_code ec;
  auto json = boost::json::parse( resp.text, ec );
  if ( ec )
  {
    LOG_WARN << "Error parsing response for latitude: " << latitude <<
      "; longitude: " << longitude << ".  Error: " << ec.message();
    return O{ std::unexpect, ec.message() };
  }

  auto& doc = json.get_object();
  if ( !doc.contains( "data"sv ) )
  {
    LOG_WARN << "No data in response for latitude: " << latitude <<
      "; longitude: " << longitude << ". " << resp.text;
    return O{ std::unexpect, "No data in response" };
  }

  auto& data = doc["data"sv];
  if ( !data.is_array() )
  {
    LOG_WARN << "data not array in response for latitude: " << latitude <<
      "; longitude: " << longitude << ". " << resp.text;
    return O{ std::unexpect, "Invalid type for data in response" };
  }

  auto& arr = data.get_array();
  if ( arr.empty() )
  {
    LOG_WARN << "data array empty in response for latitude: " << latitude <<
      "; longitude: " << longitude << ". " << resp.text;
    return O{ std::unexpect, "Empty response data" };
  }

  auto& a = arr.front();
  if ( !a.is_object() )
  {
    LOG_WARN << "data array entry not object in response for latitude: " << latitude <<
      "; longitude: " << longitude << ". " << resp.text;
    return O{ std::unexpect, "Non-object in data array" };
  }
  auto& ad = a.get_object();

  auto address = O{ std::in_place };
  address.value().street.reserve( 1 );
  if ( ad.contains( "name"sv ) && ad["name"sv].is_string() ) address.value().street.emplace_back( ad["name"sv].get_string() );
  if ( ad.contains( "locality"sv ) && ad["locality"sv].is_string() ) address.value().city = ad["locality"sv].get_string();
  if ( ad.contains( "region"sv ) && ad["region"sv].is_string() ) address.value().state = ad["region"sv].get_string();
  else if ( ad.contains( "region_code"sv ) && ad["region_code"sv].is_string() ) address.value().state = ad["region_code"sv].get_string();
  if ( ad.contains( "county"sv ) && ad["county"sv].is_string() ) address.value().county = ad["county"sv].get_string();
  if ( ad.contains( "postal_code"sv ) && ad["postal_code"sv].is_string() ) address.value().postalCode = ad["postal_code"sv].get_string();
  if ( ad.contains( "country"sv ) && ad["country"sv].is_string() ) address.value().country = ad["country"sv].get_string();
  else if ( ad.contains( "country_code"sv ) && ad["country_code"sv].is_string() ) address.value().country = ad["country_code"sv].get_string();
  if ( ad.contains( "label"sv ) && ad["label"sv].is_string() ) address.value().text = ad["label"sv].get_string();
  address.value().location = Point{};
  address.value().location->latitude = latitude;
  address.value().location->longitude = longitude;
  if ( ad.contains( "distance"sv ) && ad["distance"sv].is_double() ) address.value().location->accuracy = ad["distance"sv].get_double();

  return address;
}

std::expected<Point, std::string> spt::geocode::fromAddress( const std::string& address, const std::string& key )
{
  using O = std::expected<Point, std::string>;
  if ( address.empty() ) return O{ std::unexpect, "Empty address" };

  auto resp = cpr::Get( cpr::Url{ "https://api.positionstack.com/v1/forward" },
      cpr::Parameters{ { "access_key", key },
          { "query", address },
          { "output", "json" }, { "limit", "1" } } );

  if ( resp.status_code != 200 )
  {
    LOG_WARN << "Error retrieving address for address: " << address <<
        ".  Response status " << resp.status_line <<
        ". " << resp.text;
    return O{ std::unexpect, resp.text };
  }

  boost::system::error_code ec;
  auto json = boost::json::parse( resp.text, ec );
  if ( ec )
  {
    LOG_WARN << "Error parsing response for address: " << address << ".  Error: " << ec.message();
    return O{ std::unexpect, ec.message() };
  }

  auto& doc = json.get_object();
  if ( !doc.contains( "data"sv ) )
  {
    LOG_WARN << "No data in response for address: " << address << ". " << resp.text;
    return O{ std::unexpect, "No data in response" };
  }

  auto& data = doc["data"sv];
  if ( !data.is_array() )
  {
    LOG_WARN << "data not array in response for address: " << address << ". " << resp.text;
    return O{ std::unexpect, "Invalid type for data in response" };
  }

  auto& arr = data.get_array();
  if ( arr.empty() )
  {
    LOG_WARN << "data array empty in response for address: " << address << ". " << resp.text;
    return O{ std::unexpect, "Empty response data" };
  }

  auto& a = arr.front();
  if ( !a.is_object() )
  {
    LOG_WARN << "data array entry not object in response for address: " << address << ". " << resp.text;
    return O{ std::unexpect, "Non-object in data array" };
  }
  auto& ad = a.get_object();

  if ( !ad.contains( "latitude"sv ) || !ad.contains( "longitude"sv ) )
  {
    LOG_WARN << "data array entry does not contain geocodes in response for address: " << address << ". " << resp.text;
    return O{ std::unexpect, "Data does not contain coordinates" };
  }

  auto p = O{ std::in_place };
  p.value().latitude = ad["latitude"sv].is_double() ? ad["latitude"sv].get_double() : 0.0;
  p.value().longitude = ad["longitude"sv].is_double() ? ad["longitude"sv].get_double() : 0.0;
  if ( ad.contains( "distance"sv ) && ad["distance"sv].is_double() ) p.value().accuracy = ad["distance"sv].get_double();
  return p;
}

bool spt::geocode::within( const Point& point, const Polygon& polygon )
{
  auto p =  std::array{ point.latitude, point.longitude };

  auto poly = std::vector<std::array<double, 2>>{};
  poly.reserve( polygon.size() );
  for ( const auto& gl : polygon ) poly.push_back( { gl.latitude, gl.longitude } );
  return geofence::isIn<double>( poly, p );
}
