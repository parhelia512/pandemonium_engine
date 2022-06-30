#ifndef HTTP_SERVER_SIMPLE_H
#define HTTP_SERVER_SIMPLE_H

/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "core/io/image_loader.h"
#include "core/io/json.h"
#include "core/io/stream_peer_ssl.h"
#include "core/io/tcp_server.h"
#include "core/io/zip_io.h"

#include "core/project_settings.h"

class HTTPServerSimple : public Reference {
public:
	void stop();

	Error listen(int p_port, IP_Address p_address, bool p_use_ssl, String p_ssl_key, String p_ssl_cert);
	bool is_listening() const;
	void _send_response();
	void poll();

	HTTPServerSimple();

private:
	Ref<TCP_Server> server;
	Map<String, String> mimes;
	Ref<StreamPeerTCP> tcp;
	Ref<StreamPeerSSL> ssl;
	Ref<StreamPeer> peer;
	Ref<CryptoKey> key;
	Ref<X509Certificate> cert;
	bool use_ssl = false;
	uint64_t time = 0;
	uint8_t req_buf[4096];
	int req_pos = 0;

	void _clear_client();
	void _set_internal_certs(Ref<Crypto> p_crypto);
};

#endif
