/*************************************************************************/
/*  java_pandemonium_view_wrapper.cpp                                          */
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

#include "java_pandemonium_view_wrapper.h"

PandemoniumJavaViewWrapper::PandemoniumJavaViewWrapper(jobject pandemonium_view) {
	JNIEnv *env = get_jni_env();
	ERR_FAIL_NULL(env);

	_pandemonium_view = env->NewGlobalRef(pandemonium_view);

	_cls = (jclass)env->NewGlobalRef(env->GetObjectClass(pandemonium_view));

	int android_device_api_level = android_get_device_api_level();
	if (android_device_api_level >= __ANDROID_API_N__) {
		_set_pointer_icon = env->GetMethodID(_cls, "setPointerIcon", "(I)V");
	}
	if (android_device_api_level >= __ANDROID_API_O__) {
		_request_pointer_capture = env->GetMethodID(_cls, "requestPointerCapture", "()V");
		_release_pointer_capture = env->GetMethodID(_cls, "releasePointerCapture", "()V");
	}
}

bool PandemoniumJavaViewWrapper::can_update_pointer_icon() const {
	return _set_pointer_icon != nullptr;
}

bool PandemoniumJavaViewWrapper::can_capture_pointer() const {
	return _request_pointer_capture != nullptr && _release_pointer_capture != nullptr;
}

void PandemoniumJavaViewWrapper::request_pointer_capture() {
	if (_request_pointer_capture != nullptr) {
		JNIEnv *env = get_jni_env();
		ERR_FAIL_NULL(env);

		env->CallVoidMethod(_pandemonium_view, _request_pointer_capture);
	}
}

void PandemoniumJavaViewWrapper::release_pointer_capture() {
	if (_release_pointer_capture != nullptr) {
		JNIEnv *env = get_jni_env();
		ERR_FAIL_NULL(env);

		env->CallVoidMethod(_pandemonium_view, _release_pointer_capture);
	}
}

void PandemoniumJavaViewWrapper::set_pointer_icon(int pointer_type) {
	if (_set_pointer_icon != nullptr) {
		JNIEnv *env = get_jni_env();
		ERR_FAIL_NULL(env);

		env->CallVoidMethod(_pandemonium_view, _set_pointer_icon, pointer_type);
	}
}

PandemoniumJavaViewWrapper::~PandemoniumJavaViewWrapper() {
	JNIEnv *env = get_jni_env();
	ERR_FAIL_NULL(env);

	env->DeleteGlobalRef(_pandemonium_view);
	env->DeleteGlobalRef(_cls);
}