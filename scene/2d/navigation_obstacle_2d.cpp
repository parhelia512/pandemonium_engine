/*************************************************************************/
/*  navigation_obstacle_2d.cpp                                           */
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

#include "navigation_obstacle_2d.h"

#include "core/config/engine.h"
#include "scene/2d/collision_shape_2d.h"
#include "scene/2d/navigation_2d.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/physics_body_2d.h"
#include "scene/resources/world_2d.h"
#include "servers/navigation_2d_server.h"

void NavigationObstacle2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_obstacle_rid"), &NavigationObstacle2D::get_obstacle_rid);
	ClassDB::bind_method(D_METHOD("get_agent_rid"), &NavigationObstacle2D::get_agent_rid);

	ClassDB::bind_method(D_METHOD("set_navigation", "navigation"), &NavigationObstacle2D::set_navigation_node);
	ClassDB::bind_method(D_METHOD("get_navigation"), &NavigationObstacle2D::get_navigation_node);

	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &NavigationObstacle2D::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &NavigationObstacle2D::get_radius);

	ClassDB::bind_method(D_METHOD("set_velocity", "velocity"), &NavigationObstacle2D::set_velocity);
	ClassDB::bind_method(D_METHOD("get_velocity"), &NavigationObstacle2D::get_velocity);

	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &NavigationObstacle2D::set_vertices);
	ClassDB::bind_method(D_METHOD("get_vertices"), &NavigationObstacle2D::get_vertices_bind);

	ClassDB::bind_method(D_METHOD("set_avoidance_layers", "layers"), &NavigationObstacle2D::set_avoidance_layers);
	ClassDB::bind_method(D_METHOD("get_avoidance_layers"), &NavigationObstacle2D::get_avoidance_layers);
	ClassDB::bind_method(D_METHOD("set_avoidance_layer_value", "layer_number", "value"), &NavigationObstacle2D::set_avoidance_layer_value);
	ClassDB::bind_method(D_METHOD("get_avoidance_layer_value", "layer_number"), &NavigationObstacle2D::get_avoidance_layer_value);

	ADD_GROUP("Avoidance", "avoidance_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "velocity", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_velocity", "get_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "radius", PROPERTY_HINT_RANGE, "0.0,500,0.01,suffix:px"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "vertices"), "set_vertices", "get_vertices");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "avoidance_layers", PROPERTY_HINT_LAYERS_AVOIDANCE), "set_avoidance_layers", "get_avoidance_layers");
}

void NavigationObstacle2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Search the navigation node and set it
			{
				Navigation2D *nav = nullptr;
				Node *p = get_parent();
				while (p != nullptr) {
					nav = Object::cast_to<Navigation2D>(p);
					if (nav != nullptr) {
						p = nullptr;
					} else {
						p = p->get_parent();
					}
				}

				navigation = nav;
			}

			if (navigation) {
				_update_map(navigation->get_rid());
			} else if (map_override.is_valid()) {
				_update_map(map_override);
			} else if (is_inside_tree()) {
				_update_map(get_world_2d()->get_navigation_map());
			} else {
				_update_map(RID());
			}
			previous_transform = get_global_transform();
			// need to trigger map controlled agent assignment somehow for the fake_agent since obstacles use no callback like regular agents
			Navigation2DServer::get_singleton()->agent_set_avoidance_enabled(fake_agent, radius > 0);
			_update_position(get_global_transform().get_origin());

			set_physics_process_internal(true);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			set_navigation(nullptr);
			set_physics_process_internal(false);
			_update_map(RID());
			request_ready(); // required to solve an issue with losing the navigation

		} break;
		case NOTIFICATION_PAUSED: {
			if (!can_process()) {
				map_before_pause = map_current;
				_update_map(RID());
			} else if (can_process() && !(map_before_pause == RID())) {
				_update_map(map_before_pause);
				map_before_pause = RID();
			}
		} break;
		case NOTIFICATION_UNPAUSED: {
			if (!can_process()) {
				map_before_pause = map_current;
				_update_map(RID());
			} else if (can_process() && !(map_before_pause == RID())) {
				_update_map(map_before_pause);
				map_before_pause = RID();
			}
		} break;
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			if (is_inside_tree()) {
				_update_position(get_global_transform().get_origin());

				if (velocity_submitted) {
					velocity_submitted = false;
					// only update if there is a noticeable change, else the rvo agent preferred velocity stays the same
					if (!previous_velocity.is_equal_approx(velocity)) {
						Navigation2DServer::get_singleton()->agent_set_velocity(fake_agent, velocity);
					}
					previous_velocity = velocity;
				}
			}
		} break;

		case NOTIFICATION_DRAW: {
#ifdef DEBUG_ENABLED
			if (is_inside_tree()) {
				bool is_debug_enabled = false;
				if (Engine::get_singleton()->is_editor_hint()) {
					is_debug_enabled = true;
				} else if (Navigation2DServer::get_singleton()->get_debug_enabled() && Navigation2DServer::get_singleton()->get_debug_avoidance_enabled()) {
					is_debug_enabled = true;
				}

				if (is_debug_enabled) {
					_update_fake_agent_radius_debug();
					_update_static_obstacle_debug();
				}
			}
#endif // DEBUG_ENABLED
		} break;
	}
}

NavigationObstacle2D::NavigationObstacle2D() {
	navigation = nullptr;

	obstacle = Navigation2DServer::get_singleton()->obstacle_create();
	fake_agent = Navigation2DServer::get_singleton()->agent_create();

	// change properties of the fake agent so it can act as fake obstacle with a radius
	Navigation2DServer::get_singleton()->agent_set_neighbor_dist(fake_agent, 0.0);
	Navigation2DServer::get_singleton()->agent_set_max_neighbors(fake_agent, 0);
	Navigation2DServer::get_singleton()->agent_set_time_horizon_agents(fake_agent, 0.0);
	Navigation2DServer::get_singleton()->agent_set_time_horizon_obstacles(fake_agent, 0.0);
	Navigation2DServer::get_singleton()->agent_set_max_speed(fake_agent, 0.0);
	Navigation2DServer::get_singleton()->agent_set_avoidance_mask(fake_agent, 0);
	Navigation2DServer::get_singleton()->agent_set_avoidance_priority(fake_agent, 1.0);
	Navigation2DServer::get_singleton()->agent_set_avoidance_enabled(fake_agent, radius > 0);

	set_radius(radius);
	set_vertices(vertices);
	set_avoidance_layers(avoidance_layers);
}

NavigationObstacle2D::~NavigationObstacle2D() {
	ERR_FAIL_NULL(Navigation2DServer::get_singleton());
	Navigation2DServer::get_singleton()->free(obstacle);
	obstacle = RID();

	Navigation2DServer::get_singleton()->free(fake_agent);
	fake_agent = RID();
}

void NavigationObstacle2D::set_navigation(Navigation2D *p_nav) {
	if (navigation == p_nav) {
		return; // Pointless
	}

	navigation = p_nav;

	if (navigation) {
		_update_map(navigation->get_rid());
	} else if (map_override.is_valid()) {
		_update_map(map_override);
	} else if (is_inside_tree()) {
		_update_map(get_world_2d()->get_navigation_map());
	} else {
		_update_map(RID());
	}
}

void NavigationObstacle2D::set_navigation_node(Node *p_nav) {
	Navigation2D *nav = Object::cast_to<Navigation2D>(p_nav);
	ERR_FAIL_COND(nav == nullptr);
	set_navigation(nav);
}

Node *NavigationObstacle2D::get_navigation_node() const {
	return Object::cast_to<Node>(navigation);
}

void NavigationObstacle2D::set_vertices(const Vector<Vector2> &p_vertices) {
	vertices = p_vertices;
	Navigation2DServer::get_singleton()->obstacle_set_vertices(obstacle, vertices);
	if (is_inside_tree() && (Engine::get_singleton()->is_editor_hint() || get_tree()->is_debugging_navigation_hint())) {
		update();
	}
}

void NavigationObstacle2D::set_navigation_map(RID p_navigation_map) {
	if (map_override == p_navigation_map) {
		return;
	}
	map_override = p_navigation_map;
	_update_map(map_override);
}

RID NavigationObstacle2D::get_navigation_map() const {
	if (navigation) {
		return navigation->get_rid();
	} else if (map_override.is_valid()) {
		return map_override;
	} else if (is_inside_tree()) {
		return get_world_2d()->get_navigation_map();
	}

	return RID();
}

void NavigationObstacle2D::set_radius(real_t p_radius) {
	ERR_FAIL_COND_MSG(p_radius < 0.0, "Radius must be positive.");
	if (Math::is_equal_approx(radius, p_radius)) {
		return;
	}

	radius = p_radius;

	Navigation2DServer::get_singleton()->agent_set_avoidance_enabled(fake_agent, radius > 0.0);
	Navigation2DServer::get_singleton()->agent_set_radius(fake_agent, radius);
	if (is_inside_tree() && (Engine::get_singleton()->is_editor_hint() || get_tree()->is_debugging_navigation_hint())) {
		update();
	}
}

void NavigationObstacle2D::set_avoidance_layers(uint32_t p_layers) {
	avoidance_layers = p_layers;
	Navigation2DServer::get_singleton()->obstacle_set_avoidance_layers(obstacle, avoidance_layers);
	Navigation2DServer::get_singleton()->agent_set_avoidance_layers(fake_agent, avoidance_layers);
}

uint32_t NavigationObstacle2D::get_avoidance_layers() const {
	return avoidance_layers;
}

void NavigationObstacle2D::set_avoidance_layer_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1, "Avoidance layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_MSG(p_layer_number > 32, "Avoidance layer number must be between 1 and 32 inclusive.");
	uint32_t avoidance_layers_new = get_avoidance_layers();
	if (p_value) {
		avoidance_layers_new |= 1 << (p_layer_number - 1);
	} else {
		avoidance_layers_new &= ~(1 << (p_layer_number - 1));
	}
	set_avoidance_layers(avoidance_layers_new);
}

bool NavigationObstacle2D::get_avoidance_layer_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Avoidance layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_V_MSG(p_layer_number > 32, false, "Avoidance layer number must be between 1 and 32 inclusive.");
	return get_avoidance_layers() & (1 << (p_layer_number - 1));
}

void NavigationObstacle2D::set_velocity(const Vector2 p_velocity) {
	velocity = p_velocity;
	velocity_submitted = true;
	Navigation2DServer::get_singleton()->agent_set_velocity(fake_agent, velocity);
}

void NavigationObstacle2D::_update_map(RID p_map) {
	Navigation2DServer::get_singleton()->obstacle_set_map(obstacle, p_map);
	Navigation2DServer::get_singleton()->agent_set_map(fake_agent, p_map);
	map_current = p_map;
}

void NavigationObstacle2D::_update_position(const Vector2 p_position) {
	if (vertices.size() > 0) {
		Navigation2DServer::get_singleton()->obstacle_set_position(obstacle, p_position);
	}
	if (radius > 0.0) {
		Navigation2DServer::get_singleton()->agent_set_position(fake_agent, p_position);
	}
}

#ifdef DEBUG_ENABLED
void NavigationObstacle2D::_update_fake_agent_radius_debug() {
	if (radius > 0.0 && Navigation2DServer::get_singleton()->get_debug_navigation_avoidance_enable_obstacles_radius()) {
		Color debug_radius_color = Navigation2DServer::get_singleton()->get_debug_navigation_avoidance_obstacles_radius_color();
		draw_circle(get_global_transform().get_origin(), radius, debug_radius_color);
	}
}
#endif // DEBUG_ENABLED

#ifdef DEBUG_ENABLED
void NavigationObstacle2D::_update_static_obstacle_debug() {
	if (get_vertices().size() > 2 && Navigation2DServer::get_singleton()->get_debug_navigation_avoidance_enable_obstacles_static()) {
		bool obstacle_pushes_inward = Geometry::is_polygon_clockwise(get_vertices());

		Color debug_static_obstacle_face_color;

		if (obstacle_pushes_inward) {
			debug_static_obstacle_face_color = Navigation2DServer::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushin_face_color();
		} else {
			debug_static_obstacle_face_color = Navigation2DServer::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushout_face_color();
		}

		Vector<Vector2> debug_obstacle_polygon_vertices = get_vertices();

		Vector<Color> debug_obstacle_polygon_colors;
		debug_obstacle_polygon_colors.resize(debug_obstacle_polygon_vertices.size());
		debug_obstacle_polygon_colors.fill(debug_static_obstacle_face_color);

		RS::get_singleton()->canvas_item_add_polygon(get_canvas_item(), debug_obstacle_polygon_vertices, debug_obstacle_polygon_colors);

		Color debug_static_obstacle_edge_color;

		if (obstacle_pushes_inward) {
			debug_static_obstacle_edge_color = Navigation2DServer::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushin_edge_color();
		} else {
			debug_static_obstacle_edge_color = Navigation2DServer::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushout_edge_color();
		}

		Vector<Vector2> debug_obstacle_line_vertices = get_vertices();
		debug_obstacle_line_vertices.push_back(debug_obstacle_line_vertices[0]);
		debug_obstacle_line_vertices.resize(debug_obstacle_line_vertices.size());

		Vector<Color> debug_obstacle_line_colors;
		debug_obstacle_line_colors.resize(debug_obstacle_line_vertices.size());
		debug_obstacle_line_colors.fill(debug_static_obstacle_edge_color);

		RS::get_singleton()->canvas_item_add_polyline(get_canvas_item(), debug_obstacle_line_vertices, debug_obstacle_line_colors, 4.0);
	}
}
#endif // DEBUG_ENABLED