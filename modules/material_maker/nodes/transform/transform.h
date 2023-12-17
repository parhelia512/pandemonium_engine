#ifndef MM_TRANSFORM_H
#define MM_TRANSFORM_H

/*************************************************************************/
/*  transform.h                                                          */
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

#include "../mm_node.h"
#include "../mm_node_universal_property.h"

class MMTransform : public MMNode {
	GDCLASS(MMTransform, MMNode);

public:
	Ref<MMNodeUniversalProperty> get_image();
	void set_image(const Ref<MMNodeUniversalProperty> &val);

	Ref<MMNodeUniversalProperty> get_input();
	void set_input(const Ref<MMNodeUniversalProperty> &val);

	Ref<MMNodeUniversalProperty> get_translate_x();
	void set_translate_x(const Ref<MMNodeUniversalProperty> &val);

	Ref<MMNodeUniversalProperty> get_translate_y();
	void set_translate_y(const Ref<MMNodeUniversalProperty> &val);

	Ref<MMNodeUniversalProperty> get_rotate();
	void set_rotate(const Ref<MMNodeUniversalProperty> &val);

	Ref<MMNodeUniversalProperty> get_scale_x();
	void set_scale_x(const Ref<MMNodeUniversalProperty> &val);

	Ref<MMNodeUniversalProperty> get_scale_y();
	void set_scale_y(const Ref<MMNodeUniversalProperty> &val);

	int get_mode() const;
	void set_mode(const int val);

	void _init_properties();
	void _register_methods(MMGraphNode *mm_graph_node);
	void _render(const Ref<MMMaterial> &material);
	Color _get_value_for(const Vector2 &uv, const int pseed);

	MMTransform();
	~MMTransform();

protected:
	static void _bind_methods();

	Ref<MMNodeUniversalProperty> image;
	Ref<MMNodeUniversalProperty> input;
	Ref<MMNodeUniversalProperty> translate_x;
	Ref<MMNodeUniversalProperty> translate_y;
	Ref<MMNodeUniversalProperty> rotate;
	Ref<MMNodeUniversalProperty> scale_x;
	Ref<MMNodeUniversalProperty> scale_y;
	//export(int, "Clamp,Repeat,Extend")
	int mode;
};

#endif
