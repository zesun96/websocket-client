/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_IMPL_VERIFIED_TLS_TRANSPORT_H
#define WEBSOCKET_IMPL_VERIFIED_TLS_TRANSPORT_H

#include "tlstransport.hpp"

namespace wsc::impl {

class VerifiedTlsTransport final : public TlsTransport {
public:
	VerifiedTlsTransport(variant<shared_ptr<TcpTransport>, shared_ptr<HttpProxyTransport>> lower,
	                     string host, certificate_ptr certificate, state_callback callback,
	                     optional<string> cacert);
	~VerifiedTlsTransport();

private:
#if USE_MBEDTLS
	mbedtls_x509_crt mCaCert;
#endif
};

} // namespace wsc::impl

#endif
