//
// Created by Rakesh on 20/07/2024.
//

#pragma once

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <expected>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include <boost/math/constants/constants.hpp>

namespace spt::geocode
{
  struct Point
  {
    double latitude{ 0.0 };
    double longitude{ 0.0 };
    double accuracy{ 0.0 };
  };

  struct Address
  {
    std::vector<std::string> street;
    std::string city;
    std::string state;
    std::string county;
    std::string postalCode;
    std::string country;
    std::string text;
    std::optional<Point> location{ std::nullopt };
  };

  std::string toLocationCode( double latitude, double longitude );
  inline std::string toLocationCode( const Point& point ) { return toLocationCode( point.latitude, point.longitude ); }

  std::expected<Point, std::string> fromLocationCode( const std::string& code );

  std::expected<Address, std::string> address( double latitude, double longitude, const std::string& key );
  inline std::expected<Address, std::string> address( const Point& point, const std::string& key ) { return address( point.latitude, point.longitude, key ); }

  std::expected<Point, std::string> fromAddress( const std::string& address, const std::string& key );
  inline std::expected<Point, std::string> fromAddress( const Address& address, const std::string& key ) { return fromAddress( address.text, key ); }

  using Polygon = std::vector<Point>;
  bool within( const Point& point, const Polygon& polygon );

  template <typename T>
  concept LatLng = requires( T t )
  {
    std::is_default_constructible<T>{};
    T::latitude;
    T::longitude;
  };

  struct Distance
  {
    double distance;
    double azimuth;
  };

  template <LatLng P>
  Distance distance( const P& lhs, const P& rhs )
  {
    const auto degreesToRadians = []( const double degree ) { return degree/180.0 * boost::math::constants::pi<double>(); };

    constexpr double req = 6378137.0;             //Radius at equator
    constexpr double flat = 1 / 298.257223563;    //flattening of earth
    constexpr double rpol = (1 - flat) * req;

    double sin_sigma{ 0.0 };
    double cos_sigma{ 0.0 };
    double sigma{ 0.0 };
    double sin_alpha{ 0.0 };
    double cos_sq_alpha{ 0.0 };
    double cos2sigma{ 0.0 };
    double C{ 0.0 };
    double lam_pre{ 0.0 };

    // convert to radians
    const auto latp = degreesToRadians( lhs.latitude );
    const auto latc = degreesToRadians( rhs.latitude );
    const auto longp = degreesToRadians( lhs.longitude );
    const auto longc = degreesToRadians( rhs.longitude );

    const double u1 = std::atan( ( 1 - flat ) * std::tan( latc ) );
    const double u2 = std::atan( ( 1 - flat ) * std::tan( latp ) );

    double lon = longp - longc;
    double lam = lon;
    double tol = std::pow( 10., -12. ); // iteration tolerance
    double diff = 1.;

    while ( std::abs( diff ) > tol )
    {
      sin_sigma = std::sqrt( std::pow( ( std::cos( u2 ) * std::sin( lam ) ), 2. ) +
        std::pow( std::cos( u1 ) * std::sin( u2 ) - std::sin( u1 ) * std::cos( u2 ) * std::cos( lam ), 2. ) );
      cos_sigma = std::sin( u1 ) * std::sin( u2 ) + std::cos( u1 ) * std::cos( u2 ) * std::cos( lam );
      sigma = std::atan( sin_sigma / cos_sigma );
      sin_alpha = ( std::cos( u1 ) * std::cos( u2 ) * std::sin( lam ) ) / sin_sigma;
      cos_sq_alpha = 1 - std::pow( sin_alpha, 2. );
      cos2sigma = cos_sigma - ( ( 2 * std::sin( u1 ) * std::sin( u2 ) ) / cos_sq_alpha );
      C = ( flat / 16. ) * cos_sq_alpha * ( 4 + flat * ( 4 - 3 * cos_sq_alpha ) );
      lam_pre = lam;
      lam = lon + ( 1 - C ) * flat * sin_alpha * ( sigma + C * sin_sigma * ( cos2sigma + C * cos_sigma * ( 2 * std::pow( cos2sigma, 2. ) - 1 ) ) );
      diff = std::abs( lam_pre - lam );
    }

    const double usq = cos_sq_alpha * ( ( std::pow( req, 2. ) - std::pow( rpol, 2. ) ) / std::pow( rpol ,2. ) );
    const double A = 1 + ( usq / 16384. ) * ( 4096 + usq * ( -768 + usq * ( 320 - 175 * usq ) ) );
    const double B = ( usq / 1024. ) * ( 256 + usq * ( -128 + usq * ( 74 - 47 * usq ) ) );
    const double delta_sig = B * sin_sigma * ( cos2sigma + 0.25 * B * ( cos_sigma * ( -1 + 2 * std::pow( cos2sigma, 2. ) ) -
      ( 1 / 6. ) * B * cos2sigma * ( -3 + 4 * std::pow( sin_sigma, 2.) ) * ( -3 + 4 * std::pow( cos2sigma, 2. ) ) ) );
    const double dis = rpol * A * ( sigma - delta_sig );
    const double azi1 = std::atan2( ( std::cos( u2 ) * std::sin( lam ) ),
      ( std::cos( u1 ) * std::sin( u2 ) - std::sin( u1 ) * std::cos( u2 ) * std::cos( lam ) ) );

    return Distance{ .distance = dis, .azimuth = azi1 };
  }

  template <LatLng P>
  struct Cluster
  {
    P centroid;
    std::vector<const P*> points;
  };

  template <LatLng P>
  std::vector<Cluster<P>> cluster( const std::vector<P>& points, const int rounds, int numClusters )
  {
    struct PointDecorator
    {
      const P* point;
      int cluster{ -1 }; // no default cluster
      double minDist{ std::numeric_limits<double>::max() }; // default infinite distance to nearest cluster
    };

    if ( points.empty() ) return {};

    auto vec = std::vector<PointDecorator>{};
    vec.reserve( points.size() );
    for ( const auto& p : points )
    {
      vec.emplace_back();
      vec.back().point = &p;
    }

    const auto n = points.size();
    numClusters = std::min( static_cast<int>( n ), numClusters );

    auto centroids = std::vector<P>{};
    centroids.reserve( numClusters );
    std::srand( std::time( 0 ) );
    for ( int i = 0; i < numClusters; ++i )
    {
      centroids.emplace_back();
      const auto& p = points.at( std::rand() % n );
      centroids.back().latitude = p.latitude;
      centroids.back().longitude = p.longitude;
    }

    for ( int i = 0; i < rounds; ++i )
    {
      // For each centroid, compute distance from centroid to each point
      // and update point's cluster if necessary
      for ( auto c = std::cbegin( centroids ); c != std::cend( centroids ); ++c )
      {
        int clusterId = c - std::begin( centroids );

        for ( auto& p : vec )
        {
          if ( const auto [dist, azimuth] = distance( *c, *p.point ); dist < p.minDist )
          {
            p.minDist = dist;
            p.cluster = clusterId;
          }
        }
      }

      // Create vectors to keep track of data needed to compute means
      auto nPoints = std::vector( numClusters, 0 );
      auto sumX = std::vector( numClusters, 0.0 );
      auto sumY = std::vector( numClusters, 0.0 );

      // Iterate over points to append data to centroids
      for ( auto& p : vec )
      {
        nPoints[p.cluster] += 1;
        sumX[p.cluster] += p.point->latitude;
        sumY[p.cluster] += p.point->longitude;

        p.minDist = std::numeric_limits<double>::max();  // reset distance
      }

      // Compute the new centroids
      for ( auto c = std::begin( centroids ); c != std::end( centroids ); ++c )
      {
        auto clusterId = c - std::begin( centroids );
        c->latitude = sumX[clusterId] / nPoints[clusterId];
        c->longitude = sumY[clusterId] / nPoints[clusterId];
      }
    }

    auto out = std::vector<Cluster<P>>{};
    out.reserve( numClusters );
    for ( const auto& p : centroids )
    {
      out.emplace_back();
      out.back().centroid.latitude = p.latitude;
      out.back().centroid.longitude = p.longitude;
      out.back().points.reserve( points.size() / numClusters );
    }

    for ( const auto p : vec ) out[p.cluster].points.push_back( p.point );
    std::ranges::sort( out, []( const Cluster<P>& lhs, const Cluster<P>& rhs ) { return lhs.points.size() > rhs.points.size(); } );

    return out;
  }
}