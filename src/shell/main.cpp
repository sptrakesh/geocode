//
// Created by Rakesh on 31/08/2024.
//

#include "clara.hpp"
#include "../lib/geocode/geocode.hpp"
#include "../log/NanoLog.hpp"

#include <cstring>
#include <format>
#include <iostream>
#include <tuple>

#include <boost/json/parse.hpp>
#include <readline/readline.h>
#include <readline/history.h>

namespace
{
  namespace shell
  {
    using std::operator ""sv;

    enum Colours {
      NC=-1,
      BLACK,
      RED,
      GREEN,
      YELLOW,
      BLUE,
      MAGENTA,
      CYAN,
      WHITE,
    };

    std::string setColour( int font, int back = -1, int style = -1 )
    {
      if ( font >= 0 ) font += 30;
      else font = 0;
      if ( back >= 0 ) back += 40;
      else back = 0;

      if ( back > 0 && style > 0 )
      {
        return std::format( "\033[{};{};{}m", font, back, style );
      }

      if ( back > 0 )
      {
        return std::format( "\033[{};{}m", font, back );
      }
      return std::format( "\033[{}m", font );
    }

    void help()
    {
      std::cout << "\033[1mAvailable commands\033[0m\n";
      std::cout << "  \033[1maddress\033[0m \033[3m<Geo-coordinates JSON array>\033[0m - Look up the postal address for a geo-coordinate.  Eg. address [41.9215927, -87.695327]\n";
      std::cout << "  \033[1mcentroid\033[0m \033[3m<Geo-coordiantes JSON array>\033[0m - Compute the centroid for geo-coordinates JSON arrays.  Eg. centroid [[41.9461021, -87.6977005], [41.9215927, -87.6953278], [41.9121971, -87.6807251], [41.8827209, -87.6352386], [41.8839951, -87.6347198], [41.8830872, -87.6359787], [41.883255, -87.6354523], [41.8830147, -87.6354752]]\n";
      std::cout << "  \033[1mcoordinates\033[0m \033[3m<Postal address>\033[0m - Look up the geo-coordinates for the postal address.  Eg. coordinates 565 5 Ave, Manhattan, New York, NY, USA\n";
      std::cout << "  \033[1mdistance\033[0m \033[3m<Geo-coordinates JSON array of 2 points>\033[0m - Compute the geodesic distance between two points.  Eg. distance [[51.752021,-1.257726], [51.507351, -0.127758]]\n";
      std::cout << "  \033[1mencode\033[0m \033[3m<Geo-coordinates JSON array>\033[0m - Encode the geo-coordinate as a open location code.  Eg. encode [47.0000625, 8.0000625]\n";
      std::cout << "  \033[1mdecode\033[0m \033[3m<Open location code>\033[0m - Decode the open location code as a geo-coordinate.  Eg. decode 8FVC2222+22\n";
    }

    std::string_view trim( std::string_view in )
    {
      auto left = in.begin();
      for ( ;; ++left )
      {
        if ( left == in.end() ) return in;
        if ( !std::isspace( *left ) ) break;
      }
      auto right = in.end() - 1;
      for ( ; right > left && std::isspace( *right ); --right );
      return { left, static_cast<std::size_t>(std::distance( left, right ) + 1) };
    }

    std::tuple<std::string_view, std::size_t> command( std::string_view line )
    {
      auto idx = line.find( ' ', 0 );
      if ( idx == std::string_view::npos ) return { line, idx };
      return { line.substr( 0, idx ), idx };
    }

    std::tuple<std::string_view, std::size_t> value( std::string_view line, std::size_t begin )
    {
      auto idx = line.find( ' ', begin + 1 );
      while ( idx != std::string::npos && line.substr( begin + 1, idx - begin - 1 ).empty() )
      {
        ++begin;
        idx = line.find( ' ', begin + 1 );
      }

      return { line.substr( begin + 1 ), idx };
    }

    void address( std::string_view line, std::size_t idx )
    {
      const char* ps = std::getenv( "POSITION_STACK_KEY" );
      if ( ps == nullptr )
      {
        std::cout << setColour(RED) << "POSITION_STACK_KEY environment variable not set." << setColour(NC) << "  Please set to the key for accessing positionstack service.\n";
        return;
      }

      auto [v, end] = value( line, idx );
      if ( end <= idx )
      {
        std::cout << setColour(RED) << "Cannot parse value from " << setColour(NC) << line << '\n';
        return;
      }

      auto ec = boost::system::error_code();
      auto res = boost::json::parse( v, ec );
      if ( ec )
      {
        std::cout << setColour(RED) << "JSON parse error: " << setColour(NC) << ec.message() << '\n';
        return;
      }

      if ( !res.is_array() )
      {
        std::cout << setColour(RED) << "Value is not an array" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      auto& arr = res.as_array();
      if ( arr.size() != 2 )
      {
        std::cout << setColour(RED) << "Value is not an geo-coordinate point as array" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      for ( const auto& p : arr )
      {
        if ( !p.is_double() )
        {
          std::cout << setColour(RED) << "Value is not an geo-coordinate point" << setColour(NC) << " (" << p << ").\n";
          return;
        }
      }

      auto enc = spt::geocode::address( spt::geocode::Point{ .latitude = arr[0].as_double(), .longitude = arr[1].as_double() }, ps );
      if ( !enc.has_value() )
      {
        std::cout << setColour(RED) << "Error looking up geo-coordinates for" << setColour(NC) << " (" << v << ").\n" << enc.error() << ".\n";
        return;
      }

      auto obj = boost::json::object();
      if ( !enc->street.empty() )
      {
        auto st = boost::json::array{};
        for ( const auto& street : enc->street ) st.emplace_back( street );
        obj.emplace( "street"sv, st );
      }
      if ( !enc->city.empty() ) obj.emplace( "city"sv, enc->city );
      if ( !enc->county.empty() ) obj.emplace( "county"sv, enc->county );
      if ( !enc->state.empty() ) obj.emplace( "state"sv, enc->state );
      if ( !enc->postalCode.empty() ) obj.emplace( "postalCode"sv, enc->postalCode );
      if ( !enc->country.empty() ) obj.emplace( "country"sv, enc->country );
      if ( enc->location ) obj.emplace( "distance"sv, enc->location->accuracy );

      std::cout << obj << '\n';
    }

    void coordinates( std::string_view line, std::size_t idx )
    {
      const char* ps = std::getenv( "POSITION_STACK_KEY" );
      if ( ps == nullptr )
      {
        std::cout << setColour(RED) << "POSITION_STACK_KEY environment variable not set." << setColour(NC) << "  Please set to the key for accessing positionstack service.\n";
        return;
      }

      auto [v, end] = value( line, idx );
      if ( end <= idx )
      {
        std::cout << setColour(NC) << "Cannot parse value from " << setColour(NC) << line << '\n';
        return;
      }

      const auto res = spt::geocode::fromAddress( std::string{ v }, ps );
      if ( !res.has_value() )
      {
        std::cout << setColour(RED) << "Cannot lookup coordinates for address" << setColour(NC) << " (" << v << ").\n" << res.error() << ".\n";
        return;
      }

      auto obj = boost::json::object();
      obj.emplace( "latitude"sv, res->latitude );
      obj.emplace( "longitude"sv, res->longitude );

      std::cout << obj << '\n';
    }

    void distance( std::string_view line, std::size_t idx )
    {
      auto [v, end] = value( line, idx );
      if ( end <= idx )
      {
        std::cout << setColour(RED) << "Cannot parse value from " << setColour(NC) << line << '\n';
        return;
      }

      auto ec = boost::system::error_code();
      auto res = boost::json::parse( v, ec );
      if ( ec )
      {
        std::cout << setColour(RED) << "JSON parse error: " << setColour(NC) << ec.message() << '\n';
        return;
      }

      if ( !res.is_array() )
      {
        std::cout << setColour(RED) << "Value is not an array" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      const auto& arr = res.as_array();
      if ( arr.size() != 2 )
      {
        std::cout << setColour(RED) << "Value is not an array of two geo-coordinate points" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      auto points = std::vector<spt::geocode::Point>{};
      points.reserve( arr.size() );
      for ( const auto& p : arr )
      {
        if ( !p.is_array() )
        {
          std::cout << setColour(RED) << "Value is not an array of geo-coordinate points" << setColour(NC) << " (" << p << ").\n";
          return;
        }

        auto pa = p.as_array();
        if ( pa.size() != 2 )
        {
          std::cout << setColour(RED) << "Value is not an array of geo-coordinate points" << setColour(NC) << " (" << p << ").\n";
          return;
        }

        for ( const auto& pt : pa )
        {
          if ( !pt.is_double() )
          {
            std::cout << setColour(RED) << "Value is not an array of geo-coordinate points" << setColour(NC) << " (" << p << ").\n";
            return;
          }
        }

        points.emplace_back();
        points.back().latitude = pa[0].as_double();
        points.back().longitude = pa[1].as_double();
      }

      const auto dist = spt::geocode::distance( points.front(), points.back() );
      std::cout << dist.distance << setColour(BLUE) << " metres" << setColour(NC) << '\n';
    }

    void centroid( std::string_view line, std::size_t idx )
    {
      auto [v, end] = value( line, idx );
      if ( end <= idx )
      {
        std::cout << setColour(RED) << "Cannot parse value from " << setColour(NC) << line << '\n';
        return;
      }

      auto ec = boost::system::error_code();
      auto res = boost::json::parse( v, ec );
      if ( ec )
      {
        std::cout << setColour(RED) << "JSON parse error: " << setColour(NC) << ec.message() << '\n';
        return;
      }

      if ( !res.is_array() )
      {
        std::cout << setColour(RED) << "Value is not an array" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      const auto& arr = res.as_array();
      if ( arr.size() < 2 )
      {
        std::cout << setColour(RED) << "Value is not an array of two geo-coordinate points" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      auto points = std::vector<spt::geocode::Point>{};
      points.reserve( arr.size() );
      for ( const auto& p : arr )
      {
        if ( !p.is_array() )
        {
          std::cout << setColour(RED) << "Value is not an array of geo-coordinate points" << setColour(NC) << " (" << p << ").\n";
          return;
        }

        auto pa = p.as_array();
        if ( pa.size() != 2 )
        {
          std::cout << setColour(RED) << "Value is not an array of geo-coordinate points" << setColour(NC) << " (" << p << ").\n";
          return;
        }

        for ( const auto& pt : pa )
        {
          if ( !pt.is_double() )
          {
            std::cout << setColour(RED) << "Value is not an array of geo-coordinate points" << setColour(NC) << " (" << p << ").\n";
            return;
          }
        }

        points.emplace_back();
        points.back().latitude = pa[0].as_double();
        points.back().longitude = pa[1].as_double();
      }

      const auto centre = spt::geocode::centroid( std::span<const spt::geocode::Point>{ points } );
      auto array = boost::json::array{ centre.latitude, centre.longitude };
      std::cout << array << '\n';
    }

    void encode( std::string_view line, std::size_t idx )
    {
      auto [v, end] = value( line, idx );
      if ( end <= idx )
      {
        std::cout << setColour(RED) << "Cannot parse value from " << setColour(NC) << line << '\n';
        return;
      }

      auto ec = boost::system::error_code();
      auto res = boost::json::parse( v, ec );
      if ( ec )
      {
        std::cout << setColour(RED) << "JSON parse error: " << setColour(NC)  << ec.message() << '\n';
        return;
      }

      if ( !res.is_array() )
      {
        std::cout << setColour(RED) << "Value is not an array" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      auto& arr = res.as_array();
      if ( arr.size() != 2 )
      {
        std::cout << setColour(RED) << "Value is not an geo-coordinate point as array" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      for ( const auto& p : arr )
      {
        if ( !p.is_double() )
        {
          std::cout << setColour(RED) << "Value is not an geo-coordinate point" << setColour(NC) << " (" << p << ").\n";
          return;
        }
      }

      auto enc = spt::geocode::toLocationCode( arr[0].as_double(), arr[1].as_double() );
      if ( enc.empty() )
      {
        std::cout << setColour(RED) << "Cannot encode geo-coordinate" << setColour(NC) << " (" << v << ").\n";
        return;
      }
      std::cout << enc << '\n';
    }

    void decode( std::string_view line, std::size_t idx )
    {
      auto [v, end] = value( line, idx );
      if ( end <= idx )
      {
        std::cout << setColour(RED) << "Cannot parse value from " << setColour(NC) << line << '\n';
        return;
      }

      auto enc = spt::geocode::fromLocationCode( v );
      if ( !enc.has_value() )
      {
        std::cout << setColour(RED) << "Cannot decode open location code" << setColour(NC) << " (" << v << ").\n";
        return;
      }

      std::cout << boost::json::array{ enc->latitude, enc->longitude } << '\n';
    }

    void run()
    {
      std::cout << "Enter commands followed by <ENTER>" << '\n';
      std::cout << "Enter \033[1mhelp\033[0m for help about commands" << '\n';
      std::cout << "Enter \033[1mexit\033[0m or \033[1mquit\033[0m to exit shell\n";

      // Disable tab completion
      rl_bind_key( '\t', rl_insert );

      std::string previous;
      previous.reserve( 128 );

      char* buf;
      while ( ( buf = readline( "geocode> " ) ) != nullptr )
      {
#ifdef __STDC_LIB_EXT1__
        auto len = strnlen_s( buf, 32*1024 );
#else
        auto len = strlen( buf ); // flawfinder: ignore
#endif
        if ( len == 0 )
        {
          std::free( buf );
          continue;
        }

        if ( previous != std::string{ buf } ) add_history( buf );

        auto line = std::string_view{ buf, len };
        line = trim( line );
        if ( line == "exit"sv || line == "quit"sv )
        {
          std::cout << "Bye\n";
          break;
        }

        if ( line == "help"sv ) help();
        else if ( line.empty() ) { /* noop */ }
        else
        {
          auto[cmd, idx] = command( line );

          if ( "address"sv == cmd )
          {
            address( line, idx );
          }
          else if ( "coordinates"sv == cmd )
          {
            coordinates( line, idx );
          }
          else if ( "centroid"sv == cmd )
          {
            centroid( line, idx );
          }
          else if ( "distance"sv == cmd )
          {
            distance( line, idx );
          }
          else if ( "encode"sv == cmd )
          {
            encode( line, idx );
          }
          else if ( "decode"sv == cmd )
          {
            decode( line, idx );
          }
          else
          {
            std::cout << "Unknown command " << cmd << '\n';
          }
        }

        previous.clear();
        previous.append( buf, len );
        std::free( buf );
      }
    }
  }
}

int main( int argc, char const * const * argv )
{
  using clara::Opt;
#ifdef __APPLE__
  std::string logLevel{ "debug" };
#else
  std::string logLevel{"info"};
#endif
  std::string dir{"/tmp/"};
  auto help = false;

  auto options = clara::Help(help) |
      Opt(logLevel, "info")["-l"]["--log-level"]("Log level to use [debug|info|warn|critical] (default info).") |
      Opt(dir, "/tmp/")["-o"]["--log-dir"]("Log directory (default /tmp/)");

  auto result = options.parse( clara::Args( argc, argv ) );
  if ( !result )
  {
    std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
    exit( 1 );
  }

  if ( help )
  {
    options.writeToStream( std::cout );
    exit( 0 );
  }

  if ( logLevel == "debug" ) nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  else if ( logLevel == "info" ) nanolog::set_log_level( nanolog::LogLevel::INFO );
  else if ( logLevel == "warn" ) nanolog::set_log_level( nanolog::LogLevel::WARN );
  else if ( logLevel == "critical" ) nanolog::set_log_level( nanolog::LogLevel::CRIT );
  nanolog::initialize( nanolog::GuaranteedLogger(), dir, "geocode-shell", false );

  shell::run();

  return EXIT_SUCCESS;
}
