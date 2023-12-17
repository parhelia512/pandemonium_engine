#ifndef TEXTURE_BUTTON_H
#define TEXTURE_BUTTON_H

/*************************************************************************/
/*  texture_button.h                                                     */
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

#include "core/object/reference.h"
#include "scene/gui/base_button.h"

class BitMap;

class TextureButton : public BaseButton {
	GDCLASS(TextureButton, BaseButton);

public:
	enum StretchMode {
		STRETCH_SCALE,
		STRETCH_TILE,
		STRETCH_KEEP,
		STRETCH_KEEP_CENTERED,
		STRETCH_KEEP_ASPECT,
		STRETCH_KEEP_ASPECT_CENTERED,
		STRETCH_KEEP_ASPECT_COVERED,
	};

private:
	Ref<Texture> normal;
	Ref<Texture> pressed;
	Ref<Texture> hover;
	Ref<Texture> disabled;
	Ref<Texture> focused;
	Ref<BitMap> click_mask;
	bool expand;
	StretchMode stretch_mode;

	Rect2 _texture_region;
	Rect2 _position_rect;
	bool _tile;

	bool hflip;
	bool vflip;

	void _set_texture(Ref<Texture> *p_destination, const Ref<Texture> &p_texture);
	void _texture_changed();

protected:
	virtual Size2 get_minimum_size() const;
	virtual bool has_point(const Point2 &p_point) const;
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_normal_texture(const Ref<Texture> &p_normal);
	void set_pressed_texture(const Ref<Texture> &p_pressed);
	void set_hover_texture(const Ref<Texture> &p_hover);
	void set_disabled_texture(const Ref<Texture> &p_disabled);
	void set_focused_texture(const Ref<Texture> &p_focused);
	void set_click_mask(const Ref<BitMap> &p_click_mask);

	Ref<Texture> get_normal_texture() const;
	Ref<Texture> get_pressed_texture() const;
	Ref<Texture> get_hover_texture() const;
	Ref<Texture> get_disabled_texture() const;
	Ref<Texture> get_focused_texture() const;
	Ref<BitMap> get_click_mask() const;

	bool get_expand() const;
	void set_expand(bool p_expand);

	void set_stretch_mode(StretchMode p_stretch_mode);
	StretchMode get_stretch_mode() const;

	void set_flip_h(bool p_flip);
	bool is_flipped_h() const;

	void set_flip_v(bool p_flip);
	bool is_flipped_v() const;

	TextureButton();
};

VARIANT_ENUM_CAST(TextureButton::StretchMode);
#endif // TEXTURE_BUTTON_H
