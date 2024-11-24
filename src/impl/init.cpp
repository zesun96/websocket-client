/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "init.hpp"
#include "certificate.hpp"
#include "internals.hpp"
#include "pollservice.hpp"
#include "threadpool.hpp"
#include "tls.hpp"
#include "utils.hpp"

#include "tlstransport.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <thread>

namespace wsc::impl {

struct Init::TokenPayload {
	TokenPayload(std::shared_future<void> *cleanupFuture) {
		Init::Instance().doInit();
		if (cleanupFuture)
			*cleanupFuture = cleanupPromise.get_future().share();
	}

	~TokenPayload() {
		std::thread t(
		    [](std::promise<void> promise) {
			    utils::this_thread::set_name("wsc cleanup");
			    try {
				    Init::Instance().doCleanup();
				    promise.set_value();
			    } catch (const std::exception &e) {
				    // PLOG_WARNING << e.what();
				    promise.set_exception(std::make_exception_ptr(e));
			    }
		    },
		    std::move(cleanupPromise));
		t.detach();
	}

	std::promise<void> cleanupPromise;
};

Init &Init::Instance() {
	static Init *instance = new Init;
	return *instance;
}

Init::Init() {
	std::promise<void> p;
	p.set_value();
	mCleanupFuture = p.get_future(); // make it ready
}

Init::~Init() {}

init_token Init::token() {
	std::lock_guard lock(mMutex);
	if (auto locked = mWeak.lock())
		return locked;

	mGlobal = std::make_shared<TokenPayload>(&mCleanupFuture);
	mWeak = *mGlobal;
	return *mGlobal;
}

void Init::preload() {
	std::lock_guard lock(mMutex);
	if (!mGlobal) {
		mGlobal = std::make_shared<TokenPayload>(&mCleanupFuture);
		mWeak = *mGlobal;
	}
}

std::shared_future<void> Init::cleanup() {
	std::lock_guard lock(mMutex);
	mGlobal.reset();
	return mCleanupFuture;
}

void Init::setSctpSettings(SctpSettings s) {
	std::lock_guard lock(mMutex);
	// if (mGlobal)
	//	SctpTransport::SetSettings(s);

	mCurrentSctpSettings = std::move(s); // store for next init
}

void Init::doInit() {
	// mMutex needs to be locked

	if (std::exchange(mInitialized, true))
		return;

		// PLOG_DEBUG << "Global initialization";

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		throw std::runtime_error("WSAStartup failed, error=" + std::to_string(WSAGetLastError()));
#endif

	int concurrency = std::thread::hardware_concurrency();
	int count = std::max(concurrency, MIN_THREADPOOL_SIZE);
	// PLOG_DEBUG << "Spawning " << count << " threads";
	ThreadPool::Instance().spawn(count);

	PollService::Instance().start();

#if USE_GNUTLS
	// Nothing to do
#elif USE_MBEDTLS
	// Nothing to do
#else
	openssl::init();
#endif

	TlsTransport::Init();
}

void Init::doCleanup() {
	std::lock_guard lock(mMutex);
	if (mGlobal)
		return;

	if (!std::exchange(mInitialized, false))
		return;

	// PLOG_DEBUG << "Global cleanup";

	ThreadPool::Instance().join();
	ThreadPool::Instance().clear();
	PollService::Instance().join();

	TlsTransport::Cleanup();

#ifdef _WIN32
	WSACleanup();
#endif
}

} // namespace wsc::impl
