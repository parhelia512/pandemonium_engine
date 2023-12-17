/*************************************************************************/
/*  shape_cast_2d.cpp                                                    */
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

#include "shape_cast_2d.h"

#include "core/config/engine.h"
#include "core/core_string_names.h"
#include "scene/2d/collision_object_2d.h"
#include "scene/2d/physics_body_2d.h"
#include "scene/resources/shapes_2d/circle_shape_2d.h"
#include "servers/physics_2d_server.h"

void ShapeCast2D::set_target_position(const Vector2 &p_point) {
	target_position = p_point;
	if (is_inside_tree() && (Engine::get_singleton()->is_editor_hint() || get_tree()->is_debugging_collisions_hint())) {
		update();
	}
}

Vector2 ShapeCast2D::get_target_position() const {
	return target_position;
}

void ShapeCast2D::set_margin(real_t p_margin) {
	margin = p_margin;
}

real_t ShapeCast2D::get_margin() const {
	return margin;
}

void ShapeCast2D::set_max_results(int p_max_results) {
	max_results = p_max_results;
}

int ShapeCast2D::get_max_results() const {
	return max_results;
}

void ShapeCast2D::set_collision_mask(uint32_t p_mask) {
	collision_mask = p_mask;
}

uint32_t ShapeCast2D::get_collision_mask() const {
	return collision_mask;
}

void ShapeCast2D::set_collision_mask_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1, "Collision layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_MSG(p_layer_number > 32, "Collision layer number must be between 1 and 32 inclusive.");
	uint32_t mask = get_collision_mask();
	if (p_value) {
		mask |= 1 << (p_layer_number - 1);
	} else {
		mask &= ~(1 << (p_layer_number - 1));
	}
	set_collision_mask(mask);
}

bool ShapeCast2D::get_collision_mask_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Collision layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_V_MSG(p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
	return get_collision_mask() & (1 << (p_layer_number - 1));
}

int ShapeCast2D::get_collision_count() const {
	return result.size();
}

bool ShapeCast2D::is_colliding() const {
	return collided;
}

Object *ShapeCast2D::get_collider(int p_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_idx, result.size(), nullptr, "No collider found.");

	if (result[p_idx].collider_id == 0) {
		return nullptr;
	}
	return ObjectDB::get_instance(result[p_idx].collider_id);
}

RID ShapeCast2D::get_collider_rid(int p_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_idx, result.size(), RID(), "No collider RID found.");
	return result[p_idx].rid;
}

int ShapeCast2D::get_collider_shape(int p_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_idx, result.size(), -1, "No collider shape found.");
	return result[p_idx].shape;
}

Vector2 ShapeCast2D::get_collision_point(int p_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_idx, result.size(), Vector2(), "No collision point found.");
	return result[p_idx].point;
}

Vector2 ShapeCast2D::get_collision_normal(int p_idx) const {
	ERR_FAIL_INDEX_V_MSG(p_idx, result.size(), Vector2(), "No collision normal found.");
	return result[p_idx].normal;
}

real_t ShapeCast2D::get_closest_collision_safe_fraction() const {
	return collision_safe_fraction;
}

real_t ShapeCast2D::get_closest_collision_unsafe_fraction() const {
	return collision_unsafe_fraction;
}

void ShapeCast2D::set_enabled(bool p_enabled) {
	enabled = p_enabled;
	update();
	if (is_inside_tree() && !Engine::get_singleton()->is_editor_hint()) {
		set_physics_process_internal(p_enabled);
	}
	if (!p_enabled) {
		collided = false;
	}
}

bool ShapeCast2D::is_enabled() const {
	return enabled;
}

void ShapeCast2D::set_shape(const Ref<Shape2D> &p_shape) {
	shape = p_shape;
	if (p_shape.is_valid()) {
		shape->connect(CoreStringNames::get_singleton()->changed, this, "_redraw_shape");
		shape_rid = shape->get_rid();
	}
	update_configuration_warning();
	update();
}

Ref<Shape2D> ShapeCast2D::get_shape() const {
	return shape;
}

void ShapeCast2D::set_exclude_parent_body(bool p_exclude_parent_body) {
	if (exclude_parent_body == p_exclude_parent_body) {
		return;
	}
	exclude_parent_body = p_exclude_parent_body;

	if (!is_inside_tree()) {
		return;
	}
	if (Object::cast_to<CollisionObject2D>(get_parent())) {
		if (exclude_parent_body) {
			exclude.insert(Object::cast_to<CollisionObject2D>(get_parent())->get_rid());
		} else {
			exclude.erase(Object::cast_to<CollisionObject2D>(get_parent())->get_rid());
		}
	}
}

bool ShapeCast2D::get_exclude_parent_body() const {
	return exclude_parent_body;
}

void ShapeCast2D::_redraw_shape() {
	update();
}

void ShapeCast2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (enabled && !Engine::get_singleton()->is_editor_hint()) {
				set_physics_process_internal(true);
			} else {
				set_physics_process_internal(false);
			}
			if (Object::cast_to<CollisionObject2D>(get_parent())) {
				if (exclude_parent_body) {
					exclude.insert(Object::cast_to<CollisionObject2D>(get_parent())->get_rid());
				} else {
					exclude.erase(Object::cast_to<CollisionObject2D>(get_parent())->get_rid());
				}
			}
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (enabled) {
				set_physics_process_internal(false);
			}
		} break;

		case NOTIFICATION_DRAW: {
#ifdef TOOLS_ENABLED
			ERR_FAIL_COND(!is_inside_tree());
			if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
				break;
			}
			if (shape.is_null()) {
				break;
			}
			Color draw_col = get_tree()->get_debug_collisions_color();
			if (!enabled) {
				float g = draw_col.get_v();
				draw_col.r = g;
				draw_col.g = g;
				draw_col.b = g;
			}
			// Draw continuous chain of shapes along the cast.
			const int steps = MAX(2, target_position.length() / shape->get_rect().get_size().length() * 4);
			for (int i = 0; i <= steps; ++i) {
				Vector2 t = (real_t(i) / steps) * target_position;
				draw_set_transform(t, 0.0, Size2(1, 1));
				shape->draw(get_canvas_item(), draw_col);
			}
			draw_set_transform(Vector2(), 0.0, Size2(1, 1));

			// Draw an arrow indicating where the ShapeCast is pointing to.
			if (target_position != Vector2()) {
				Transform2D xf;
				xf.rotate(target_position.angle());
				xf.translate(Vector2(target_position.length(), 0));

				draw_line(Vector2(), target_position, draw_col, 2);

				float tsize = 8;

				Vector<Vector2> pts;
				pts.push_back(xf.xform(Vector2(tsize, 0)));
				pts.push_back(xf.xform(Vector2(0, Math_SQRT12 * tsize)));
				pts.push_back(xf.xform(Vector2(0, -Math_SQRT12 * tsize)));

				Vector<Color> cols;
				cols.push_back(draw_col);
				cols.push_back(draw_col);
				cols.push_back(draw_col);

				draw_primitive(pts, cols, Vector<Vector2>());
			}
#endif
		} break;

		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			if (!enabled) {
				break;
			}
			_update_shapecast_state();
		} break;
	}
}

void ShapeCast2D::_update_shapecast_state() {
	result.clear();

	ERR_FAIL_COND_MSG(shape.is_null(), "Invalid shape.");

	Ref<World2D> w2d = get_world_2d();
	ERR_FAIL_COND(w2d.is_null());

	Physics2DDirectSpaceState *dss = Physics2DServer::get_singleton()->space_get_direct_state(w2d->get_space());
	ERR_FAIL_COND(!dss);

	Transform2D gt = get_global_transform();

	collision_safe_fraction = 0.0;
	collision_unsafe_fraction = 0.0;

	if (target_position != Vector2()) {
		dss->cast_motion(shape_rid, gt, target_position, margin, collision_safe_fraction, collision_unsafe_fraction, exclude, collision_mask, collide_with_bodies, collide_with_areas);
		if (collision_unsafe_fraction < 1.0) {
			// Move shape transform to the point of impact,
			// so we can collect contact info at that point.
			gt.set_origin(gt.get_origin() + target_position * (collision_unsafe_fraction + CMP_EPSILON));
		}
	}

	// Regardless of whether the shape is stuck or it's moved along
	// the motion vector, we'll only consider static collisions from now on.
	RBSet<RID> intersected_objects = exclude;
	bool intersected = true;
	while (intersected && result.size() < max_results) {
		Physics2DDirectSpaceState::ShapeRestInfo info;
		intersected = dss->rest_info(shape_rid, gt, target_position, margin, &info, exclude, collision_mask, collide_with_bodies, collide_with_areas);
		if (intersected) {
			result.push_back(info);
			intersected_objects.insert(info.rid);
		}
	}
	collided = !result.empty();
}

void ShapeCast2D::force_shapecast_update() {
	_update_shapecast_state();
}

void ShapeCast2D::add_exception_rid(const RID &p_rid) {
	exclude.insert(p_rid);
}

void ShapeCast2D::add_exception(const Object *p_object) {
	ERR_FAIL_NULL(p_object);
	const CollisionObject2D *co = Object::cast_to<CollisionObject2D>(p_object);
	ERR_FAIL_COND_MSG(!co, "The passed Node must be an instance of CollisionObject2D.");
	add_exception_rid(co->get_rid());
}

void ShapeCast2D::remove_exception_rid(const RID &p_rid) {
	exclude.erase(p_rid);
}

void ShapeCast2D::remove_exception(const Object *p_object) {
	ERR_FAIL_NULL(p_object);
	const CollisionObject2D *co = Object::cast_to<CollisionObject2D>(p_object);
	ERR_FAIL_COND_MSG(!co, "The passed Node must be an instance of CollisionObject2D.");
	remove_exception_rid(co->get_rid());
}

void ShapeCast2D::clear_exceptions() {
	exclude.clear();
}

void ShapeCast2D::set_collide_with_areas(bool p_clip) {
	collide_with_areas = p_clip;
}

bool ShapeCast2D::is_collide_with_areas_enabled() const {
	return collide_with_areas;
}

void ShapeCast2D::set_collide_with_bodies(bool p_clip) {
	collide_with_bodies = p_clip;
}

bool ShapeCast2D::is_collide_with_bodies_enabled() const {
	return collide_with_bodies;
}

Array ShapeCast2D::_get_collision_result() const {
	Array ret;

	for (int i = 0; i < result.size(); ++i) {
		const Physics2DDirectSpaceState::ShapeRestInfo &sri = result[i];

		Dictionary col;
		col["point"] = sri.point;
		col["normal"] = sri.normal;
		col["rid"] = sri.rid;
		col["collider"] = ObjectDB::get_instance(sri.collider_id);
		col["collider_id"] = sri.collider_id;
		col["shape"] = sri.shape;
		col["linear_velocity"] = sri.linear_velocity;

		ret.push_back(col);
	}
	return ret;
}

String ShapeCast2D::get_configuration_warning() const {
	String warning = Node2D::get_configuration_warning();

	if (shape.is_null()) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("This node cannot interact with other objects unless a Shape2D is assigned.");
	}

	return warning;
}

void ShapeCast2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &ShapeCast2D::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &ShapeCast2D::is_enabled);

	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &ShapeCast2D::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &ShapeCast2D::get_shape);

	ClassDB::bind_method(D_METHOD("set_target_position", "local_point"), &ShapeCast2D::set_target_position);
	ClassDB::bind_method(D_METHOD("get_target_position"), &ShapeCast2D::get_target_position);

	ClassDB::bind_method(D_METHOD("set_margin", "margin"), &ShapeCast2D::set_margin);
	ClassDB::bind_method(D_METHOD("get_margin"), &ShapeCast2D::get_margin);

	ClassDB::bind_method(D_METHOD("set_max_results", "max_results"), &ShapeCast2D::set_max_results);
	ClassDB::bind_method(D_METHOD("get_max_results"), &ShapeCast2D::get_max_results);

	ClassDB::bind_method(D_METHOD("is_colliding"), &ShapeCast2D::is_colliding);
	ClassDB::bind_method(D_METHOD("get_collision_count"), &ShapeCast2D::get_collision_count);

	ClassDB::bind_method(D_METHOD("force_shapecast_update"), &ShapeCast2D::force_shapecast_update);

	ClassDB::bind_method(D_METHOD("get_collider", "index"), &ShapeCast2D::get_collider);
	ClassDB::bind_method(D_METHOD("get_collider_rid", "index"), &ShapeCast2D::get_collider_rid);
	ClassDB::bind_method(D_METHOD("get_collider_shape", "index"), &ShapeCast2D::get_collider_shape);
	ClassDB::bind_method(D_METHOD("get_collision_point", "index"), &ShapeCast2D::get_collision_point);
	ClassDB::bind_method(D_METHOD("get_collision_normal", "index"), &ShapeCast2D::get_collision_normal);

	ClassDB::bind_method(D_METHOD("get_closest_collision_safe_fraction"), &ShapeCast2D::get_closest_collision_safe_fraction);
	ClassDB::bind_method(D_METHOD("get_closest_collision_unsafe_fraction"), &ShapeCast2D::get_closest_collision_unsafe_fraction);

	ClassDB::bind_method(D_METHOD("add_exception_rid", "rid"), &ShapeCast2D::add_exception_rid);
	ClassDB::bind_method(D_METHOD("add_exception", "node"), &ShapeCast2D::add_exception);

	ClassDB::bind_method(D_METHOD("remove_exception_rid", "rid"), &ShapeCast2D::remove_exception_rid);
	ClassDB::bind_method(D_METHOD("remove_exception", "node"), &ShapeCast2D::remove_exception);

	ClassDB::bind_method(D_METHOD("clear_exceptions"), &ShapeCast2D::clear_exceptions);

	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &ShapeCast2D::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &ShapeCast2D::get_collision_mask);

	ClassDB::bind_method(D_METHOD("set_collision_mask_value", "layer_number", "value"), &ShapeCast2D::set_collision_mask_value);
	ClassDB::bind_method(D_METHOD("get_collision_mask_value", "layer_number"), &ShapeCast2D::get_collision_mask_value);

	ClassDB::bind_method(D_METHOD("set_exclude_parent_body", "mask"), &ShapeCast2D::set_exclude_parent_body);
	ClassDB::bind_method(D_METHOD("get_exclude_parent_body"), &ShapeCast2D::get_exclude_parent_body);

	ClassDB::bind_method(D_METHOD("set_collide_with_areas", "enable"), &ShapeCast2D::set_collide_with_areas);
	ClassDB::bind_method(D_METHOD("is_collide_with_areas_enabled"), &ShapeCast2D::is_collide_with_areas_enabled);

	ClassDB::bind_method(D_METHOD("set_collide_with_bodies", "enable"), &ShapeCast2D::set_collide_with_bodies);
	ClassDB::bind_method(D_METHOD("is_collide_with_bodies_enabled"), &ShapeCast2D::is_collide_with_bodies_enabled);

	ClassDB::bind_method(D_METHOD("_get_collision_result"), &ShapeCast2D::_get_collision_result);

	ClassDB::bind_method(D_METHOD("_redraw_shape"), &ShapeCast2D::_redraw_shape);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape2D"), "set_shape", "get_shape");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "exclude_parent"), "set_exclude_parent_body", "get_exclude_parent_body");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "target_position", PROPERTY_HINT_NONE), "set_target_position", "get_target_position");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "margin", PROPERTY_HINT_RANGE, "0,100,0.01"), "set_margin", "get_margin");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_results"), "set_max_results", "get_max_results");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_mask", "get_collision_mask");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "collision_result", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_SCRIPT_VARIABLE), "", "_get_collision_result");
	ADD_GROUP("Collide With", "collide_with");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collide_with_areas", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collide_with_areas", "is_collide_with_areas_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collide_with_bodies", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collide_with_bodies", "is_collide_with_bodies_enabled");
}
