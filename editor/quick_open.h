#ifndef EDITOR_QUICK_OPEN_H
#define EDITOR_QUICK_OPEN_H
/*************************************************************************/
/*  quick_open.h                                                         */
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

#include "scene/gui/dialogs.h"

#include "core/pair.h"
#include "core/reference.h"
#include "core/string_name.h"
#include "core/ustring.h"
#include "core/containers/vector.h"

class EditorFileSystemDirectory;
class InputEvent;
class LineEdit;
class Texture;
class Tree;
template <class F, class S> struct Pair;

class EditorQuickOpen : public ConfirmationDialog {
	GDCLASS(EditorQuickOpen, ConfirmationDialog);

	LineEdit *search_box;
	Tree *search_options;
	StringName base_type;
	StringName ei;
	StringName ot;

	void _update_search();

	void _sbox_input(const Ref<InputEvent> &p_ie);
	void _parse_fs(EditorFileSystemDirectory *efsd, Vector<Pair<String, Ref<Texture>>> &list);
	Vector<Pair<String, Ref<Texture>>> _sort_fs(Vector<Pair<String, Ref<Texture>>> &list);
	float _path_cmp(String search, String path) const;

	void _confirmed();
	void _text_changed(const String &p_newtext);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	StringName get_base_type() const;

	String get_selected() const;
	Vector<String> get_selected_files() const;

	void popup_dialog(const StringName &p_base, bool p_enable_multi = false, bool p_dontclear = false);
	EditorQuickOpen();
};

#endif // EDITOR_QUICK_OPEN_H
