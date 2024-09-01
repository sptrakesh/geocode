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
#include <span>
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

  inline double degreesToRadians( const double degree ) { return degree/180.0 * boost::math::constants::pi<double>(); };
  inline double radiansToDegrees( const double radians ) { return ( radians * 180.0 ) / boost::math::constants::pi<double>(); };

  /**
   * Convert the specified geo-location identified by the latitude and longitude to the Open Location Code representation.
   * @param latitude The latitude in degrees for the geo-location.
   * @param longitude The longitude in degrees for the geo-location.
   * @return The Open Location Code representation of the geo-location.
   */
  std::string toLocationCode( double latitude, double longitude );

  /**
   * Convert the specified geo-coordinate point to the Open Location Code representation.
   * @param point The *Point* representing the geo-coordinates to be encoded.
   * @return The Open Location Code representation of the point.
   */
  inline std::string toLocationCode( const Point& point ) { return toLocationCode( point.latitude, point.longitude ); }

  /**
   * Decode the Open Location Code value to a representative geo-coordinate point.
   *
   * **Note:** The Google API returns a pair of coordinates that represent the code.  Our implementation returns the
   * average values for the latitude and longitude.
   * @param code Convert the specified Open Location Code to a representative geo-coordinate point.
   * @return The decoded geo-coordinate, or an error string if the specified code is invalid.
   */
  std::expected<Point, std::string> fromLocationCode( const std::string& code );

  inline std::expected<Point, std::string> fromLocationCode( std::string_view code ) { return fromLocationCode( std::string{ code } ); };

  /**
   * Look up the closest approximate address for the specified geo-location from the *positionstack* API.
   * @param latitude The latitude for the geo-location to look up closest address for.
   * @param longitude The longitude for the geo-location to look up closest address for.
   * @param key The *positionstack* API key to use to make the request to their web service.
   * @return The returned address or an error string if the lookup fails.
   */
  std::expected<Address, std::string> address( double latitude, double longitude, const std::string& key );

  /**
   * Look up the closest approximaet address for the specified geo-coordinates from the *positionstack* API.
   * @param point The geo-coordinates for which the closest address is to be looked up.
   * @param key The *positionstack* API key to use to make the request to their web service.
   * @return The returned address or an error string if the lookup fails.
   */
  inline std::expected<Address, std::string> address( const Point& point, const std::string& key ) { return address( point.latitude, point.longitude, key ); }

  /**
   * Look up the geo-coordinates for the specified address usnig the *positionstack* API.
   * @param address The text address to look up geo-coordinates for.
   * @param key The *positionstack* API key to use to make the request to their web service.
   * @return The geo-coordinates for the address or an error string if the lookup fails.
   */
  std::expected<Point, std::string> fromAddress( const std::string& address, const std::string& key );

  /**
   * Look up the geo-coordinates for the specified address usnig the *positionstack* API.
   * @param address The address to look up geo-coordinates for.
   * @param key The *positionstack* API key to use to make the request to their web service.
   * @return The geo-coordinates for the address or an error string if the lookup fails.
   */
  inline std::expected<Point, std::string> fromAddress( const Address& address, const std::string& key ) { return fromAddress( address.text, key ); }

  using Polygon = std::vector<Point>;

  /**
   * Check whether the geo-coordinate falls within the specified geo-fence.
   * @param point The point to check within bounds.
   * @param polygon The polygon that represents the bounding area to check.
   * @return Returns `true` if the point falls within the bounding area.
   */
  bool within( const Point& point, const Polygon& polygon );

  template <typename T>
  concept LatLng = requires( T t )
  {
    std::is_default_constructible<T>{};
    T::latitude;
    T::longitude;
  };

  /**
   * Compute the centroid for the specified geo-coordinates.
   * @tparam P A geo-coordinate structure that conforms to the *LatLng* concept
   * @param points Pointers to the geo-coordinates for which the centroid is to be computed.
   * @return The computed centroid for the points.
   */
  template <LatLng P>
  P centroid( std::span<const P*> points )
  {
    P point;
    if ( points.empty() ) return point;
    if ( points.size() == 1 )
    {
      point.latitude = points.front()->latitude;
      point.longitude = points.front()->longitude;
      return point;
    }

    double x{ 0.0 };
    double y{ 0.0 };
    double z{ 0.0 };

    for ( const auto& p : points )
    {
      const auto lat = degreesToRadians( p->latitude );
      const auto lon = degreesToRadians( p->longitude );
      x += std::cos( lat ) * std::cos( lon );
      y += std::cos( lat ) * std::sin( lon );
      z += std::sin( lat );
    }

    x = x / static_cast<double>( points.size() );
    y = y / static_cast<double>( points.size() );
    z = z / static_cast<double>( points.size() );

    const auto lon = std::atan2( y, x );
    const auto hyp = std::sqrt( ( x * x ) + ( y * y ) );
    const auto lat = std::atan2( z, hyp );

    point.latitude = radiansToDegrees( lat );
    point.longitude = radiansToDegrees( lon );
    return point;
  }

  /**
   * Compute the centroid for the specified collection of geo-coordinates.
   * @tparam P The geo-coordinate structure conforming to the *LatLng* concept.
   * @param points The collection of points for which centroid is to be computed.
   * @return The computed centroid of the geo-coordinates.
   */
  template <LatLng P>
  P centroid( std::span<const P> points )
  {
    auto vec = std::vector<const P*>{};
    vec.reserve( points.size() );
    for ( const auto& p : points ) vec.push_back( &p );
    return centroid( std::span<const P*>( vec ) );
  }

  struct Distance
  {
    double distance;
    double azimuth;
  };

  /**
   * Compute the geodesic distance between two geo-coordinates using Vincenty's formula.
   * @tparam P The geo-coordinate type that conforms to the *LatLng* concept.
   * @param lhs The first point from which distance is to be computed.
   * @param rhs The second point to which distance is to be computed.
   * @return A structure representing the computed distance and azimuth.
   */
  template <LatLng P>
  Distance distance( const P& lhs, const P& rhs )
  {
    const auto haversine = [&lhs, &rhs]()
    {
      const auto lat1 = degreesToRadians( lhs.latitude );
      const auto lat2 = degreesToRadians( rhs.latitude );
      const auto long1 = degreesToRadians( lhs.longitude );
      const auto long2 = degreesToRadians( rhs.longitude );
      const auto dist = ( std::pow( std::sin( (1.0 / 2) * ( lat2 - lat1 ) ), 2 ) ) +
          ( ( std::cos( lat1 ) ) * ( std::cos( lat2 ) ) * ( std::pow( std::sin( (1.0 / 2) * ( long2 - long1 ) ), 2 ) ) );
      return 2 * std::asin( std::min( 1.0, std::sqrt( dist ) ) ) * 6372797.56085; // distance in metres
    };

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
    constexpr int maxIterations = 1000;

    int iteration = 0;
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
      if ( ++iteration > maxIterations ) return Distance{ .distance = haversine(), .azimuth = 0.0 };
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

  /**
   * Apply k-means clustering algorithm to cluster the set of coordinates around the specified number of centroids.
   * @tparam P A type that represents a geo-coordinate that conforms to the *LatLng* concept.
   * @param points The geo-coordinates that are to be clustered.
   * @param rounds  The number of rounds the algorithm is to be applied to perform the clustering.
   * @param numClusters The number of clusters to aggregate the coordinates into.
   * @return A vector of the clustered coordinates.  The vector is sorted in descending order of density around the centroids.
   */
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
    if ( points.size() == 1 )
    {
      auto vec = std::vector<Cluster<P>>{ 1, Cluster<P>{} };
      vec.front().centroid.latitude = points.front().latitude;
      vec.front().centroid.longitude = points.front().longitude;
      vec.front().points.push_back( &points.front() );
      return vec;
    }

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

      // Create vectors to keep track of data needed to compute new centroids
      auto aggregates = std::vector<std::vector<const P*>>( numClusters, std::vector<const P*>{} );
      for ( auto& agg : aggregates ) agg.reserve( static_cast<int>( points.size() ) / numClusters );

      // Iterate over points to append data to centroids
      for ( auto& p : vec )
      {
        auto idx = p.cluster;
        if ( idx < 0 ) idx = 0; // Some edge case
        aggregates[idx].push_back( p.point );
        p.minDist = std::numeric_limits<double>::max();  // reset distance
      }

      // Compute the new centroids
      for ( auto c = std::begin( centroids ); c != std::end( centroids ); ++c )
      {
        auto clusterId = c - std::begin( centroids );
        const auto cp = centroid( std::span<const P*>( aggregates[clusterId] ) );
        c->latitude = cp.latitude;
        c->longitude = cp.longitude;
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