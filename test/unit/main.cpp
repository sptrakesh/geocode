//
// Created by Rakesh on 2019-05-16.
//

#include <catch2/catch_session.hpp>

#include "../../src/log/NanoLog.hpp"

int main( int argc, char* argv[] )
{
  nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "geocode-test", false );
  return Catch::Session().run( argc, argv );
}
