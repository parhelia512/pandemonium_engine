/*************************************************************************/
/*  radial_gradient.cpp                                                  */
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

#include "radial_gradient.h"

#include "../../algos/mm_algos.h"
#include "../../editor/mm_graph_node.h"
#include "../mm_material.h"

Ref<MMNodeUniversalProperty> MMRadialGradient::get_image() {
	return image;
}

void MMRadialGradient::set_image(const Ref<MMNodeUniversalProperty> &val) {
	image = val;
}

float MMRadialGradient::get_repeat() const {
	return repeat;
}

void MMRadialGradient::set_repeat(const float val) {
	repeat = val;
	set_dirty(true);
}

void MMRadialGradient::_init_properties() {
	if (!image.is_valid()) {
		image.instance();
		image->set_default_type(MMNodeUniversalProperty::DEFAULT_TYPE_IMAGE);
	}

	image->set_output_slot_type(MMNodeUniversalProperty::SLOT_TYPE_IMAGE);
	register_output_property(image);
}

void MMRadialGradient::_register_methods(MMGraphNode *mm_graph_node) {
	mm_graph_node->add_slot_texture_universal(image);
	mm_graph_node->add_slot_float("get_repeat", "set_repeat", "repeat");
	mm_graph_node->add_slot_gradient();
}

void MMRadialGradient::_render(const Ref<MMMaterial> &material) {
	Ref<Image> img = render_image(material);
	image->set_value(img);
}

Color MMRadialGradient::_get_value_for(const Vector2 &uv, const int pseed) {
	if (interpolation_type == 0) {
		return MMAlgos::radial_gradient_type_1(uv, repeat, points);
	} else if (interpolation_type == 1) {
		return MMAlgos::radial_gradient_type_2(uv, repeat, points);
	} else if (interpolation_type == 2) {
		return MMAlgos::radial_gradient_type_3(uv, repeat, points);
	} else if (interpolation_type == 3) {
		return MMAlgos::radial_gradient_type_4(uv, repeat, points);
	}

	return Color(1, 1, 1, 1);
}

Color MMRadialGradient::_get_gradient_color(const float x) {
	if (interpolation_type == 0) {
		return MMAlgos::gradient_type_1(x, points);
	} else if (interpolation_type == 1) {
		return MMAlgos::gradient_type_2(x, points);
	} else if (interpolation_type == 2) {
		return MMAlgos::gradient_type_3(x, points);
	} else if (interpolation_type == 3) {
		return MMAlgos::gradient_type_4(x, points);
	}

	return Color(1, 1, 1, 1);
}

MMRadialGradient::MMRadialGradient() {
	repeat = 1;
}

MMRadialGradient::~MMRadialGradient() {
}

void MMRadialGradient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_image"), &MMRadialGradient::get_image);
	ClassDB::bind_method(D_METHOD("set_image", "value"), &MMRadialGradient::set_image);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "image", PROPERTY_HINT_RESOURCE_TYPE, "MMNodeUniversalProperty"), "set_image", "get_image");

	ClassDB::bind_method(D_METHOD("get_repeat"), &MMRadialGradient::get_repeat);
	ClassDB::bind_method(D_METHOD("set_repeat", "value"), &MMRadialGradient::set_repeat);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "repeat"), "set_repeat", "get_repeat");
}
