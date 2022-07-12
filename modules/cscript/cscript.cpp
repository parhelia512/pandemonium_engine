/*************************************************************************/
/*  gdscript.cpp                                                         */
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

#include "cscript.h"

#include "core/core_string_names.h"
#include "core/engine.h"
#include "core/global_constants.h"
#include "core/io/file_access_encrypted.h"
#include "core/os/file_access.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "cscript_compiler.h"

///////////////////////////

CScriptNativeClass::CScriptNativeClass(const StringName &p_name) {
	name = p_name;
}

bool CScriptNativeClass::_get(const StringName &p_name, Variant &r_ret) const {
	bool ok;
	int v = ClassDB::get_integer_constant(name, p_name, &ok);

	if (ok) {
		r_ret = v;
		return true;
	} else {
		return false;
	}
}

void CScriptNativeClass::_bind_methods() {
	ClassDB::bind_method(D_METHOD("new"), &CScriptNativeClass::_new);
}

Variant CScriptNativeClass::_new() {
	Object *o = instance();
	ERR_FAIL_COND_V_MSG(!o, Variant(), "Class type: '" + String(name) + "' is not instantiable.");

	Reference *ref = Object::cast_to<Reference>(o);
	if (ref) {
		return REF(ref);
	} else {
		return o;
	}
}

Object *CScriptNativeClass::instance() {
	return ClassDB::instance(name);
}

void CScript::_clear_pending_func_states() {
	CScriptLanguage::get_singleton()->lock.lock();
	while (SelfList<CScriptFunctionState> *E = pending_func_states.first()) {
		// Order matters since clearing the stack may already cause
		// the CSCriptFunctionState to be destroyed and thus removed from the list.
		pending_func_states.remove(E);
		E->self()->_clear_stack();
	}
	CScriptLanguage::get_singleton()->lock.unlock();
}

CScriptInstance *CScript::_create_instance(const Variant **p_args, int p_argcount, Object *p_owner, bool p_isref, Variant::CallError &r_error) {
	/* STEP 1, CREATE */

	CScriptInstance *instance = memnew(CScriptInstance);
	instance->base_ref = p_isref;
	instance->members.resize(member_indices.size());
	instance->script = Ref<CScript>(this);
	instance->owner = p_owner;
#ifdef DEBUG_ENABLED
	//needed for hot reloading
	for (Map<StringName, MemberInfo>::Element *E = member_indices.front(); E; E = E->next()) {
		instance->member_indices_cache[E->key()] = E->get().index;
	}
#endif
	instance->owner->set_script_instance(instance);

	/* STEP 2, INITIALIZE AND CONSTRUCT */

	CScriptLanguage::singleton->lock.lock();
	instances.insert(instance->owner);
	CScriptLanguage::singleton->lock.unlock();

	initializer->call(instance, p_args, p_argcount, r_error);

	if (r_error.error != Variant::CallError::CALL_OK) {
		instance->script = Ref<CScript>();
		instance->owner->set_script_instance(nullptr);
#ifndef NO_THREADS
		CScriptLanguage::singleton->lock.lock();
#endif
		instances.erase(p_owner);
#ifndef NO_THREADS
		CScriptLanguage::singleton->lock.unlock();
#endif

		ERR_FAIL_COND_V(r_error.error != Variant::CallError::CALL_OK, nullptr); //error constructing
	}

	//@TODO make thread safe
	return instance;
}

Variant CScript::_new(const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	/* STEP 1, CREATE */

	if (!valid) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
		return Variant();
	}

	r_error.error = Variant::CallError::CALL_OK;
	REF ref;
	Object *owner = nullptr;

	CScript *_baseptr = this;
	while (_baseptr->_base) {
		_baseptr = _baseptr->_base;
	}

	ERR_FAIL_COND_V(_baseptr->native.is_null(), Variant());
	if (_baseptr->native.ptr()) {
		owner = _baseptr->native->instance();
	} else {
		owner = memnew(Reference); //by default, no base means use reference
	}
	ERR_FAIL_COND_V_MSG(!owner, Variant(), "Can't inherit from a virtual class.");

	Reference *r = Object::cast_to<Reference>(owner);
	if (r) {
		ref = REF(r);
	}

	CScriptInstance *instance = _create_instance(p_args, p_argcount, owner, r != nullptr, r_error);
	if (!instance) {
		if (ref.is_null()) {
			memdelete(owner); //no owner, sorry
		}
		return Variant();
	}

	if (ref.is_valid()) {
		return ref;
	} else {
		return owner;
	}
}

bool CScript::can_instance() const {
#ifdef TOOLS_ENABLED
	return valid && (tool || ScriptServer::is_scripting_enabled());
#else
	return valid;
#endif
}

Ref<Script> CScript::get_base_script() const {
	if (_base) {
		return Ref<CScript>(_base);
	} else {
		return Ref<Script>();
	}
}

StringName CScript::get_instance_base_type() const {
	if (native.is_valid()) {
		return native->get_name();
	}
	if (base.is_valid() && base->is_valid()) {
		return base->get_instance_base_type();
	}
	return StringName();
}

struct _CScriptMemberSort {
	int index;
	StringName name;
	_FORCE_INLINE_ bool operator<(const _CScriptMemberSort &p_member) const { return index < p_member.index; }
};

#ifdef TOOLS_ENABLED

void CScript::_placeholder_erased(PlaceHolderScriptInstance *p_placeholder) {
	placeholders.erase(p_placeholder);
}
#endif

void CScript::get_script_method_list(List<MethodInfo> *p_list) const {
	const CScript *current = this;
	while (current) {
		for (const Map<StringName, CScriptFunction *>::Element *E = current->member_functions.front(); E; E = E->next()) {
			CScriptFunction *func = E->get();
			MethodInfo mi;
			mi.name = E->key();
			for (int i = 0; i < func->get_argument_count(); i++) {
				mi.arguments.push_back(func->get_argument_type(i));
			}

			mi.return_val = func->get_return_type();
			p_list->push_back(mi);
		}

		current = current->_base;
	}
}

void CScript::get_script_property_list(List<PropertyInfo> *p_list) const {
	const CScript *sptr = this;
	List<PropertyInfo> props;

	while (sptr) {
		Vector<_CScriptMemberSort> msort;
		for (Map<StringName, PropertyInfo>::Element *E = sptr->member_info.front(); E; E = E->next()) {
			_CScriptMemberSort ms;
			ERR_CONTINUE(!sptr->member_indices.has(E->key()));
			ms.index = sptr->member_indices[E->key()].index;
			ms.name = E->key();
			msort.push_back(ms);
		}

		msort.sort();
		msort.invert();
		for (int i = 0; i < msort.size(); i++) {
			props.push_front(sptr->member_info[msort[i].name]);
		}

		sptr = sptr->_base;
	}

	for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {
		p_list->push_back(E->get());
	}
}

bool CScript::has_method(const StringName &p_method) const {
	return member_functions.has(p_method);
}

MethodInfo CScript::get_method_info(const StringName &p_method) const {
	const Map<StringName, CScriptFunction *>::Element *E = member_functions.find(p_method);
	if (!E) {
		return MethodInfo();
	}

	CScriptFunction *func = E->get();
	MethodInfo mi;
	mi.name = E->key();
	for (int i = 0; i < func->get_argument_count(); i++) {
		mi.arguments.push_back(func->get_argument_type(i));
	}

	mi.return_val = func->get_return_type();
	return mi;
}

bool CScript::get_property_default_value(const StringName &p_property, Variant &r_value) const {
#ifdef TOOLS_ENABLED

	const Map<StringName, Variant>::Element *E = member_default_values_cache.find(p_property);
	if (E) {
		r_value = E->get();
		return true;
	}

	if (base_cache.is_valid()) {
		return base_cache->get_property_default_value(p_property, r_value);
	}
#endif
	return false;
}

ScriptInstance *CScript::instance_create(Object *p_this) {
	CScript *top = this;
	while (top->_base) {
		top = top->_base;
	}

	if (top->native.is_valid()) {
		if (!ClassDB::is_parent_class(p_this->get_class_name(), top->native->get_name())) {
			if (ScriptDebugger::get_singleton()) {
				CScriptLanguage::get_singleton()->debug_break_parse(_get_debug_path(), 1, "Script inherits from native type '" + String(top->native->get_name()) + "', so it can't be instanced in object of type: '" + p_this->get_class() + "'");
			}
			ERR_FAIL_V_MSG(nullptr, "Script inherits from native type '" + String(top->native->get_name()) + "', so it can't be instanced in object of type '" + p_this->get_class() + "'" + ".");
		}
	}

	Variant::CallError unchecked_error;
	return _create_instance(nullptr, 0, p_this, Object::cast_to<Reference>(p_this) != nullptr, unchecked_error);
}

PlaceHolderScriptInstance *CScript::placeholder_instance_create(Object *p_this) {
#ifdef TOOLS_ENABLED
	PlaceHolderScriptInstance *si = memnew(PlaceHolderScriptInstance(CScriptLanguage::get_singleton(), Ref<Script>(this), p_this));
	placeholders.insert(si);
	_update_exports(nullptr, false, si);
	return si;
#else
	return NULL;
#endif
}

bool CScript::instance_has(const Object *p_this) const {
	CScriptLanguage::singleton->lock.lock();
	bool hasit = instances.has((Object *)p_this);
	CScriptLanguage::singleton->lock.unlock();

	return hasit;
}

bool CScript::has_source_code() const {
	return source != "";
}
String CScript::get_source_code() const {
	return source;
}
void CScript::set_source_code(const String &p_code) {
	if (source == p_code) {
		return;
	}
	source = p_code;
#ifdef TOOLS_ENABLED
	source_changed_cache = true;
#endif
}

#ifdef TOOLS_ENABLED
void CScript::_update_exports_values(Map<StringName, Variant> &values, List<PropertyInfo> &propnames) {
	if (base_cache.is_valid()) {
		base_cache->_update_exports_values(values, propnames);
	}

	for (Map<StringName, Variant>::Element *E = member_default_values_cache.front(); E; E = E->next()) {
		values[E->key()] = E->get();
	}

	for (List<PropertyInfo>::Element *E = members_cache.front(); E; E = E->next()) {
		propnames.push_back(E->get());
	}
}
#endif

bool CScript::_update_exports(bool *r_err, bool p_recursive_call, PlaceHolderScriptInstance *p_instance_to_update) {
#ifdef TOOLS_ENABLED

	static Vector<CScript *> base_caches;
	if (!p_recursive_call) {
		base_caches.clear();
	}
	base_caches.push_back(this);

	bool changed = false;

	if (source_changed_cache) {
		source_changed_cache = false;
		changed = true;

		String basedir = path;

		if (basedir == "") {
			basedir = get_path();
		}

		if (basedir != "") {
			basedir = basedir.get_base_dir();
		}

		CScriptParser parser;
		Error err = parser.parse(source, basedir, true, path);

		if (err == OK) {
			const CScriptParser::Node *root = parser.get_parse_tree();
			ERR_FAIL_COND_V(root->type != CScriptParser::Node::TYPE_CLASS, false);

			const CScriptParser::ClassNode *c = static_cast<const CScriptParser::ClassNode *>(root);

			if (base_cache.is_valid()) {
				base_cache->inheriters_cache.erase(get_instance_id());
				base_cache = Ref<CScript>();
			}

			if (c->extends_used) {
				String path = "";
				if (String(c->extends_file) != "" && String(c->extends_file) != get_path()) {
					path = c->extends_file;
					if (path.is_rel_path()) {
						String base = get_path();
						if (base == "" || base.is_rel_path()) {
							ERR_PRINT(("Could not resolve relative path for parent class: " + path).utf8().get_data());
						} else {
							path = base.get_base_dir().plus_file(path);
						}
					}
				} else if (c->extends_class.size() != 0) {
					String base = c->extends_class[0];

					if (ScriptServer::is_global_class(base)) {
						path = ScriptServer::get_global_class_path(base);
					}
				}

				if (path != "") {
					if (path != get_path()) {
						Ref<CScript> bf = ResourceLoader::load(path);

						if (bf.is_valid()) {
							base_cache = bf;
							bf->inheriters_cache.insert(get_instance_id());
						}
					} else {
						ERR_PRINT(("Path extending itself in  " + path).utf8().get_data());
					}
				}
			}

			members_cache.clear();
			member_default_values_cache.clear();

			for (int i = 0; i < c->variables.size(); i++) {
				if (c->variables[i]._export.type == Variant::NIL) {
					continue;
				}

				members_cache.push_back(c->variables[i]._export);

				Variant::Type default_value_type = c->variables[i].default_value.get_type();
				Variant::Type export_type = c->variables[i]._export.type;

				// Convert the default value to the export type to avoid issues with the property editor and scene serialization.
				// This is done only in the export side, the script itself will use the default value with no type change.
				if (default_value_type != Variant::NIL && default_value_type != export_type) {
					Variant::CallError ce;
					const Variant *args = &c->variables[i].default_value;
					member_default_values_cache[c->variables[i].identifier] = Variant::construct(export_type, &args, 1, ce);
				} else {
					member_default_values_cache[c->variables[i].identifier] = c->variables[i].default_value;
				}
			}

			_signals.clear();

			for (int i = 0; i < c->_signals.size(); i++) {
				_signals[c->_signals[i].name] = c->_signals[i].arguments;
			}
		} else {
			placeholder_fallback_enabled = true;
			return false;
		}
	} else if (placeholder_fallback_enabled) {
		return false;
	}

	placeholder_fallback_enabled = false;

	if (base_cache.is_valid() && base_cache->is_valid()) {
		for (int i = 0; i < base_caches.size(); i++) {
			if (base_caches[i] == base_cache.ptr()) {
				if (r_err) {
					*r_err = true;
				}
				valid = false; // to show error in the editor
				base_cache->valid = false;
				base_cache->inheriters_cache.clear(); // to prevent future stackoverflows
				base_cache.unref();
				base.unref();
				_base = nullptr;
				ERR_FAIL_V_MSG(false, "Cyclic inheritance in script class.");
			}
		}
		if (base_cache->_update_exports(r_err, true)) {
			if (r_err && *r_err) {
				return false;
			}
			changed = true;
		}
	}

	if ((changed || p_instance_to_update) && placeholders.size()) { //hm :(

		// update placeholders if any
		Map<StringName, Variant> values;
		List<PropertyInfo> propnames;
		_update_exports_values(values, propnames);

		if (changed) {
			for (Set<PlaceHolderScriptInstance *>::Element *E = placeholders.front(); E; E = E->next()) {
				E->get()->update(propnames, values);
			}
		} else {
			p_instance_to_update->update(propnames, values);
		}
	}

	return changed;

#else
	return false;
#endif
}

void CScript::update_exports() {
#ifdef TOOLS_ENABLED

	bool cyclic_error = false;
	_update_exports(&cyclic_error);
	if (cyclic_error) {
		return;
	}

	Set<ObjectID> copy = inheriters_cache; //might get modified

	for (Set<ObjectID>::Element *E = copy.front(); E; E = E->next()) {
		Object *id = ObjectDB::get_instance(E->get());
		CScript *s = Object::cast_to<CScript>(id);
		if (!s) {
			continue;
		}
		s->update_exports();
	}

#endif
}

void CScript::_set_subclass_path(Ref<CScript> &p_sc, const String &p_path) {
	p_sc->path = p_path;
	for (Map<StringName, Ref<CScript>>::Element *E = p_sc->subclasses.front(); E; E = E->next()) {
		_set_subclass_path(E->get(), p_path);
	}
}

String CScript::_get_debug_path() const {
	if ((get_path().empty() || get_path().find("::") != -1) && !get_name().empty()) {
		return get_name() + " (" + get_path().get_slice("::", 0) + ")";
	} else {
		return get_path();
	}
}

Error CScript::reload(bool p_keep_state) {
	CScriptLanguage::singleton->lock.lock();
	bool has_instances = instances.size();
	CScriptLanguage::singleton->lock.unlock();

	ERR_FAIL_COND_V(!p_keep_state && has_instances, ERR_ALREADY_IN_USE);

	String basedir = path;

	if (basedir == "") {
		basedir = get_path();
	}

	if (basedir != "") {
		basedir = basedir.get_base_dir();
	}

	if (source.find("%BASE%") != -1) {
		//loading a template, don't parse
		return OK;
	}

	valid = false;
	CScriptParser parser;
	Error err = parser.parse(source, basedir, false, path);
	if (err) {
		if (ScriptDebugger::get_singleton()) {
			CScriptLanguage::get_singleton()->debug_break_parse(_get_debug_path(), parser.get_error_line(), "Parser Error: " + parser.get_error());
		}
		_err_print_error("CScript::reload", path.empty() ? "built-in" : (const char *)path.utf8().get_data(), parser.get_error_line(), ("Parse Error: " + parser.get_error()).utf8().get_data(), ERR_HANDLER_SCRIPT);
		ERR_FAIL_V(ERR_PARSE_ERROR);
	}

	bool can_run = ScriptServer::is_scripting_enabled() || parser.is_tool_script();

	CScriptCompiler compiler;
	err = compiler.compile(&parser, this, p_keep_state);

	if (err) {
		if (can_run) {
			if (ScriptDebugger::get_singleton()) {
				CScriptLanguage::get_singleton()->debug_break_parse(_get_debug_path(), compiler.get_error_line(), "Parser Error: " + compiler.get_error());
			}
			_err_print_error("CScript::reload", path.empty() ? "built-in" : (const char *)path.utf8().get_data(), compiler.get_error_line(), ("Compile Error: " + compiler.get_error()).utf8().get_data(), ERR_HANDLER_SCRIPT);
			ERR_FAIL_V(ERR_COMPILATION_FAILED);
		} else {
			return err;
		}
	}
#ifdef DEBUG_ENABLED
	for (const List<CScriptWarning>::Element *E = parser.get_warnings().front(); E; E = E->next()) {
		const CScriptWarning &warning = E->get();
		if (ScriptDebugger::get_singleton()) {
			Vector<ScriptLanguage::StackInfo> si;
			ScriptDebugger::get_singleton()->send_error("", get_path(), warning.line, warning.get_name(), warning.get_message(), ERR_HANDLER_WARNING, si);
		}
	}
#endif

	valid = true;

	for (Map<StringName, Ref<CScript>>::Element *E = subclasses.front(); E; E = E->next()) {
		_set_subclass_path(E->get(), path);
	}
	_clear_pending_func_states();

	return OK;
}

ScriptLanguage *CScript::get_language() const {
	return CScriptLanguage::get_singleton();
}

void CScript::get_constants(Map<StringName, Variant> *p_constants) {
	if (p_constants) {
		for (Map<StringName, Variant>::Element *E = constants.front(); E; E = E->next()) {
			(*p_constants)[E->key()] = E->value();
		}
	}
}

void CScript::get_members(Set<StringName> *p_members) {
	if (p_members) {
		for (Set<StringName>::Element *E = members.front(); E; E = E->next()) {
			p_members->insert(E->get());
		}
	}
}

Variant CScript::call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	CScript *top = this;
	while (top) {
		Map<StringName, CScriptFunction *>::Element *E = top->member_functions.find(p_method);
		if (E) {
			ERR_FAIL_COND_V_MSG(!E->get()->is_static(), Variant(), "Can't call non-static function '" + String(p_method) + "' in script.");

			return E->get()->call(nullptr, p_args, p_argcount, r_error);
		}
		top = top->_base;
	}

	//none found, regular

	return Script::call(p_method, p_args, p_argcount, r_error);
}

bool CScript::_get(const StringName &p_name, Variant &r_ret) const {
	{
		const CScript *top = this;
		while (top) {
			{
				const Map<StringName, Variant>::Element *E = top->constants.find(p_name);
				if (E) {
					r_ret = E->get();
					return true;
				}
			}

			{
				const Map<StringName, Ref<CScript>>::Element *E = subclasses.find(p_name);
				if (E) {
					r_ret = E->get();
					return true;
				}
			}
			top = top->_base;
		}

		if (p_name == CScriptLanguage::get_singleton()->strings._script_source) {
			r_ret = get_source_code();
			return true;
		}
	}

	return false;
}
bool CScript::_set(const StringName &p_name, const Variant &p_value) {
	if (p_name == CScriptLanguage::get_singleton()->strings._script_source) {
		set_source_code(p_value);
		reload();
	} else {
		return false;
	}

	return true;
}

void CScript::_get_property_list(List<PropertyInfo> *p_properties) const {
	p_properties->push_back(PropertyInfo(Variant::STRING, "script/source", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL));
}

void CScript::_bind_methods() {
	ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "new", &CScript::_new, MethodInfo("new"));

	ClassDB::bind_method(D_METHOD("get_as_byte_code"), &CScript::get_as_byte_code);
}

Vector<uint8_t> CScript::get_as_byte_code() const {
	CScriptTokenizerBuffer tokenizer;
	return tokenizer.parse_code_string(source);
};

Error CScript::load_byte_code(const String &p_path) {
	Vector<uint8_t> bytecode;

	if (p_path.ends_with("gde")) {
		FileAccess *fa = FileAccess::open(p_path, FileAccess::READ);
		ERR_FAIL_COND_V(!fa, ERR_CANT_OPEN);

		FileAccessEncrypted *fae = memnew(FileAccessEncrypted);
		ERR_FAIL_COND_V(!fae, ERR_CANT_OPEN);

		Vector<uint8_t> key;
		key.resize(32);
		for (int i = 0; i < key.size(); i++) {
			key.write[i] = script_encryption_key[i];
		}

		Error err = fae->open_and_parse(fa, key, FileAccessEncrypted::MODE_READ);

		if (err) {
			fa->close();
			memdelete(fa);
			memdelete(fae);

			ERR_FAIL_COND_V(err, err);
		}

		bytecode.resize(fae->get_len());
		fae->get_buffer(bytecode.ptrw(), bytecode.size());
		fae->close();
		memdelete(fae);

	} else {
		bytecode = FileAccess::get_file_as_array(p_path);
	}

	ERR_FAIL_COND_V(bytecode.size() == 0, ERR_PARSE_ERROR);
	path = p_path;

	String basedir = path;

	if (basedir == "") {
		basedir = get_path();
	}

	if (basedir != "") {
		basedir = basedir.get_base_dir();
	}

	valid = false;
	CScriptParser parser;
	Error err = parser.parse_bytecode(bytecode, basedir, get_path());
	if (err) {
		_err_print_error("CScript::load_byte_code", path.empty() ? "built-in" : (const char *)path.utf8().get_data(), parser.get_error_line(), ("Parse Error: " + parser.get_error()).utf8().get_data(), ERR_HANDLER_SCRIPT);
		ERR_FAIL_V(ERR_PARSE_ERROR);
	}

	CScriptCompiler compiler;
	err = compiler.compile(&parser, this);

	if (err) {
		_err_print_error("CScript::load_byte_code", path.empty() ? "built-in" : (const char *)path.utf8().get_data(), compiler.get_error_line(), ("Compile Error: " + compiler.get_error()).utf8().get_data(), ERR_HANDLER_SCRIPT);
		ERR_FAIL_V(ERR_COMPILATION_FAILED);
	}

	valid = true;

	for (Map<StringName, Ref<CScript>>::Element *E = subclasses.front(); E; E = E->next()) {
		_set_subclass_path(E->get(), path);
	}

	return OK;
}

Error CScript::load_source_code(const String &p_path) {
	PoolVector<uint8_t> sourcef;
	Error err;
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ, &err);
	if (err) {
		ERR_FAIL_COND_V(err, err);
	}

	uint64_t len = f->get_len();
	sourcef.resize(len + 1);
	PoolVector<uint8_t>::Write w = sourcef.write();
	uint64_t r = f->get_buffer(w.ptr(), len);
	f->close();
	memdelete(f);
	ERR_FAIL_COND_V(r != len, ERR_CANT_OPEN);
	w[len] = 0;

	String s;
	if (s.parse_utf8((const char *)w.ptr())) {
		ERR_FAIL_V_MSG(ERR_INVALID_DATA, "Script '" + p_path + "' contains invalid unicode (UTF-8), so it was not loaded. Please ensure that scripts are saved in valid UTF-8 unicode.");
	}

	source = s;
#ifdef TOOLS_ENABLED
	source_changed_cache = true;
#endif
	path = p_path;
	return OK;
}

const Map<StringName, CScriptFunction *> &CScript::debug_get_member_functions() const {
	return member_functions;
}

StringName CScript::debug_get_member_by_index(int p_idx) const {
	for (const Map<StringName, MemberInfo>::Element *E = member_indices.front(); E; E = E->next()) {
		if (E->get().index == p_idx) {
			return E->key();
		}
	}

	return "<error>";
}

Ref<CScript> CScript::get_base() const {
	return base;
}

bool CScript::inherits_script(const Ref<Script> &p_script) const {
	Ref<CScript> gd = p_script;
	if (gd.is_null()) {
		return false;
	}

	const CScript *s = this;

	while (s) {
		if (s == p_script.ptr()) {
			return true;
		}
		s = s->_base;
	}

	return false;
}

bool CScript::has_script_signal(const StringName &p_signal) const {
	if (_signals.has(p_signal)) {
		return true;
	}
	if (base.is_valid()) {
		return base->has_script_signal(p_signal);
	}
#ifdef TOOLS_ENABLED
	else if (base_cache.is_valid()) {
		return base_cache->has_script_signal(p_signal);
	}
#endif
	return false;
}
void CScript::get_script_signal_list(List<MethodInfo> *r_signals) const {
	for (const Map<StringName, Vector<StringName>>::Element *E = _signals.front(); E; E = E->next()) {
		MethodInfo mi;
		mi.name = E->key();
		for (int i = 0; i < E->get().size(); i++) {
			PropertyInfo arg;
			arg.name = E->get()[i];
			mi.arguments.push_back(arg);
		}
		r_signals->push_back(mi);
	}

	if (base.is_valid()) {
		base->get_script_signal_list(r_signals);
	}
#ifdef TOOLS_ENABLED
	else if (base_cache.is_valid()) {
		base_cache->get_script_signal_list(r_signals);
	}

#endif
}

CScript::CScript() :
		script_list(this) {
	valid = false;
	subclass_count = 0;
	initializer = nullptr;
	_base = nullptr;
	_owner = nullptr;
	tool = false;
#ifdef TOOLS_ENABLED
	source_changed_cache = false;
	placeholder_fallback_enabled = false;
#endif

#ifdef DEBUG_ENABLED
	CScriptLanguage::get_singleton()->lock.lock();
	CScriptLanguage::get_singleton()->script_list.add(&script_list);
	CScriptLanguage::get_singleton()->lock.unlock();
#endif
}

void CScript::_save_orphaned_subclasses() {
	struct ClassRefWithName {
		ObjectID id;
		String fully_qualified_name;
	};
	Vector<ClassRefWithName> weak_subclasses;
	// collect subclasses ObjectID and name
	for (Map<StringName, Ref<CScript>>::Element *E = subclasses.front(); E; E = E->next()) {
		E->get()->_owner = nullptr; //bye, you are no longer owned cause I died
		ClassRefWithName subclass;
		subclass.id = E->get()->get_instance_id();
		subclass.fully_qualified_name = E->get()->fully_qualified_name;
		weak_subclasses.push_back(subclass);
	}

	// clear subclasses to allow unused subclasses to be deleted
	subclasses.clear();
	// subclasses are also held by constants, clear those as well
	constants.clear();

	// keep orphan subclass only for subclasses that are still in use
	for (int i = 0; i < weak_subclasses.size(); i++) {
		ClassRefWithName subclass = weak_subclasses[i];
		Object *obj = ObjectDB::get_instance(subclass.id);
		if (!obj) {
			continue;
		}
		// subclass is not released
		CScriptLanguage::get_singleton()->add_orphan_subclass(subclass.fully_qualified_name, subclass.id);
	}
}

CScript::~CScript() {
	_clear_pending_func_states();

	for (Map<StringName, CScriptFunction *>::Element *E = member_functions.front(); E; E = E->next()) {
		memdelete(E->get());
	}

	_save_orphaned_subclasses();

#ifdef DEBUG_ENABLED
	CScriptLanguage::get_singleton()->lock.lock();
	CScriptLanguage::get_singleton()->script_list.remove(&script_list);
	CScriptLanguage::get_singleton()->lock.unlock();
#endif
}

//////////////////////////////
//         INSTANCE         //
//////////////////////////////

bool CScriptInstance::set(const StringName &p_name, const Variant &p_value) {
	//member
	{
		const Map<StringName, CScript::MemberInfo>::Element *E = script->member_indices.find(p_name);
		if (E) {
			const CScript::MemberInfo *member = &E->get();
			if (member->setter) {
				const Variant *val = &p_value;
				Variant::CallError err;
				call(member->setter, &val, 1, err);
				if (err.error == Variant::CallError::CALL_OK) {
					return true; //function exists, call was successful
				} else {
					return false;
				}
			} else {
				if (!member->data_type.is_type(p_value)) {
					// Try conversion
					Variant::CallError ce;
					const Variant *value = &p_value;
					Variant converted = Variant::construct(member->data_type.builtin_type, &value, 1, ce);
					if (ce.error == Variant::CallError::CALL_OK) {
						members.write[member->index] = converted;
						return true;
					} else {
						return false;
					}
				} else {
					members.write[member->index] = p_value;
				}
			}
			return true;
		}
	}

	CScript *sptr = script.ptr();
	while (sptr) {
		Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(CScriptLanguage::get_singleton()->strings._set);
		if (E) {
			Variant name = p_name;
			const Variant *args[2] = { &name, &p_value };

			Variant::CallError err;
			Variant ret = E->get()->call(this, (const Variant **)args, 2, err);
			if (err.error == Variant::CallError::CALL_OK && ret.get_type() == Variant::BOOL && ret.operator bool()) {
				return true;
			}
		}
		sptr = sptr->_base;
	}

	return false;
}

bool CScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
	const CScript *sptr = script.ptr();
	while (sptr) {
		{
			const Map<StringName, CScript::MemberInfo>::Element *E = script->member_indices.find(p_name);
			if (E) {
				if (E->get().getter) {
					Variant::CallError err;
					r_ret = const_cast<CScriptInstance *>(this)->call(E->get().getter, nullptr, 0, err);
					if (err.error == Variant::CallError::CALL_OK) {
						return true;
					}
				}
				r_ret = members[E->get().index];
				return true; //index found
			}
		}

		{
			const CScript *sl = sptr;
			while (sl) {
				const Map<StringName, Variant>::Element *E = sl->constants.find(p_name);
				if (E) {
					r_ret = E->get();
					return true; //index found
				}
				sl = sl->_base;
			}
		}

		{
			const Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(CScriptLanguage::get_singleton()->strings._get);
			if (E) {
				Variant name = p_name;
				const Variant *args[1] = { &name };

				Variant::CallError err;
				Variant ret = const_cast<CScriptFunction *>(E->get())->call(const_cast<CScriptInstance *>(this), (const Variant **)args, 1, err);
				if (err.error == Variant::CallError::CALL_OK && ret.get_type() != Variant::NIL) {
					r_ret = ret;
					return true;
				}
			}
		}
		sptr = sptr->_base;
	}

	return false;
}

Variant::Type CScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
	const CScript *sptr = script.ptr();
	while (sptr) {
		if (sptr->member_info.has(p_name)) {
			if (r_is_valid) {
				*r_is_valid = true;
			}
			return sptr->member_info[p_name].type;
		}
		sptr = sptr->_base;
	}

	if (r_is_valid) {
		*r_is_valid = false;
	}
	return Variant::NIL;
}

void CScriptInstance::get_property_list(List<PropertyInfo> *p_properties) const {
	// exported members, not done yet!

	const CScript *sptr = script.ptr();
	List<PropertyInfo> props;

	while (sptr) {
		const Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(CScriptLanguage::get_singleton()->strings._get_property_list);
		if (E) {
			Variant::CallError err;
			Variant ret = const_cast<CScriptFunction *>(E->get())->call(const_cast<CScriptInstance *>(this), nullptr, 0, err);
			if (err.error == Variant::CallError::CALL_OK) {
				ERR_FAIL_COND_MSG(ret.get_type() != Variant::ARRAY, "Wrong type for _get_property_list, must be an array of dictionaries.");

				Array arr = ret;
				for (int i = 0; i < arr.size(); i++) {
					Dictionary d = arr[i];
					ERR_CONTINUE(!d.has("name"));
					ERR_CONTINUE(!d.has("type"));
					PropertyInfo pinfo;
					pinfo.type = Variant::Type(d["type"].operator int());
					ERR_CONTINUE(pinfo.type < 0 || pinfo.type >= Variant::VARIANT_MAX);
					pinfo.name = d["name"];
					ERR_CONTINUE(pinfo.name == "");
					if (d.has("hint")) {
						pinfo.hint = PropertyHint(d["hint"].operator int());
					}
					if (d.has("hint_string")) {
						pinfo.hint_string = d["hint_string"];
					}
					if (d.has("usage")) {
						pinfo.usage = d["usage"];
					}

					props.push_back(pinfo);
				}
			}
		}

		//instance a fake script for editing the values

		Vector<_CScriptMemberSort> msort;
		for (Map<StringName, PropertyInfo>::Element *F = sptr->member_info.front(); F; F = F->next()) {
			_CScriptMemberSort ms;
			ERR_CONTINUE(!sptr->member_indices.has(F->key()));
			ms.index = sptr->member_indices[F->key()].index;
			ms.name = F->key();
			msort.push_back(ms);
		}

		msort.sort();
		msort.invert();
		for (int i = 0; i < msort.size(); i++) {
			props.push_front(sptr->member_info[msort[i].name]);
		}

		sptr = sptr->_base;
	}

	for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {
		p_properties->push_back(E->get());
	}
}

void CScriptInstance::get_method_list(List<MethodInfo> *p_list) const {
	const CScript *sptr = script.ptr();
	while (sptr) {
		for (Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.front(); E; E = E->next()) {
			MethodInfo mi;
			mi.name = E->key();
			mi.flags |= METHOD_FLAG_FROM_SCRIPT;
			for (int i = 0; i < E->get()->get_argument_count(); i++) {
				mi.arguments.push_back(PropertyInfo(Variant::NIL, "arg" + itos(i)));
			}
			p_list->push_back(mi);
		}
		sptr = sptr->_base;
	}
}

bool CScriptInstance::has_method(const StringName &p_method) const {
	const CScript *sptr = script.ptr();
	while (sptr) {
		const Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(p_method);
		if (E) {
			return true;
		}
		sptr = sptr->_base;
	}

	return false;
}
Variant CScriptInstance::call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	CScript *sptr = script.ptr();
	while (sptr) {
		Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(p_method);
		if (E) {
			return E->get()->call(this, p_args, p_argcount, r_error);
		}
		sptr = sptr->_base;
	}
	r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
	return Variant();
}

void CScriptInstance::call_multilevel(const StringName &p_method, const Variant **p_args, int p_argcount) {
	CScript *sptr = script.ptr();
	Variant::CallError ce;

	while (sptr) {
		Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(p_method);
		if (E) {
			E->get()->call(this, p_args, p_argcount, ce);
		}
		sptr = sptr->_base;
	}
}

void CScriptInstance::_ml_call_reversed(CScript *sptr, const StringName &p_method, const Variant **p_args, int p_argcount) {
	if (sptr->_base) {
		_ml_call_reversed(sptr->_base, p_method, p_args, p_argcount);
	}

	Variant::CallError ce;

	Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(p_method);
	if (E) {
		E->get()->call(this, p_args, p_argcount, ce);
	}
}

void CScriptInstance::call_multilevel_reversed(const StringName &p_method, const Variant **p_args, int p_argcount) {
	if (script.ptr()) {
		_ml_call_reversed(script.ptr(), p_method, p_args, p_argcount);
	}
}

void CScriptInstance::notification(int p_notification) {
	//notification is not virtual, it gets called at ALL levels just like in C.
	Variant value = p_notification;
	const Variant *args[1] = { &value };

	CScript *sptr = script.ptr();
	while (sptr) {
		Map<StringName, CScriptFunction *>::Element *E = sptr->member_functions.find(CScriptLanguage::get_singleton()->strings._notification);
		if (E) {
			Variant::CallError err;
			E->get()->call(this, args, 1, err);
			if (err.error != Variant::CallError::CALL_OK) {
				//print error about notification call
			}
		}
		sptr = sptr->_base;
	}
}

String CScriptInstance::to_string(bool *r_valid) {
	if (has_method(CoreStringNames::get_singleton()->_to_string)) {
		Variant::CallError ce;
		Variant ret = call(CoreStringNames::get_singleton()->_to_string, nullptr, 0, ce);
		if (ce.error == Variant::CallError::CALL_OK) {
			if (ret.get_type() != Variant::STRING) {
				if (r_valid) {
					*r_valid = false;
				}
				ERR_FAIL_V_MSG(String(), "Wrong type for " + CoreStringNames::get_singleton()->_to_string + ", must be a String.");
			}
			if (r_valid) {
				*r_valid = true;
			}
			return ret.operator String();
		}
	}
	if (r_valid) {
		*r_valid = false;
	}
	return String();
}

Ref<Script> CScriptInstance::get_script() const {
	return script;
}

ScriptLanguage *CScriptInstance::get_language() {
	return CScriptLanguage::get_singleton();
}

MultiplayerAPI::RPCMode CScriptInstance::get_rpc_mode(const StringName &p_method) const {
	const CScript *cscript = script.ptr();

	while (cscript) {
		const Map<StringName, CScriptFunction *>::Element *E = cscript->member_functions.find(p_method);
		if (E) {
			if (E->get()->get_rpc_mode() != MultiplayerAPI::RPC_MODE_DISABLED) {
				return E->get()->get_rpc_mode();
			}
		}
		cscript = cscript->_base;
	}

	return MultiplayerAPI::RPC_MODE_DISABLED;
}

MultiplayerAPI::RPCMode CScriptInstance::get_rset_mode(const StringName &p_variable) const {
	const CScript *cscript = script.ptr();

	while (cscript) {
		const Map<StringName, CScript::MemberInfo>::Element *E = cscript->member_indices.find(p_variable);
		if (E) {
			if (E->get().rpc_mode) {
				return E->get().rpc_mode;
			}
		}
		cscript = cscript->_base;
	}

	return MultiplayerAPI::RPC_MODE_DISABLED;
}

void CScriptInstance::reload_members() {
#ifdef DEBUG_ENABLED

	members.resize(script->member_indices.size()); //resize

	Vector<Variant> new_members;
	new_members.resize(script->member_indices.size());

	//pass the values to the new indices
	for (Map<StringName, CScript::MemberInfo>::Element *E = script->member_indices.front(); E; E = E->next()) {
		if (member_indices_cache.has(E->key())) {
			Variant value = members[member_indices_cache[E->key()]];
			new_members.write[E->get().index] = value;
		}
	}

	//apply
	members = new_members;

	//pass the values to the new indices
	member_indices_cache.clear();
	for (Map<StringName, CScript::MemberInfo>::Element *E = script->member_indices.front(); E; E = E->next()) {
		member_indices_cache[E->key()] = E->get().index;
	}

#endif
}

CScriptInstance::CScriptInstance() {
	owner = nullptr;
	base_ref = false;
}

CScriptInstance::~CScriptInstance() {
	CScriptLanguage::singleton->lock.lock();

	while (SelfList<CScriptFunctionState> *E = pending_func_states.first()) {
		// Order matters since clearing the stack may already cause
		// the CSCriptFunctionState to be destroyed and thus removed from the list.
		pending_func_states.remove(E);
		E->self()->_clear_stack();
	}

	if (script.is_valid() && owner) {
		script->instances.erase(owner);
	}

	CScriptLanguage::singleton->lock.unlock();
}

/************* SCRIPT LANGUAGE **************/

CScriptLanguage *CScriptLanguage::singleton = nullptr;

String CScriptLanguage::get_name() const {
	return "CScript";
}

/* LANGUAGE FUNCTIONS */

void CScriptLanguage::_add_global(const StringName &p_name, const Variant &p_value) {
	if (globals.has(p_name)) {
		//overwrite existing
		global_array.write[globals[p_name]] = p_value;
		return;
	}
	globals[p_name] = global_array.size();
	global_array.push_back(p_value);
	_global_array = global_array.ptrw();
}

void CScriptLanguage::add_global_constant(const StringName &p_variable, const Variant &p_value) {
	_add_global(p_variable, p_value);
}

void CScriptLanguage::add_named_global_constant(const StringName &p_name, const Variant &p_value) {
	named_globals[p_name] = p_value;
}

void CScriptLanguage::remove_named_global_constant(const StringName &p_name) {
	ERR_FAIL_COND(!named_globals.has(p_name));
	named_globals.erase(p_name);
}

void CScriptLanguage::init() {
	//populate global constants
	int gcc = GlobalConstants::get_global_constant_count();
	for (int i = 0; i < gcc; i++) {
		_add_global(StaticCString::create(GlobalConstants::get_global_constant_name(i)), GlobalConstants::get_global_constant_value(i));
	}

	_add_global(StaticCString::create("PI"), Math_PI);
	_add_global(StaticCString::create("TAU"), Math_TAU);
	_add_global(StaticCString::create("INF"), Math_INF);
	_add_global(StaticCString::create("NAN"), Math_NAN);

	//populate native classes

	List<StringName> class_list;
	ClassDB::get_class_list(&class_list);
	for (List<StringName>::Element *E = class_list.front(); E; E = E->next()) {
		StringName n = E->get();
		String s = String(n);
		if (s.begins_with("_")) {
			n = s.substr(1, s.length());
		}

		if (globals.has(n)) {
			continue;
		}
		Ref<CScriptNativeClass> nc = memnew(CScriptNativeClass(E->get()));
		_add_global(n, nc);
	}

	//populate singletons

	List<Engine::Singleton> singletons;
	Engine::get_singleton()->get_singletons(&singletons);
	for (List<Engine::Singleton>::Element *E = singletons.front(); E; E = E->next()) {
		_add_global(E->get().name, E->get().ptr);
	}
}

String CScriptLanguage::get_type() const {
	return "CScript";
}
String CScriptLanguage::get_extension() const {
	return "cpps";
}
Error CScriptLanguage::execute_file(const String &p_path) {
	// ??
	return OK;
}
void CScriptLanguage::finish() {
}

void CScriptLanguage::profiling_start() {
#ifdef DEBUG_ENABLED
	lock.lock();

	SelfList<CScriptFunction> *elem = function_list.first();
	while (elem) {
		elem->self()->profile.call_count = 0;
		elem->self()->profile.self_time = 0;
		elem->self()->profile.total_time = 0;
		elem->self()->profile.frame_call_count = 0;
		elem->self()->profile.frame_self_time = 0;
		elem->self()->profile.frame_total_time = 0;
		elem->self()->profile.last_frame_call_count = 0;
		elem->self()->profile.last_frame_self_time = 0;
		elem->self()->profile.last_frame_total_time = 0;
		elem = elem->next();
	}

	profiling = true;
	lock.unlock();
#endif
}

void CScriptLanguage::profiling_stop() {
#ifdef DEBUG_ENABLED
	lock.lock();
	profiling = false;
	lock.unlock();
#endif
}

int CScriptLanguage::profiling_get_accumulated_data(ProfilingInfo *p_info_arr, int p_info_max) {
	int current = 0;
#ifdef DEBUG_ENABLED
	lock.lock();

	SelfList<CScriptFunction> *elem = function_list.first();
	while (elem) {
		if (current >= p_info_max) {
			break;
		}
		p_info_arr[current].call_count = elem->self()->profile.call_count;
		p_info_arr[current].self_time = elem->self()->profile.self_time;
		p_info_arr[current].total_time = elem->self()->profile.total_time;
		p_info_arr[current].signature = elem->self()->profile.signature;
		elem = elem->next();
		current++;
	}

	lock.unlock();
#endif

	return current;
}

int CScriptLanguage::profiling_get_frame_data(ProfilingInfo *p_info_arr, int p_info_max) {
	int current = 0;

#ifdef DEBUG_ENABLED
	lock.lock();

	SelfList<CScriptFunction> *elem = function_list.first();
	while (elem) {
		if (current >= p_info_max) {
			break;
		}
		if (elem->self()->profile.last_frame_call_count > 0) {
			p_info_arr[current].call_count = elem->self()->profile.last_frame_call_count;
			p_info_arr[current].self_time = elem->self()->profile.last_frame_self_time;
			p_info_arr[current].total_time = elem->self()->profile.last_frame_total_time;
			p_info_arr[current].signature = elem->self()->profile.signature;
			current++;
		}
		elem = elem->next();
	}

	lock.unlock();
#endif

	return current;
}

struct CScriptDepSort {
	//must support sorting so inheritance works properly (parent must be reloaded first)
	bool operator()(const Ref<CScript> &A, const Ref<CScript> &B) const {
		if (A == B) {
			return false; //shouldn't happen but..
		}
		const CScript *I = B->get_base().ptr();
		while (I) {
			if (I == A.ptr()) {
				// A is a base of B
				return true;
			}

			I = I->get_base().ptr();
		}

		return false; //not a base
	}
};

void CScriptLanguage::reload_all_scripts() {
#ifdef DEBUG_ENABLED
	print_verbose("CScript: Reloading all scripts");
	lock.lock();

	List<Ref<CScript>> scripts;

	SelfList<CScript> *elem = script_list.first();
	while (elem) {
		if (elem->self()->get_path().is_resource_file()) {
			print_verbose("CScript: Found: " + elem->self()->get_path());
			scripts.push_back(Ref<CScript>(elem->self())); //cast to gdscript to avoid being erased by accident
		}
		elem = elem->next();
	}

	lock.unlock();

	//as scripts are going to be reloaded, must proceed without locking here

	scripts.sort_custom<CScriptDepSort>(); //update in inheritance dependency order

	for (List<Ref<CScript>>::Element *E = scripts.front(); E; E = E->next()) {
		print_verbose("CScript: Reloading: " + E->get()->get_path());
		E->get()->load_source_code(E->get()->get_path());
		E->get()->reload(true);
	}
#endif
}

void CScriptLanguage::reload_tool_script(const Ref<Script> &p_script, bool p_soft_reload) {
#ifdef DEBUG_ENABLED

	lock.lock();

	List<Ref<CScript>> scripts;

	SelfList<CScript> *elem = script_list.first();
	while (elem) {
		if (elem->self()->get_path().is_resource_file()) {
			scripts.push_back(Ref<CScript>(elem->self())); //cast to gdscript to avoid being erased by accident
		}
		elem = elem->next();
	}

	lock.unlock();

	//when someone asks you why dynamically typed languages are easier to write....

	Map<Ref<CScript>, Map<ObjectID, List<Pair<StringName, Variant>>>> to_reload;

	//as scripts are going to be reloaded, must proceed without locking here

	scripts.sort_custom<CScriptDepSort>(); //update in inheritance dependency order

	for (List<Ref<CScript>>::Element *E = scripts.front(); E; E = E->next()) {
		bool reload = E->get() == p_script || to_reload.has(E->get()->get_base());

		if (!reload) {
			continue;
		}

		to_reload.insert(E->get(), Map<ObjectID, List<Pair<StringName, Variant>>>());

		if (!p_soft_reload) {
			//save state and remove script from instances
			Map<ObjectID, List<Pair<StringName, Variant>>> &map = to_reload[E->get()];

			while (E->get()->instances.front()) {
				Object *obj = E->get()->instances.front()->get();
				//save instance info
				List<Pair<StringName, Variant>> state;
				if (obj->get_script_instance()) {
					obj->get_script_instance()->get_property_state(state);
					map[obj->get_instance_id()] = state;
					obj->set_script(RefPtr());
				}
			}

//same thing for placeholders
#ifdef TOOLS_ENABLED

			while (E->get()->placeholders.size()) {
				Object *obj = E->get()->placeholders.front()->get()->get_owner();

				//save instance info
				if (obj->get_script_instance()) {
					map.insert(obj->get_instance_id(), List<Pair<StringName, Variant>>());
					List<Pair<StringName, Variant>> &state = map[obj->get_instance_id()];
					obj->get_script_instance()->get_property_state(state);
					obj->set_script(RefPtr());
				} else {
					// no instance found. Let's remove it so we don't loop forever
					E->get()->placeholders.erase(E->get()->placeholders.front()->get());
				}
			}

#endif

			for (Map<ObjectID, List<Pair<StringName, Variant>>>::Element *F = E->get()->pending_reload_state.front(); F; F = F->next()) {
				map[F->key()] = F->get(); //pending to reload, use this one instead
			}
		}
	}

	for (Map<Ref<CScript>, Map<ObjectID, List<Pair<StringName, Variant>>>>::Element *E = to_reload.front(); E; E = E->next()) {
		Ref<CScript> scr = E->key();
		scr->reload(p_soft_reload);

		//restore state if saved
		for (Map<ObjectID, List<Pair<StringName, Variant>>>::Element *F = E->get().front(); F; F = F->next()) {
			List<Pair<StringName, Variant>> &saved_state = F->get();

			Object *obj = ObjectDB::get_instance(F->key());
			if (!obj) {
				continue;
			}

			if (!p_soft_reload) {
				//clear it just in case (may be a pending reload state)
				obj->set_script(RefPtr());
			}
			obj->set_script(scr.get_ref_ptr());

			ScriptInstance *script_instance = obj->get_script_instance();

			if (!script_instance) {
				//failed, save reload state for next time if not saved
				if (!scr->pending_reload_state.has(obj->get_instance_id())) {
					scr->pending_reload_state[obj->get_instance_id()] = saved_state;
				}
				continue;
			}

			if (script_instance->is_placeholder() && scr->is_placeholder_fallback_enabled()) {
				PlaceHolderScriptInstance *placeholder = static_cast<PlaceHolderScriptInstance *>(script_instance);
				for (List<Pair<StringName, Variant>>::Element *G = saved_state.front(); G; G = G->next()) {
					placeholder->property_set_fallback(G->get().first, G->get().second);
				}
			} else {
				for (List<Pair<StringName, Variant>>::Element *G = saved_state.front(); G; G = G->next()) {
					script_instance->set(G->get().first, G->get().second);
				}
			}

			scr->pending_reload_state.erase(obj->get_instance_id()); //as it reloaded, remove pending state
		}

		//if instance states were saved, set them!
	}

#endif
}

void CScriptLanguage::frame() {
	calls = 0;

#ifdef DEBUG_ENABLED
	if (profiling) {
		lock.lock();

		SelfList<CScriptFunction> *elem = function_list.first();
		while (elem) {
			elem->self()->profile.last_frame_call_count = elem->self()->profile.frame_call_count;
			elem->self()->profile.last_frame_self_time = elem->self()->profile.frame_self_time;
			elem->self()->profile.last_frame_total_time = elem->self()->profile.frame_total_time;
			elem->self()->profile.frame_call_count = 0;
			elem->self()->profile.frame_self_time = 0;
			elem->self()->profile.frame_total_time = 0;
			elem = elem->next();
		}

		lock.unlock();
	}

#endif
}

/* EDITOR FUNCTIONS */
void CScriptLanguage::get_reserved_words(List<String> *p_words) const {
	static const char *_reserved_words[] = {
		// operators
		"and",
		"in",
		"not",
		"or",
		// types and values
		"false",
		"float",
		"int",
		"bool",
		"null",
		"PI",
		"TAU",
		"INF",
		"NAN",
		"self",
		"true",
		"void",
		// functions
		"as",
		"assert",
		"breakpoint",
		"class",
		"class_name",
		"extends",
		"is",
		"func",
		"preload",
		"setget",
		"signal",
		"tool",
		"yield",
		// var
		"const",
		"enum",
		"export",
		"onready",
		"static",
		"var",
		// control flow
		"break",
		"continue",
		"if",
		"elif",
		"else",
		"for",
		"pass",
		"return",
		"match",
		"while",
		"remote",
		"sync",
		"master",
		"puppet",
		"slave",
		"remotesync",
		"mastersync",
		"puppetsync",
		nullptr
	};

	const char **w = _reserved_words;

	while (*w) {
		p_words->push_back(*w);
		w++;
	}

	for (int i = 0; i < CScriptFunctions::FUNC_MAX; i++) {
		p_words->push_back(CScriptFunctions::get_func_name(CScriptFunctions::Function(i)));
	}
}

bool CScriptLanguage::is_control_flow_keyword(String p_keyword) const {
	return p_keyword == "break" ||
			p_keyword == "continue" ||
			p_keyword == "elif" ||
			p_keyword == "else" ||
			p_keyword == "if" ||
			p_keyword == "for" ||
			p_keyword == "match" ||
			p_keyword == "pass" ||
			p_keyword == "return" ||
			p_keyword == "while";
}

bool CScriptLanguage::handles_global_class_type(const String &p_type) const {
	return p_type == "CScript";
}

String CScriptLanguage::get_global_class_name(const String &p_path, String *r_base_type, String *r_icon_path) const {
	PoolVector<uint8_t> sourcef;
	Error err;
	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ, &err);
	if (err) {
		return String();
	}

	String source = f->get_as_utf8_string();

	CScriptParser parser;
	parser.parse(source, p_path.get_base_dir(), true, p_path, false, nullptr, true);

	if (parser.get_parse_tree() && parser.get_parse_tree()->type == CScriptParser::Node::TYPE_CLASS) {
		const CScriptParser::ClassNode *c = static_cast<const CScriptParser::ClassNode *>(parser.get_parse_tree());
		if (r_icon_path) {
			if (c->icon_path.empty() || c->icon_path.is_abs_path()) {
				*r_icon_path = c->icon_path;
			} else if (c->icon_path.is_rel_path()) {
				*r_icon_path = p_path.get_base_dir().plus_file(c->icon_path).simplify_path();
			}
		}
		if (r_base_type) {
			const CScriptParser::ClassNode *subclass = c;
			String path = p_path;
			CScriptParser subparser;
			while (subclass) {
				if (subclass->extends_used) {
					if (subclass->extends_file) {
						if (subclass->extends_class.size() == 0) {
							get_global_class_name(subclass->extends_file, r_base_type);
							subclass = nullptr;
							break;
						} else {
							Vector<StringName> extend_classes = subclass->extends_class;

							FileAccessRef subfile = FileAccess::open(subclass->extends_file, FileAccess::READ);
							if (!subfile) {
								break;
							}
							String subsource = subfile->get_as_utf8_string();

							if (subsource.empty()) {
								break;
							}
							String subpath = subclass->extends_file;
							if (subpath.is_rel_path()) {
								subpath = path.get_base_dir().plus_file(subpath).simplify_path();
							}

							if (OK != subparser.parse(subsource, subpath.get_base_dir(), true, subpath, false, nullptr, true)) {
								break;
							}
							path = subpath;
							if (!subparser.get_parse_tree() || subparser.get_parse_tree()->type != CScriptParser::Node::TYPE_CLASS) {
								break;
							}
							subclass = static_cast<const CScriptParser::ClassNode *>(subparser.get_parse_tree());

							while (extend_classes.size() > 0) {
								bool found = false;
								for (int i = 0; i < subclass->subclasses.size(); i++) {
									const CScriptParser::ClassNode *inner_class = subclass->subclasses[i];
									if (inner_class->name == extend_classes[0]) {
										extend_classes.remove(0);
										found = true;
										subclass = inner_class;
										break;
									}
								}
								if (!found) {
									subclass = nullptr;
									break;
								}
							}
						}
					} else if (subclass->extends_class.size() == 1) {
						*r_base_type = subclass->extends_class[0];
						subclass = nullptr;
					} else {
						break;
					}
				} else {
					*r_base_type = "Reference";
					subclass = nullptr;
				}
			}
		}
		return c->name;
	}

	return String();
}

#ifdef DEBUG_ENABLED
String CScriptWarning::get_message() const {
#define CHECK_SYMBOLS(m_amount) ERR_FAIL_COND_V(symbols.size() < m_amount, String());

	switch (code) {
		case UNASSIGNED_VARIABLE_OP_ASSIGN: {
			CHECK_SYMBOLS(1);
			return "Using assignment with operation but the variable '" + symbols[0] + "' was not previously assigned a value.";
		} break;
		case UNASSIGNED_VARIABLE: {
			CHECK_SYMBOLS(1);
			return "The variable '" + symbols[0] + "' was used but never assigned a value.";
		} break;
		case UNUSED_VARIABLE: {
			CHECK_SYMBOLS(1);
			return "The local variable '" + symbols[0] + "' is declared but never used in the block. If this is intended, prefix it with an underscore: '_" + symbols[0] + "'";
		} break;
		case SHADOWED_VARIABLE: {
			CHECK_SYMBOLS(2);
			return "The local variable '" + symbols[0] + "' is shadowing an already-defined variable at line " + symbols[1] + ".";
		} break;
		case UNUSED_CLASS_VARIABLE: {
			CHECK_SYMBOLS(1);
			return "The class variable '" + symbols[0] + "' is declared but never used in the script.";
		} break;
		case UNUSED_ARGUMENT: {
			CHECK_SYMBOLS(2);
			return "The argument '" + symbols[1] + "' is never used in the function '" + symbols[0] + "'. If this is intended, prefix it with an underscore: '_" + symbols[1] + "'";
		} break;
		case UNREACHABLE_CODE: {
			CHECK_SYMBOLS(1);
			return "Unreachable code (statement after return) in function '" + symbols[0] + "()'.";
		} break;
		case STANDALONE_EXPRESSION: {
			return "Standalone expression (the line has no effect).";
		} break;
		case VOID_ASSIGNMENT: {
			CHECK_SYMBOLS(1);
			return "Assignment operation, but the function '" + symbols[0] + "()' returns void.";
		} break;
		case NARROWING_CONVERSION: {
			return "Narrowing conversion (float is converted to int and loses precision).";
		} break;
		case FUNCTION_MAY_YIELD: {
			CHECK_SYMBOLS(1);
			return "Assigned variable is typed but the function '" + symbols[0] + "()' may yield and return a CScriptFunctionState instead.";
		} break;
		case VARIABLE_CONFLICTS_FUNCTION: {
			CHECK_SYMBOLS(1);
			return "Variable declaration of '" + symbols[0] + "' conflicts with a function of the same name.";
		} break;
		case FUNCTION_CONFLICTS_VARIABLE: {
			CHECK_SYMBOLS(1);
			return "Function declaration of '" + symbols[0] + "()' conflicts with a variable of the same name.";
		} break;
		case FUNCTION_CONFLICTS_CONSTANT: {
			CHECK_SYMBOLS(1);
			return "Function declaration of '" + symbols[0] + "()' conflicts with a constant of the same name.";
		} break;
		case INCOMPATIBLE_TERNARY: {
			return "Values of the ternary conditional are not mutually compatible.";
		} break;
		case UNUSED_SIGNAL: {
			CHECK_SYMBOLS(1);
			return "The signal '" + symbols[0] + "' is declared but never emitted.";
		} break;
		case RETURN_VALUE_DISCARDED: {
			CHECK_SYMBOLS(1);
			return "The function '" + symbols[0] + "()' returns a value, but this value is never used.";
		} break;
		case PROPERTY_USED_AS_FUNCTION: {
			CHECK_SYMBOLS(2);
			return "The method '" + symbols[0] + "()' was not found in base '" + symbols[1] + "' but there's a property with the same name. Did you mean to access it?";
		} break;
		case CONSTANT_USED_AS_FUNCTION: {
			CHECK_SYMBOLS(2);
			return "The method '" + symbols[0] + "()' was not found in base '" + symbols[1] + "' but there's a constant with the same name. Did you mean to access it?";
		} break;
		case FUNCTION_USED_AS_PROPERTY: {
			CHECK_SYMBOLS(2);
			return "The property '" + symbols[0] + "' was not found in base '" + symbols[1] + "' but there's a method with the same name. Did you mean to call it?";
		} break;
		case INTEGER_DIVISION: {
			return "Integer division, decimal part will be discarded.";
		} break;
		case UNSAFE_PROPERTY_ACCESS: {
			CHECK_SYMBOLS(2);
			return "The property '" + symbols[0] + "' is not present on the inferred type '" + symbols[1] + "' (but may be present on a subtype).";
		} break;
		case UNSAFE_METHOD_ACCESS: {
			CHECK_SYMBOLS(2);
			return "The method '" + symbols[0] + "' is not present on the inferred type '" + symbols[1] + "' (but may be present on a subtype).";
		} break;
		case UNSAFE_CAST: {
			CHECK_SYMBOLS(1);
			return "The value is cast to '" + symbols[0] + "' but has an unknown type.";
		} break;
		case UNSAFE_CALL_ARGUMENT: {
			CHECK_SYMBOLS(4);
			return "The argument '" + symbols[0] + "' of the function '" + symbols[1] + "' requires a the subtype '" + symbols[2] + "' but the supertype '" + symbols[3] + "' was provided";
		} break;
		case DEPRECATED_KEYWORD: {
			CHECK_SYMBOLS(2);
			return "The '" + symbols[0] + "' keyword is deprecated and will be removed in a future release, please replace its uses by '" + symbols[1] + "'.";
		} break;
		case STANDALONE_TERNARY: {
			return "Standalone ternary conditional operator: the return value is being discarded.";
		}
		case EXPORT_HINT_TYPE_MISTMATCH: {
			CHECK_SYMBOLS(2);
			return vformat("The type of the default value (%s) doesn't match the type of the export hint (%s). The type won't be coerced.", symbols[0], symbols[1]);
		}
		case WARNING_MAX:
			break; // Can't happen, but silences warning
	}
	ERR_FAIL_V_MSG(String(), "Invalid CScript warning code: " + get_name_from_code(code) + ".");

#undef CHECK_SYMBOLS
}

String CScriptWarning::get_name() const {
	return get_name_from_code(code);
}

String CScriptWarning::get_name_from_code(Code p_code) {
	ERR_FAIL_COND_V(p_code < 0 || p_code >= WARNING_MAX, String());

	static const char *names[] = {
		"UNASSIGNED_VARIABLE",
		"UNASSIGNED_VARIABLE_OP_ASSIGN",
		"UNUSED_VARIABLE",
		"SHADOWED_VARIABLE",
		"UNUSED_CLASS_VARIABLE",
		"UNUSED_ARGUMENT",
		"UNREACHABLE_CODE",
		"STANDALONE_EXPRESSION",
		"VOID_ASSIGNMENT",
		"NARROWING_CONVERSION",
		"FUNCTION_MAY_YIELD",
		"VARIABLE_CONFLICTS_FUNCTION",
		"FUNCTION_CONFLICTS_VARIABLE",
		"FUNCTION_CONFLICTS_CONSTANT",
		"INCOMPATIBLE_TERNARY",
		"UNUSED_SIGNAL",
		"RETURN_VALUE_DISCARDED",
		"PROPERTY_USED_AS_FUNCTION",
		"CONSTANT_USED_AS_FUNCTION",
		"FUNCTION_USED_AS_PROPERTY",
		"INTEGER_DIVISION",
		"UNSAFE_PROPERTY_ACCESS",
		"UNSAFE_METHOD_ACCESS",
		"UNSAFE_CAST",
		"UNSAFE_CALL_ARGUMENT",
		"DEPRECATED_KEYWORD",
		"STANDALONE_TERNARY",
		"EXPORT_HINT_TYPE_MISTMATCH",
		nullptr
	};

	return names[(int)p_code];
}

CScriptWarning::Code CScriptWarning::get_code_from_name(const String &p_name) {
	for (int i = 0; i < WARNING_MAX; i++) {
		if (get_name_from_code((Code)i) == p_name) {
			return (Code)i;
		}
	}

	ERR_FAIL_V_MSG(WARNING_MAX, "Invalid CScript warning name: " + p_name);
}

#endif // DEBUG_ENABLED

CScriptLanguage::CScriptLanguage() {
	calls = 0;
	ERR_FAIL_COND(singleton);
	singleton = this;
	strings._init = StaticCString::create("_init");
	strings._notification = StaticCString::create("_notification");
	strings._set = StaticCString::create("_set");
	strings._get = StaticCString::create("_get");
	strings._get_property_list = StaticCString::create("_get_property_list");
	strings._script_source = StaticCString::create("script/source");
	_debug_parse_err_line = -1;
	_debug_parse_err_file = "";

	profiling = false;
	script_frame_time = 0;

	_debug_call_stack_pos = 0;
	int dmcs = GLOBAL_DEF("debug/settings/cscript/max_call_stack", 1024);
	ProjectSettings::get_singleton()->set_custom_property_info("debug/settings/cscript/max_call_stack", PropertyInfo(Variant::INT, "debug/settings/cscript/max_call_stack", PROPERTY_HINT_RANGE, "1024,4096,1,or_greater")); //minimum is 1024

	if (ScriptDebugger::get_singleton()) {
		//debugging enabled!

		_debug_max_call_stack = dmcs;
		_call_stack = memnew_arr(CallLevel, _debug_max_call_stack + 1);

	} else {
		_debug_max_call_stack = 0;
		_call_stack = nullptr;
	}

#ifdef DEBUG_ENABLED
	GLOBAL_DEF("debug/cscript/warnings/enable", true);
	GLOBAL_DEF("debug/cscript/warnings/treat_warnings_as_errors", false);
	GLOBAL_DEF("debug/cscript/warnings/exclude_addons", true);
	GLOBAL_DEF("debug/cscript/completion/autocomplete_setters_and_getters", false);
	for (int i = 0; i < (int)CScriptWarning::WARNING_MAX; i++) {
		String warning = CScriptWarning::get_name_from_code((CScriptWarning::Code)i).to_lower();
		bool default_enabled = !warning.begins_with("unsafe_") && i != CScriptWarning::UNUSED_CLASS_VARIABLE;
		GLOBAL_DEF("debug/cscript/warnings/" + warning, default_enabled);
	}
#endif // DEBUG_ENABLED
}

CScriptLanguage::~CScriptLanguage() {
	if (_call_stack) {
		memdelete_arr(_call_stack);
	}

	// Clear dependencies between scripts, to ensure cyclic references are broken (to avoid leaks at exit).
	SelfList<CScript> *s = script_list.first();
	while (s) {
		CScript *script = s->self();
		// This ensures the current script is not released before we can check what's the next one
		// in the list (we can't get the next upfront because we don't know if the reference breaking
		// will cause it -or any other after it, for that matter- to be released so the next one
		// is not the same as before).
		script->reference();

		for (Map<StringName, CScriptFunction *>::Element *E = script->member_functions.front(); E; E = E->next()) {
			CScriptFunction *func = E->get();
			for (int i = 0; i < func->argument_types.size(); i++) {
				func->argument_types.write[i].script_type_ref = Ref<Script>();
			}
			func->return_type.script_type_ref = Ref<Script>();
		}
		for (Map<StringName, CScript::MemberInfo>::Element *E = script->member_indices.front(); E; E = E->next()) {
			E->get().data_type.script_type_ref = Ref<Script>();
		}

		s = s->next();
		script->unreference();
	}

	singleton = nullptr;
}

void CScriptLanguage::add_orphan_subclass(const String &p_qualified_name, const ObjectID &p_subclass) {
	orphan_subclasses[p_qualified_name] = p_subclass;
}

Ref<CScript> CScriptLanguage::get_orphan_subclass(const String &p_qualified_name) {
	Map<String, ObjectID>::Element *orphan_subclass_element = orphan_subclasses.find(p_qualified_name);
	if (!orphan_subclass_element) {
		return Ref<CScript>();
	}
	ObjectID orphan_subclass = orphan_subclass_element->get();
	Object *obj = ObjectDB::get_instance(orphan_subclass);
	orphan_subclasses.erase(orphan_subclass_element);
	if (!obj) {
		return Ref<CScript>();
	}
	return Ref<CScript>(Object::cast_to<CScript>(obj));
}

/*************** RESOURCE ***************/

RES ResourceFormatLoaderCScript::load(const String &p_path, const String &p_original_path, Error *r_error) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	CScript *script = memnew(CScript);

	Ref<CScript> scriptres(script);

	if (p_path.ends_with(".cppse") || p_path.ends_with(".cppsc")) {
		script->set_script_path(p_original_path); // script needs this.
		script->set_path(p_original_path, true);
		Error err = script->load_byte_code(p_path);
		ERR_FAIL_COND_V_MSG(err != OK, RES(), "Cannot load byte code from file '" + p_path + "'.");

	} else {
		Error err = script->load_source_code(p_path);
		ERR_FAIL_COND_V_MSG(err != OK, RES(), "Cannot load source code from file '" + p_path + "'.");

		script->set_script_path(p_original_path); // script needs this.
		script->set_path(p_original_path, true);

		script->reload();
	}
	if (r_error) {
		*r_error = OK;
	}

	return scriptres;
}

void ResourceFormatLoaderCScript::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("cpps");
	p_extensions->push_back("cppsc");
	p_extensions->push_back("cppse");
}

bool ResourceFormatLoaderCScript::handles_type(const String &p_type) const {
	return (p_type == "Script" || p_type == "CScript");
}

String ResourceFormatLoaderCScript::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "cpps" || el == "cppsc" || el == "cppse") {
		return "CScript";
	}
	return "";
}

void ResourceFormatLoaderCScript::get_dependencies(const String &p_path, List<String> *p_dependencies, bool p_add_types) {
	FileAccessRef file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_MSG(!file, "Cannot open file '" + p_path + "'.");

	String source = file->get_as_utf8_string();
	if (source.empty()) {
		return;
	}

	CScriptParser parser;
	if (OK != parser.parse(source, p_path.get_base_dir(), true, p_path, false, nullptr, true)) {
		return;
	}

	for (const List<String>::Element *E = parser.get_dependencies().front(); E; E = E->next()) {
		p_dependencies->push_back(E->get());
	}
}

Error ResourceFormatSaverCScript::save(const String &p_path, const RES &p_resource, uint32_t p_flags) {
	Ref<CScript> sqscr = p_resource;
	ERR_FAIL_COND_V(sqscr.is_null(), ERR_INVALID_PARAMETER);

	String source = sqscr->get_source_code();

	Error err;
	FileAccess *file = FileAccess::open(p_path, FileAccess::WRITE, &err);

	ERR_FAIL_COND_V_MSG(err, err, "Cannot save CScript file '" + p_path + "'.");

	file->store_string(source);
	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
		memdelete(file);
		return ERR_CANT_CREATE;
	}
	file->close();
	memdelete(file);

	if (ScriptServer::is_reload_scripts_on_save_enabled()) {
		CScriptLanguage::get_singleton()->reload_tool_script(p_resource, false);
	}

	return OK;
}

void ResourceFormatSaverCScript::get_recognized_extensions(const RES &p_resource, List<String> *p_extensions) const {
	if (Object::cast_to<CScript>(*p_resource)) {
		p_extensions->push_back("cpps");
	}
}
bool ResourceFormatSaverCScript::recognize(const RES &p_resource) const {
	return Object::cast_to<CScript>(*p_resource) != nullptr;
}
