#ifndef NAVIGATION_OBSTACLE_H
#define NAVIGATION_OBSTACLE_H
/*************************************************************************/
/*  navigation_obstacle.h                                                */
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




#include "scene/main/node.h"

class Navigation;
class Spatial;

class NavigationObstacle : public Node {
	GDCLASS(NavigationObstacle, Node);

	Spatial *parent_spatial = nullptr;
	Navigation *navigation;

	RID agent;
	bool estimate_radius = true;
	real_t radius = 1.0;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &property) const;
	void _notification(int p_what);

public:
	NavigationObstacle();
	virtual ~NavigationObstacle();

	void set_navigation(Navigation *p_nav);
	const Navigation *get_navigation() const {
		return navigation;
	}

	void set_navigation_node(Node *p_nav);
	Node *get_navigation_node() const;

	RID get_rid() const {
		return agent;
	}

	void set_estimate_radius(bool p_estimate_radius);
	bool is_radius_estimated() const {
		return estimate_radius;
	}
	void set_radius(real_t p_radius);
	real_t get_radius() const {
		return radius;
	}

	virtual String get_configuration_warning() const;

private:
	void initialize_agent();
	void reevaluate_agent_radius();
	real_t estimate_agent_radius() const;
};

#endif
