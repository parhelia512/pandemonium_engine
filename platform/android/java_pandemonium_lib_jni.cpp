/*************************************************************************/
/*  java_pandemonium_lib_jni.cpp                                         */
/*************************************************************************/
/*                         This file is part of:                         */
/*                          PANDEMONIUM ENGINE                           */
/*             https://github.com/Relintai/pandemonium_engine            */
/*************************************************************************/
/* Copyright (c) 2022-present Péter Magyar.                              */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
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

#include "java_pandemonium_lib_jni.h"
#include "java_pandemonium_io_wrapper.h"
#include "java_pandemonium_wrapper.h"

#include "android/asset_manager_jni.h"
#include "android_input_handler.h"
#include "api/java_class_wrapper.h"
#include "api/jni_singleton.h"
#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "dir_access_jandroid.h"
#include "file_access_android.h"
#include "file_access_filesystem_jandroid.h"
#include "jni_utils.h"
#include "main/input_default.h"
#include "main/main.h"
#include "net_socket_android.h"
#include "os_android.h"
#include "string_android.h"
#include "thread_jandroid.h"
#include "tts_android.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_settings.h"
#endif

#include <android/input.h>
#include <unistd.h>

static JavaClassWrapper *java_class_wrapper = NULL;
static OS_Android *os_android = NULL;
static AndroidInputHandler *input_handler = NULL;
static PandemoniumJavaWrapper *pandemonium_java = NULL;
static PandemoniumIOJavaWrapper *pandemonium_io_java = NULL;

static bool initialized = false;
static SafeNumeric<int> step; // Shared between UI and render threads

static Size2 new_size;
static Vector3 accelerometer;
static Vector3 gravity;
static Vector3 magnetometer;
static Vector3 gyroscope;

static void _initialize_java_modules() {
	if (!ProjectSettings::get_singleton()->has_setting("android/modules")) {
		return;
	}

	String modules = ProjectSettings::get_singleton()->get("android/modules");
	modules = modules.strip_edges();
	if (modules == String()) {
		return;
	}
	Vector<String> mods = modules.split(",", false);

	if (mods.size()) {
		jobject cls = pandemonium_java->get_class_loader();

		// TODO create wrapper for class loader

		JNIEnv *env = get_jni_env();
		jclass classLoader = env->FindClass("java/lang/ClassLoader");
		jmethodID findClass = env->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

		for (int i = 0; i < mods.size(); i++) {
			String m = mods[i];

			// Deprecated in Pandemonium 3.2.2, it's now a plugin to enable in export preset.
			if (m == "java/src/org/pandemoniumengine/pandemonium/PandemoniumPaymentV3") {
				WARN_PRINT("PandemoniumPaymentV3 is deprecated and is replaced by the 'PandemoniumPayment' plugin, which should be enabled in the Android export preset.");
				print_line("Skipping Android module: " + m);
				continue;
			}

			print_line("Loading Android module: " + m);
			jstring strClassName = env->NewStringUTF(m.utf8().get_data());
			jclass singletonClass = (jclass)env->CallObjectMethod(cls, findClass, strClassName);
			ERR_CONTINUE_MSG(!singletonClass, "Couldn't find singleton for class: " + m + ".");

			jmethodID initialize = env->GetStaticMethodID(singletonClass, "initialize", "(Landroid/app/Activity;)Ljava/src/org/pandemoniumengine/pandemonium/Pandemonium$SingletonBase;");
			ERR_CONTINUE_MSG(!initialize, "Couldn't find proper initialize function 'public static Pandemonium.SingletonBase Class::initialize(Activity p_activity)' initializer for singleton class: " + m + ".");

			jobject obj = env->CallStaticObjectMethod(singletonClass, initialize, pandemonium_java->get_activity());
			env->NewGlobalRef(obj);
		}
	}
}

static void _terminate(JNIEnv *env, bool p_restart = false) {
	step.set(-1); // Ensure no further steps are attempted and no further events are sent

	// lets cleanup
	if (java_class_wrapper) {
		memdelete(java_class_wrapper);
	}
	if (input_handler) {
		delete input_handler;
	}
	// Whether restarting is handled by 'Main::cleanup()'
	bool restart_on_cleanup = false;
	if (os_android) {
		restart_on_cleanup = os_android->is_restart_on_exit_set();
		os_android->main_loop_end();
		Main::cleanup();
		delete os_android;
	}
	if (pandemonium_io_java) {
		delete pandemonium_io_java;
	}
	if (pandemonium_java) {
		pandemonium_java->destroy_offscreen_gl(env);
		if (!restart_on_cleanup) {
			if (p_restart) {
				pandemonium_java->restart(env);
			} else {
				pandemonium_java->force_quit(env);
			}
		}
		delete pandemonium_java;
	}
}

extern "C" {

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_setVirtualKeyboardHeight(JNIEnv *env, jclass clazz, jint p_height) {
	if (pandemonium_io_java) {
		pandemonium_io_java->set_vk_height(p_height);
	}
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_initialize(JNIEnv *env, jclass clazz, jobject p_activity, jobject p_pandemonium_instance, jobject p_asset_manager, jobject p_pandemonium_io, jobject p_net_utils, jobject p_directory_access_handler, jobject p_file_access_handler, jboolean p_use_apk_expansion) {
	initialized = true;

	JavaVM *jvm;
	env->GetJavaVM(&jvm);

	// create our wrapper classes
	pandemonium_java = new PandemoniumJavaWrapper(env, p_activity, p_pandemonium_instance);
	pandemonium_io_java = new PandemoniumIOJavaWrapper(env, p_pandemonium_io);

	init_thread_jandroid(jvm, env);

	jobject amgr = env->NewGlobalRef(p_asset_manager);

	FileAccessAndroid::asset_manager = AAssetManager_fromJava(env, amgr);

	DirAccessJAndroid::setup(p_directory_access_handler);
	FileAccessFilesystemJAndroid::setup(p_file_access_handler);
	NetSocketAndroid::setup(p_net_utils);
	TTS_Android::setup(godot_java->get_member_object("tts", "Lorg/pandemoniumengine/pandemonium/tts/PandemoniumTTS;", env));

	os_android = new OS_Android(pandemonium_java, pandemonium_io_java, p_use_apk_expansion);

	char wd[500];
	getcwd(wd, 500);

	pandemonium_java->on_video_init(env);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_ondestroy(JNIEnv *env, jclass clazz) {
	_terminate(env, false);
}

JNIEXPORT jboolean JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_setup(JNIEnv *env, jclass clazz, jobjectArray p_cmdline) {
	setup_android_thread();

	const char **cmdline = NULL;
	jstring *j_cmdline = NULL;
	int cmdlen = 0;
	if (p_cmdline) {
		cmdlen = env->GetArrayLength(p_cmdline);
		if (cmdlen) {
			cmdline = (const char **)memalloc((cmdlen + 1) * sizeof(const char *));
			ERR_FAIL_NULL_V_MSG(cmdline, false, "Out of memory.");
			cmdline[cmdlen] = NULL;
			j_cmdline = (jstring *)memalloc(cmdlen * sizeof(jstring));
			ERR_FAIL_NULL_V_MSG(j_cmdline, false, "Out of memory.");

			for (int i = 0; i < cmdlen; i++) {
				jstring string = (jstring)env->GetObjectArrayElement(p_cmdline, i);
				const char *rawString = env->GetStringUTFChars(string, 0);

				cmdline[i] = rawString;
				j_cmdline[i] = string;
			}
		}
	}

	Error err = Main::setup(OS_Android::ANDROID_EXEC_PATH, cmdlen, (char **)cmdline, false);
	if (cmdline) {
		if (j_cmdline) {
			for (int i = 0; i < cmdlen; ++i) {
				env->ReleaseStringUTFChars(j_cmdline[i], cmdline[i]);
			}
			memfree(j_cmdline);
		}
		memfree(cmdline);
	}

	// Note: --help and --version return ERR_HELP, but this should be translated to 0 if exit codes are propagated.
	if (err != OK) {
		return false;
	}

	java_class_wrapper = memnew(JavaClassWrapper(pandemonium_java->get_activity()));
	ClassDB::register_class<JNISingleton>();
	_initialize_java_modules();
	return true;
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_resize(JNIEnv *env, jclass clazz, jint width, jint height) {
	if (os_android)
		os_android->set_display_size(Size2(width, height));
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_newcontext(JNIEnv *env, jclass clazz) {
	if (os_android) {
		if (step.get() == 0) {
			// During startup
			os_android->set_offscreen_gl_available(pandemonium_java->create_offscreen_gl(env));
		} else {
			// GL context recreated because it was lost; restart app to let it reload everything
			_terminate(env, true);
		}
	}
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_back(JNIEnv *env, jclass clazz) {
	if (step.get() == 0)
		return;

	if (os_android->get_main_loop()) {
		os_android->get_main_loop()->notification(MainLoop::NOTIFICATION_WM_GO_BACK_REQUEST);
	}
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_ttsCallback(JNIEnv *env, jclass clazz, jint event, jint id, jint pos) {
	TTS_Android::_java_utterance_callback(event, id, pos);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_step(JNIEnv *env, jclass clazz) {
	if (step.get() == -1)
		return;

	if (step.get() == 0) {
		// Since Pandemonium is initialized on the UI thread, _main_thread_id was set to that thread's id,
		// but for Pandemonium purposes, the main thread is the one running the game loop
		Main::setup2(Thread::get_caller_id());
		input_handler = new AndroidInputHandler();
		step.increment();
		return;
	}

	if (step.get() == 1) {
		if (!Main::start()) {
			return; // should exit instead and print the error
		}

		pandemonium_java->on_pandemonium_setup_completed(env);
		os_android->main_loop_begin();
		pandemonium_java->on_pandemonium_main_loop_started(env);
		step.increment();
	}

	os_android->process_accelerometer(accelerometer);
	os_android->process_gravity(gravity);
	os_android->process_magnetometer(magnetometer);
	os_android->process_gyroscope(gyroscope);

	if (os_android->main_loop_iterate()) {
		_terminate(env, false);
	}
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_dispatchMouseEvent(JNIEnv *env, jclass clazz, jint p_event_type, jint p_button_mask, jfloat p_x, jfloat p_y, jfloat p_delta_x, jfloat p_delta_y, jboolean p_double_click, jboolean p_source_mouse_relative) {
	if (step.get() <= 0) {
		return;
	}

	input_handler->process_mouse_event(p_event_type, p_button_mask, Point2(p_x, p_y), Vector2(p_delta_x, p_delta_y), p_double_click, p_source_mouse_relative);
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_dispatchTouchEvent(JNIEnv *env, jclass clazz, jint ev, jint pointer, jint pointer_count, jfloatArray position, jboolean p_double_tap) {
	if (step.get() <= 0)
		return;

	Vector<AndroidInputHandler::TouchPos> points;
	for (int i = 0; i < pointer_count; i++) {
		jfloat p[3];
		env->GetFloatArrayRegion(position, i * 3, 3, p);
		AndroidInputHandler::TouchPos tp;
		tp.pos = Point2(p[1], p[2]);
		tp.id = (int)p[0];
		points.push_back(tp);
	}

	input_handler->process_touch_event(ev, pointer, points, p_double_tap);
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_magnify(JNIEnv *env, jclass clazz, jfloat p_x, jfloat p_y, jfloat p_factor) {
	if (step.get() <= 0) {
		return;
	}

	input_handler->process_magnify(Point2(p_x, p_y), p_factor);
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_pan(JNIEnv *env, jclass clazz, jfloat p_x, jfloat p_y, jfloat p_delta_x, jfloat p_delta_y) {
	if (step.get() <= 0) {
		return;
	}

	input_handler->process_pan(Point2(p_x, p_y), Vector2(p_delta_x, p_delta_y));
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_joybutton(JNIEnv *env, jclass clazz, jint p_device, jint p_button, jboolean p_pressed) {
	if (step.get() <= 0) {
		return;
	}

	AndroidInputHandler::JoypadEvent jevent;
	jevent.device = p_device;
	jevent.type = AndroidInputHandler::JOY_EVENT_BUTTON;
	jevent.index = p_button;
	jevent.pressed = p_pressed;

	input_handler->process_joy_event(jevent);
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_joyaxis(JNIEnv *env, jclass clazz, jint p_device, jint p_axis, jfloat p_value) {
	if (step.get() <= 0)
		return;

	AndroidInputHandler::JoypadEvent jevent;
	jevent.device = p_device;
	jevent.type = AndroidInputHandler::JOY_EVENT_AXIS;
	jevent.index = p_axis;
	jevent.value = p_value;

	input_handler->process_joy_event(jevent);
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_joyhat(JNIEnv *env, jclass clazz, jint p_device, jint p_hat_x, jint p_hat_y) {
	if (step.get() <= 0)
		return;

	AndroidInputHandler::JoypadEvent jevent;
	jevent.device = p_device;
	jevent.type = AndroidInputHandler::JOY_EVENT_HAT;
	int hat = 0;
	if (p_hat_x != 0) {
		if (p_hat_x < 0)
			hat |= InputDefault::HAT_MASK_LEFT;
		else
			hat |= InputDefault::HAT_MASK_RIGHT;
	}
	if (p_hat_y != 0) {
		if (p_hat_y < 0)
			hat |= InputDefault::HAT_MASK_UP;
		else
			hat |= InputDefault::HAT_MASK_DOWN;
	}
	jevent.hat = hat;

	input_handler->process_joy_event(jevent);
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_joyconnectionchanged(JNIEnv *env, jclass clazz, jint p_device, jboolean p_connected, jstring p_name) {
	if (step.get() <= 0)
		return;

	String name = jstring_to_string(p_name, env);
	input_handler->joy_connection_changed(p_device, p_connected, name);
}

// Called on the UI thread
JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_key(JNIEnv *env, jclass clazz, jint p_scancode, jint p_physical_scancode, jint p_unicode, jboolean p_pressed) {
	if (step.get() <= 0)
		return;

	input_handler->process_key_event(p_scancode, p_physical_scancode, p_unicode, p_pressed);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_accelerometer(JNIEnv *env, jclass clazz, jfloat x, jfloat y, jfloat z) {
	accelerometer = Vector3(x, y, z);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_gravity(JNIEnv *env, jclass clazz, jfloat x, jfloat y, jfloat z) {
	gravity = Vector3(x, y, z);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_magnetometer(JNIEnv *env, jclass clazz, jfloat x, jfloat y, jfloat z) {
	magnetometer = Vector3(x, y, z);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_gyroscope(JNIEnv *env, jclass clazz, jfloat x, jfloat y, jfloat z) {
	gyroscope = Vector3(x, y, z);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_focusin(JNIEnv *env, jclass clazz) {
	if (step.get() <= 0)
		return;

	os_android->main_loop_focusin();
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_focusout(JNIEnv *env, jclass clazz) {
	if (step.get() <= 0)
		return;

	os_android->main_loop_focusout();
}

JNIEXPORT jstring JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_getGlobal(JNIEnv *env, jclass clazz, jstring path) {
	String js = jstring_to_string(path, env);

	return env->NewStringUTF(ProjectSettings::get_singleton()->get(js).operator String().utf8().get_data());
}

JNIEXPORT jstring JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_getEditorSetting(JNIEnv *env, jclass clazz, jstring p_setting_key) {
	String editor_setting = "";
#ifdef TOOLS_ENABLED
	String godot_setting_key = jstring_to_string(p_setting_key, env);
	editor_setting = EDITOR_GET(godot_setting_key).operator String();
#else
	WARN_PRINT("Access to the Editor Settings in only available on Editor builds");
#endif

	return env->NewStringUTF(editor_setting.utf8().get_data());
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_callobject(JNIEnv *env, jclass clazz, jlong ID, jstring method, jobjectArray params) {
	Object *obj = ObjectDB::get_instance(ID);
	ERR_FAIL_COND(!obj);

	int res = env->PushLocalFrame(16);
	ERR_FAIL_COND(res != 0);

	String str_method = jstring_to_string(method, env);

	int count = env->GetArrayLength(params);
	Variant *vlist = (Variant *)alloca(sizeof(Variant) * count);
	Variant **vptr = (Variant **)alloca(sizeof(Variant *) * count);
	for (int i = 0; i < count; i++) {
		jobject obj = env->GetObjectArrayElement(params, i);
		Variant v;
		if (obj)
			v = _jobject_to_variant(env, obj);
		memnew_placement(&vlist[i], Variant);
		vlist[i] = v;
		vptr[i] = &vlist[i];
		env->DeleteLocalRef(obj);
	};

	Variant::CallError err;
	obj->call(str_method, (const Variant **)vptr, count, err);
	// something

	env->PopLocalFrame(NULL);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_calldeferred(JNIEnv *env, jclass clazz, jlong ID, jstring method, jobjectArray params) {
	Object *obj = ObjectDB::get_instance(ID);
	ERR_FAIL_COND(!obj);

	int res = env->PushLocalFrame(16);
	ERR_FAIL_COND(res != 0);

	String str_method = jstring_to_string(method, env);

	int count = env->GetArrayLength(params);
	Variant args[VARIANT_ARG_MAX];

	for (int i = 0; i < MIN(count, VARIANT_ARG_MAX); i++) {
		jobject obj = env->GetObjectArrayElement(params, i);
		if (obj)
			args[i] = _jobject_to_variant(env, obj);
		env->DeleteLocalRef(obj);
	};

	static_assert(VARIANT_ARG_MAX == 8, "This code needs to be updated if VARIANT_ARG_MAX != 8");
	obj->call_deferred(str_method, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
	// something
	env->PopLocalFrame(NULL);
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_requestPermissionResult(JNIEnv *env, jclass clazz, jstring p_permission, jboolean p_result) {
	String permission = jstring_to_string(p_permission, env);
	if (permission == "android.permission.RECORD_AUDIO" && p_result) {
		AudioDriver::get_singleton()->capture_start();
	}

	if (os_android->get_main_loop()) {
		os_android->get_main_loop()->emit_signal("on_request_permissions_result", permission, p_result == JNI_TRUE);
	}
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_onRendererResumed(JNIEnv *env, jclass clazz) {
	if (step.get() <= 0)
		return;

	if (os_android->get_main_loop()) {
		os_android->get_main_loop()->notification(MainLoop::NOTIFICATION_APP_RESUMED);
	}
}

JNIEXPORT void JNICALL Java_org_pandemoniumengine_pandemonium_PandemoniumLib_onRendererPaused(JNIEnv *env, jclass clazz) {
	if (step.get() <= 0)
		return;

	if (os_android->get_main_loop()) {
		os_android->get_main_loop()->notification(MainLoop::NOTIFICATION_APP_PAUSED);
	}
}
}
