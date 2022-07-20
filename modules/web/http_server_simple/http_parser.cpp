#include "http_parser.h"

#include "../http/web_server_request.h"
#include "./http_parser/http_parser.h"

#include "simple_web_server_request.h"

Ref<SimpleWebServerRequest> HTTPParser::get_next_request() {
	ERR_FAIL_COND_V(_requests.size() == 0, Ref<SimpleWebServerRequest>());

	Ref<SimpleWebServerRequest> rn = _requests[0];

	_requests.remove(0);

	return rn;
}

int HTTPParser::get_request_count() const {
	return _requests.size();
}

bool HTTPParser::is_ready() const {
	return _is_ready;
}

bool HTTPParser::is_finished() const {
	return !_request.is_valid();
}

void HTTPParser::reset() {
	_partial_data = "";
	_is_ready = false;
	_content_type = REQUEST_CONTENT_URLENCODED;
}

//returns the index where processing was ended -> start of the next query if != data_length
int HTTPParser::read_from_buffer(const char *p_buffer, const int p_data_length) {
	int parsed_bytes = 0;

	parsed_bytes = static_cast<int>(http_parser_execute(parser, settings, p_buffer, p_data_length));

	return parsed_bytes;
}

HTTPParser::HTTPParser() {
	_is_ready = false;
	_content_type = REQUEST_CONTENT_URLENCODED;

	settings = memnew(http_parser_settings);

	settings->on_message_begin = _on_message_begin_cb;
	settings->on_url = _on_url_cb;
	settings->on_status = _on_status_cb;
	settings->on_header_field = _on_header_field_cb;
	settings->on_header_value = _on_header_value_cb;
	settings->on_headers_complete = _on_headers_complete_cb;
	settings->on_body = _on_body_cb;
	settings->on_message_complete = _on_message_complete_cb;
	settings->on_chunk_header = _on_chunk_header_cb;
	settings->on_chunk_complete = _on_chunk_complete_cb;

	//parser = malloc(sizeof(http_parser));
	parser = memnew(http_parser);
	http_parser_init(parser, HTTP_REQUEST);

	parser->data = this;
}

HTTPParser::~HTTPParser() {
	memdelete(parser);
	memdelete(settings);
	parser = nullptr;
}

void HTTPParser::_bind_methods() {
}

String HTTPParser::chr_len_to_str(const char *at, size_t length) {
	String ret;
	ret.resize(length + 1);

	CharType *p = ret.ptrw();

	for (size_t i = 0; i <= length; ++i) {
		p[i] = at[i];
	}

	return ret;
}

#define MESSAGE_DEBUG 0

int HTTPParser::on_message_begin() {
	if (_request.is_valid()) {
		ERR_PRINT("Request was valid!");
	}

	_in_header = true;
	_content_type = REQUEST_CONTENT_URLENCODED;
	_multipart_boundary = "";

	_request.instance();

#if MESSAGE_DEBUG
	ERR_PRINT("begin");
#endif

	return 0;
}
int HTTPParser::on_url(const char *at, size_t length) {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

	String s = chr_len_to_str(at, length);

#if MESSAGE_DEBUG
	ERR_PRINT("url " + s);
#endif

	_request->set_parser_path(s);

	return 0;
}
int HTTPParser::on_status(const char *at, size_t length) {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

	String s = chr_len_to_str(at, length);

#if MESSAGE_DEBUG
	ERR_PRINT("status " + s);
#endif

	return 0;
}
int HTTPParser::on_header_field(const char *at, size_t length) {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

	String s = chr_len_to_str(at, length);

#if MESSAGE_DEBUG
	ERR_PRINT("header_field " + s);
#endif

	_queued_header_field = s;

	return 0;
}
int HTTPParser::on_header_value(const char *at, size_t length) {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

	String s = chr_len_to_str(at, length);

#if MESSAGE_DEBUG
	ERR_PRINT("header_val " + s);
#endif

	_request->add_parameter(_queued_header_field, s);

	if (_queued_header_field == "Host") {
		_request->set_host(s);
	} else if (_queued_header_field == "Content-Type") {
		// It can be:
		// application/x-www-form-urlencoded (default) -> ignore, as its the default
		// text/plain -> useful only for debugging "They are not reliably interpretable by computer"
		// multipart/form-data

		if (s.begins_with("multipart/form-data")) {
			_content_type = REQUEST_CONTENT_MULTIPART_FORM_DATA;

			int bs = s.find("boundary=");

			if (bs == -1) {
				//Error! boundary must exist TODO set an error variable and close the connection
				return 0;
			}

			bs += 9; //skip ahead to the end of "boundary="

			_multipart_boundary = s.substr(bs);
			_multipart_boundary = _multipart_boundary.strip_edges();

			if (_multipart_boundary == "") {
				//Error!  TODO set an error variable and close the connection
			}

		} else if (s.begins_with("text/plain")) {
			_content_type = REQUEST_CONTENT_TEXT_PLAIN;
			//maybe just close the connection?
		}
	}

	return 0;
}
int HTTPParser::on_headers_complete() {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

#if MESSAGE_DEBUG
	ERR_PRINT("headers_complete");
#endif

	//Check content length, and send error if bigger than server limit (add)

	_in_header = false;
	_partial_data = "";

	return 0;
}
int HTTPParser::on_body(const char *at, size_t length) {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

	String s = chr_len_to_str(at, length);

#if MESSAGE_DEBUG
	ERR_PRINT("on_body " + s);
#endif

	if (_content_type == REQUEST_CONTENT_MULTIPART_FORM_DATA) {
		//first boundary -> ignore, with everythong before it
		//find the first \n\n -> process boundary_header
		//cut it out from the string.
		//if file-> create HTTPTempFile class -> try to find boundary, if cant be found append everything to the HTTPTempFile, except the last boundary.size() - 1 characters from the string.
		//else try to find boundary, if cant be found just append everythong to _partial data and return -> if can be found handle it as normal form param

		//try parse
	} else if (_content_type == REQUEST_CONTENT_URLENCODED) {
		_partial_data += s;
	} else {
		//ignore. Maybe close connection?
	}

	return 0;
}
int HTTPParser::on_message_complete() {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

#if MESSAGE_DEBUG
	ERR_PRINT("msg_copmlete");
#endif

	if (_content_type == REQUEST_CONTENT_MULTIPART_FORM_DATA) {
		//the parser seems to cut out the last boundary, so finish parsing the last element, and send _partial_data to a file if a file is being uploaded
	} else if (_content_type == REQUEST_CONTENT_URLENCODED) {
		//Parse the content into the request
		//Also add content body
	} else {
		//Add content body to the request?
	}

	_requests.push_back(_request);
	_request.unref();

	return 0;
}
int HTTPParser::on_chunk_header() {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

#if MESSAGE_DEBUG
	ERR_PRINT("chunk_header");
#endif

	return 0;
}
int HTTPParser::on_chunk_complete() {
	ERR_FAIL_COND_V(!_request.is_valid(), 0);

#if MESSAGE_DEBUG
	ERR_PRINT("chunk_complete");
#endif

	return 0;
}

int HTTPParser::_on_message_begin_cb(http_parser *parser) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_message_begin();
}
int HTTPParser::_on_url_cb(http_parser *parser, const char *at, size_t length) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_url(at, length);
}
int HTTPParser::_on_status_cb(http_parser *parser, const char *at, size_t length) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_status(at, length);
}
int HTTPParser::_on_header_field_cb(http_parser *parser, const char *at, size_t length) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_header_field(at, length);
}
int HTTPParser::_on_header_value_cb(http_parser *parser, const char *at, size_t length) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_header_value(at, length);
}
int HTTPParser::_on_headers_complete_cb(http_parser *parser) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_headers_complete();
}
int HTTPParser::_on_body_cb(http_parser *parser, const char *at, size_t length) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_body(at, length);
}
int HTTPParser::_on_message_complete_cb(http_parser *parser) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_message_complete();
}
int HTTPParser::_on_chunk_header_cb(http_parser *parser) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_chunk_header();
}
int HTTPParser::_on_chunk_complete_cb(http_parser *parser) {
	HTTPParser *p = reinterpret_cast<HTTPParser *>(parser->data);
	return p->on_chunk_complete();
}
