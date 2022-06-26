/*
Copyright (c) 2022 Péter Magyar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "http_server_enums.h"

void HTTPServerEnums::_bind_methods() {
	BIND_ENUM_CONSTANT(HTTP_METHOD_GET);
	BIND_ENUM_CONSTANT(HTTP_METHOD_POST);
	BIND_ENUM_CONSTANT(HTTP_METHOD_HEAD);
	BIND_ENUM_CONSTANT(HTTP_METHOD_PUT);
	BIND_ENUM_CONSTANT(HTTP_METHOD_DELETE);
	BIND_ENUM_CONSTANT(HTTP_METHOD_OPTIONS);
	BIND_ENUM_CONSTANT(HTTP_METHOD_PATCH);
	BIND_ENUM_CONSTANT(HTTP_METHOD_INVALID);

	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_UNKNOWN);

	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_100_CONTINUE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_101_SWITCHING_PROTOCOLS);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_102_PROCESSING);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_103_EARLY_HINTS);

	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_200_OK);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_201_CREATED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_202_ACCEPTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_203_NON_AUTHORITATIVE_INFORMATION);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_204_NO_CONTENT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_205_RESET_CONTENT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_206_PARTIAL_CONTENT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_207_MULTI_STATUS);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_208_ALREADY_REPORTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_226_IM_USED);

	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_300_MULTIPLE_CHOICES);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_301_MOVED_PERMANENTLY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_302_FOUND);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_303_SEE_OTHER);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_304_NOT_MODIFIED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_305_USE_PROXY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_306_UNUSED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_307_TEMPORARY_REDIRECT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_308_PERMANENT_REDIRECT);

	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_400_BAD_REQUEST);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_401_UNAUTHORIZED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_402_PAYMENT_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_403_FORBIDDEN);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_404_NOT_FOUND);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_405_METHOD_NOT_ALLOWED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_406_NOT_ACCEPTABLE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_407_PROXY_AUTHENTICATION_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_408_REQUEST_TIMEOUT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_409_CONFLICT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_410_GONE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_411_LENGTH_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_412_PRECONDITION_FAILED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_413_REQUEST_ENTITY_TOO_LARGE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_414_REQUEST_URI_TOO_LARGE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_415_UNSUPPORTED_MEDIA_TYPE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_416_REQUESTED_RANGE_NOT_SATISFIABLE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_417_EXPECTATION_FAILED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_418_IM_A_TEAPOT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_421_MISDIRECTED_REQUEST);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_422_UNPROCESSABLE_ENTITY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_423_LOCKED);

	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_424_FAILED_DEPENDENCY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_425_TOO_EARLY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_426_UPGRADE_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_428_PRECONDITION_REQUIRED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_429_TOO_MANY_REQUESTS);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_431_REQUEST_HEADER_FIELDS_TOO_LARGE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_451_UNAVAILABLE_FOR_LEGAL_REASONS);

	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_501_NOT_IMPLEMENTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_502_BAD_GATEWAY);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_503_SERVICE_UNAVAILABLE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_504_GATEWAY_TIMEOUT);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_505_HTTP_VERSION_NOT_SUPPORTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_506_VARIANT_ALSO_NEGOTIATES);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_507_INSUFFICIENT_STORAGE);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_508_LOOP_DETECTED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_510_NOT_EXTENDED);
	BIND_ENUM_CONSTANT(HTTP_STATUS_CODE_511_NETWORK_AUTHENTICATION_REQUIRED);
}

HTTPServerEnums::HTTPServerEnums() {
}

HTTPServerEnums::~HTTPServerEnums() {
}
