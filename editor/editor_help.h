#ifndef EDITOR_HELP_H
#define EDITOR_HELP_H

/*************************************************************************/
/*  editor_help.h                                                        */
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

#include "scene/gui/box_container.h"
#include "scene/gui/panel_container.h"

#include "editor/doc/doc_data.h"
#include "scene/gui/rich_text_label.h"

#include "core/containers/rb_map.h"
#include "core/containers/vector.h"
#include "core/error/error_list.h"
#include "core/math/color.h"
#include "core/object/object.h"
#include "core/object/reference.h"
#include "core/string/ustring.h"

class ConfirmationDialog;
class HSplitContainer;
class InputEvent;
class Label;
class LineEdit;
class TextureButton;
class ToolButton;
template <class F, class S>
struct Pair;

class FindBar : public HBoxContainer {
	GDCLASS(FindBar, HBoxContainer);

	LineEdit *search_text;
	ToolButton *find_prev;
	ToolButton *find_next;
	Label *matches_label;
	TextureButton *hide_button;
	String prev_search;

	RichTextLabel *rich_text_label;

	int results_count;

	void _hide_bar();

	void _search_text_changed(const String &p_text);
	void _search_text_entered(const String &p_text);

	void _update_results_count();
	void _update_matches_label();

protected:
	void _notification(int p_what);
	void _unhandled_input(const Ref<InputEvent> &p_event);

	bool _search(bool p_search_previous = false);

	static void _bind_methods();

public:
	void set_rich_text_label(RichTextLabel *p_rich_text_label);

	void popup_search();

	bool search_prev();
	bool search_next();

	FindBar();
};

class EditorHelp : public VBoxContainer {
	GDCLASS(EditorHelp, VBoxContainer);

	enum Page {

		PAGE_CLASS_LIST,
		PAGE_CLASS_DESC,
		PAGE_CLASS_PREV,
		PAGE_CLASS_NEXT,
		PAGE_SEARCH,
		CLASS_SEARCH,

	};

	bool select_locked;

	String prev_search;

	String edited_class;

	Vector<Pair<String, int>> section_line;
	RBMap<String, int> method_line;
	RBMap<String, int> signal_line;
	RBMap<String, int> property_line;
	RBMap<String, int> theme_property_line;
	RBMap<String, int> constant_line;
	RBMap<String, int> enum_line;
	RBMap<String, RBMap<String, int>> enum_values_line;
	int description_line;

	RichTextLabel *class_desc;
	HSplitContainer *h_split;
	static DocData *doc;

	ConfirmationDialog *search_dialog;
	LineEdit *search;
	FindBar *find_bar;

	String base_path;

	Color title_color;
	Color text_color;
	Color headline_color;
	Color base_type_color;
	Color type_color;
	Color comment_color;
	Color symbol_color;
	Color value_color;
	Color qualifier_color;

	void _init_colors();
	void _help_callback(const String &p_topic);

	void _add_text(const String &p_bbcode);
	bool scroll_locked;

	//void _button_pressed(int p_idx);
	void _add_type(const String &p_type, const String &p_enum = String());
	void _add_type_icon(const String &p_type, int p_size = 0);
	void _add_method(const DocData::MethodDoc &p_method, bool p_overview = true);

	void _add_bulletpoint();

	void _class_list_select(const String &p_select);
	void _class_desc_select(const String &p_select);
	void _class_desc_input(const Ref<InputEvent> &p_input);
	void _class_desc_resized();

	Error _goto_desc(const String &p_class, int p_vscr = -1);
	//void _update_history_buttons();
	void _update_doc();

	void _request_help(const String &p_string);
	void _search(bool p_search_previous = false);

	void _unhandled_key_input(const Ref<InputEvent> &p_ev);

	String _fix_constant(const String &p_constant) const;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	static void generate_doc();
	static DocData *get_doc_data() { return doc; }

	void go_to_help(const String &p_help);
	void go_to_class(const String &p_class, int p_scroll = 0);

	Vector<Pair<String, int>> get_sections();
	void scroll_to_section(int p_section_index);

	void popup_search();
	void search_again(bool p_search_previous = false);

	String get_class();

	void set_focused() { class_desc->grab_focus(); }

	int get_scroll() const;
	void set_scroll(int p_scroll);

	EditorHelp();
	~EditorHelp();
};

class EditorHelpBit : public PanelContainer {
	GDCLASS(EditorHelpBit, PanelContainer);

	RichTextLabel *rich_text;
	void _go_to_help(String p_what);
	void _meta_clicked(String p_select);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	RichTextLabel *get_rich_text() { return rich_text; }
	void set_text(const String &p_text);
	EditorHelpBit();
};

#endif // EDITOR_HELP_H
