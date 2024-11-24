/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "websocket.hpp"
#include <iostream>

int main(int argc, char **argv) {
	wsc::WebSocket ws;

	ws.onOpen([&ws]() {
		std::cout << "WebSocket open" << std::endl;
		ws.send("{\"type\": \"hello websocket\"}");
	});

	ws.onMessage([](std::variant<wsc::binary, wsc::string> message) {
		if (std::holds_alternative<wsc::string>(message)) {
			std::cout << "WebSocket received: " << std::get<wsc::string>(message) << std::endl;
		}
	});

	ws.open("ws://localhost:8080/ws");
	while (1) {
	}
	return 0;
}
