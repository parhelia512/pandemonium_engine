/*************************************************************************/
/*  variant_call.cpp                                                     */
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

#include "variant.h"

#include "core/color_names.inc"
#include "core/core_string_names.h"
#include "core/crypto/crypto_core.h"
#include "core/io/compression.h"
#include "core/object.h"
#include "core/object_rc.h"
#include "core/os/os.h"
#include "core/script_language.h"

typedef void (*VariantFunc)(Variant &r_ret, Variant &p_self, const Variant **p_args);
typedef void (*VariantConstructFunc)(Variant &r_ret, const Variant **p_args);

struct _VariantCall {
	static void Vector3_dot(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		r_ret = reinterpret_cast<Vector3 *>(p_self._data._mem)->dot(*reinterpret_cast<const Vector3 *>(p_args[0]->_data._mem));
	}

	struct FuncData {
		int arg_count;
		Vector<Variant> default_args;
		Vector<Variant::Type> arg_types;
		Vector<StringName> arg_names;
		Variant::Type return_type;

		bool _const;
		bool returns;

		VariantFunc func;

		_FORCE_INLINE_ bool verify_arguments(const Variant **p_args, Variant::CallError &r_error) {
			if (arg_count == 0) {
				return true;
			}

			const Variant::Type *tptr = &arg_types[0];

			for (int i = 0; i < arg_count; i++) {
				if (tptr[i] == Variant::NIL || tptr[i] == p_args[i]->type) {
					continue; // all good
				}
				if (!Variant::can_convert(p_args[i]->type, tptr[i])) {
					r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
					r_error.argument = i;
					r_error.expected = tptr[i];
					return false;
				}
			}
			return true;
		}

		_FORCE_INLINE_ void call(Variant &r_ret, Variant &p_self, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
#ifdef DEBUG_ENABLED
			if (p_argcount > arg_count) {
				r_error.error = Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS;
				r_error.argument = arg_count;
				return;
			} else
#endif
					if (p_argcount < arg_count) {
				int def_argcount = default_args.size();
#ifdef DEBUG_ENABLED
				if (p_argcount < (arg_count - def_argcount)) {
					r_error.error = Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
					r_error.argument = arg_count - def_argcount;
					return;
				}

#endif
				ERR_FAIL_COND(p_argcount > VARIANT_ARG_MAX);
				const Variant *newargs[VARIANT_ARG_MAX];
				for (int i = 0; i < p_argcount; i++) {
					newargs[i] = p_args[i];
				}
				// fill in any remaining parameters with defaults
				int first_default_arg = arg_count - def_argcount;
				for (int i = p_argcount; i < arg_count; i++) {
					newargs[i] = &default_args[i - first_default_arg];
				}
#ifdef DEBUG_ENABLED
				if (!verify_arguments(newargs, r_error)) {
					return;
				}
#endif
				func(r_ret, p_self, newargs);
			} else {
#ifdef DEBUG_ENABLED
				if (!verify_arguments(p_args, r_error)) {
					return;
				}
#endif
				func(r_ret, p_self, p_args);
			}
		}
	};

	struct TypeFunc {
		Map<StringName, FuncData> functions;
	};

	static TypeFunc *type_funcs;

	struct Arg {
		StringName name;
		Variant::Type type;
		Arg() { type = Variant::NIL; }
		Arg(Variant::Type p_type, const StringName &p_name) :
				name(p_name),
				type(p_type) {
		}
	};

	//void addfunc(Variant::Type p_type, const StringName& p_name,VariantFunc p_func);

	static void make_func_return_variant(Variant::Type p_type, const StringName &p_name) {
#ifdef DEBUG_ENABLED
		type_funcs[p_type].functions[p_name].returns = true;
#endif
	}

	static void addfunc(bool p_const, Variant::Type p_type, Variant::Type p_return, bool p_has_return, const StringName &p_name, VariantFunc p_func, const Vector<Variant> &p_defaultarg, const Arg &p_argtype1 = Arg(), const Arg &p_argtype2 = Arg(), const Arg &p_argtype3 = Arg(), const Arg &p_argtype4 = Arg(), const Arg &p_argtype5 = Arg()) {
		FuncData funcdata;
		funcdata.func = p_func;
		funcdata.default_args = p_defaultarg;
		funcdata._const = p_const;
		funcdata.returns = p_has_return;
		funcdata.return_type = p_return;

		if (p_argtype1.name) {
			funcdata.arg_types.push_back(p_argtype1.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype1.name);
#endif

		} else {
			goto end;
		}

		if (p_argtype2.name) {
			funcdata.arg_types.push_back(p_argtype2.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype2.name);
#endif

		} else {
			goto end;
		}

		if (p_argtype3.name) {
			funcdata.arg_types.push_back(p_argtype3.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype3.name);
#endif

		} else {
			goto end;
		}

		if (p_argtype4.name) {
			funcdata.arg_types.push_back(p_argtype4.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype4.name);
#endif
		} else {
			goto end;
		}

		if (p_argtype5.name) {
			funcdata.arg_types.push_back(p_argtype5.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype5.name);
#endif
		} else {
			goto end;
		}

	end:

		funcdata.arg_count = funcdata.arg_types.size();
		type_funcs[p_type].functions[p_name] = funcdata;
	}

#define VCALL_LOCALMEM0(m_type, m_method)                                                              \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._mem)->m_method();                                     \
	}
#define VCALL_LOCALMEM0R(m_type, m_method)                                                             \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		r_ret = reinterpret_cast<m_type *>(p_self._data._mem)->m_method();                             \
	}
#define VCALL_LOCALMEM0RI(m_type, m_method, m_internal_method)                                         \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		r_ret = reinterpret_cast<m_type *>(p_self._data._mem)->m_internal_method();                    \
	}
#define VCALL_LOCALMEM1(m_type, m_method)                                                              \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0]);                           \
	}
#define VCALL_LOCALMEM1R(m_type, m_method)                                                             \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		r_ret = reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0]);                   \
	}
#define VCALL_LOCALMEM2(m_type, m_method)                                                              \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1]);               \
	}
#define VCALL_LOCALMEM2R(m_type, m_method)                                                             \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		r_ret = reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1]);       \
	}
#define VCALL_LOCALMEM3(m_type, m_method)                                                              \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1], *p_args[2]);   \
	}
#define VCALL_LOCALMEM3R(m_type, m_method)                                                                   \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {       \
		r_ret = reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1], *p_args[2]); \
	}
#define VCALL_LOCALMEM4(m_type, m_method)                                                                        \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {           \
		reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3]); \
	}
#define VCALL_LOCALMEM4R(m_type, m_method)                                                                               \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {                   \
		r_ret = reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3]); \
	}
#define VCALL_LOCALMEM5(m_type, m_method)                                                                                    \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {                       \
		reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3], *p_args[4]); \
	}
#define VCALL_LOCALMEM5R(m_type, m_method)                                                                                           \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {                               \
		r_ret = reinterpret_cast<m_type *>(p_self._data._mem)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3], *p_args[4]); \
	}

	// built-in functions of localmem based types

	VCALL_LOCALMEM1R(String, casecmp_to);
	VCALL_LOCALMEM1R(String, nocasecmp_to);
	VCALL_LOCALMEM1R(String, naturalnocasecmp_to);
	VCALL_LOCALMEM0R(String, length);
	VCALL_LOCALMEM3R(String, count);
	VCALL_LOCALMEM3R(String, countn);
	VCALL_LOCALMEM2R(String, substr);
	VCALL_LOCALMEM2R(String, substr_index);
	VCALL_LOCALMEM2R(String, get_slice);
	VCALL_LOCALMEM2R(String, find);
	VCALL_LOCALMEM1R(String, find_last);
	VCALL_LOCALMEM2R(String, findn);
	VCALL_LOCALMEM2R(String, rfind);
	VCALL_LOCALMEM2R(String, rfindn);
	VCALL_LOCALMEM1R(String, find_first_difference_index);
	VCALL_LOCALMEM2R(String, is_word_at);
	VCALL_LOCALMEM1R(String, match);
	VCALL_LOCALMEM1R(String, matchn);
	VCALL_LOCALMEM1R(String, begins_with);
	VCALL_LOCALMEM1R(String, ends_with);
	VCALL_LOCALMEM1R(String, is_subsequence_of);
	VCALL_LOCALMEM1R(String, is_subsequence_ofi);
	VCALL_LOCALMEM0R(String, bigrams);
	VCALL_LOCALMEM1R(String, similarity);
	VCALL_LOCALMEM2R(String, format);
	VCALL_LOCALMEM2R(String, replace);
	VCALL_LOCALMEM2R(String, replacen);
	VCALL_LOCALMEM0R(String, newline_to_br);
	VCALL_LOCALMEM1R(String, repeat);
	VCALL_LOCALMEM2R(String, insert);
	VCALL_LOCALMEM0R(String, capitalize);
	VCALL_LOCALMEM3R(String, split);
	VCALL_LOCALMEM3R(String, rsplit);
	VCALL_LOCALMEM2R(String, split_floats);
	VCALL_LOCALMEM1R(String, join);
	VCALL_LOCALMEM0R(String, to_upper);
	VCALL_LOCALMEM0R(String, to_lower);
	VCALL_LOCALMEM1R(String, left);
	VCALL_LOCALMEM1R(String, right);
	VCALL_LOCALMEM1R(String, indent);
	VCALL_LOCALMEM0R(String, dedent);
	VCALL_LOCALMEM2R(String, strip_edges);
	VCALL_LOCALMEM0R(String, strip_escapes);
	VCALL_LOCALMEM1R(String, lstrip);
	VCALL_LOCALMEM1R(String, rstrip);
	VCALL_LOCALMEM0R(String, get_extension);
	VCALL_LOCALMEM0R(String, get_basename);
	VCALL_LOCALMEM1R(String, plus_file);
	VCALL_LOCALMEM1R(String, ord_at);
	VCALL_LOCALMEM2(String, erase);
	VCALL_LOCALMEM0R(String, hash);
	VCALL_LOCALMEM0R(String, md5_text);
	VCALL_LOCALMEM0R(String, sha1_text);
	VCALL_LOCALMEM0R(String, sha256_text);
	VCALL_LOCALMEM0R(String, md5_buffer);
	VCALL_LOCALMEM0R(String, sha1_buffer);
	VCALL_LOCALMEM0R(String, sha256_buffer);
	VCALL_LOCALMEM0R(String, empty);
	VCALL_LOCALMEM1R(String, humanize_size);
	VCALL_LOCALMEM0R(String, is_abs_path);
	VCALL_LOCALMEM0R(String, simplify_path);
	VCALL_LOCALMEM0R(String, is_rel_path);
	VCALL_LOCALMEM0R(String, get_base_dir);
	VCALL_LOCALMEM0R(String, get_file);
	VCALL_LOCALMEM1R(String, append_path);
	VCALL_LOCALMEM0R(String, path_clean_end_slash);
	VCALL_LOCALMEM0R(String, path_ensure_end_slash);
	VCALL_LOCALMEM0R(String, path_get_prev_dir);
	VCALL_LOCALMEM0R(String, xml_escape);
	VCALL_LOCALMEM0R(String, xml_unescape);
	VCALL_LOCALMEM0R(String, http_escape);
	VCALL_LOCALMEM0R(String, http_unescape);
	VCALL_LOCALMEM0R(String, c_escape);
	VCALL_LOCALMEM0R(String, c_unescape);
	VCALL_LOCALMEM0R(String, json_escape);
	VCALL_LOCALMEM0R(String, percent_encode);
	VCALL_LOCALMEM0R(String, percent_decode);
	VCALL_LOCALMEM0R(String, validate_node_name);
	VCALL_LOCALMEM0R(String, is_valid_identifier);
	VCALL_LOCALMEM0R(String, is_valid_integer);
	VCALL_LOCALMEM0R(String, is_valid_float);
	VCALL_LOCALMEM1R(String, is_valid_hex_number);
	VCALL_LOCALMEM0R(String, is_valid_html_color);
	VCALL_LOCALMEM0R(String, is_valid_ip_address);
	VCALL_LOCALMEM0R(String, is_valid_filename);
	VCALL_LOCALMEM0R(String, is_valid_bool);
	VCALL_LOCALMEM0R(String, is_valid_unsigned_integer);
	VCALL_LOCALMEM0R(String, is_numeric);
	VCALL_LOCALMEM0R(String, is_zero);
	VCALL_LOCALMEM0RI(String, to_int, to_int64);
	VCALL_LOCALMEM0R(String, to_float);
	VCALL_LOCALMEM0R(String, to_bool);
	VCALL_LOCALMEM0R(String, to_uint);
	VCALL_LOCALMEM0RI(String, hex_to_int, hex_to_int64);
	VCALL_LOCALMEM1R(String, pad_decimals);
	VCALL_LOCALMEM1R(String, pad_zeros);
	VCALL_LOCALMEM1R(String, trim_prefix);
	VCALL_LOCALMEM1R(String, trim_suffix);

	static void _call_String_to_ascii(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		String *s = reinterpret_cast<String *>(p_self._data._mem);
		if (s->empty()) {
			r_ret = PoolByteArray();
			return;
		}
		CharString charstr = s->ascii();

		PoolByteArray retval;
		size_t len = charstr.length();
		retval.resize(len);
		PoolByteArray::Write w = retval.write();
		memcpy(w.ptr(), charstr.ptr(), len);
		w.release();

		r_ret = retval;
	}

	static void _call_String_to_utf8(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		String *s = reinterpret_cast<String *>(p_self._data._mem);
		if (s->empty()) {
			r_ret = PoolByteArray();
			return;
		}
		CharString charstr = s->utf8();

		PoolByteArray retval;
		size_t len = charstr.length();
		retval.resize(len);
		PoolByteArray::Write w = retval.write();
		memcpy(w.ptr(), charstr.ptr(), len);
		w.release();

		r_ret = retval;
	}

	static void _call_String_to_wchar(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		String *s = reinterpret_cast<String *>(p_self._data._mem);
		if (s->empty()) {
			r_ret = PoolByteArray();
			return;
		}

		PoolByteArray retval;
		size_t len = s->length() * sizeof(wchar_t);
		retval.resize(len);
		PoolByteArray::Write w = retval.write();
		memcpy(w.ptr(), s->ptr(), len);
		w.release();

		r_ret = retval;
	}

	VCALL_LOCALMEM1R(Vector2, distance_to);
	VCALL_LOCALMEM1R(Vector2, distance_squared_to);
	VCALL_LOCALMEM0R(Vector2, length);
	VCALL_LOCALMEM0R(Vector2, length_squared);
	VCALL_LOCALMEM0R(Vector2, normalized);
	VCALL_LOCALMEM0R(Vector2, is_normalized);
	VCALL_LOCALMEM1R(Vector2, is_equal_approx);
	VCALL_LOCALMEM1R(Vector2, posmod);
	VCALL_LOCALMEM1R(Vector2, posmodv);
	VCALL_LOCALMEM1R(Vector2, project);
	VCALL_LOCALMEM1R(Vector2, angle_to);
	VCALL_LOCALMEM1R(Vector2, angle_to_point);
	VCALL_LOCALMEM1R(Vector2, direction_to);
	VCALL_LOCALMEM2R(Vector2, linear_interpolate);
	VCALL_LOCALMEM2R(Vector2, slerp);
	VCALL_LOCALMEM4R(Vector2, cubic_interpolate);
	VCALL_LOCALMEM2R(Vector2, move_toward);
	VCALL_LOCALMEM1R(Vector2, rotated);
	VCALL_LOCALMEM0R(Vector2, tangent);
	VCALL_LOCALMEM0R(Vector2, floor);
	VCALL_LOCALMEM0R(Vector2, ceil);
	VCALL_LOCALMEM0R(Vector2, round);
	VCALL_LOCALMEM1R(Vector2, snapped);
	VCALL_LOCALMEM0R(Vector2, aspect);
	VCALL_LOCALMEM1R(Vector2, dot);
	VCALL_LOCALMEM1R(Vector2, slide);
	VCALL_LOCALMEM1R(Vector2, bounce);
	VCALL_LOCALMEM1R(Vector2, reflect);
	VCALL_LOCALMEM0R(Vector2, angle);
	VCALL_LOCALMEM1R(Vector2, cross);
	VCALL_LOCALMEM0R(Vector2, abs);
	VCALL_LOCALMEM1R(Vector2, clamped);
	VCALL_LOCALMEM1R(Vector2, limit_length);
	VCALL_LOCALMEM0R(Vector2, sign);

	VCALL_LOCALMEM2R(Vector2i, linear_interpolate);

	VCALL_LOCALMEM0R(Rect2, get_area);
	VCALL_LOCALMEM0R(Rect2, get_center);
	VCALL_LOCALMEM2R(Rect2, intersects);
	VCALL_LOCALMEM1R(Rect2, distance_to);
	VCALL_LOCALMEM2R(Rect2, intersects_transformed);
	static void _call_Rect2_intersects_segment(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		Point2 pos;
		Point2 normal;
		if (reinterpret_cast<Rect2 *>(p_self._data._mem)->intersects_segment(*p_args[0], *p_args[1], &pos, &normal)) {
			Array arr;
			arr.push_back(pos);
			arr.push_back(normal);
			r_ret = arr;
		} else {
			r_ret = Variant();
		}
	}
	VCALL_LOCALMEM1R(Rect2, encloses);
	VCALL_LOCALMEM0R(Rect2, has_no_area);
	VCALL_LOCALMEM1R(Rect2, clip);
	VCALL_LOCALMEM1R(Rect2, intersection);
	VCALL_LOCALMEM1R(Rect2, merge);
	VCALL_LOCALMEM1R(Rect2, has_point);
	VCALL_LOCALMEM1R(Rect2, is_equal_approx);
	VCALL_LOCALMEM1R(Rect2, grow);
	VCALL_LOCALMEM1(Rect2, grow_by);
	VCALL_LOCALMEM2R(Rect2, grow_margin);
	VCALL_LOCALMEM2R(Rect2, grow_side);
	VCALL_LOCALMEM4R(Rect2, grow_individual);
	VCALL_LOCALMEM1R(Rect2, expand);
	VCALL_LOCALMEM1(Rect2, expand_to);
	VCALL_LOCALMEM0R(Rect2, abs);
	VCALL_LOCALMEM1R(Rect2, get_support);
	VCALL_LOCALMEM1(Rect2, set_end);
	VCALL_LOCALMEM0R(Rect2, get_end);

	VCALL_LOCALMEM0R(Rect2i, get_area);
	VCALL_LOCALMEM0R(Rect2i, get_center);
	VCALL_LOCALMEM1R(Rect2i, intersects);
	VCALL_LOCALMEM1R(Rect2i, encloses);
	VCALL_LOCALMEM0R(Rect2i, has_no_area);
	VCALL_LOCALMEM1R(Rect2i, clip);
	VCALL_LOCALMEM1R(Rect2i, intersection);
	VCALL_LOCALMEM1R(Rect2i, merge);
	VCALL_LOCALMEM1R(Rect2i, has_point);
	VCALL_LOCALMEM1R(Rect2i, grow);
	VCALL_LOCALMEM2R(Rect2i, grow_margin);
	VCALL_LOCALMEM2R(Rect2i, grow_side);
	VCALL_LOCALMEM4R(Rect2i, grow_individual);
	VCALL_LOCALMEM1R(Rect2i, expand);
	VCALL_LOCALMEM1(Rect2i, expand_to);
	VCALL_LOCALMEM0R(Rect2i, abs);
	VCALL_LOCALMEM1(Rect2i, set_end);
	VCALL_LOCALMEM0R(Rect2i, get_end);

	VCALL_LOCALMEM0R(Vector3, min_axis);
	VCALL_LOCALMEM0R(Vector3, max_axis);
	VCALL_LOCALMEM1R(Vector3, distance_to);
	VCALL_LOCALMEM1R(Vector3, distance_squared_to);
	VCALL_LOCALMEM0R(Vector3, length);
	VCALL_LOCALMEM0R(Vector3, length_squared);
	VCALL_LOCALMEM0R(Vector3, normalized);
	VCALL_LOCALMEM0R(Vector3, is_normalized);
	VCALL_LOCALMEM1R(Vector3, is_equal_approx);
	VCALL_LOCALMEM0R(Vector3, inverse);
	VCALL_LOCALMEM1R(Vector3, snapped);
	VCALL_LOCALMEM2R(Vector3, rotated);
	VCALL_LOCALMEM2R(Vector3, linear_interpolate);
	VCALL_LOCALMEM2R(Vector3, slerp);
	VCALL_LOCALMEM4R(Vector3, cubic_interpolate);
	VCALL_LOCALMEM2R(Vector3, move_toward);
	VCALL_LOCALMEM1R(Vector3, dot);
	VCALL_LOCALMEM1R(Vector3, cross);
	VCALL_LOCALMEM1R(Vector3, outer);
	VCALL_LOCALMEM0R(Vector3, to_diagonal_matrix);
	VCALL_LOCALMEM0R(Vector3, abs);
	VCALL_LOCALMEM0R(Vector3, floor);
	VCALL_LOCALMEM0R(Vector3, ceil);
	VCALL_LOCALMEM0R(Vector3, round);
	VCALL_LOCALMEM1R(Vector3, posmod);
	VCALL_LOCALMEM1R(Vector3, posmodv);
	VCALL_LOCALMEM1R(Vector3, project);
	VCALL_LOCALMEM1R(Vector3, angle_to);
	VCALL_LOCALMEM2R(Vector3, signed_angle_to);
	VCALL_LOCALMEM1R(Vector3, direction_to);
	VCALL_LOCALMEM1R(Vector3, slide);
	VCALL_LOCALMEM1R(Vector3, bounce);
	VCALL_LOCALMEM1R(Vector3, reflect);
	VCALL_LOCALMEM1R(Vector3, limit_length);
	VCALL_LOCALMEM0R(Vector3, sign);

	VCALL_LOCALMEM0R(Vector3i, min_axis_index);
	VCALL_LOCALMEM0R(Vector3i, max_axis_index);
	VCALL_LOCALMEM0R(Vector3i, length);
	VCALL_LOCALMEM0R(Vector3i, length_squared);
	VCALL_LOCALMEM2R(Vector3i, linear_interpolate);
	VCALL_LOCALMEM0R(Vector3i, abs);
	VCALL_LOCALMEM0R(Vector3i, sign);

	VCALL_LOCALMEM1(Plane, set_normal);
	VCALL_LOCALMEM0R(Plane, get_normal);
	VCALL_LOCALMEM0(Plane, normalize);
	VCALL_LOCALMEM0R(Plane, normalized);
	VCALL_LOCALMEM0R(Plane, center);
	VCALL_LOCALMEM0R(Plane, get_any_point);
	VCALL_LOCALMEM0R(Plane, get_any_perpendicular_normal);
	VCALL_LOCALMEM1R(Plane, is_point_over);
	VCALL_LOCALMEM1R(Plane, distance_to);
	VCALL_LOCALMEM2R(Plane, has_point);
	//return vector3 if intersected, nil if not
	static void _call_Plane_intersect_3(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		Vector3 result;
		if (reinterpret_cast<Plane *>(p_self._data._mem)->intersect_3(*p_args[0], *p_args[1], &result)) {
			r_ret = result;
		} else {
			r_ret = Variant();
		}
	}
	static void _call_Plane_intersects_ray(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		Vector3 result;
		if (reinterpret_cast<Plane *>(p_self._data._mem)->intersects_ray(*p_args[0], *p_args[1], &result)) {
			r_ret = result;
		} else {
			r_ret = Variant();
		}
	}
	static void _call_Plane_intersects_segment(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		Vector3 result;
		if (reinterpret_cast<Plane *>(p_self._data._mem)->intersects_segment(*p_args[0], *p_args[1], &result)) {
			r_ret = result;
		} else {
			r_ret = Variant();
		}
	}
	VCALL_LOCALMEM1R(Plane, project);
	VCALL_LOCALMEM1R(Plane, is_equal_approx);
	VCALL_LOCALMEM1R(Plane, is_equal_approx_any_side);

	VCALL_LOCALMEM0R(Quaternion, length);
	VCALL_LOCALMEM0R(Quaternion, length_squared);
	VCALL_LOCALMEM0R(Quaternion, normalized);
	VCALL_LOCALMEM0R(Quaternion, is_normalized);
	VCALL_LOCALMEM1R(Quaternion, is_equal_approx);
	VCALL_LOCALMEM0R(Quaternion, inverse);
	VCALL_LOCALMEM1R(Quaternion, angle_to);
	VCALL_LOCALMEM1R(Quaternion, dot);
	VCALL_LOCALMEM1R(Quaternion, xform);
	VCALL_LOCALMEM2R(Quaternion, slerp);
	VCALL_LOCALMEM2R(Quaternion, slerpni);
	VCALL_LOCALMEM4R(Quaternion, cubic_slerp);
	VCALL_LOCALMEM0R(Quaternion, get_euler);
	VCALL_LOCALMEM1(Quaternion, set_euler);
	VCALL_LOCALMEM2(Quaternion, set_axis_angle);

	VCALL_LOCALMEM0R(Color, to_rgba32);
	VCALL_LOCALMEM0R(Color, to_argb32);
	VCALL_LOCALMEM0R(Color, to_abgr32);
	VCALL_LOCALMEM0R(Color, to_rgba64);
	VCALL_LOCALMEM0R(Color, to_argb64);
	VCALL_LOCALMEM0R(Color, to_abgr64);
	VCALL_LOCALMEM0R(Color, gray);
	VCALL_LOCALMEM0R(Color, get_h);
	VCALL_LOCALMEM0R(Color, get_s);
	VCALL_LOCALMEM0R(Color, get_v);
	VCALL_LOCALMEM4(Color, set_hsv);
	VCALL_LOCALMEM1R(Color, is_equal_approx);
	VCALL_LOCALMEM2R(Color, clamp);
	VCALL_LOCALMEM0(Color, invert);
	VCALL_LOCALMEM0(Color, contrast);
	VCALL_LOCALMEM0R(Color, inverted);
	VCALL_LOCALMEM0R(Color, contrasted);
	VCALL_LOCALMEM0R(Color, get_luminance);
	VCALL_LOCALMEM2R(Color, linear_interpolate);
	VCALL_LOCALMEM1R(Color, darkened);
	VCALL_LOCALMEM1R(Color, lightened);
	VCALL_LOCALMEM0R(Color, to_rgbe9995);
	VCALL_LOCALMEM1R(Color, blend);
	VCALL_LOCALMEM0R(Color, to_linear);
	VCALL_LOCALMEM0R(Color, to_srgb);
	VCALL_LOCALMEM1R(Color, to_html);
	VCALL_LOCALMEM4R(Color, from_hsv);
	VCALL_LOCALMEM0R(Color, get_r8);
	VCALL_LOCALMEM1(Color, set_r8);
	VCALL_LOCALMEM0R(Color, get_g8);
	VCALL_LOCALMEM1(Color, set_g8);
	VCALL_LOCALMEM0R(Color, get_b8);
	VCALL_LOCALMEM1(Color, set_b8);
	VCALL_LOCALMEM0R(Color, get_a8);
	VCALL_LOCALMEM1(Color, set_a8);
	VCALL_LOCALMEM1(Color, set_h);
	VCALL_LOCALMEM1(Color, set_s);
	VCALL_LOCALMEM1(Color, set_v);

	VCALL_LOCALMEM0R(RID, get_id);

	VCALL_LOCALMEM0R(NodePath, is_absolute);
	VCALL_LOCALMEM0R(NodePath, get_name_count);
	VCALL_LOCALMEM1R(NodePath, get_name);
	VCALL_LOCALMEM0R(NodePath, get_subname_count);
	VCALL_LOCALMEM1R(NodePath, get_subname);
	VCALL_LOCALMEM0R(NodePath, get_concatenated_subnames);
	VCALL_LOCALMEM0R(NodePath, get_as_property_path);
	VCALL_LOCALMEM0R(NodePath, is_empty);

	VCALL_LOCALMEM0R(Dictionary, size);
	VCALL_LOCALMEM0R(Dictionary, empty);
	VCALL_LOCALMEM0(Dictionary, clear);
	VCALL_LOCALMEM2(Dictionary, merge);
	VCALL_LOCALMEM1R(Dictionary, has);
	VCALL_LOCALMEM1R(Dictionary, has_all);
	VCALL_LOCALMEM1R(Dictionary, erase);
	VCALL_LOCALMEM0R(Dictionary, hash);
	VCALL_LOCALMEM0R(Dictionary, keys);
	VCALL_LOCALMEM0R(Dictionary, values);
	VCALL_LOCALMEM1R(Dictionary, duplicate);
	VCALL_LOCALMEM2R(Dictionary, get);

	VCALL_LOCALMEM2(Array, set);
	VCALL_LOCALMEM1R(Array, get);
	VCALL_LOCALMEM0R(Array, size);
	VCALL_LOCALMEM0R(Array, empty);
	VCALL_LOCALMEM0(Array, clear);
	VCALL_LOCALMEM0R(Array, hash);
	VCALL_LOCALMEM1(Array, push_back);
	VCALL_LOCALMEM1(Array, push_front);
	VCALL_LOCALMEM0R(Array, pop_back);
	VCALL_LOCALMEM0R(Array, pop_front);
	VCALL_LOCALMEM1R(Array, pop_at);
	VCALL_LOCALMEM1(Array, append);
	VCALL_LOCALMEM1(Array, append_array);
	VCALL_LOCALMEM1(Array, fill);
	VCALL_LOCALMEM1(Array, resize);
	VCALL_LOCALMEM2(Array, insert);
	VCALL_LOCALMEM1(Array, remove);
	VCALL_LOCALMEM0R(Array, front);
	VCALL_LOCALMEM0R(Array, back);
	VCALL_LOCALMEM2R(Array, find);
	VCALL_LOCALMEM2R(Array, rfind);
	VCALL_LOCALMEM1R(Array, find_last);
	VCALL_LOCALMEM1R(Array, count);
	VCALL_LOCALMEM1R(Array, has);
	VCALL_LOCALMEM1(Array, erase);
	VCALL_LOCALMEM0(Array, sort);
	VCALL_LOCALMEM2(Array, sort_custom);
	VCALL_LOCALMEM0(Array, shuffle);
	VCALL_LOCALMEM2R(Array, bsearch);
	VCALL_LOCALMEM4R(Array, bsearch_custom);
	VCALL_LOCALMEM1R(Array, duplicate);
	VCALL_LOCALMEM4R(Array, slice);
	VCALL_LOCALMEM0(Array, invert);
	VCALL_LOCALMEM0R(Array, max);
	VCALL_LOCALMEM0R(Array, min);

	static void _call_PoolByteArray_get_string_from_ascii(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		PoolByteArray *ba = reinterpret_cast<PoolByteArray *>(p_self._data._mem);
		String s;
		if (ba->size() > 0) {
			PoolByteArray::Read r = ba->read();
			CharString cs;
			cs.resize(ba->size() + 1);
			memcpy(cs.ptrw(), r.ptr(), ba->size());
			cs[ba->size()] = 0;

			s = cs.get_data();
		}
		r_ret = s;
	}

	static void _call_PoolByteArray_get_string_from_utf8(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		PoolByteArray *ba = reinterpret_cast<PoolByteArray *>(p_self._data._mem);
		String s;
		if (ba->size() > 0) {
			PoolByteArray::Read r = ba->read();
			s.parse_utf8((const char *)r.ptr(), ba->size());
		}
		r_ret = s;
	}

	static void _call_PoolByteArray_compress(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		PoolByteArray *ba = reinterpret_cast<PoolByteArray *>(p_self._data._mem);
		PoolByteArray compressed;
		if (ba->size() > 0) {
			Compression::Mode mode = (Compression::Mode)(int)(*p_args[0]);

			compressed.resize(Compression::get_max_compressed_buffer_size(ba->size(), mode));
			int result = Compression::compress(compressed.write().ptr(), ba->read().ptr(), ba->size(), mode);

			result = result >= 0 ? result : 0;
			compressed.resize(result);
		}
		r_ret = compressed;
	}

	static void _call_PoolByteArray_decompress(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		PoolByteArray *ba = reinterpret_cast<PoolByteArray *>(p_self._data._mem);
		PoolByteArray decompressed;
		Compression::Mode mode = (Compression::Mode)(int)(*p_args[1]);

		int buffer_size = (int)(*p_args[0]);

		if (buffer_size <= 0) {
			r_ret = decompressed;
			ERR_FAIL_MSG("Decompression buffer size must be greater than zero.");
		}
		if (ba->size() == 0) {
			r_ret = decompressed;
			ERR_FAIL_MSG("Compressed buffer size must be greater than zero.");
		}

		decompressed.resize(buffer_size);
		int result = Compression::decompress(decompressed.write().ptr(), buffer_size, ba->read().ptr(), ba->size(), mode);

		result = result >= 0 ? result : 0;
		decompressed.resize(result);

		r_ret = decompressed;
	}

	static void _call_PoolByteArray_decompress_dynamic(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		PoolByteArray *ba = reinterpret_cast<PoolByteArray *>(p_self._data._mem);
		PoolByteArray decompressed;
		int max_output_size = (int)(*p_args[0]);
		Compression::Mode mode = (Compression::Mode)(int)(*p_args[1]);

		decompressed.resize(1024);
		int result = Compression::decompress_dynamic(&decompressed, max_output_size, ba->read().ptr(), ba->size(), mode);

		if (result == OK) {
			r_ret = decompressed;
		} else {
			decompressed.resize(0);
			r_ret = decompressed;
			ERR_FAIL_MSG("Decompression failed.");
		}
	}

	static void _call_PoolByteArray_hex_encode(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		PoolByteArray *ba = reinterpret_cast<PoolByteArray *>(p_self._data._mem);
		if (ba->size() == 0) {
			r_ret = String();
			return;
		}
		PoolByteArray::Read r = ba->read();
		String s = String::hex_encode_buffer(&r[0], ba->size());
		r_ret = s;
	}

	VCALL_LOCALMEM0R(PoolByteArray, size);
	VCALL_LOCALMEM0R(PoolByteArray, empty);
	VCALL_LOCALMEM2(PoolByteArray, set);
	VCALL_LOCALMEM1R(PoolByteArray, get);
	VCALL_LOCALMEM1(PoolByteArray, push_back);
	VCALL_LOCALMEM1(PoolByteArray, fill);
	VCALL_LOCALMEM1(PoolByteArray, resize);
	VCALL_LOCALMEM2R(PoolByteArray, insert);
	VCALL_LOCALMEM1(PoolByteArray, remove);
	VCALL_LOCALMEM1(PoolByteArray, append);
	VCALL_LOCALMEM1(PoolByteArray, append_array);
	VCALL_LOCALMEM0(PoolByteArray, invert);
	VCALL_LOCALMEM2R(PoolByteArray, subarray);
	VCALL_LOCALMEM1R(PoolByteArray, contains);
	VCALL_LOCALMEM2R(PoolByteArray, find);
	VCALL_LOCALMEM2R(PoolByteArray, rfind);
	VCALL_LOCALMEM1R(PoolByteArray, count);
	VCALL_LOCALMEM1R(PoolByteArray, has);
	VCALL_LOCALMEM0(PoolByteArray, clear);
	VCALL_LOCALMEM0(PoolByteArray, sort);

	VCALL_LOCALMEM0R(PoolIntArray, size);
	VCALL_LOCALMEM0R(PoolIntArray, empty);
	VCALL_LOCALMEM2(PoolIntArray, set);
	VCALL_LOCALMEM1R(PoolIntArray, get);
	VCALL_LOCALMEM1(PoolIntArray, push_back);
	VCALL_LOCALMEM1(PoolIntArray, fill);
	VCALL_LOCALMEM1(PoolIntArray, resize);
	VCALL_LOCALMEM2R(PoolIntArray, insert);
	VCALL_LOCALMEM1(PoolIntArray, remove);
	VCALL_LOCALMEM1(PoolIntArray, append);
	VCALL_LOCALMEM1(PoolIntArray, append_array);
	VCALL_LOCALMEM0(PoolIntArray, invert);
	VCALL_LOCALMEM1R(PoolIntArray, contains);
	VCALL_LOCALMEM2R(PoolIntArray, find);
	VCALL_LOCALMEM2R(PoolIntArray, rfind);
	VCALL_LOCALMEM1R(PoolIntArray, count);
	VCALL_LOCALMEM1R(PoolIntArray, has);
	VCALL_LOCALMEM0(PoolIntArray, clear);
	VCALL_LOCALMEM0(PoolIntArray, sort);

	VCALL_LOCALMEM0R(PoolRealArray, size);
	VCALL_LOCALMEM0R(PoolRealArray, empty);
	VCALL_LOCALMEM2(PoolRealArray, set);
	VCALL_LOCALMEM1R(PoolRealArray, get);
	VCALL_LOCALMEM1(PoolRealArray, push_back);
	VCALL_LOCALMEM1(PoolRealArray, fill);
	VCALL_LOCALMEM1(PoolRealArray, resize);
	VCALL_LOCALMEM2R(PoolRealArray, insert);
	VCALL_LOCALMEM1(PoolRealArray, remove);
	VCALL_LOCALMEM1(PoolRealArray, append);
	VCALL_LOCALMEM1(PoolRealArray, append_array);
	VCALL_LOCALMEM0(PoolRealArray, invert);
	VCALL_LOCALMEM1R(PoolRealArray, contains);
	VCALL_LOCALMEM2R(PoolRealArray, find);
	VCALL_LOCALMEM2R(PoolRealArray, rfind);
	VCALL_LOCALMEM1R(PoolRealArray, count);
	VCALL_LOCALMEM1R(PoolRealArray, has);
	VCALL_LOCALMEM0(PoolRealArray, clear);
	VCALL_LOCALMEM0(PoolRealArray, sort);

	VCALL_LOCALMEM0R(PoolStringArray, size);
	VCALL_LOCALMEM0R(PoolStringArray, empty);
	VCALL_LOCALMEM2(PoolStringArray, set);
	VCALL_LOCALMEM1R(PoolStringArray, get);
	VCALL_LOCALMEM1(PoolStringArray, push_back);
	VCALL_LOCALMEM1(PoolStringArray, fill);
	VCALL_LOCALMEM1(PoolStringArray, resize);
	VCALL_LOCALMEM2R(PoolStringArray, insert);
	VCALL_LOCALMEM1(PoolStringArray, remove);
	VCALL_LOCALMEM1(PoolStringArray, append);
	VCALL_LOCALMEM1(PoolStringArray, append_array);
	VCALL_LOCALMEM0(PoolStringArray, invert);
	VCALL_LOCALMEM1R(PoolStringArray, join);
	VCALL_LOCALMEM1R(PoolStringArray, contains);
	VCALL_LOCALMEM2R(PoolStringArray, find);
	VCALL_LOCALMEM2R(PoolStringArray, rfind);
	VCALL_LOCALMEM1R(PoolStringArray, count);
	VCALL_LOCALMEM1R(PoolStringArray, has);
	VCALL_LOCALMEM0(PoolStringArray, clear);
	VCALL_LOCALMEM0(PoolStringArray, sort);

	VCALL_LOCALMEM0R(PoolVector2Array, size);
	VCALL_LOCALMEM0R(PoolVector2Array, empty);
	VCALL_LOCALMEM2(PoolVector2Array, set);
	VCALL_LOCALMEM1R(PoolVector2Array, get);
	VCALL_LOCALMEM1(PoolVector2Array, push_back);
	VCALL_LOCALMEM1(PoolVector2Array, fill);
	VCALL_LOCALMEM1(PoolVector2Array, resize);
	VCALL_LOCALMEM2R(PoolVector2Array, insert);
	VCALL_LOCALMEM1(PoolVector2Array, remove);
	VCALL_LOCALMEM1(PoolVector2Array, append);
	VCALL_LOCALMEM1(PoolVector2Array, append_array);
	VCALL_LOCALMEM0(PoolVector2Array, invert);
	VCALL_LOCALMEM1R(PoolVector2Array, contains);
	VCALL_LOCALMEM2R(PoolVector2Array, find);
	VCALL_LOCALMEM2R(PoolVector2Array, rfind);
	VCALL_LOCALMEM1R(PoolVector2Array, count);
	VCALL_LOCALMEM1R(PoolVector2Array, has);
	VCALL_LOCALMEM0(PoolVector2Array, clear);
	VCALL_LOCALMEM0(PoolVector2Array, sort);

	VCALL_LOCALMEM0R(PoolVector2iArray, size);
	VCALL_LOCALMEM0R(PoolVector2iArray, empty);
	VCALL_LOCALMEM2(PoolVector2iArray, set);
	VCALL_LOCALMEM1R(PoolVector2iArray, get);
	VCALL_LOCALMEM1(PoolVector2iArray, push_back);
	VCALL_LOCALMEM1(PoolVector2iArray, fill);
	VCALL_LOCALMEM1(PoolVector2iArray, resize);
	VCALL_LOCALMEM2R(PoolVector2iArray, insert);
	VCALL_LOCALMEM1(PoolVector2iArray, remove);
	VCALL_LOCALMEM1(PoolVector2iArray, append);
	VCALL_LOCALMEM1(PoolVector2iArray, append_array);
	VCALL_LOCALMEM0(PoolVector2iArray, invert);
	VCALL_LOCALMEM1R(PoolVector2iArray, contains);
	VCALL_LOCALMEM2R(PoolVector2iArray, find);
	VCALL_LOCALMEM2R(PoolVector2iArray, rfind);
	VCALL_LOCALMEM1R(PoolVector2iArray, count);
	VCALL_LOCALMEM1R(PoolVector2iArray, has);
	VCALL_LOCALMEM0(PoolVector2iArray, clear);
	VCALL_LOCALMEM0(PoolVector2iArray, sort);

	VCALL_LOCALMEM0R(PoolVector3Array, size);
	VCALL_LOCALMEM0R(PoolVector3Array, empty);
	VCALL_LOCALMEM2(PoolVector3Array, set);
	VCALL_LOCALMEM1R(PoolVector3Array, get);
	VCALL_LOCALMEM1(PoolVector3Array, push_back);
	VCALL_LOCALMEM1(PoolVector3Array, fill);
	VCALL_LOCALMEM1(PoolVector3Array, resize);
	VCALL_LOCALMEM2R(PoolVector3Array, insert);
	VCALL_LOCALMEM1(PoolVector3Array, remove);
	VCALL_LOCALMEM1(PoolVector3Array, append);
	VCALL_LOCALMEM1(PoolVector3Array, append_array);
	VCALL_LOCALMEM0(PoolVector3Array, invert);
	VCALL_LOCALMEM1R(PoolVector3Array, contains);
	VCALL_LOCALMEM2R(PoolVector3Array, find);
	VCALL_LOCALMEM2R(PoolVector3Array, rfind);
	VCALL_LOCALMEM1R(PoolVector3Array, count);
	VCALL_LOCALMEM1R(PoolVector3Array, has);
	VCALL_LOCALMEM0(PoolVector3Array, clear);
	VCALL_LOCALMEM0(PoolVector3Array, sort);

	VCALL_LOCALMEM0R(PoolVector3iArray, size);
	VCALL_LOCALMEM0R(PoolVector3iArray, empty);
	VCALL_LOCALMEM2(PoolVector3iArray, set);
	VCALL_LOCALMEM1R(PoolVector3iArray, get);
	VCALL_LOCALMEM1(PoolVector3iArray, push_back);
	VCALL_LOCALMEM1(PoolVector3iArray, fill);
	VCALL_LOCALMEM1(PoolVector3iArray, resize);
	VCALL_LOCALMEM2R(PoolVector3iArray, insert);
	VCALL_LOCALMEM1(PoolVector3iArray, remove);
	VCALL_LOCALMEM1(PoolVector3iArray, append);
	VCALL_LOCALMEM1(PoolVector3iArray, append_array);
	VCALL_LOCALMEM0(PoolVector3iArray, invert);
	VCALL_LOCALMEM1R(PoolVector3iArray, contains);
	VCALL_LOCALMEM2R(PoolVector3iArray, find);
	VCALL_LOCALMEM2R(PoolVector3iArray, rfind);
	VCALL_LOCALMEM1R(PoolVector3iArray, count);
	VCALL_LOCALMEM1R(PoolVector3iArray, has);
	VCALL_LOCALMEM0(PoolVector3iArray, clear);
	VCALL_LOCALMEM0(PoolVector3iArray, sort);

	VCALL_LOCALMEM0R(PoolColorArray, size);
	VCALL_LOCALMEM0R(PoolColorArray, empty);
	VCALL_LOCALMEM2(PoolColorArray, set);
	VCALL_LOCALMEM1R(PoolColorArray, get);
	VCALL_LOCALMEM1(PoolColorArray, push_back);
	VCALL_LOCALMEM1(PoolColorArray, fill);
	VCALL_LOCALMEM1(PoolColorArray, resize);
	VCALL_LOCALMEM2R(PoolColorArray, insert);
	VCALL_LOCALMEM1(PoolColorArray, remove);
	VCALL_LOCALMEM1(PoolColorArray, append);
	VCALL_LOCALMEM1(PoolColorArray, append_array);
	VCALL_LOCALMEM0(PoolColorArray, invert);
	VCALL_LOCALMEM1R(PoolColorArray, contains);
	VCALL_LOCALMEM2R(PoolColorArray, find);
	VCALL_LOCALMEM2R(PoolColorArray, rfind);
	VCALL_LOCALMEM1R(PoolColorArray, count);
	VCALL_LOCALMEM1R(PoolColorArray, has);
	VCALL_LOCALMEM0(PoolColorArray, clear);
	VCALL_LOCALMEM0(PoolColorArray, sort);

#define VCALL_PTR0(m_type, m_method)                                                                   \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._ptr)->m_method();                                     \
	}
#define VCALL_PTR0R(m_type, m_method)                                                                  \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		r_ret = reinterpret_cast<m_type *>(p_self._data._ptr)->m_method();                             \
	}
#define VCALL_PTR1(m_type, m_method)                                                                   \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0]);                           \
	}
#define VCALL_PTR1R(m_type, m_method)                                                                  \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		r_ret = reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0]);                   \
	}
#define VCALL_PTR2(m_type, m_method)                                                                   \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1]);               \
	}
#define VCALL_PTR2R(m_type, m_method)                                                                  \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		r_ret = reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1]);       \
	}
#define VCALL_PTR3(m_type, m_method)                                                                   \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) { \
		reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1], *p_args[2]);   \
	}
#define VCALL_PTR3R(m_type, m_method)                                                                        \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {       \
		r_ret = reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1], *p_args[2]); \
	}
#define VCALL_PTR4(m_type, m_method)                                                                             \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {           \
		reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3]); \
	}
#define VCALL_PTR4R(m_type, m_method)                                                                                    \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {                   \
		r_ret = reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3]); \
	}
#define VCALL_PTR5(m_type, m_method)                                                                                         \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {                       \
		reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3], *p_args[4]); \
	}
#define VCALL_PTR5R(m_type, m_method)                                                                                                \
	static void _call_##m_type##_##m_method(Variant &r_ret, Variant &p_self, const Variant **p_args) {                               \
		r_ret = reinterpret_cast<m_type *>(p_self._data._ptr)->m_method(*p_args[0], *p_args[1], *p_args[2], *p_args[3], *p_args[4]); \
	}

	VCALL_PTR0R(AABB, get_volume);
	VCALL_PTR0R(AABB, has_no_volume);
	VCALL_PTR0R(AABB, has_no_surface);
	VCALL_PTR1R(AABB, is_equal_approx);
	VCALL_PTR1R(AABB, intersects);
	VCALL_PTR1R(AABB, intersects_inclusive);
	VCALL_PTR1R(AABB, encloses);
	VCALL_PTR1R(AABB, merge);
	VCALL_PTR1(AABB, merge_with);
	VCALL_PTR1R(AABB, intersection);
	VCALL_PTR2R(AABB, intersects_segment);
	VCALL_PTR2R(AABB, intersects_ray);
	VCALL_PTR4R(AABB, smits_intersect_ray);
	VCALL_PTR1R(AABB, intersects_plane);
	VCALL_PTR1R(AABB, has_point);
	VCALL_PTR1R(AABB, get_support);
	VCALL_PTR0R(AABB, get_longest_axis);
	VCALL_PTR0R(AABB, get_longest_axis_index);
	VCALL_PTR0R(AABB, get_longest_axis_size);
	VCALL_PTR0R(AABB, get_shortest_axis);
	VCALL_PTR0R(AABB, get_shortest_axis_index);
	VCALL_PTR0R(AABB, get_shortest_axis_size);
	VCALL_PTR1R(AABB, grow);
	VCALL_PTR1(AABB, grow_by);
	VCALL_PTR1R(AABB, get_endpoint);
	VCALL_PTR1R(AABB, expand);
	VCALL_PTR1(AABB, expand_to);
	VCALL_PTR1(AABB, create_from_points);
	VCALL_PTR0R(AABB, abs);
	VCALL_PTR2R(AABB, intersects_segmentv);
	VCALL_PTR2R(AABB, intersects_rayv);
	VCALL_PTR1(AABB, quantize);
	VCALL_PTR1R(AABB, quantized);
	//Property
	//VCALL_PTR1(AABB, set_end);
	//VCALL_PTR0R(AABB, get_end);
	VCALL_PTR0R(AABB, get_center);

	VCALL_PTR0R(Transform2D, inverse);
	VCALL_PTR0R(Transform2D, affine_inverse);
	VCALL_PTR0R(Transform2D, get_rotation);
	VCALL_PTR0R(Transform2D, get_origin);
	VCALL_PTR0R(Transform2D, get_scale);
	VCALL_PTR0R(Transform2D, orthonormalized);
	VCALL_PTR1R(Transform2D, rotated);
	VCALL_PTR1R(Transform2D, scaled);
	VCALL_PTR1R(Transform2D, translated_local);
	VCALL_PTR2R(Transform2D, interpolate_with);
	VCALL_PTR1R(Transform2D, is_equal_approx);

	static void _call_Transform2D_xform(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR2:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform(p_args[0]->operator Vector2());
				return;
			case Variant::VECTOR2I:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform(p_args[0]->operator Vector2i());
				return;
			case Variant::RECT2:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform(p_args[0]->operator Rect2());
				return;
			case Variant::RECT2I:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform(p_args[0]->operator Rect2i());
				return;
			case Variant::POOL_VECTOR2_ARRAY:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform(p_args[0]->operator PoolVector2Array());
				return;
			case Variant::POOL_VECTOR2I_ARRAY:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform(p_args[0]->operator PoolVector2iArray());
				return;
			default:
				r_ret = Variant();
		}
	}

	static void _call_Transform2D_xform_inv(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR2:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Vector2());
				return;
			case Variant::VECTOR2I:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Vector2i());
				return;
			case Variant::RECT2:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Rect2());
				return;
			case Variant::RECT2I:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Rect2i());
				return;
			case Variant::POOL_VECTOR2_ARRAY:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform_inv(p_args[0]->operator PoolVector2Array());
				return;
			case Variant::POOL_VECTOR2I_ARRAY:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->xform_inv(p_args[0]->operator PoolVector2iArray());
				return;
			default:
				r_ret = Variant();
		}
	}

	static void _call_Transform2D_basis_xform(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR2:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->basis_xform(p_args[0]->operator Vector2());
				return;
			case Variant::VECTOR2I:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->basis_xform(p_args[0]->operator Vector2i());
				return;
			default:
				r_ret = Variant();
		}
	}

	static void _call_Transform2D_basis_xform_inv(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR2:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->basis_xform_inv(p_args[0]->operator Vector2());
				return;
			case Variant::VECTOR2I:
				r_ret = reinterpret_cast<Transform2D *>(p_self._data._ptr)->basis_xform_inv(p_args[0]->operator Vector2i());
				return;
			default:
				r_ret = Variant();
		}
	}

	VCALL_PTR0(Basis, invert);
	VCALL_PTR0R(Basis, inverse);
	VCALL_PTR0(Basis, transpose);
	VCALL_PTR0R(Basis, transposed);
	VCALL_PTR0R(Basis, determinant);
	VCALL_PTR1(Basis, from_z);
	VCALL_PTR2(Basis, rotate);
	VCALL_PTR2R(Basis, rotated);
	VCALL_PTR2(Basis, rotate_local);
	VCALL_PTR2R(Basis, rotated_local);
	VCALL_PTR1(Basis, rotatev);
	VCALL_PTR1R(Basis, rotatedv);
	VCALL_PTR1(Basis, rotateq);
	VCALL_PTR1R(Basis, rotatedq);
	VCALL_PTR0R(Basis, get_rotation_euler);
	VCALL_PTR0R(Basis, get_rotation_quaternion);
	VCALL_PTR0R(Basis, get_rotation);
	VCALL_PTR2(Basis, rotate_to_align);
	VCALL_PTR0R(Basis, get_euler_xyz);
	VCALL_PTR1(Basis, set_euler_xyz);
	VCALL_PTR0R(Basis, get_euler_xzy);
	VCALL_PTR1(Basis, set_euler_xzy);
	VCALL_PTR0R(Basis, get_euler_yzx);
	VCALL_PTR1(Basis, set_euler_yzx);
	VCALL_PTR0R(Basis, get_euler_yxz);
	VCALL_PTR1(Basis, set_euler_yxz);
	VCALL_PTR0R(Basis, get_euler_zxy);
	VCALL_PTR1(Basis, set_euler_zxy);
	VCALL_PTR0R(Basis, get_euler_zyx);
	VCALL_PTR1(Basis, set_euler_zyx);
	VCALL_PTR0R(Basis, get_euler);
	VCALL_PTR1(Basis, set_euler);
	VCALL_PTR0R(Basis, get_quaternion);
	VCALL_PTR1(Basis, set_quaternion);
	VCALL_PTR1(Basis, scale);
	VCALL_PTR1R(Basis, scaled);
	VCALL_PTR1(Basis, scale_local);
	VCALL_PTR1R(Basis, scaled_local);
	VCALL_PTR1(Basis, scale_orthogonal);
	VCALL_PTR1R(Basis, scaled_orthogonal);
	VCALL_PTR0(Basis, make_scale_uniform);
	VCALL_PTR0R(Basis, get_uniform_scale);
	VCALL_PTR0R(Basis, get_scale);
	VCALL_PTR0R(Basis, get_scale_abs);
	VCALL_PTR0R(Basis, get_scale_local);
	VCALL_PTR3(Basis, set_axis_angle_scale);
	VCALL_PTR2(Basis, set_euler_scale);
	VCALL_PTR2(Basis, set_quaternion_scale);
	VCALL_PTR1R(Basis, tdotx);
	VCALL_PTR1R(Basis, tdoty);
	VCALL_PTR1R(Basis, tdotz);
	VCALL_PTR1R(Basis, is_equal_approx);
	VCALL_PTR3R(Basis, is_equal_approx_ratio);
	VCALL_PTR0R(Basis, get_orthogonal_index);
	VCALL_PTR1(Basis, set_orthogonal_index);
	VCALL_PTR1(Basis, set_diagonal);
	VCALL_PTR0(Basis, is_orthogonal);
	VCALL_PTR0(Basis, is_diagonal);
	VCALL_PTR0(Basis, is_rotation);
	VCALL_PTR2R(Basis, slerp);
	VCALL_PTR2R(Basis, lerp);
	VCALL_PTR1R(Basis, get_column);
	VCALL_PTR2(Basis, set_column);
	VCALL_PTR3(Basis, set_columns);
	VCALL_PTR1R(Basis, get_row);
	VCALL_PTR2(Basis, set_row);
	VCALL_PTR1R(Basis, get_axis);
	VCALL_PTR2(Basis, set_axis);
	VCALL_PTR0R(Basis, get_main_diagonal);
	VCALL_PTR0(Basis, set_zero);
	VCALL_PTR1R(Basis, transpose_xform);
	VCALL_PTR0(Basis, orthonormalize);
	VCALL_PTR0R(Basis, orthonormalized);
	VCALL_PTR0(Basis, orthogonalize);
	VCALL_PTR0R(Basis, orthogonalized);
	VCALL_PTR0R(Basis, is_symmetric);
	VCALL_PTR0R(Basis, diagonalize);

	static void _call_Basis_xform(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR3:
				r_ret = reinterpret_cast<Basis *>(p_self._data._ptr)->xform(p_args[0]->operator Vector3());
				return;
			case Variant::VECTOR3I:
				r_ret = reinterpret_cast<Basis *>(p_self._data._ptr)->xform(p_args[0]->operator Vector3i());
				return;
			default:
				r_ret = Variant();
		}
	}

	static void _call_Basis_xform_inv(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR3:
				r_ret = reinterpret_cast<Basis *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Vector3());
				return;
			case Variant::VECTOR3I:
				r_ret = reinterpret_cast<Basis *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Vector3i());
				return;
			default:
				r_ret = Variant();
		}
	}

	VCALL_PTR0R(Transform, inverse);
	VCALL_PTR0R(Transform, affine_inverse);
	VCALL_PTR2R(Transform, rotated);
	VCALL_PTR1R(Transform, scaled);
	VCALL_PTR1R(Transform, translated);
	VCALL_PTR0R(Transform, orthonormalized);
	VCALL_PTR2R(Transform, looking_at);
	VCALL_PTR2R(Transform, interpolate_with);
	VCALL_PTR1R(Transform, is_equal_approx);

	static void _call_Transform_xform(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR3:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform(p_args[0]->operator Vector3());
				return;
			case Variant::VECTOR3I:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform(p_args[0]->operator Vector3i());
				return;
			case Variant::PLANE:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform(p_args[0]->operator Plane());
				return;
			case Variant::AABB:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform(p_args[0]->operator ::AABB());
				return;
			case Variant::POOL_VECTOR3_ARRAY:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform(p_args[0]->operator ::PoolVector3Array());
				return;
			case Variant::POOL_VECTOR3I_ARRAY:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform(p_args[0]->operator ::PoolVector3iArray());
				return;
			default:
				r_ret = Variant();
		}
	}

	static void _call_Transform_xform_inv(Variant &r_ret, Variant &p_self, const Variant **p_args) {
		switch (p_args[0]->type) {
			case Variant::VECTOR3:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Vector3());
				return;
			case Variant::VECTOR3I:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Vector3i());
				return;
			case Variant::PLANE:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform_inv(p_args[0]->operator Plane());
				return;
			case Variant::AABB:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform_inv(p_args[0]->operator ::AABB());
				return;
			case Variant::POOL_VECTOR3_ARRAY:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform_inv(p_args[0]->operator ::PoolVector3Array());
				return;
			case Variant::POOL_VECTOR3I_ARRAY:
				r_ret = reinterpret_cast<Transform *>(p_self._data._ptr)->xform_inv(p_args[0]->operator ::PoolVector3iArray());
				return;
			default:
				r_ret = Variant();
		}
	}

	struct ConstructData {
		int arg_count;
		Vector<Variant::Type> arg_types;
		Vector<String> arg_names;
		VariantConstructFunc func;
	};

	struct ConstructFunc {
		List<ConstructData> constructors;
	};

	static ConstructFunc *construct_funcs;

	static void Vector2_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Vector2(*p_args[0], *p_args[1]);
	}

	static void Vector2i_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Vector2i(*p_args[0], *p_args[1]);
	}

	static void Rect2_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Rect2(*p_args[0], *p_args[1]);
	}

	static void Rect2_init2(Variant &r_ret, const Variant **p_args) {
		r_ret = Rect2(*p_args[0], *p_args[1], *p_args[2], *p_args[3]);
	}

	static void Rect2i_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Rect2i(*p_args[0], *p_args[1]);
	}

	static void Rect2i_init2(Variant &r_ret, const Variant **p_args) {
		r_ret = Rect2i(*p_args[0], *p_args[1], *p_args[2], *p_args[3]);
	}

	static void Transform2D_init2(Variant &r_ret, const Variant **p_args) {
		Transform2D m(*p_args[0], *p_args[1]);
		r_ret = m;
	}

	static void Transform2D_init3(Variant &r_ret, const Variant **p_args) {
		Transform2D m;
		m[0] = *p_args[0];
		m[1] = *p_args[1];
		m[2] = *p_args[2];
		r_ret = m;
	}

	static void Vector3_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Vector3(*p_args[0], *p_args[1], *p_args[2]);
	}

	static void Vector3i_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Vector3i(*p_args[0], *p_args[1], *p_args[2]);
	}

	static void Plane_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Plane(*p_args[0], *p_args[1], *p_args[2], *p_args[3]);
	}

	static void Plane_init2(Variant &r_ret, const Variant **p_args) {
		r_ret = Plane(*p_args[0], *p_args[1], *p_args[2]);
	}

	static void Plane_init3(Variant &r_ret, const Variant **p_args) {
		r_ret = Plane(p_args[0]->operator Vector3(), p_args[1]->operator real_t());
	}
	static void Plane_init4(Variant &r_ret, const Variant **p_args) {
		r_ret = Plane(p_args[0]->operator Vector3(), p_args[1]->operator Vector3());
	}

	static void Quaternion_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Quaternion(*p_args[0], *p_args[1], *p_args[2], *p_args[3]);
	}

	static void Quaternion_init2(Variant &r_ret, const Variant **p_args) {
		r_ret = Quaternion(((Vector3)(*p_args[0])), ((real_t)(*p_args[1])));
	}

	static void Quaternion_init3(Variant &r_ret, const Variant **p_args) {
		r_ret = Quaternion(((Vector3)(*p_args[0])));
	}

	static void Color_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = Color(*p_args[0], *p_args[1], *p_args[2], *p_args[3]);
	}

	static void Color_init2(Variant &r_ret, const Variant **p_args) {
		r_ret = Color(*p_args[0], *p_args[1], *p_args[2]);
	}

	static void Color_init3(Variant &r_ret, const Variant **p_args) {
		r_ret = Color::html(*p_args[0]);
	}

	static void Color_init4(Variant &r_ret, const Variant **p_args) {
		r_ret = Color::hex(*p_args[0]);
	}

	static void AABB_init1(Variant &r_ret, const Variant **p_args) {
		r_ret = ::AABB(*p_args[0], *p_args[1]);
	}

	static void Basis_init1(Variant &r_ret, const Variant **p_args) {
		Basis m;
		m.set_column(0, *p_args[0]);
		m.set_column(1, *p_args[1]);
		m.set_column(2, *p_args[2]);
		r_ret = m;
	}

	static void Basis_init2(Variant &r_ret, const Variant **p_args) {
		r_ret = Basis(p_args[0]->operator Vector3(), p_args[1]->operator real_t());
	}

	static void Transform_init1(Variant &r_ret, const Variant **p_args) {
		Transform t;
		t.basis.set_column(0, *p_args[0]);
		t.basis.set_column(1, *p_args[1]);
		t.basis.set_column(2, *p_args[2]);
		t.origin = *p_args[3];
		r_ret = t;
	}

	static void Transform_init2(Variant &r_ret, const Variant **p_args) {
		r_ret = Transform(p_args[0]->operator Basis(), p_args[1]->operator Vector3());
	}

	static void add_constructor(VariantConstructFunc p_func, const Variant::Type p_type,
			const String &p_name1 = "", const Variant::Type p_type1 = Variant::NIL,
			const String &p_name2 = "", const Variant::Type p_type2 = Variant::NIL,
			const String &p_name3 = "", const Variant::Type p_type3 = Variant::NIL,
			const String &p_name4 = "", const Variant::Type p_type4 = Variant::NIL) {
		ConstructData cd;
		cd.func = p_func;
		cd.arg_count = 0;

		if (p_name1 == "") {
			goto end;
		}
		cd.arg_count++;
		cd.arg_names.push_back(p_name1);
		cd.arg_types.push_back(p_type1);

		if (p_name2 == "") {
			goto end;
		}
		cd.arg_count++;
		cd.arg_names.push_back(p_name2);
		cd.arg_types.push_back(p_type2);

		if (p_name3 == "") {
			goto end;
		}
		cd.arg_count++;
		cd.arg_names.push_back(p_name3);
		cd.arg_types.push_back(p_type3);

		if (p_name4 == "") {
			goto end;
		}
		cd.arg_count++;
		cd.arg_names.push_back(p_name4);
		cd.arg_types.push_back(p_type4);

	end:

		construct_funcs[p_type].constructors.push_back(cd);
	}

	struct ConstantData {
		Map<StringName, int> value;
#ifdef DEBUG_ENABLED
		List<StringName> value_ordered;
#endif
		Map<StringName, Variant> variant_value;
#ifdef DEBUG_ENABLED
		List<StringName> variant_value_ordered;
#endif
	};

	static ConstantData *constant_data;

	static void add_constant(int p_type, StringName p_constant_name, int p_constant_value) {
		constant_data[p_type].value[p_constant_name] = p_constant_value;
#ifdef DEBUG_ENABLED
		constant_data[p_type].value_ordered.push_back(p_constant_name);
#endif
	}

	static void add_variant_constant(int p_type, StringName p_constant_name, const Variant &p_constant_value) {
		constant_data[p_type].variant_value[p_constant_name] = p_constant_value;
#ifdef DEBUG_ENABLED
		constant_data[p_type].variant_value_ordered.push_back(p_constant_name);
#endif
	}
};

_VariantCall::TypeFunc *_VariantCall::type_funcs = nullptr;
_VariantCall::ConstructFunc *_VariantCall::construct_funcs = nullptr;
_VariantCall::ConstantData *_VariantCall::constant_data = nullptr;

Variant Variant::call(const StringName &p_method, const Variant **p_args, int p_argcount, CallError &r_error) {
	Variant ret;
	call_ptr(p_method, p_args, p_argcount, &ret, r_error);
	return ret;
}

void Variant::call_ptr(const StringName &p_method, const Variant **p_args, int p_argcount, Variant *r_ret, CallError &r_error) {
	Variant ret;

	if (type == Variant::OBJECT) {
		//call object
		Object *obj = _OBJ_PTR(*this);
		if (unlikely(!obj)) {
#ifdef DEBUG_ENABLED
			if (_get_obj().rc) {
				ERR_PRINT("Attempted method call on a deleted object.");
			}
#endif
			r_error.error = CallError::CALL_ERROR_INSTANCE_IS_NULL;
			return;
		}

		ret = obj->call(p_method, p_args, p_argcount, r_error);

		//else if (type==Variant::METHOD) {

	} else {
		r_error.error = Variant::CallError::CALL_OK;

		Map<StringName, _VariantCall::FuncData>::Element *E = _VariantCall::type_funcs[type].functions.find(p_method);
#ifdef DEBUG_ENABLED
		if (!E) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			return;
		}
#endif
		_VariantCall::FuncData &funcdata = E->get();
		funcdata.call(ret, *this, p_args, p_argcount, r_error);
	}

	if (r_error.error == Variant::CallError::CALL_OK && r_ret) {
		*r_ret = ret;
	}
}

#define VCALL(m_type, m_method) _VariantCall::_call_##m_type##_##m_method

Variant Variant::construct(const Variant::Type p_type, const Variant **p_args, int p_argcount, CallError &r_error, bool p_strict) {
	r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
	ERR_FAIL_INDEX_V(p_type, VARIANT_MAX, Variant());

	r_error.error = Variant::CallError::CALL_OK;
	if (p_argcount == 0) { //generic construct

		switch (p_type) {
			case NIL:
				return Variant();

			// atomic types
			case BOOL:
				return Variant(false);
			case INT:
				return 0;
			case REAL:
				return 0.0f;
			case STRING:
				return String();

			// math types
			case VECTOR2:
				return Vector2(); // 5
			case VECTOR2I:
				return Vector2i();
			case RECT2:
				return Rect2();
			case RECT2I:
				return Rect2i();
			case VECTOR3:
				return Vector3();
			case VECTOR3I:
				return Vector3i();
			case TRANSFORM2D:
				return Transform2D();
			case PLANE:
				return Plane();
			case QUATERNION:
				return Quaternion();
			case AABB:
				return ::AABB(); // 10
			case BASIS:
				return Basis();
			case TRANSFORM:
				return Transform();

			// misc types
			case COLOR:
				return Color();
			case NODE_PATH:
				return NodePath(); // 15
			case _RID:
				return RID();
			case OBJECT:
				return (Object *)nullptr;
			case STRING_NAME:
				return StringName();
			case DICTIONARY:
				return Dictionary();
			case ARRAY:
				return Array(); // 20
			case POOL_BYTE_ARRAY:
				return PoolByteArray();
			case POOL_INT_ARRAY:
				return PoolIntArray();
			case POOL_REAL_ARRAY:
				return PoolRealArray();
			case POOL_STRING_ARRAY:
				return PoolStringArray();
			case POOL_VECTOR2_ARRAY:
				return PoolVector2Array();
			case POOL_VECTOR2I_ARRAY:
				return PoolVector2iArray();
			case POOL_VECTOR3_ARRAY:
				return PoolVector3Array();
			case POOL_VECTOR3I_ARRAY:
				return PoolVector3iArray();
			case POOL_COLOR_ARRAY:
				return PoolColorArray();
			default:
				return Variant();
		}

	} else if (p_argcount == 1 && p_args[0]->type == p_type) {
		return *p_args[0]; //copy construct
	} else if (p_argcount == 1 && (!p_strict || Variant::can_convert(p_args[0]->type, p_type))) {
		//near match construct

		switch (p_type) {
			case NIL: {
				return Variant();
			} break;
			case BOOL: {
				return Variant(bool(*p_args[0]));
			}
			case INT: {
				return (int64_t(*p_args[0]));
			}
			case REAL: {
				return double(*p_args[0]);
			}
			case STRING: {
				return String(*p_args[0]);
			}
			case VECTOR2: {
				return Vector2(*p_args[0]);
			}
			case VECTOR2I: {
				//TODO Don't cast like this
				return Vector2i(Vector2(*p_args[0]));
			}
			case RECT2:
				return (Rect2(*p_args[0]));
			case RECT2I:
				//TODO Don't cast like this
				return (Rect2i(Rect2(*p_args[0])));
			case VECTOR3:
				return (Vector3(*p_args[0]));
			case VECTOR3I:
				return (Vector3i(*p_args[0]));
			case TRANSFORM2D:
				return (Transform2D(p_args[0]->operator Transform2D()));
			case PLANE:
				return (Plane(*p_args[0]));
			case QUATERNION:
				return (p_args[0]->operator Quaternion());
			case AABB:
				return (::AABB(*p_args[0])); // 10
			case BASIS:
				return (Basis(p_args[0]->operator Basis()));
			case TRANSFORM:
				return (Transform(p_args[0]->operator Transform()));

			// misc types
			case COLOR:
				return p_args[0]->type == Variant::STRING ? Color::html(*p_args[0]) : Color::hex(*p_args[0]);
			case NODE_PATH:
				return (NodePath(p_args[0]->operator NodePath())); // 15
			case _RID:
				return (RID(*p_args[0]));
			case OBJECT:
				return ((Object *)(p_args[0]->operator Object *()));
			case STRING_NAME:
				return (StringName(p_args[0]->operator StringName()));
			case DICTIONARY:
				return p_args[0]->operator Dictionary();
			case ARRAY:
				return p_args[0]->operator Array(); // 20

			// arrays
			case POOL_BYTE_ARRAY:
				return (PoolByteArray(*p_args[0]));
			case POOL_INT_ARRAY:
				return (PoolIntArray(*p_args[0]));
			case POOL_REAL_ARRAY:
				return (PoolRealArray(*p_args[0]));
			case POOL_STRING_ARRAY:
				return (PoolStringArray(*p_args[0]));
			case POOL_VECTOR2_ARRAY:
				return (PoolVector2Array(*p_args[0]));
			case POOL_VECTOR2I_ARRAY:
				return (PoolVector2iArray(*p_args[0]));
			case POOL_VECTOR3_ARRAY:
				return (PoolVector3Array(*p_args[0]));
			case POOL_VECTOR3I_ARRAY:
				return (PoolVector3iArray(*p_args[0]));
			case POOL_COLOR_ARRAY:
				return (PoolColorArray(*p_args[0]));
			default:
				return Variant();
		}
	} else if (p_argcount >= 1) {
		_VariantCall::ConstructFunc &c = _VariantCall::construct_funcs[p_type];

		for (List<_VariantCall::ConstructData>::Element *E = c.constructors.front(); E; E = E->next()) {
			const _VariantCall::ConstructData &cd = E->get();

			if (cd.arg_count != p_argcount) {
				continue;
			}

			//validate parameters
			for (int i = 0; i < cd.arg_count; i++) {
				if (!Variant::can_convert(p_args[i]->type, cd.arg_types[i])) {
					r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT; //no such constructor
					r_error.argument = i;
					r_error.expected = cd.arg_types[i];
					return Variant();
				}
			}

			Variant v;
			cd.func(v, p_args);
			return v;
		}
	}
	r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD; //no such constructor
	return Variant();
}

bool Variant::has_method(const StringName &p_method) const {
	if (type == OBJECT) {
		Object *obj = _OBJ_PTR(*this);
		if (unlikely(!obj)) {
#ifdef DEBUG_ENABLED
			if (_get_obj().rc) {
				ERR_PRINT("Attempted method check on a deleted object.");
			}
#endif
			return false;
		}
		return obj->has_method(p_method);
	}

	const _VariantCall::TypeFunc &tf = _VariantCall::type_funcs[type];
	return tf.functions.has(p_method);
}

Vector<Variant::Type> Variant::get_method_argument_types(Variant::Type p_type, const StringName &p_method) {
	ERR_FAIL_INDEX_V(p_type, Variant::VARIANT_MAX, Vector<Variant::Type>());
	const _VariantCall::TypeFunc &tf = _VariantCall::type_funcs[p_type];

	const Map<StringName, _VariantCall::FuncData>::Element *E = tf.functions.find(p_method);
	if (!E) {
		return Vector<Variant::Type>();
	}

	return E->get().arg_types;
}

bool Variant::is_method_const(Variant::Type p_type, const StringName &p_method) {
	ERR_FAIL_INDEX_V(p_type, Variant::VARIANT_MAX, false);
	const _VariantCall::TypeFunc &tf = _VariantCall::type_funcs[p_type];

	const Map<StringName, _VariantCall::FuncData>::Element *E = tf.functions.find(p_method);
	if (!E) {
		return false;
	}

	return E->get()._const;
}

Vector<StringName> Variant::get_method_argument_names(Variant::Type p_type, const StringName &p_method) {
	ERR_FAIL_INDEX_V(p_type, Variant::VARIANT_MAX, Vector<StringName>());
	const _VariantCall::TypeFunc &tf = _VariantCall::type_funcs[p_type];

	const Map<StringName, _VariantCall::FuncData>::Element *E = tf.functions.find(p_method);
	if (!E) {
		return Vector<StringName>();
	}

	return E->get().arg_names;
}

Variant::Type Variant::get_method_return_type(Variant::Type p_type, const StringName &p_method, bool *r_has_return) {
	ERR_FAIL_INDEX_V(p_type, Variant::VARIANT_MAX, Variant::NIL);
	const _VariantCall::TypeFunc &tf = _VariantCall::type_funcs[p_type];

	const Map<StringName, _VariantCall::FuncData>::Element *E = tf.functions.find(p_method);
	if (!E) {
		return Variant::NIL;
	}

	if (r_has_return) {
		*r_has_return = E->get().returns;
	}

	return E->get().return_type;
}

Vector<Variant> Variant::get_method_default_arguments(Variant::Type p_type, const StringName &p_method) {
	ERR_FAIL_INDEX_V(p_type, Variant::VARIANT_MAX, Vector<Variant>());
	const _VariantCall::TypeFunc &tf = _VariantCall::type_funcs[p_type];

	const Map<StringName, _VariantCall::FuncData>::Element *E = tf.functions.find(p_method);
	ERR_FAIL_COND_V(!E, Vector<Variant>());

	return E->get().default_args;
}

void Variant::get_method_list(List<MethodInfo> *p_list) const {
	const _VariantCall::TypeFunc &tf = _VariantCall::type_funcs[type];

	for (const Map<StringName, _VariantCall::FuncData>::Element *E = tf.functions.front(); E; E = E->next()) {
		const _VariantCall::FuncData &fd = E->get();

		MethodInfo mi;
		mi.name = E->key();

		if (fd._const) {
			mi.flags |= METHOD_FLAG_CONST;
		}

		for (int i = 0; i < fd.arg_types.size(); i++) {
			PropertyInfo pi;
			pi.type = fd.arg_types[i];
#ifdef DEBUG_ENABLED
			pi.name = fd.arg_names[i];
#endif
			mi.arguments.push_back(pi);
		}

		mi.default_arguments = fd.default_args;
		PropertyInfo ret;
#ifdef DEBUG_ENABLED
		ret.type = fd.return_type;
		if (fd.returns) {
			ret.name = "ret";
		}
		mi.return_val = ret;
#endif

		p_list->push_back(mi);
	}
}

void Variant::get_constructor_list(Variant::Type p_type, List<MethodInfo> *p_list) {
	ERR_FAIL_INDEX(p_type, VARIANT_MAX);

	//custom constructors
	for (const List<_VariantCall::ConstructData>::Element *E = _VariantCall::construct_funcs[p_type].constructors.front(); E; E = E->next()) {
		const _VariantCall::ConstructData &cd = E->get();
		MethodInfo mi;
		mi.name = Variant::get_type_name(p_type);
		mi.return_val.type = p_type;
		for (int i = 0; i < cd.arg_count; i++) {
			PropertyInfo pi;
			pi.name = cd.arg_names[i];
			pi.type = cd.arg_types[i];
			mi.arguments.push_back(pi);
		}
		p_list->push_back(mi);
	}
	//default constructors
	for (int i = 0; i < VARIANT_MAX; i++) {
		if (i == p_type) {
			continue;
		}
		if (!Variant::can_convert(Variant::Type(i), p_type)) {
			continue;
		}

		MethodInfo mi;
		mi.name = Variant::get_type_name(p_type);
		PropertyInfo pi;
		pi.name = "from";
		pi.type = Variant::Type(i);
		mi.arguments.push_back(pi);
		mi.return_val.type = p_type;
		p_list->push_back(mi);
	}
}

void Variant::get_constants_for_type(Variant::Type p_type, List<StringName> *p_constants) {
	ERR_FAIL_INDEX(p_type, Variant::VARIANT_MAX);

	_VariantCall::ConstantData &cd = _VariantCall::constant_data[p_type];

#ifdef DEBUG_ENABLED
	for (List<StringName>::Element *E = cd.value_ordered.front(); E; E = E->next()) {
		p_constants->push_back(E->get());
#else
	for (Map<StringName, int>::Element *E = cd.value.front(); E; E = E->next()) {
		p_constants->push_back(E->key());
#endif
	}

#ifdef DEBUG_ENABLED
	for (List<StringName>::Element *E = cd.variant_value_ordered.front(); E; E = E->next()) {
		p_constants->push_back(E->get());
#else
	for (Map<StringName, Variant>::Element *E = cd.variant_value.front(); E; E = E->next()) {
		p_constants->push_back(E->key());
#endif
	}
}

bool Variant::has_constant(Variant::Type p_type, const StringName &p_value) {
	ERR_FAIL_INDEX_V(p_type, Variant::VARIANT_MAX, false);
	_VariantCall::ConstantData &cd = _VariantCall::constant_data[p_type];
	return cd.value.has(p_value) || cd.variant_value.has(p_value);
}

Variant Variant::get_constant_value(Variant::Type p_type, const StringName &p_value, bool *r_valid) {
	if (r_valid) {
		*r_valid = false;
	}

	ERR_FAIL_INDEX_V(p_type, Variant::VARIANT_MAX, 0);
	_VariantCall::ConstantData &cd = _VariantCall::constant_data[p_type];

	Map<StringName, int>::Element *E = cd.value.find(p_value);
	if (!E) {
		Map<StringName, Variant>::Element *F = cd.variant_value.find(p_value);
		if (F) {
			if (r_valid) {
				*r_valid = true;
			}
			return F->get();
		} else {
			return -1;
		}
	}
	if (r_valid) {
		*r_valid = true;
	}

	return E->get();
}

void register_variant_methods() {
	_VariantCall::type_funcs = memnew_arr(_VariantCall::TypeFunc, Variant::VARIANT_MAX);

	_VariantCall::construct_funcs = memnew_arr(_VariantCall::ConstructFunc, Variant::VARIANT_MAX);
	_VariantCall::constant_data = memnew_arr(_VariantCall::ConstantData, Variant::VARIANT_MAX);

#define ADDFUNC0R(m_vtype, m_ret, m_class, m_method, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg);
#define ADDFUNC1R(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)));
#define ADDFUNC2R(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)));
#define ADDFUNC3R(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)));
#define ADDFUNC4R(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_arg4, m_argname4, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)), _VariantCall::Arg(Variant::m_arg4, _scs_create(m_argname4)));

#define ADDFUNC0RNC(m_vtype, m_ret, m_class, m_method, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg);
#define ADDFUNC1RNC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)));
#define ADDFUNC2RNC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)));
#define ADDFUNC3RNC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)));
#define ADDFUNC4RNC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_arg4, m_argname4, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, true, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)), _VariantCall::Arg(Variant::m_arg4, _scs_create(m_argname4)));

#define ADDFUNC0(m_vtype, m_ret, m_class, m_method, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg);
#define ADDFUNC1(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)));
#define ADDFUNC2(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)));
#define ADDFUNC3(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)));
#define ADDFUNC4(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_arg4, m_argname4, m_defarg) \
	_VariantCall::addfunc(true, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)), _VariantCall::Arg(Variant::m_arg4, _scs_create(m_argname4)));

#define ADDFUNC0NC(m_vtype, m_ret, m_class, m_method, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg);
#define ADDFUNC1NC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)));
#define ADDFUNC2NC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)));
#define ADDFUNC3NC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)));
#define ADDFUNC4NC(m_vtype, m_ret, m_class, m_method, m_arg1, m_argname1, m_arg2, m_argname2, m_arg3, m_argname3, m_arg4, m_argname4, m_defarg) \
	_VariantCall::addfunc(false, Variant::m_vtype, Variant::m_ret, false, _scs_create(#m_method), VCALL(m_class, m_method), m_defarg, _VariantCall::Arg(Variant::m_arg1, _scs_create(m_argname1)), _VariantCall::Arg(Variant::m_arg2, _scs_create(m_argname2)), _VariantCall::Arg(Variant::m_arg3, _scs_create(m_argname3)), _VariantCall::Arg(Variant::m_arg4, _scs_create(m_argname4)));

	/* STRING */
	ADDFUNC1R(STRING, INT, String, casecmp_to, STRING, "to", varray());
	ADDFUNC1R(STRING, INT, String, nocasecmp_to, STRING, "to", varray());
	ADDFUNC1R(STRING, INT, String, naturalnocasecmp_to, STRING, "to", varray());
	ADDFUNC0R(STRING, INT, String, length, varray());

	ADDFUNC2R(STRING, STRING, String, substr, INT, "from", INT, "len", varray(-1));
	ADDFUNC2R(STRING, STRING, String, substr_index, INT, "start_index", INT, "end_index", varray());

	ADDFUNC2R(STRING, STRING, String, get_slice, STRING, "delimiter", INT, "slice", varray());

	ADDFUNC2R(STRING, INT, String, find, STRING, "what", INT, "from", varray(0));

	ADDFUNC3R(STRING, INT, String, count, STRING, "what", INT, "from", INT, "to", varray(0, 0));
	ADDFUNC3R(STRING, INT, String, countn, STRING, "what", INT, "from", INT, "to", varray(0, 0));

	ADDFUNC1R(STRING, INT, String, find_last, STRING, "what", varray());
	ADDFUNC2R(STRING, INT, String, findn, STRING, "what", INT, "from", varray(0));
	ADDFUNC2R(STRING, INT, String, rfind, STRING, "what", INT, "from", varray(-1));
	ADDFUNC2R(STRING, INT, String, rfindn, STRING, "what", INT, "from", varray(-1));

	ADDFUNC1R(STRING, INT, String, find_first_difference_index, STRING, "what", varray(0));

	ADDFUNC2R(STRING, INT, String, is_word_at, INT, "index", STRING, "str", varray());

	ADDFUNC1R(STRING, BOOL, String, match, STRING, "expr", varray());
	ADDFUNC1R(STRING, BOOL, String, matchn, STRING, "expr", varray());
	ADDFUNC1R(STRING, BOOL, String, begins_with, STRING, "text", varray());
	ADDFUNC1R(STRING, BOOL, String, ends_with, STRING, "text", varray());
	ADDFUNC1R(STRING, BOOL, String, is_subsequence_of, STRING, "text", varray());
	ADDFUNC1R(STRING, BOOL, String, is_subsequence_ofi, STRING, "text", varray());
	ADDFUNC0R(STRING, POOL_STRING_ARRAY, String, bigrams, varray());
	ADDFUNC1R(STRING, REAL, String, similarity, STRING, "text", varray());

	ADDFUNC2R(STRING, STRING, String, format, NIL, "values", STRING, "placeholder", varray("{_}"));
	ADDFUNC2R(STRING, STRING, String, replace, STRING, "what", STRING, "forwhat", varray());
	ADDFUNC2R(STRING, STRING, String, replacen, STRING, "what", STRING, "forwhat", varray());

	ADDFUNC0R(STRING, STRING, String, newline_to_br, varray());

	ADDFUNC1R(STRING, STRING, String, repeat, INT, "count", varray());
	ADDFUNC2R(STRING, STRING, String, insert, INT, "position", STRING, "what", varray());
	ADDFUNC0R(STRING, STRING, String, capitalize, varray());
	ADDFUNC3R(STRING, POOL_STRING_ARRAY, String, split, STRING, "delimiter", BOOL, "allow_empty", INT, "maxsplit", varray(true, 0));
	ADDFUNC3R(STRING, POOL_STRING_ARRAY, String, rsplit, STRING, "delimiter", BOOL, "allow_empty", INT, "maxsplit", varray(true, 0));
	ADDFUNC2R(STRING, POOL_REAL_ARRAY, String, split_floats, STRING, "delimiter", BOOL, "allow_empty", varray(true));
	ADDFUNC1R(STRING, STRING, String, join, POOL_STRING_ARRAY, "parts", varray());

	ADDFUNC0R(STRING, STRING, String, to_upper, varray());
	ADDFUNC0R(STRING, STRING, String, to_lower, varray());

	ADDFUNC1R(STRING, STRING, String, left, INT, "position", varray());
	ADDFUNC1R(STRING, STRING, String, right, INT, "position", varray());
	ADDFUNC2R(STRING, STRING, String, strip_edges, BOOL, "left", BOOL, "right", varray(true, true));
	ADDFUNC0R(STRING, STRING, String, strip_escapes, varray());
	ADDFUNC1R(STRING, STRING, String, lstrip, STRING, "chars", varray());
	ADDFUNC1R(STRING, STRING, String, rstrip, STRING, "chars", varray());
	ADDFUNC0R(STRING, STRING, String, get_extension, varray());
	ADDFUNC0R(STRING, STRING, String, get_basename, varray());
	ADDFUNC1R(STRING, STRING, String, plus_file, STRING, "file", varray());
	ADDFUNC1R(STRING, INT, String, ord_at, INT, "at", varray());
	ADDFUNC1R(STRING, STRING, String, indent, STRING, "prefix", varray());
	ADDFUNC0R(STRING, STRING, String, dedent, varray());
	ADDFUNC2(STRING, NIL, String, erase, INT, "position", INT, "chars", varray());

	ADDFUNC0R(STRING, INT, String, hash, varray());
	ADDFUNC0R(STRING, STRING, String, md5_text, varray());
	ADDFUNC0R(STRING, STRING, String, sha1_text, varray());
	ADDFUNC0R(STRING, STRING, String, sha256_text, varray());
	ADDFUNC0R(STRING, POOL_BYTE_ARRAY, String, md5_buffer, varray());
	ADDFUNC0R(STRING, POOL_BYTE_ARRAY, String, sha1_buffer, varray());
	ADDFUNC0R(STRING, POOL_BYTE_ARRAY, String, sha256_buffer, varray());

	ADDFUNC0R(STRING, BOOL, String, empty, varray());
	ADDFUNC1R(STRING, STRING, String, humanize_size, INT, "size", varray());

	ADDFUNC0R(STRING, BOOL, String, is_abs_path, varray());
	ADDFUNC0R(STRING, STRING, String, simplify_path, varray());
	ADDFUNC0R(STRING, BOOL, String, is_rel_path, varray());
	ADDFUNC0R(STRING, STRING, String, get_base_dir, varray());
	ADDFUNC0R(STRING, STRING, String, get_file, varray());
	ADDFUNC1R(STRING, STRING, String, append_path, STRING, "path", varray());
	ADDFUNC0R(STRING, STRING, String, path_clean_end_slash, varray());
	ADDFUNC0R(STRING, STRING, String, path_ensure_end_slash, varray());
	ADDFUNC0R(STRING, STRING, String, path_get_prev_dir, varray());

	ADDFUNC0R(STRING, STRING, String, xml_escape, varray());
	ADDFUNC0R(STRING, STRING, String, xml_unescape, varray());
	ADDFUNC0R(STRING, STRING, String, http_escape, varray());
	ADDFUNC0R(STRING, STRING, String, http_unescape, varray());
	ADDFUNC0R(STRING, STRING, String, c_escape, varray());
	ADDFUNC0R(STRING, STRING, String, c_unescape, varray());
	ADDFUNC0R(STRING, STRING, String, json_escape, varray());
	ADDFUNC0R(STRING, STRING, String, percent_encode, varray());
	ADDFUNC0R(STRING, STRING, String, percent_decode, varray());
	ADDFUNC0R(STRING, STRING, String, validate_node_name, varray());

	ADDFUNC0R(STRING, BOOL, String, is_valid_identifier, varray());
	ADDFUNC0R(STRING, BOOL, String, is_valid_integer, varray());
	ADDFUNC0R(STRING, BOOL, String, is_valid_float, varray());
	ADDFUNC1R(STRING, BOOL, String, is_valid_hex_number, BOOL, "with_prefix", varray(false));
	ADDFUNC0R(STRING, BOOL, String, is_valid_html_color, varray());
	ADDFUNC0R(STRING, BOOL, String, is_valid_ip_address, varray());
	ADDFUNC0R(STRING, BOOL, String, is_valid_filename, varray());
	ADDFUNC0R(STRING, BOOL, String, is_valid_bool, varray());
	ADDFUNC0R(STRING, BOOL, String, is_valid_unsigned_integer, varray());

	ADDFUNC0R(STRING, BOOL, String, is_numeric, varray());
	ADDFUNC0R(STRING, BOOL, String, is_zero, varray());

	ADDFUNC0R(STRING, INT, String, to_int, varray());
	ADDFUNC0R(STRING, REAL, String, to_float, varray());
	ADDFUNC0R(STRING, INT, String, to_bool, varray());
	ADDFUNC0R(STRING, INT, String, to_uint, varray());

	ADDFUNC0R(STRING, INT, String, hex_to_int, varray());
	ADDFUNC1R(STRING, STRING, String, pad_decimals, INT, "digits", varray());
	ADDFUNC1R(STRING, STRING, String, pad_zeros, INT, "digits", varray());
	ADDFUNC1R(STRING, STRING, String, trim_prefix, STRING, "prefix", varray());
	ADDFUNC1R(STRING, STRING, String, trim_suffix, STRING, "suffix", varray());

	ADDFUNC0R(STRING, POOL_BYTE_ARRAY, String, to_ascii, varray());
	ADDFUNC0R(STRING, POOL_BYTE_ARRAY, String, to_utf8, varray());
	ADDFUNC0R(STRING, POOL_BYTE_ARRAY, String, to_wchar, varray());

	ADDFUNC0R(VECTOR2, REAL, Vector2, angle, varray());
	ADDFUNC1R(VECTOR2, REAL, Vector2, angle_to, VECTOR2, "to", varray());
	ADDFUNC1R(VECTOR2, REAL, Vector2, angle_to_point, VECTOR2, "to", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, direction_to, VECTOR2, "b", varray());
	ADDFUNC1R(VECTOR2, REAL, Vector2, distance_to, VECTOR2, "to", varray());
	ADDFUNC1R(VECTOR2, REAL, Vector2, distance_squared_to, VECTOR2, "to", varray());
	ADDFUNC0R(VECTOR2, REAL, Vector2, length, varray());
	ADDFUNC0R(VECTOR2, REAL, Vector2, length_squared, varray());
	ADDFUNC0R(VECTOR2, VECTOR2, Vector2, normalized, varray());
	ADDFUNC0R(VECTOR2, BOOL, Vector2, is_normalized, varray());
	ADDFUNC1R(VECTOR2, BOOL, Vector2, is_equal_approx, VECTOR2, "v", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, posmod, REAL, "mod", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, posmodv, VECTOR2, "modv", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, project, VECTOR2, "b", varray());
	ADDFUNC2R(VECTOR2, VECTOR2, Vector2, linear_interpolate, VECTOR2, "to", REAL, "weight", varray());
	ADDFUNC2R(VECTOR2, VECTOR2, Vector2, slerp, VECTOR2, "to", REAL, "weight", varray());
	ADDFUNC4R(VECTOR2, VECTOR2, Vector2, cubic_interpolate, VECTOR2, "b", VECTOR2, "pre_a", VECTOR2, "post_b", REAL, "weight", varray());
	ADDFUNC2R(VECTOR2, VECTOR2, Vector2, move_toward, VECTOR2, "to", REAL, "delta", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, rotated, REAL, "phi", varray());
	ADDFUNC0R(VECTOR2, VECTOR2, Vector2, tangent, varray());
	ADDFUNC0R(VECTOR2, VECTOR2, Vector2, floor, varray());
	ADDFUNC0R(VECTOR2, VECTOR2, Vector2, ceil, varray());
	ADDFUNC0R(VECTOR2, VECTOR2, Vector2, round, varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, snapped, VECTOR2, "by", varray());
	ADDFUNC0R(VECTOR2, REAL, Vector2, aspect, varray());
	ADDFUNC1R(VECTOR2, REAL, Vector2, dot, VECTOR2, "with", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, slide, VECTOR2, "n", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, bounce, VECTOR2, "n", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, reflect, VECTOR2, "n", varray());
	ADDFUNC1R(VECTOR2, REAL, Vector2, cross, VECTOR2, "with", varray());
	ADDFUNC0R(VECTOR2, VECTOR2, Vector2, abs, varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, clamped, REAL, "length", varray());
	ADDFUNC1R(VECTOR2, VECTOR2, Vector2, limit_length, REAL, "length", varray(1.0));
	ADDFUNC0R(VECTOR2, VECTOR2, Vector2, sign, varray());

	ADDFUNC2R(VECTOR2I, VECTOR2I, Vector2i, linear_interpolate, VECTOR2, "to", REAL, "weight", varray());

	ADDFUNC0R(RECT2, REAL, Rect2, get_area, varray());
	ADDFUNC0R(RECT2, VECTOR2, Rect2, get_center, varray());
	ADDFUNC2R(RECT2, BOOL, Rect2, intersects, RECT2, "b", BOOL, "include_borders", varray(false));
	ADDFUNC1R(RECT2, REAL, Rect2, distance_to, VECTOR2, "point", varray());
	ADDFUNC2R(RECT2, BOOL, Rect2, intersects_transformed, TRANSFORM2D, "xform", RECT2, "rect", varray());
	ADDFUNC2R(RECT2, NIL, Rect2, intersects_segment, VECTOR2, "from", VECTOR2, "to", varray());
	ADDFUNC1R(RECT2, BOOL, Rect2, encloses, RECT2, "b", varray());
	ADDFUNC0R(RECT2, BOOL, Rect2, has_no_area, varray());
	ADDFUNC1R(RECT2, RECT2, Rect2, clip, RECT2, "b", varray());
	ADDFUNC1R(RECT2, RECT2, Rect2, intersection, RECT2, "rect", varray());
	ADDFUNC1R(RECT2, RECT2, Rect2, merge, RECT2, "b", varray());
	ADDFUNC1R(RECT2, BOOL, Rect2, has_point, VECTOR2, "point", varray());
	ADDFUNC1R(RECT2, BOOL, Rect2, is_equal_approx, RECT2, "rect", varray());
	ADDFUNC1R(RECT2, RECT2, Rect2, grow, REAL, "by", varray());
	ADDFUNC1(RECT2, NIL, Rect2, grow_by, REAL, "by", varray());
	ADDFUNC2R(RECT2, RECT2, Rect2, grow_margin, INT, "margin", REAL, "by", varray());
	ADDFUNC2R(RECT2, RECT2, Rect2, grow_side, INT, "side", REAL, "by", varray());
	ADDFUNC4R(RECT2, RECT2, Rect2, grow_individual, REAL, "left", REAL, "top", REAL, "right", REAL, " bottom", varray());
	ADDFUNC1R(RECT2, RECT2, Rect2, expand, VECTOR2, "to", varray());
	ADDFUNC1R(RECT2, RECT2, Rect2, expand_to, VECTOR2, "to", varray());
	ADDFUNC0R(RECT2, RECT2, Rect2, abs, varray());
	ADDFUNC1R(RECT2, VECTOR2, Rect2, get_support, VECTOR2, "normal", varray());
	ADDFUNC1(RECT2, NIL, Rect2, set_end, VECTOR2, "end", varray());
	ADDFUNC0R(RECT2, VECTOR2, Rect2, get_end, varray());

	ADDFUNC0R(RECT2I, INT, Rect2i, get_area, varray());
	ADDFUNC0R(RECT2I, VECTOR2I, Rect2i, get_center, varray());
	ADDFUNC1R(RECT2I, BOOL, Rect2i, intersects, RECT2I, "b", varray());
	ADDFUNC1R(RECT2I, BOOL, Rect2i, encloses, RECT2I, "b", varray());
	ADDFUNC0R(RECT2I, BOOL, Rect2i, has_no_area, varray());
	ADDFUNC1R(RECT2I, RECT2I, Rect2i, clip, RECT2I, "b", varray());
	ADDFUNC1R(RECT2I, RECT2I, Rect2i, intersection, RECT2I, "rect", varray());
	ADDFUNC1R(RECT2I, RECT2I, Rect2i, merge, RECT2I, "b", varray());
	ADDFUNC1R(RECT2I, BOOL, Rect2i, has_point, VECTOR2I, "point", varray());
	ADDFUNC1R(RECT2I, RECT2I, Rect2i, grow, INT, "by", varray());
	ADDFUNC2R(RECT2I, RECT2I, Rect2i, grow_margin, INT, "margin", INT, "by", varray());
	ADDFUNC2R(RECT2I, RECT2I, Rect2i, grow_side, INT, "side", INT, "by", varray());
	ADDFUNC4R(RECT2I, RECT2I, Rect2i, grow_individual, INT, "left", INT, "top", INT, "right", INT, " bottom", varray());
	ADDFUNC1R(RECT2I, RECT2I, Rect2i, expand, VECTOR2I, "to", varray());
	ADDFUNC1R(RECT2I, RECT2I, Rect2i, expand_to, VECTOR2I, "to", varray());
	ADDFUNC0R(RECT2I, RECT2I, Rect2i, abs, varray());
	ADDFUNC1(RECT2I, NIL, Rect2i, set_end, VECTOR2I, "end", varray());
	ADDFUNC0R(RECT2I, VECTOR2I, Rect2i, get_end, varray());

	ADDFUNC0R(VECTOR3, INT, Vector3, min_axis, varray());
	ADDFUNC0R(VECTOR3, INT, Vector3, max_axis, varray());
	ADDFUNC1R(VECTOR3, REAL, Vector3, angle_to, VECTOR3, "to", varray());
	ADDFUNC2R(VECTOR3, REAL, Vector3, signed_angle_to, VECTOR3, "to", VECTOR3, "axis", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, direction_to, VECTOR3, "b", varray());
	ADDFUNC1R(VECTOR3, REAL, Vector3, distance_to, VECTOR3, "b", varray());
	ADDFUNC1R(VECTOR3, REAL, Vector3, distance_squared_to, VECTOR3, "b", varray());
	ADDFUNC0R(VECTOR3, REAL, Vector3, length, varray());
	ADDFUNC0R(VECTOR3, REAL, Vector3, length_squared, varray());
	ADDFUNC0R(VECTOR3, VECTOR3, Vector3, normalized, varray());
	ADDFUNC0R(VECTOR3, BOOL, Vector3, is_normalized, varray());
	ADDFUNC1R(VECTOR3, BOOL, Vector3, is_equal_approx, VECTOR3, "v", varray());
	ADDFUNC0R(VECTOR3, VECTOR3, Vector3, inverse, varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, snapped, VECTOR3, "by", varray());
	ADDFUNC2R(VECTOR3, VECTOR3, Vector3, rotated, VECTOR3, "axis", REAL, "phi", varray());
	ADDFUNC2R(VECTOR3, VECTOR3, Vector3, linear_interpolate, VECTOR3, "to", REAL, "weight", varray());
	ADDFUNC2R(VECTOR3, VECTOR3, Vector3, slerp, VECTOR3, "to", REAL, "weight", varray());
	ADDFUNC4R(VECTOR3, VECTOR3, Vector3, cubic_interpolate, VECTOR3, "b", VECTOR3, "pre_a", VECTOR3, "post_b", REAL, "weight", varray());
	ADDFUNC2R(VECTOR3, VECTOR3, Vector3, move_toward, VECTOR3, "to", REAL, "delta", varray());
	ADDFUNC1R(VECTOR3, REAL, Vector3, dot, VECTOR3, "b", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, cross, VECTOR3, "b", varray());
	ADDFUNC1R(VECTOR3, BASIS, Vector3, outer, VECTOR3, "b", varray());
	ADDFUNC0R(VECTOR3, BASIS, Vector3, to_diagonal_matrix, varray());
	ADDFUNC0R(VECTOR3, VECTOR3, Vector3, abs, varray());
	ADDFUNC0R(VECTOR3, VECTOR3, Vector3, floor, varray());
	ADDFUNC0R(VECTOR3, VECTOR3, Vector3, ceil, varray());
	ADDFUNC0R(VECTOR3, VECTOR3, Vector3, round, varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, posmod, REAL, "mod", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, posmodv, VECTOR3, "modv", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, project, VECTOR3, "b", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, slide, VECTOR3, "n", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, bounce, VECTOR3, "n", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, reflect, VECTOR3, "n", varray());
	ADDFUNC1R(VECTOR3, VECTOR3, Vector3, limit_length, REAL, "length", varray(1.0));
	ADDFUNC0R(VECTOR3, VECTOR3, Vector3, sign, varray());

	ADDFUNC0R(VECTOR3I, INT, Vector3i, min_axis_index, varray());
	ADDFUNC0R(VECTOR3I, INT, Vector3i, max_axis_index, varray());
	ADDFUNC0R(VECTOR3I, REAL, Vector3, length, varray());
	ADDFUNC0R(VECTOR3I, REAL, Vector3, length_squared, varray());
	ADDFUNC2R(VECTOR3I, VECTOR3I, Vector3, linear_interpolate, VECTOR3I, "to", REAL, "weight", varray());

	ADDFUNC1(PLANE, NIL, Plane, set_normal, VECTOR3, "normal", varray());
	ADDFUNC0R(PLANE, VECTOR3, Plane, get_normal, varray());
	ADDFUNC0(PLANE, NIL, Plane, normalize, varray());
	ADDFUNC0R(PLANE, PLANE, Plane, normalized, varray());
	ADDFUNC0R(PLANE, VECTOR3, Plane, center, varray());
	ADDFUNC0R(PLANE, VECTOR3, Plane, get_any_point, varray());
	ADDFUNC0R(PLANE, VECTOR3, Plane, get_any_perpendicular_normal, varray());
	ADDFUNC1R(PLANE, BOOL, Plane, is_point_over, VECTOR3, "point", varray());
	ADDFUNC1R(PLANE, REAL, Plane, distance_to, VECTOR3, "point", varray());
	ADDFUNC2R(PLANE, BOOL, Plane, has_point, VECTOR3, "point", REAL, "epsilon", varray(CMP_EPSILON));
	ADDFUNC2R(PLANE, VECTOR3, Plane, intersect_3, PLANE, "b", PLANE, "c", varray());
	ADDFUNC2R(PLANE, VECTOR3, Plane, intersects_ray, VECTOR3, "from", VECTOR3, "dir", varray());
	ADDFUNC2R(PLANE, VECTOR3, Plane, intersects_segment, VECTOR3, "begin", VECTOR3, "end", varray());
	ADDFUNC1R(PLANE, VECTOR3, Plane, project, VECTOR3, "point", varray());
	ADDFUNC1R(PLANE, BOOL, Plane, is_equal_approx, PLANE, "plane", varray());
	ADDFUNC1R(PLANE, BOOL, Plane, is_equal_approx_any_side, PLANE, "plane", varray());

	ADDFUNC0R(QUATERNION, REAL, Quaternion, length, varray());
	ADDFUNC0R(QUATERNION, REAL, Quaternion, length_squared, varray());
	ADDFUNC0R(QUATERNION, QUATERNION, Quaternion, normalized, varray());
	ADDFUNC0R(QUATERNION, BOOL, Quaternion, is_normalized, varray());
	ADDFUNC1R(QUATERNION, BOOL, Quaternion, is_equal_approx, QUATERNION, "quat", varray());
	ADDFUNC0R(QUATERNION, QUATERNION, Quaternion, inverse, varray());
	ADDFUNC1R(QUATERNION, REAL, Quaternion, angle_to, QUATERNION, "to", varray());
	ADDFUNC1R(QUATERNION, REAL, Quaternion, dot, QUATERNION, "b", varray());
	ADDFUNC1R(QUATERNION, VECTOR3, Quaternion, xform, VECTOR3, "v", varray());
	ADDFUNC2R(QUATERNION, QUATERNION, Quaternion, slerp, QUATERNION, "to", REAL, "weight", varray());
	ADDFUNC2R(QUATERNION, QUATERNION, Quaternion, slerpni, QUATERNION, "to", REAL, "weight", varray());
	ADDFUNC4R(QUATERNION, QUATERNION, Quaternion, cubic_slerp, QUATERNION, "b", QUATERNION, "pre_a", QUATERNION, "post_b", REAL, "weight", varray());
	ADDFUNC0R(QUATERNION, VECTOR3, Quaternion, get_euler, varray());
	ADDFUNC1(QUATERNION, NIL, Quaternion, set_euler, VECTOR3, "euler", varray());
	ADDFUNC2(QUATERNION, NIL, Quaternion, set_axis_angle, VECTOR3, "axis", REAL, "angle", varray());

	ADDFUNC0R(COLOR, INT, Color, to_rgba32, varray());
	ADDFUNC0R(COLOR, INT, Color, to_argb32, varray());
	ADDFUNC0R(COLOR, INT, Color, to_abgr32, varray());
	ADDFUNC0R(COLOR, INT, Color, to_rgba64, varray());
	ADDFUNC0R(COLOR, INT, Color, to_argb64, varray());
	ADDFUNC0R(COLOR, INT, Color, to_abgr64, varray());
	ADDFUNC0R(COLOR, REAL, Color, gray, varray());
	ADDFUNC0R(COLOR, REAL, Color, get_h, varray());
	ADDFUNC0R(COLOR, REAL, Color, get_s, varray());
	ADDFUNC0R(COLOR, REAL, Color, get_v, varray());
	ADDFUNC4(COLOR, NIL, Color, set_hsv, REAL, "h", REAL, "s", REAL, "v", REAL, "a", varray(1.0));
	ADDFUNC1R(COLOR, BOOL, Color, is_equal_approx, COLOR, "color", varray());
	ADDFUNC2R(COLOR, COLOR, Color, clamp, COLOR, "min", COLOR, "max", varray(Color(0, 0, 0, 0), Color(1, 1, 1, 1)));
	ADDFUNC0(COLOR, NIL, Color, invert, varray());
	ADDFUNC0R(COLOR, COLOR, Color, inverted, varray());
	ADDFUNC0R(COLOR, COLOR, Color, contrasted, varray());
	ADDFUNC0R(COLOR, REAL, Color, get_luminance, varray());
	ADDFUNC2R(COLOR, COLOR, Color, linear_interpolate, COLOR, "to", REAL, "weight", varray());
	ADDFUNC1R(COLOR, COLOR, Color, darkened, REAL, "amount", varray());
	ADDFUNC1R(COLOR, COLOR, Color, lightened, REAL, "amount", varray());
	ADDFUNC0R(COLOR, INT, Color, to_rgbe9995, varray());
	ADDFUNC1R(COLOR, COLOR, Color, blend, COLOR, "over", varray());
	ADDFUNC0R(COLOR, COLOR, Color, to_linear, varray());
	ADDFUNC0R(COLOR, COLOR, Color, to_srgb, varray());
	ADDFUNC1R(COLOR, STRING, Color, to_html, BOOL, "with_alpha", varray(true));
	ADDFUNC4R(COLOR, COLOR, Color, from_hsv, REAL, "h", REAL, "s", REAL, "v", REAL, "a", varray(1.0));
	ADDFUNC0R(COLOR, INT, Color, get_r8, varray());
	ADDFUNC1(COLOR, NIL, Color, set_r8, INT, "r", varray());
	ADDFUNC0R(COLOR, INT, Color, get_g8, varray());
	ADDFUNC1(COLOR, NIL, Color, set_g8, INT, "g", varray());
	ADDFUNC0R(COLOR, INT, Color, get_b8, varray());
	ADDFUNC1(COLOR, NIL, Color, set_b8, INT, "b", varray());
	ADDFUNC0R(COLOR, INT, Color, get_a8, varray());
	ADDFUNC1(COLOR, NIL, Color, set_a8, INT, "a", varray());
	ADDFUNC1(COLOR, NIL, Color, set_h, INT, "h", varray());
	ADDFUNC1(COLOR, NIL, Color, set_s, INT, "s", varray());
	ADDFUNC1(COLOR, NIL, Color, set_v, INT, "v", varray());

	ADDFUNC0R(_RID, INT, RID, get_id, varray());

	ADDFUNC0R(NODE_PATH, BOOL, NodePath, is_absolute, varray());
	ADDFUNC0R(NODE_PATH, INT, NodePath, get_name_count, varray());
	ADDFUNC1R(NODE_PATH, STRING, NodePath, get_name, INT, "idx", varray());
	ADDFUNC0R(NODE_PATH, INT, NodePath, get_subname_count, varray());
	ADDFUNC1R(NODE_PATH, STRING, NodePath, get_subname, INT, "idx", varray());
	ADDFUNC0R(NODE_PATH, STRING, NodePath, get_concatenated_subnames, varray());
	ADDFUNC0R(NODE_PATH, NODE_PATH, NodePath, get_as_property_path, varray());
	ADDFUNC0R(NODE_PATH, BOOL, NodePath, is_empty, varray());

	ADDFUNC0R(DICTIONARY, INT, Dictionary, size, varray());
	ADDFUNC0R(DICTIONARY, BOOL, Dictionary, empty, varray());
	ADDFUNC0NC(DICTIONARY, NIL, Dictionary, clear, varray());
	ADDFUNC2NC(DICTIONARY, NIL, Dictionary, merge, DICTIONARY, "dictionary", BOOL, "overwrite", varray(false));
	ADDFUNC1R(DICTIONARY, BOOL, Dictionary, has, NIL, "key", varray());
	ADDFUNC1R(DICTIONARY, BOOL, Dictionary, has_all, ARRAY, "keys", varray());
	ADDFUNC1RNC(DICTIONARY, BOOL, Dictionary, erase, NIL, "key", varray());
	ADDFUNC0R(DICTIONARY, INT, Dictionary, hash, varray());
	ADDFUNC0R(DICTIONARY, ARRAY, Dictionary, keys, varray());
	ADDFUNC0R(DICTIONARY, ARRAY, Dictionary, values, varray());
	ADDFUNC1R(DICTIONARY, DICTIONARY, Dictionary, duplicate, BOOL, "deep", varray(false));
	ADDFUNC2R(DICTIONARY, NIL, Dictionary, get, NIL, "key", NIL, "default", varray(Variant()));

	ADDFUNC0R(ARRAY, INT, Array, size, varray());
	ADDFUNC0R(ARRAY, BOOL, Array, empty, varray());
	ADDFUNC0NC(ARRAY, NIL, Array, clear, varray());
	ADDFUNC0R(ARRAY, INT, Array, hash, varray());
	ADDFUNC1NC(ARRAY, NIL, Array, push_back, NIL, "value", varray());
	ADDFUNC1NC(ARRAY, NIL, Array, push_front, NIL, "value", varray());
	ADDFUNC1NC(ARRAY, NIL, Array, fill, NIL, "value", varray());
	ADDFUNC1NC(ARRAY, NIL, Array, append, NIL, "value", varray());
	ADDFUNC1NC(ARRAY, NIL, Array, append_array, ARRAY, "array", varray());
	ADDFUNC1NC(ARRAY, NIL, Array, resize, INT, "size", varray());
	ADDFUNC2NC(ARRAY, NIL, Array, insert, INT, "position", NIL, "value", varray());
	ADDFUNC1NC(ARRAY, NIL, Array, remove, INT, "position", varray());
	ADDFUNC1NC(ARRAY, NIL, Array, erase, NIL, "value", varray());
	ADDFUNC0R(ARRAY, NIL, Array, front, varray());
	ADDFUNC0R(ARRAY, NIL, Array, back, varray());
	ADDFUNC2R(ARRAY, INT, Array, find, NIL, "what", INT, "from", varray(0));
	ADDFUNC2R(ARRAY, INT, Array, rfind, NIL, "what", INT, "from", varray(-1));
	ADDFUNC1R(ARRAY, INT, Array, find_last, NIL, "value", varray());
	ADDFUNC1R(ARRAY, INT, Array, count, NIL, "value", varray());
	ADDFUNC1R(ARRAY, BOOL, Array, has, NIL, "value", varray());
	ADDFUNC0RNC(ARRAY, NIL, Array, pop_back, varray());
	ADDFUNC0RNC(ARRAY, NIL, Array, pop_front, varray());
	ADDFUNC1RNC(ARRAY, NIL, Array, pop_at, INT, "position", varray());
	ADDFUNC0NC(ARRAY, NIL, Array, sort, varray());
	ADDFUNC2NC(ARRAY, NIL, Array, sort_custom, OBJECT, "obj", STRING, "func", varray());
	ADDFUNC0NC(ARRAY, NIL, Array, shuffle, varray());
	ADDFUNC2R(ARRAY, INT, Array, bsearch, NIL, "value", BOOL, "before", varray(true));
	ADDFUNC4R(ARRAY, INT, Array, bsearch_custom, NIL, "value", OBJECT, "obj", STRING, "func", BOOL, "before", varray(true));
	ADDFUNC0NC(ARRAY, NIL, Array, invert, varray());
	ADDFUNC1R(ARRAY, ARRAY, Array, duplicate, BOOL, "deep", varray(false));
	ADDFUNC4R(ARRAY, ARRAY, Array, slice, INT, "begin", INT, "end", INT, "step", BOOL, "deep", varray(1, false));
	ADDFUNC0R(ARRAY, NIL, Array, max, varray());
	ADDFUNC0R(ARRAY, NIL, Array, min, varray());

	ADDFUNC0R(POOL_BYTE_ARRAY, INT, PoolByteArray, size, varray());
	ADDFUNC0R(POOL_BYTE_ARRAY, BOOL, PoolByteArray, empty, varray());
	ADDFUNC2(POOL_BYTE_ARRAY, NIL, PoolByteArray, set, INT, "idx", INT, "byte", varray());
	ADDFUNC1(POOL_BYTE_ARRAY, NIL, PoolByteArray, push_back, INT, "byte", varray());
	ADDFUNC1(POOL_BYTE_ARRAY, NIL, PoolByteArray, fill, INT, "byte", varray());
	ADDFUNC1(POOL_BYTE_ARRAY, NIL, PoolByteArray, append, INT, "byte", varray());
	ADDFUNC1(POOL_BYTE_ARRAY, NIL, PoolByteArray, append_array, POOL_BYTE_ARRAY, "array", varray());
	ADDFUNC1(POOL_BYTE_ARRAY, NIL, PoolByteArray, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_BYTE_ARRAY, INT, PoolByteArray, insert, INT, "idx", INT, "byte", varray());
	ADDFUNC1(POOL_BYTE_ARRAY, NIL, PoolByteArray, resize, INT, "idx", varray());
	ADDFUNC0(POOL_BYTE_ARRAY, NIL, PoolByteArray, invert, varray());
	ADDFUNC2R(POOL_BYTE_ARRAY, POOL_BYTE_ARRAY, PoolByteArray, subarray, INT, "from", INT, "to", varray());
	ADDFUNC1R(POOL_BYTE_ARRAY, BOOL, PoolByteArray, contains, INT, "value", varray())
	ADDFUNC2R(POOL_BYTE_ARRAY, INT, PoolByteArray, find, INT, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_BYTE_ARRAY, INT, PoolByteArray, rfind, INT, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_BYTE_ARRAY, INT, PoolByteArray, count, INT, "value", varray());
	ADDFUNC1R(POOL_BYTE_ARRAY, BOOL, PoolByteArray, has, INT, "value", varray());
	ADDFUNC0(POOL_BYTE_ARRAY, NIL, PoolByteArray, clear, varray());
	ADDFUNC0(POOL_BYTE_ARRAY, NIL, PoolByteArray, sort, varray());

	ADDFUNC0R(POOL_BYTE_ARRAY, STRING, PoolByteArray, get_string_from_ascii, varray());
	ADDFUNC0R(POOL_BYTE_ARRAY, STRING, PoolByteArray, get_string_from_utf8, varray());
	ADDFUNC0R(POOL_BYTE_ARRAY, STRING, PoolByteArray, hex_encode, varray());
	ADDFUNC1R(POOL_BYTE_ARRAY, POOL_BYTE_ARRAY, PoolByteArray, compress, INT, "compression_mode", varray(0));
	ADDFUNC2R(POOL_BYTE_ARRAY, POOL_BYTE_ARRAY, PoolByteArray, decompress, INT, "buffer_size", INT, "compression_mode", varray(0));
	ADDFUNC2R(POOL_BYTE_ARRAY, POOL_BYTE_ARRAY, PoolByteArray, decompress_dynamic, INT, "max_output_size", INT, "compression_mode", varray(0));

	ADDFUNC0R(POOL_INT_ARRAY, INT, PoolIntArray, size, varray());
	ADDFUNC0R(POOL_INT_ARRAY, BOOL, PoolIntArray, empty, varray());
	ADDFUNC2(POOL_INT_ARRAY, NIL, PoolIntArray, set, INT, "idx", INT, "integer", varray());
	ADDFUNC1(POOL_INT_ARRAY, NIL, PoolIntArray, push_back, INT, "integer", varray());
	ADDFUNC1(POOL_INT_ARRAY, NIL, PoolIntArray, fill, INT, "integer", varray());
	ADDFUNC1(POOL_INT_ARRAY, NIL, PoolIntArray, append, INT, "integer", varray());
	ADDFUNC1(POOL_INT_ARRAY, NIL, PoolIntArray, append_array, POOL_INT_ARRAY, "array", varray());
	ADDFUNC1(POOL_INT_ARRAY, NIL, PoolIntArray, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_INT_ARRAY, INT, PoolIntArray, insert, INT, "idx", INT, "integer", varray());
	ADDFUNC1(POOL_INT_ARRAY, NIL, PoolIntArray, resize, INT, "idx", varray());
	ADDFUNC0(POOL_INT_ARRAY, NIL, PoolIntArray, invert, varray());
	ADDFUNC1R(POOL_INT_ARRAY, BOOL, PoolIntArray, contains, INT, "value", varray())
	ADDFUNC2R(POOL_INT_ARRAY, INT, PoolIntArray, find, INT, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_INT_ARRAY, INT, PoolIntArray, rfind, INT, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_INT_ARRAY, INT, PoolIntArray, count, INT, "value", varray());
	ADDFUNC1R(POOL_INT_ARRAY, BOOL, PoolIntArray, has, INT, "value", varray());
	ADDFUNC0(POOL_INT_ARRAY, NIL, PoolIntArray, clear, varray());
	ADDFUNC0(POOL_INT_ARRAY, NIL, PoolIntArray, sort, varray());

	ADDFUNC0R(POOL_REAL_ARRAY, INT, PoolRealArray, size, varray());
	ADDFUNC0R(POOL_REAL_ARRAY, BOOL, PoolRealArray, empty, varray());
	ADDFUNC2(POOL_REAL_ARRAY, NIL, PoolRealArray, set, INT, "idx", REAL, "value", varray());
	ADDFUNC1(POOL_REAL_ARRAY, NIL, PoolRealArray, push_back, REAL, "value", varray());
	ADDFUNC1(POOL_REAL_ARRAY, NIL, PoolRealArray, fill, REAL, "value", varray());
	ADDFUNC1(POOL_REAL_ARRAY, NIL, PoolRealArray, append, REAL, "value", varray());
	ADDFUNC1(POOL_REAL_ARRAY, NIL, PoolRealArray, append_array, POOL_REAL_ARRAY, "array", varray());
	ADDFUNC1(POOL_REAL_ARRAY, NIL, PoolRealArray, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_REAL_ARRAY, INT, PoolRealArray, insert, INT, "idx", REAL, "value", varray());
	ADDFUNC1(POOL_REAL_ARRAY, NIL, PoolRealArray, resize, INT, "idx", varray());
	ADDFUNC0(POOL_REAL_ARRAY, NIL, PoolRealArray, invert, varray());
	ADDFUNC1R(POOL_REAL_ARRAY, BOOL, PoolRealArray, contains, REAL, "value", varray())
	ADDFUNC2R(POOL_REAL_ARRAY, INT, PoolRealArray, find, REAL, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_REAL_ARRAY, INT, PoolRealArray, rfind, REAL, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_REAL_ARRAY, INT, PoolRealArray, count, REAL, "value", varray());
	ADDFUNC1R(POOL_REAL_ARRAY, BOOL, PoolRealArray, has, REAL, "value", varray());
	ADDFUNC0(POOL_REAL_ARRAY, NIL, PoolRealArray, clear, varray());
	ADDFUNC0(POOL_REAL_ARRAY, NIL, PoolRealArray, sort, varray());

	ADDFUNC0R(POOL_STRING_ARRAY, INT, PoolStringArray, size, varray());
	ADDFUNC0R(POOL_STRING_ARRAY, BOOL, PoolStringArray, empty, varray());
	ADDFUNC2(POOL_STRING_ARRAY, NIL, PoolStringArray, set, INT, "idx", STRING, "string", varray());
	ADDFUNC1(POOL_STRING_ARRAY, NIL, PoolStringArray, push_back, STRING, "string", varray());
	ADDFUNC1(POOL_STRING_ARRAY, NIL, PoolStringArray, fill, STRING, "string", varray());
	ADDFUNC1(POOL_STRING_ARRAY, NIL, PoolStringArray, append, STRING, "string", varray());
	ADDFUNC1(POOL_STRING_ARRAY, NIL, PoolStringArray, append_array, POOL_STRING_ARRAY, "array", varray());
	ADDFUNC1(POOL_STRING_ARRAY, NIL, PoolStringArray, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_STRING_ARRAY, INT, PoolStringArray, insert, INT, "idx", STRING, "string", varray());
	ADDFUNC1(POOL_STRING_ARRAY, NIL, PoolStringArray, resize, INT, "idx", varray());
	ADDFUNC0(POOL_STRING_ARRAY, NIL, PoolStringArray, invert, varray());
	ADDFUNC1(POOL_STRING_ARRAY, STRING, PoolStringArray, join, STRING, "delimiter", varray());
	ADDFUNC1R(POOL_STRING_ARRAY, BOOL, PoolStringArray, contains, STRING, "value", varray())
	ADDFUNC2R(POOL_STRING_ARRAY, INT, PoolStringArray, find, STRING, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_STRING_ARRAY, INT, PoolStringArray, rfind, STRING, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_STRING_ARRAY, INT, PoolStringArray, count, STRING, "value", varray());
	ADDFUNC1R(POOL_STRING_ARRAY, BOOL, PoolStringArray, has, STRING, "value", varray());
	ADDFUNC0(POOL_STRING_ARRAY, NIL, PoolStringArray, clear, varray());
	ADDFUNC0(POOL_STRING_ARRAY, NIL, PoolStringArray, sort, varray());

	ADDFUNC0R(POOL_VECTOR2_ARRAY, INT, PoolVector2Array, size, varray());
	ADDFUNC0R(POOL_VECTOR2_ARRAY, BOOL, PoolVector2Array, empty, varray());
	ADDFUNC2(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, set, INT, "idx", VECTOR2, "vector2", varray());
	ADDFUNC1(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, push_back, VECTOR2, "vector2", varray());
	ADDFUNC1(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, fill, VECTOR2, "vector2", varray());
	ADDFUNC1(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, append, VECTOR2, "vector2", varray());
	ADDFUNC1(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, append_array, POOL_VECTOR2_ARRAY, "array", varray());
	ADDFUNC1(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_VECTOR2_ARRAY, INT, PoolVector2Array, insert, INT, "idx", VECTOR2, "vector2", varray());
	ADDFUNC1(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, resize, INT, "idx", varray());
	ADDFUNC0(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, invert, varray());
	ADDFUNC1R(POOL_VECTOR2_ARRAY, BOOL, PoolVector2Array, contains, VECTOR2, "value", varray())
	ADDFUNC2R(POOL_VECTOR2_ARRAY, INT, PoolVector2Array, find, VECTOR2, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_VECTOR2_ARRAY, INT, PoolVector2Array, rfind, VECTOR2, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_VECTOR2_ARRAY, INT, PoolVector2Array, count, VECTOR2, "value", varray());
	ADDFUNC1R(POOL_VECTOR2_ARRAY, BOOL, PoolVector2Array, has, VECTOR2, "value", varray());
	ADDFUNC0(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, clear, varray());
	ADDFUNC0(POOL_VECTOR2_ARRAY, NIL, PoolVector2Array, sort, varray());

	ADDFUNC0R(POOL_VECTOR2I_ARRAY, INT, PoolVector2iArray, size, varray());
	ADDFUNC0R(POOL_VECTOR2I_ARRAY, BOOL, PoolVector2iArray, empty, varray());
	ADDFUNC2(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, set, INT, "idx", VECTOR2I, "vector2i", varray());
	ADDFUNC1(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, push_back, VECTOR2I, "vector2i", varray());
	ADDFUNC1(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, fill, VECTOR2I, "vector2i", varray());
	ADDFUNC1(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, append, VECTOR2I, "vector2i", varray());
	ADDFUNC1(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, append_array, POOL_VECTOR2I_ARRAY, "array", varray());
	ADDFUNC1(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_VECTOR2I_ARRAY, INT, PoolVector2iArray, insert, INT, "idx", VECTOR2I, "vector2i", varray());
	ADDFUNC1(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, resize, INT, "idx", varray());
	ADDFUNC0(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, invert, varray());
	ADDFUNC1R(POOL_VECTOR2I_ARRAY, BOOL, PoolVector2iArray, contains, VECTOR2I, "value", varray())
	ADDFUNC2R(POOL_VECTOR2I_ARRAY, INT, PoolVector2iArray, find, VECTOR2I, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_VECTOR2I_ARRAY, INT, PoolVector2iArray, rfind, VECTOR2I, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_VECTOR2I_ARRAY, INT, PoolVector2iArray, count, VECTOR2I, "value", varray());
	ADDFUNC1R(POOL_VECTOR2I_ARRAY, BOOL, PoolVector2iArray, has, VECTOR2I, "value", varray());
	ADDFUNC0(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, clear, varray());
	ADDFUNC0(POOL_VECTOR2I_ARRAY, NIL, PoolVector2iArray, sort, varray());

	ADDFUNC0R(POOL_VECTOR3_ARRAY, INT, PoolVector3Array, size, varray());
	ADDFUNC0R(POOL_VECTOR3_ARRAY, BOOL, PoolVector3Array, empty, varray());
	ADDFUNC2(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, set, INT, "idx", VECTOR3, "vector3", varray());
	ADDFUNC1(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, push_back, VECTOR3, "vector3", varray());
	ADDFUNC1(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, fill, VECTOR3, "vector3", varray());
	ADDFUNC1(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, append, VECTOR3, "vector3", varray());
	ADDFUNC1(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, append_array, POOL_VECTOR3_ARRAY, "array", varray());
	ADDFUNC1(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_VECTOR3_ARRAY, INT, PoolVector3Array, insert, INT, "idx", VECTOR3, "vector3", varray());
	ADDFUNC1(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, resize, INT, "idx", varray());
	ADDFUNC0(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, invert, varray());
	ADDFUNC1R(POOL_VECTOR3_ARRAY, BOOL, PoolVector3Array, contains, VECTOR3, "value", varray())
	ADDFUNC2R(POOL_VECTOR3_ARRAY, INT, PoolVector3Array, find, VECTOR3, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_VECTOR3_ARRAY, INT, PoolVector3Array, rfind, VECTOR3, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_VECTOR3_ARRAY, INT, PoolVector3Array, count, VECTOR3, "value", varray());
	ADDFUNC1R(POOL_VECTOR3_ARRAY, BOOL, PoolVector3Array, has, VECTOR3, "value", varray());
	ADDFUNC0(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, clear, varray());
	ADDFUNC0(POOL_VECTOR3_ARRAY, NIL, PoolVector3Array, sort, varray());

	ADDFUNC0R(POOL_VECTOR3I_ARRAY, INT, PoolVector3iArray, size, varray());
	ADDFUNC0R(POOL_VECTOR3I_ARRAY, BOOL, PoolVector3iArray, empty, varray());
	ADDFUNC2(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, set, INT, "idx", VECTOR3I, "vector3i", varray());
	ADDFUNC1(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, push_back, VECTOR3I, "vector3i", varray());
	ADDFUNC1(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, fill, VECTOR3I, "vector3i", varray());
	ADDFUNC1(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, append, VECTOR3I, "vector3i", varray());
	ADDFUNC1(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, append_array, POOL_VECTOR3I_ARRAY, "array", varray());
	ADDFUNC1(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_VECTOR3I_ARRAY, INT, PoolVector3iArray, insert, INT, "idx", VECTOR3I, "vector3i", varray());
	ADDFUNC1(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, resize, INT, "idx", varray());
	ADDFUNC0(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, invert, varray());
	ADDFUNC1R(POOL_VECTOR3I_ARRAY, BOOL, PoolVector3iArray, contains, VECTOR3I, "value", varray())
	ADDFUNC2R(POOL_VECTOR3I_ARRAY, INT, PoolVector3iArray, find, VECTOR3I, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_VECTOR3I_ARRAY, INT, PoolVector3iArray, rfind, VECTOR3I, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_VECTOR3I_ARRAY, INT, PoolVector3iArray, count, VECTOR3I, "value", varray());
	ADDFUNC1R(POOL_VECTOR3I_ARRAY, BOOL, PoolVector3iArray, has, VECTOR3I, "value", varray());
	ADDFUNC0(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, clear, varray());
	ADDFUNC0(POOL_VECTOR3I_ARRAY, NIL, PoolVector3iArray, sort, varray());

	ADDFUNC0R(POOL_COLOR_ARRAY, INT, PoolColorArray, size, varray());
	ADDFUNC0R(POOL_COLOR_ARRAY, BOOL, PoolColorArray, empty, varray());
	ADDFUNC2(POOL_COLOR_ARRAY, NIL, PoolColorArray, set, INT, "idx", COLOR, "color", varray());
	ADDFUNC1(POOL_COLOR_ARRAY, NIL, PoolColorArray, push_back, COLOR, "color", varray());
	ADDFUNC1(POOL_COLOR_ARRAY, NIL, PoolColorArray, fill, COLOR, "color", varray());
	ADDFUNC1(POOL_COLOR_ARRAY, NIL, PoolColorArray, append, COLOR, "color", varray());
	ADDFUNC1(POOL_COLOR_ARRAY, NIL, PoolColorArray, append_array, POOL_COLOR_ARRAY, "array", varray());
	ADDFUNC1(POOL_COLOR_ARRAY, NIL, PoolColorArray, remove, INT, "idx", varray());
	ADDFUNC2R(POOL_COLOR_ARRAY, INT, PoolColorArray, insert, INT, "idx", COLOR, "color", varray());
	ADDFUNC1(POOL_COLOR_ARRAY, NIL, PoolColorArray, resize, INT, "idx", varray());
	ADDFUNC0(POOL_COLOR_ARRAY, NIL, PoolColorArray, invert, varray());
	ADDFUNC1R(POOL_COLOR_ARRAY, BOOL, PoolColorArray, contains, COLOR, "value", varray())
	ADDFUNC2R(POOL_COLOR_ARRAY, INT, PoolColorArray, find, COLOR, "value", INT, "from", varray(0))
	ADDFUNC2R(POOL_COLOR_ARRAY, INT, PoolColorArray, rfind, COLOR, "value", INT, "from", varray(-1));
	ADDFUNC1R(POOL_COLOR_ARRAY, INT, PoolColorArray, count, COLOR, "value", varray());
	ADDFUNC1R(POOL_COLOR_ARRAY, BOOL, PoolColorArray, has, COLOR, "value", varray());
	ADDFUNC0(POOL_COLOR_ARRAY, NIL, PoolColorArray, clear, varray());
	ADDFUNC0(POOL_COLOR_ARRAY, NIL, PoolColorArray, sort, varray());

	//pointerbased
	ADDFUNC0R(AABB, REAL, AABB, get_volume, varray());
	ADDFUNC0R(AABB, BOOL, AABB, has_no_volume, varray());
	ADDFUNC0R(AABB, BOOL, AABB, has_no_surface, varray());
	ADDFUNC1R(AABB, BOOL, AABB, is_equal_approx, AABB, "aabb", varray());
	ADDFUNC1R(AABB, BOOL, AABB, intersects, AABB, "with", varray());
	ADDFUNC1R(AABB, BOOL, AABB, intersects_inclusive, AABB, "aabb", varray());
	ADDFUNC1R(AABB, BOOL, AABB, encloses, AABB, "with", varray());
	ADDFUNC1R(AABB, AABB, AABB, merge, AABB, "with", varray());
	ADDFUNC1(AABB, NIL, AABB, merge_with, AABB, "with", varray());
	ADDFUNC1R(AABB, AABB, AABB, intersection, AABB, "with", varray());
	ADDFUNC2R(AABB, BOOL, AABB, intersects_segment, VECTOR3, "from", VECTOR3, "to", varray());
	ADDFUNC2R(AABB, BOOL, AABB, intersects_ray, VECTOR3, "from", VECTOR3, "dir", varray());
	ADDFUNC4R(AABB, BOOL, AABB, smits_intersect_ray, VECTOR3, "from", VECTOR3, "dir", REAL, "t0", REAL, "t1", varray());
	ADDFUNC1R(AABB, BOOL, AABB, intersects_plane, PLANE, "plane", varray());
	ADDFUNC1R(AABB, BOOL, AABB, has_point, VECTOR3, "point", varray());
	ADDFUNC1R(AABB, VECTOR3, AABB, get_support, VECTOR3, "dir", varray());
	ADDFUNC0R(AABB, VECTOR3, AABB, get_longest_axis, varray());
	ADDFUNC0R(AABB, INT, AABB, get_longest_axis_index, varray());
	ADDFUNC0R(AABB, REAL, AABB, get_longest_axis_size, varray());
	ADDFUNC0R(AABB, VECTOR3, AABB, get_shortest_axis, varray());
	ADDFUNC0R(AABB, INT, AABB, get_shortest_axis_index, varray());
	ADDFUNC0R(AABB, REAL, AABB, get_shortest_axis_size, varray());
	ADDFUNC1R(AABB, AABB, AABB, grow, REAL, "by", varray());
	ADDFUNC1(AABB, NIL, AABB, grow_by, REAL, "amount", varray());
	ADDFUNC1R(AABB, VECTOR3, AABB, get_endpoint, INT, "idx", varray());
	ADDFUNC1R(AABB, AABB, AABB, expand, VECTOR3, "to_point", varray());
	ADDFUNC1(AABB, NIL, AABB, expand_to, VECTOR3, "vector", varray());
	ADDFUNC1R(AABB, BOOL, AABB, create_from_points, POOL_VECTOR3_ARRAY, "points", varray());
	ADDFUNC0R(AABB, AABB, AABB, abs, varray());
	ADDFUNC2R(AABB, VECTOR3, AABB, intersects_segmentv, VECTOR3, "from", VECTOR3, "to", varray());
	ADDFUNC2R(AABB, VECTOR3, AABB, intersects_rayv, VECTOR3, "from", VECTOR3, "dir", varray());
	ADDFUNC1(AABB, NIL, AABB, quantize, REAL, "unit", varray());
	ADDFUNC1R(AABB, AABB, AABB, quantized, REAL, "unit", varray());
	//Property
	//ADDFUNC1(AABB, NIL, AABB, set_end, VECTOR3, "vector", varray());
	//ADDFUNC0R(AABB, VECTOR3, AABB, get_end, varray());
	ADDFUNC0R(AABB, VECTOR3, AABB, get_center, varray());

	ADDFUNC0R(TRANSFORM2D, TRANSFORM2D, Transform2D, inverse, varray());
	ADDFUNC0R(TRANSFORM2D, TRANSFORM2D, Transform2D, affine_inverse, varray());
	ADDFUNC0R(TRANSFORM2D, REAL, Transform2D, get_rotation, varray());
	ADDFUNC0R(TRANSFORM2D, VECTOR2, Transform2D, get_origin, varray());
	ADDFUNC0R(TRANSFORM2D, VECTOR2, Transform2D, get_scale, varray());
	ADDFUNC0R(TRANSFORM2D, TRANSFORM2D, Transform2D, orthonormalized, varray());
	ADDFUNC1R(TRANSFORM2D, TRANSFORM2D, Transform2D, rotated, REAL, "phi", varray());
	ADDFUNC1R(TRANSFORM2D, TRANSFORM2D, Transform2D, scaled, VECTOR2, "scale", varray());
	ADDFUNC1R(TRANSFORM2D, TRANSFORM2D, Transform2D, translated_local, VECTOR2, "offset", varray());
	ADDFUNC1R(TRANSFORM2D, NIL, Transform2D, xform, NIL, "v", varray());
	ADDFUNC1R(TRANSFORM2D, NIL, Transform2D, xform_inv, NIL, "v", varray());
	ADDFUNC1R(TRANSFORM2D, VECTOR2, Transform2D, basis_xform, VECTOR2, "v", varray());
	ADDFUNC1R(TRANSFORM2D, VECTOR2, Transform2D, basis_xform_inv, VECTOR2, "v", varray());
	ADDFUNC2R(TRANSFORM2D, TRANSFORM2D, Transform2D, interpolate_with, TRANSFORM2D, "transform", REAL, "weight", varray());
	ADDFUNC1R(TRANSFORM2D, BOOL, Transform2D, is_equal_approx, TRANSFORM2D, "transform", varray());

	ADDFUNC0(BASIS, NIL, Basis, invert, varray());
	ADDFUNC0R(BASIS, BASIS, Basis, inverse, varray());
	ADDFUNC0(BASIS, NIL, Basis, transpose, varray());
	ADDFUNC0R(BASIS, BASIS, Basis, transposed, varray());
	ADDFUNC0R(BASIS, REAL, Basis, determinant, varray());
	ADDFUNC1(BASIS, NIL, Basis, from_z, VECTOR3, "z", varray());
	ADDFUNC2(BASIS, NIL, Basis, rotate, VECTOR3, "axis", REAL, "phi", varray());
	ADDFUNC2R(BASIS, BASIS, Basis, rotated, VECTOR3, "axis", REAL, "phi", varray());
	ADDFUNC2(BASIS, NIL, Basis, rotate_local, VECTOR3, "axis", REAL, "phi", varray());
	ADDFUNC2R(BASIS, BASIS, Basis, rotate_local, VECTOR3, "axis", REAL, "phi", varray());
	ADDFUNC1(BASIS, NIL, Basis, rotatev, VECTOR3, "euler", varray());
	ADDFUNC1R(BASIS, BASIS, Basis, rotatedv, VECTOR3, "euler", varray());
	ADDFUNC1(BASIS, NIL, Basis, rotateq, QUATERNION, "quat", varray());
	ADDFUNC1R(BASIS, BASIS, Basis, rotatedq, QUATERNION, "quat", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_rotation_euler, varray());
	ADDFUNC0R(BASIS, QUATERNION, Basis, get_rotation_quaternion, varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_rotation, varray());
	ADDFUNC2(BASIS, NIL, Basis, rotate_to_align, VECTOR3, "direction", VECTOR3, "end_direction", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_euler_xyz, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_euler_xyz, VECTOR3, "euler", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_euler_xzy, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_euler_xzy, VECTOR3, "euler", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_euler_xyz, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_euler_xyz, VECTOR3, "euler", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_euler_yxz, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_euler_yxz, VECTOR3, "euler", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_euler_zxy, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_euler_zxy, VECTOR3, "euler", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_euler_zyx, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_euler_zyx, VECTOR3, "euler", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_euler, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_euler, VECTOR3, "euler", varray());
	ADDFUNC0R(BASIS, QUATERNION, Basis, get_quaternion, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_quaternion, QUATERNION, "quaternion", varray());
	ADDFUNC1(BASIS, NIL, Basis, scale, VECTOR3, "scale", varray());
	ADDFUNC1R(BASIS, BASIS, Basis, scaled, VECTOR3, "scale", varray());
	ADDFUNC1(BASIS, NIL, Basis, scale_local, VECTOR3, "scale", varray());
	ADDFUNC1R(BASIS, BASIS, Basis, scaled_local, VECTOR3, "scale", varray());
	ADDFUNC1(BASIS, NIL, Basis, scale_orthogonal, VECTOR3, "scale", varray());
	ADDFUNC1R(BASIS, BASIS, Basis, scaled_orthogonal, VECTOR3, "scale", varray());
	ADDFUNC0(BASIS, NIL, Basis, make_scale_uniform, varray());
	ADDFUNC0R(BASIS, REAL, Basis, get_uniform_scale, varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_scale, varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_scale_abs, varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_scale_local, varray());
	ADDFUNC3(BASIS, NIL, Basis, set_axis_angle_scale, VECTOR3, "axis", REAL, "phi", VECTOR3, "scale", varray());
	ADDFUNC2(BASIS, NIL, Basis, set_euler_scale, VECTOR3, "euler", VECTOR3, "scale", varray());
	ADDFUNC2(BASIS, NIL, Basis, set_quaternion_scale, QUATERNION, "quat", VECTOR3, "scale", varray());
	ADDFUNC1R(BASIS, REAL, Basis, tdotx, VECTOR3, "with", varray());
	ADDFUNC1R(BASIS, REAL, Basis, tdoty, VECTOR3, "with", varray());
	ADDFUNC1R(BASIS, REAL, Basis, tdotz, VECTOR3, "with", varray());
	ADDFUNC1R(BASIS, BOOL, Basis, is_equal_approx, BASIS, "b", varray());
	ADDFUNC2R(BASIS, BOOL, Basis, is_equal_approx_ratio, BASIS, "b", REAL, "epsilon", varray(CMP_EPSILON));
	ADDFUNC0R(BASIS, INT, Basis, get_orthogonal_index, varray());
	ADDFUNC1(BASIS, NIL, Basis, set_orthogonal_index, INT, "index", varray());
	ADDFUNC1(BASIS, NIL, Basis, set_diagonal, VECTOR3, "diag", varray());
	ADDFUNC0R(BASIS, BOOL, Basis, is_orthogonal, varray());
	ADDFUNC0R(BASIS, BOOL, Basis, is_diagonal, varray());
	ADDFUNC0R(BASIS, BOOL, Basis, is_rotation, varray());
	ADDFUNC2R(BASIS, BASIS, Basis, slerp, BASIS, "to", REAL, "weight", varray());
	ADDFUNC2R(BASIS, BASIS, Basis, lerp, BASIS, "to", REAL, "weight", varray());
	ADDFUNC1(BASIS, VECTOR3, Basis, get_column, INT, "i", varray());
	ADDFUNC2(BASIS, NIL, Basis, set_column, INT, "index", VECTOR3, "value", varray());
	ADDFUNC3(BASIS, NIL, Basis, set_columns, VECTOR3, "x", VECTOR3, "y", VECTOR3, "z", varray());
	ADDFUNC1(BASIS, VECTOR3, Basis, get_row, INT, "i", varray());
	ADDFUNC2(BASIS, NIL, Basis, set_row, INT, "i", VECTOR3, "axis", varray());
	ADDFUNC1(BASIS, VECTOR3, Basis, get_axis, INT, "i", varray());
	ADDFUNC2(BASIS, NIL, Basis, set_axis, INT, "i", VECTOR3, "axis", varray());
	ADDFUNC0R(BASIS, VECTOR3, Basis, get_main_diagonal, varray());
	ADDFUNC0(BASIS, NIL, Basis, set_zero, varray());
	ADDFUNC1R(BASIS, BASIS, Basis, transpose_xform, BASIS, "m", varray());
	ADDFUNC0(BASIS, NIL, Basis, orthonormalize, varray());
	ADDFUNC0R(BASIS, BASIS, Basis, orthonormalized, varray());
	ADDFUNC0(BASIS, NIL, Basis, orthogonalize, varray());
	ADDFUNC0R(BASIS, BASIS, Basis, orthogonalized, varray());
	ADDFUNC0R(BASIS, BOOL, Basis, is_symmetric, varray());
	ADDFUNC0R(BASIS, BASIS, Basis, diagonalize, varray());
	ADDFUNC1R(BASIS, VECTOR3, Basis, xform, NIL, "v3_or_v3i", varray());
	ADDFUNC1R(BASIS, VECTOR3, Basis, xform_inv, NIL, "v3_or_v3i", varray());

	ADDFUNC0R(TRANSFORM, TRANSFORM, Transform, inverse, varray());
	ADDFUNC0R(TRANSFORM, TRANSFORM, Transform, affine_inverse, varray());
	ADDFUNC0R(TRANSFORM, TRANSFORM, Transform, orthonormalized, varray());
	ADDFUNC2R(TRANSFORM, TRANSFORM, Transform, rotated, VECTOR3, "axis", REAL, "phi", varray());
	ADDFUNC1R(TRANSFORM, TRANSFORM, Transform, scaled, VECTOR3, "scale", varray());
	ADDFUNC1R(TRANSFORM, TRANSFORM, Transform, translated, VECTOR3, "offset", varray());
	ADDFUNC2R(TRANSFORM, TRANSFORM, Transform, looking_at, VECTOR3, "target", VECTOR3, "up", varray());
	ADDFUNC2R(TRANSFORM, TRANSFORM, Transform, interpolate_with, TRANSFORM, "transform", REAL, "weight", varray());
	ADDFUNC1R(TRANSFORM, BOOL, Transform, is_equal_approx, TRANSFORM, "transform", varray());
	ADDFUNC1R(TRANSFORM, NIL, Transform, xform, NIL, "v", varray());
	ADDFUNC1R(TRANSFORM, NIL, Transform, xform_inv, NIL, "v", varray());

	/* REGISTER CONSTRUCTORS */

	_VariantCall::add_constructor(_VariantCall::Vector2_init1, Variant::VECTOR2, "x", Variant::REAL, "y", Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Vector2i_init1, Variant::VECTOR2I, "x", Variant::INT, "y", Variant::INT);

	_VariantCall::add_constructor(_VariantCall::Rect2_init1, Variant::RECT2, "position", Variant::VECTOR2, "size", Variant::VECTOR2);
	_VariantCall::add_constructor(_VariantCall::Rect2_init2, Variant::RECT2, "x", Variant::REAL, "y", Variant::REAL, "width", Variant::REAL, "height", Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Transform2D_init2, Variant::TRANSFORM2D, "rotation", Variant::REAL, "position", Variant::VECTOR2);
	_VariantCall::add_constructor(_VariantCall::Transform2D_init3, Variant::TRANSFORM2D, "x_axis", Variant::VECTOR2, "y_axis", Variant::VECTOR2, "origin", Variant::VECTOR2);

	_VariantCall::add_constructor(_VariantCall::Vector3_init1, Variant::VECTOR3, "x", Variant::REAL, "y", Variant::REAL, "z", Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Vector3i_init1, Variant::VECTOR3I, "x", Variant::INT, "y", Variant::INT, "z", Variant::INT);

	_VariantCall::add_constructor(_VariantCall::Plane_init1, Variant::PLANE, "a", Variant::REAL, "b", Variant::REAL, "c", Variant::REAL, "d", Variant::REAL);
	_VariantCall::add_constructor(_VariantCall::Plane_init2, Variant::PLANE, "v1", Variant::VECTOR3, "v2", Variant::VECTOR3, "v3", Variant::VECTOR3);
	_VariantCall::add_constructor(_VariantCall::Plane_init3, Variant::PLANE, "normal", Variant::VECTOR3, "d", Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Quaternion_init1, Variant::QUATERNION, "x", Variant::REAL, "y", Variant::REAL, "z", Variant::REAL, "w", Variant::REAL);
	_VariantCall::add_constructor(_VariantCall::Quaternion_init2, Variant::QUATERNION, "axis", Variant::VECTOR3, "angle", Variant::REAL);
	_VariantCall::add_constructor(_VariantCall::Quaternion_init3, Variant::QUATERNION, "euler", Variant::VECTOR3);

	_VariantCall::add_constructor(_VariantCall::Color_init1, Variant::COLOR, "r", Variant::REAL, "g", Variant::REAL, "b", Variant::REAL, "a", Variant::REAL);
	_VariantCall::add_constructor(_VariantCall::Color_init2, Variant::COLOR, "r", Variant::REAL, "g", Variant::REAL, "b", Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::AABB_init1, Variant::AABB, "position", Variant::VECTOR3, "size", Variant::VECTOR3);

	_VariantCall::add_constructor(_VariantCall::Basis_init1, Variant::BASIS, "x_axis", Variant::VECTOR3, "y_axis", Variant::VECTOR3, "z_axis", Variant::VECTOR3);
	_VariantCall::add_constructor(_VariantCall::Basis_init2, Variant::BASIS, "axis", Variant::VECTOR3, "phi", Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Transform_init1, Variant::TRANSFORM, "x_axis", Variant::VECTOR3, "y_axis", Variant::VECTOR3, "z_axis", Variant::VECTOR3, "origin", Variant::VECTOR3);
	_VariantCall::add_constructor(_VariantCall::Transform_init2, Variant::TRANSFORM, "basis", Variant::BASIS, "origin", Variant::VECTOR3);

	/* REGISTER CONSTANTS */

	_populate_named_colors();
	for (Map<String, Color>::Element *color = _named_colors.front(); color; color = color->next()) {
		_VariantCall::add_variant_constant(Variant::COLOR, color->key(), color->value());
	}

	_VariantCall::add_constant(Variant::VECTOR3, "AXIS_X", Vector3::AXIS_X);
	_VariantCall::add_constant(Variant::VECTOR3, "AXIS_Y", Vector3::AXIS_Y);
	_VariantCall::add_constant(Variant::VECTOR3, "AXIS_Z", Vector3::AXIS_Z);

	_VariantCall::add_variant_constant(Variant::VECTOR3, "ZERO", Vector3(0, 0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "ONE", Vector3(1, 1, 1));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "INF", Vector3(Math_INF, Math_INF, Math_INF));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "LEFT", Vector3(-1, 0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "RIGHT", Vector3(1, 0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "UP", Vector3(0, 1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "DOWN", Vector3(0, -1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "FORWARD", Vector3(0, 0, -1));
	_VariantCall::add_variant_constant(Variant::VECTOR3, "BACK", Vector3(0, 0, 1));

	_VariantCall::add_constant(Variant::VECTOR3I, "AXIS_X", Vector3i::AXIS_X);
	_VariantCall::add_constant(Variant::VECTOR3I, "AXIS_Y", Vector3i::AXIS_Y);
	_VariantCall::add_constant(Variant::VECTOR3I, "AXIS_Z", Vector3i::AXIS_Z);

	_VariantCall::add_variant_constant(Variant::VECTOR3I, "ZERO", Vector3i(0, 0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3I, "ONE", Vector3i(1, 1, 1));
	_VariantCall::add_variant_constant(Variant::VECTOR3I, "LEFT", Vector3i(-1, 0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3I, "RIGHT", Vector3i(1, 0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3I, "UP", Vector3i(0, 1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3I, "DOWN", Vector3i(0, -1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR3I, "FORWARD", Vector3i(0, 0, -1));
	_VariantCall::add_variant_constant(Variant::VECTOR3I, "BACK", Vector3i(0, 0, 1));

	_VariantCall::add_constant(Variant::VECTOR2, "AXIS_X", Vector2::AXIS_X);
	_VariantCall::add_constant(Variant::VECTOR2, "AXIS_Y", Vector2::AXIS_Y);

	_VariantCall::add_variant_constant(Variant::VECTOR2, "ZERO", Vector2(0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR2, "ONE", Vector2(1, 1));
	_VariantCall::add_variant_constant(Variant::VECTOR2, "INF", Vector2(Math_INF, Math_INF));
	_VariantCall::add_variant_constant(Variant::VECTOR2, "LEFT", Vector2(-1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR2, "RIGHT", Vector2(1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR2, "UP", Vector2(0, -1));
	_VariantCall::add_variant_constant(Variant::VECTOR2, "DOWN", Vector2(0, 1));

	_VariantCall::add_constant(Variant::VECTOR2I, "AXIS_X", Vector2i::AXIS_X);
	_VariantCall::add_constant(Variant::VECTOR2I, "AXIS_Y", Vector2i::AXIS_Y);

	_VariantCall::add_variant_constant(Variant::VECTOR2I, "ZERO", Vector2i(0, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR2I, "ONE", Vector2i(1, 1));
	_VariantCall::add_variant_constant(Variant::VECTOR2I, "LEFT", Vector2i(-1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR2I, "RIGHT", Vector2i(1, 0));
	_VariantCall::add_variant_constant(Variant::VECTOR2I, "UP", Vector2i(0, -1));
	_VariantCall::add_variant_constant(Variant::VECTOR2I, "DOWN", Vector2i(0, 1));

	_VariantCall::add_variant_constant(Variant::TRANSFORM2D, "IDENTITY", Transform2D());
	_VariantCall::add_variant_constant(Variant::TRANSFORM2D, "FLIP_X", Transform2D(-1, 0, 0, 1, 0, 0));
	_VariantCall::add_variant_constant(Variant::TRANSFORM2D, "FLIP_Y", Transform2D(1, 0, 0, -1, 0, 0));

	Transform identity_transform = Transform();
	Transform flip_x_transform = Transform(-1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0);
	Transform flip_y_transform = Transform(1, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0);
	Transform flip_z_transform = Transform(1, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0);
	_VariantCall::add_variant_constant(Variant::TRANSFORM, "IDENTITY", identity_transform);
	_VariantCall::add_variant_constant(Variant::TRANSFORM, "FLIP_X", flip_x_transform);
	_VariantCall::add_variant_constant(Variant::TRANSFORM, "FLIP_Y", flip_y_transform);
	_VariantCall::add_variant_constant(Variant::TRANSFORM, "FLIP_Z", flip_z_transform);

	Basis identity_basis = Basis();
	Basis flip_x_basis = Basis(-1, 0, 0, 0, 1, 0, 0, 0, 1);
	Basis flip_y_basis = Basis(1, 0, 0, 0, -1, 0, 0, 0, 1);
	Basis flip_z_basis = Basis(1, 0, 0, 0, 1, 0, 0, 0, -1);
	_VariantCall::add_variant_constant(Variant::BASIS, "IDENTITY", identity_basis);
	_VariantCall::add_variant_constant(Variant::BASIS, "FLIP_X", flip_x_basis);
	_VariantCall::add_variant_constant(Variant::BASIS, "FLIP_Y", flip_y_basis);
	_VariantCall::add_variant_constant(Variant::BASIS, "FLIP_Z", flip_z_basis);

	_VariantCall::add_variant_constant(Variant::PLANE, "PLANE_YZ", Plane(Vector3(1, 0, 0), 0));
	_VariantCall::add_variant_constant(Variant::PLANE, "PLANE_XZ", Plane(Vector3(0, 1, 0), 0));
	_VariantCall::add_variant_constant(Variant::PLANE, "PLANE_XY", Plane(Vector3(0, 0, 1), 0));

	_VariantCall::add_variant_constant(Variant::QUATERNION, "IDENTITY", Quaternion(0, 0, 0, 1));
}

void unregister_variant_methods() {
	memdelete_arr(_VariantCall::type_funcs);
	memdelete_arr(_VariantCall::construct_funcs);
	memdelete_arr(_VariantCall::constant_data);
}
