/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "configuration.hpp"

#include "impl/utils.hpp"

#include <cassert>
#include <regex>

namespace {

bool parse_url(const std::string &url, std::vector<std::optional<std::string>> &result) {
	// Modified regex from RFC 3986, see https://www.rfc-editor.org/rfc/rfc3986.html#appendix-B
	static const char *rs =
	    R"(^(([^:.@/?#]+):)?(/{0,2}((([^:@]*)(:([^@]*))?)@)?(([^:/?#]*)(:([^/?#]*))?))?([^?#]*)(\?([^#]*))?(#(.*))?)";
	static const std::regex r(rs, std::regex::extended);

	std::smatch m;
	if (!std::regex_match(url, m, r) || m[10].length() == 0)
		return false;

	result.resize(m.size());
	std::transform(m.begin(), m.end(), result.begin(), [](const auto &sm) {
		return sm.length() > 0 ? std::make_optional(std::string(sm)) : std::nullopt;
	});

	assert(result.size() == 18);
	return true;
}

} // namespace

namespace wsc {

namespace utils = impl::utils;

ProxyServer::ProxyServer(const string &url) {
	std::vector<optional<string>> opt;
	if (!parse_url(url, opt))
		throw std::invalid_argument("Invalid proxy server URL: " + url);

	string scheme = opt[2].value_or("http");
	if (scheme == "http" || scheme == "HTTP")
		type = Type::Http;
	else if (scheme == "socks5" || scheme == "SOCKS5")
		type = Type::Socks5;
	else
		throw std::invalid_argument("Unknown proxy server protocol: " + scheme);

	username = opt[6];
	password = opt[8];

	hostname = opt[10].value();
	while (!hostname.empty() && hostname.front() == '[')
		hostname.erase(hostname.begin());
	while (!hostname.empty() && hostname.back() == ']')
		hostname.pop_back();

	string service = opt[12].value_or(type == Type::Socks5 ? "1080" : "3128");
	try {
		port = uint16_t(std::stoul(service));
	} catch (...) {
		throw std::invalid_argument("Invalid proxy server port in URL: " + service);
	}
}

ProxyServer::ProxyServer(Type type_, string hostname_, uint16_t port_)
    : type(type_), hostname(std::move(hostname_)), port(port_) {}

ProxyServer::ProxyServer(Type type_, string hostname_, uint16_t port_, string username_,
                         string password_)
    : type(type_), hostname(std::move(hostname_)), port(port_), username(std::move(username_)),
      password(std::move(password_)) {}

} // namespace wsc
