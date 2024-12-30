#pragma once
#include "Windows/PlatformInclude.hpp"

#include <boost/asio/ssl/context.hpp>

namespace TwitchBot {

auto AddRootCerts(boost::asio::ssl::context &ctx) -> bool;

}