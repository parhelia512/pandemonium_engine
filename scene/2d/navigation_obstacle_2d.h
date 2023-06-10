#ifndef NAVIGATION_OBSTACLE_2D_H
#define NAVIGATION_OBSTACLE_2D_H
/*************************************************************************/
/*  navigation_obstacle_2d.h                                             */
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

#include "scene/2d/node_2d.h"

class Navigation2D;
class Node2D;

class NavigationObstacle2D : public Node2D {
	GDCLASS(NavigationObstacle2D, Node2D);

	Navigation2D *navigation;

	RID obstacle;
	RID map_before_pause;
	RID map_override; //!!!
	RID map_current;

	real_t radius = 0.0;

	Vector<Vector2> vertices;

	RID fake_agent;
	uint32_t avoidance_layers = 1;

	Transform2D previous_transform;

	Vector2 velocity;
	Vector2 previous_velocity;
	bool velocity_submitted = false;

#ifdef DEBUG_ENABLED
private:
	void _update_fake_agent_radius_debug();
	void _update_static_obstacle_debug();
#endif // DEBUG_ENABLED

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	NavigationObstacle2D();
	virtual ~NavigationObstacle2D();

	void set_navigation(Navigation2D *p_nav);
	const Navigation2D *get_navigation() const {
		return navigation;
	}

	void set_navigation_node(Node *p_nav);
	Node *get_navigation_node() const;

	RID get_obstacle_rid() const { return obstacle; }
	RID get_agent_rid() const { return fake_agent; }

	void set_navigation_map(RID p_navigation_map);
	RID get_navigation_map() const;

	void set_radius(real_t p_radius);
	real_t get_radius() const {
		return radius;
	}

	void set_vertices(const Vector<Vector2> &p_vertices);
	const Vector<Vector2> &get_vertices() const { return vertices; };

	void set_avoidance_layers(uint32_t p_layers);
	uint32_t get_avoidance_layers() const;

	void set_avoidance_mask(uint32_t p_mask);
	uint32_t get_avoidance_mask() const;

	void set_avoidance_layer_value(int p_layer_number, bool p_value);
	bool get_avoidance_layer_value(int p_layer_number) const;

	void set_velocity(const Vector2 p_velocity);
	Vector2 get_velocity() const { return velocity; };

	void _avoidance_done(Vector3 p_new_velocity); // Dummy

private:
	void _update_map(RID p_map);
	void _update_position(const Vector2 p_position);
};

#endif
