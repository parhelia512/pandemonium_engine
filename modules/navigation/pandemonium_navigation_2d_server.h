#ifndef PANDEMONIUM_NAVIGATION_2D_SERVER_H
#define PANDEMONIUM_NAVIGATION_2D_SERVER_H

/*************************************************************************/
/*  pandemonium_navigation_2d_server.h                                   */
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

#include "core/containers/rid.h"
#include "core/object/object.h"
#include "scene/resources/navigation_2d/navigation_polygon.h"
#include "servers/navigation_2d_server.h"

// This server exposes the 3D `NavigationServer` features in the 2D world.
class PandemoniumNavigation2DServer : public Navigation2DServer {
	GDCLASS(PandemoniumNavigation2DServer, Navigation2DServer);

	void _emit_map_changed(RID p_map);

protected:
	static void _bind_methods();

public:
	virtual Array get_maps() const;

	/// Create a new map.
	virtual RID map_create();

	/// Set map active.
	virtual void map_set_active(RID p_map, bool p_active);

	/// Returns true if the map is active.
	virtual bool map_is_active(RID p_map) const;

	/// Set the map cell size used to weld the navigation mesh polygons.
	virtual void map_set_cell_size(RID p_map, real_t p_cell_size);

	/// Returns the map cell size.
	virtual real_t map_get_cell_size(RID p_map) const;

	virtual void map_set_use_edge_connections(RID p_map, bool p_enabled);
	virtual bool map_get_use_edge_connections(RID p_map) const;

	/// Set the map edge connection margin used to weld the compatible region edges.
	virtual void map_set_edge_connection_margin(RID p_map, real_t p_connection_margin);

	/// Returns the edge connection margin of this map.
	virtual real_t map_get_edge_connection_margin(RID p_map) const;

	/// Set the map link connection radius used to attach links to the nav mesh.
	virtual void map_set_link_connection_radius(RID p_map, real_t p_connection_radius);

	/// Returns the link connection radius of this map.
	virtual real_t map_get_link_connection_radius(RID p_map) const;

	/// Returns the navigation path to reach the destination from the origin.
	virtual Vector<Vector2> map_get_path(RID p_map, Vector2 p_origin, Vector2 p_destination, bool p_optimize, uint32_t p_navigation_layers = 1) const;

	virtual Vector2 map_get_closest_point(RID p_map, const Vector2 &p_point) const;
	virtual RID map_get_closest_point_owner(RID p_map, const Vector2 &p_point) const;

	virtual Array map_get_links(RID p_map) const;
	virtual Array map_get_regions(RID p_map) const;
	virtual Array map_get_agents(RID p_map) const;
	virtual Array map_get_obstacles(RID p_map) const;

	virtual void map_force_update(RID p_map);

	/// Creates a new region.
	virtual RID region_create();

	virtual void region_set_enabled(RID p_region, bool p_enabled);
	virtual bool region_get_enabled(RID p_region) const;

	virtual void region_set_use_edge_connections(RID p_region, bool p_enabled);
	virtual bool region_get_use_edge_connections(RID p_region) const;

	/// Set the enter_cost of a region
	virtual void region_set_enter_cost(RID p_region, real_t p_enter_cost);
	virtual real_t region_get_enter_cost(RID p_region) const;

	/// Set the travel_cost of a region
	virtual void region_set_travel_cost(RID p_region, real_t p_travel_cost);
	virtual real_t region_get_travel_cost(RID p_region) const;

	/// Set the node which manages this region.
	virtual void region_set_owner_id(RID p_region, ObjectID p_owner_id);
	virtual ObjectID region_get_owner_id(RID p_region) const;

	virtual bool region_owns_point(RID p_region, const Vector2 &p_point) const;

	/// Set the map of this region.
	virtual void region_set_map(RID p_region, RID p_map);
	virtual RID region_get_map(RID p_region) const;

	/// Set the region's layers
	virtual void region_set_navigation_layers(RID p_region, uint32_t p_navigation_layers);
	virtual uint32_t region_get_navigation_layers(RID p_region) const;

	/// Set the global transformation of this region.
	virtual void region_set_transform(RID p_region, Transform2D p_transform);

	/// Set the navigation poly of this region.
	virtual void region_set_navigation_polygon(RID p_region, Ref<NavigationPolygon> p_navigation_mesh);

	/// Get a list of a region's connection to other regions.
	virtual int region_get_connections_count(RID p_region) const;
	virtual Vector2 region_get_connection_pathway_start(RID p_region, int p_connection_id) const;
	virtual Vector2 region_get_connection_pathway_end(RID p_region, int p_connection_id) const;

	/// Creates a new link between positions in the nav map.
	virtual RID link_create();

	/// Set the map of this link.
	virtual void link_set_map(RID p_link, RID p_map);
	virtual RID link_get_map(RID p_link) const;

	virtual void link_set_enabled(RID p_link, bool p_enabled);
	virtual bool link_get_enabled(RID p_link) const;

	/// Set whether this link travels in both directions.
	virtual void link_set_bidirectional(RID p_link, bool p_bidirectional);
	virtual bool link_is_bidirectional(RID p_link) const;

	/// Set the link's layers.
	virtual void link_set_navigation_layers(RID p_link, uint32_t p_navigation_layers);
	virtual uint32_t link_get_navigation_layers(RID p_link) const;

	/// Set the start position of the link.
	virtual void link_set_start_position(RID p_link, Vector2 p_position);
	virtual Vector2 link_get_start_position(RID p_link) const;

	/// Set the end position of the link.
	virtual void link_set_end_position(RID p_link, Vector2 p_position);
	virtual Vector2 link_get_end_position(RID p_link) const;

	/// Set the enter cost of the link.
	virtual void link_set_enter_cost(RID p_link, real_t p_enter_cost);
	virtual real_t link_get_enter_cost(RID p_link) const;

	/// Set the travel cost of the link.
	virtual void link_set_travel_cost(RID p_link, real_t p_travel_cost);
	virtual real_t link_get_travel_cost(RID p_link) const;

	/// Set the node which manages this link.
	virtual void link_set_owner_id(RID p_link, ObjectID p_owner_id);
	virtual ObjectID link_get_owner_id(RID p_link) const;

	/// Creates the agent.
	virtual RID agent_create();

	/// Put the agent in the map.
	virtual void agent_set_map(RID p_agent, RID p_map);
	virtual RID agent_get_map(RID p_agent) const;

	virtual void agent_set_paused(RID p_agent, bool p_paused);
	virtual bool agent_get_paused(RID p_agent) const;

	virtual void agent_set_avoidance_enabled(RID p_agent, bool p_enabled);
	virtual bool agent_get_avoidance_enabled(RID p_agent) const;

	/// The maximum distance (center point to
	/// center point) to other agents this agent
	/// takes into account in the navigation. The
	/// larger this number, the longer the running
	/// time of the simulation. If the number is too
	/// low, the simulation will not be safe.
	/// Must be non-negative.
	virtual void agent_set_neighbor_distance(RID p_agent, real_t p_dist);

	/// The maximum number of other agents this
	/// agent takes into account in the navigation.
	/// The larger this number, the longer the
	/// running time of the simulation. If the
	/// number is too low, the simulation will not
	/// be safe.
	virtual void agent_set_max_neighbors(RID p_agent, int p_count);

	/// The minimal amount of time for which this
	/// agent's velocities that are computed by the
	/// simulation are safe with respect to other
	/// agents. The larger this number, the sooner
	/// this agent will respond to the presence of
	/// other agents, but the less freedom this
	/// agent has in choosing its velocities.
	/// Must be positive.
	virtual void agent_set_time_horizon_agents(RID p_agent, real_t p_time_horizon);
	virtual void agent_set_time_horizon_obstacles(RID p_agent, real_t p_time_horizon);

	/// The radius of this agent.
	/// Must be non-negative.
	virtual void agent_set_radius(RID p_agent, real_t p_radius);

	/// The maximum speed of this agent.
	/// Must be non-negative.
	virtual void agent_set_max_speed(RID p_agent, real_t p_max_speed);

	/// forces and agent velocity change in the avoidance simulation, adds simulation instability if done recklessly
	virtual void agent_set_velocity_forced(RID p_agent, Vector2 p_velocity);

	/// The wanted velocity for the agent as a "suggestion" to the avoidance simulation.
	/// The simulation will try to fulfil this velocity wish if possible but may change the velocity depending on other agent's and obstacles'.
	virtual void agent_set_velocity(RID p_agent, Vector2 p_velocity);

	/// Position of the agent in world space.
	virtual void agent_set_position(RID p_agent, Vector2 p_position);

	/// Returns true if the map got changed the previous frame.
	virtual bool agent_is_map_changed(RID p_agent) const;

	/// Callback called at the end of the RVO process
	virtual void agent_set_avoidance_callback(RID p_agent, ObjectID p_object_id, StringName p_method, Variant p_udata = Variant());

	virtual void agent_set_avoidance_layers(RID p_agent, uint32_t p_layers);
	virtual void agent_set_avoidance_mask(RID p_agent, uint32_t p_mask);
	virtual void agent_set_avoidance_priority(RID p_agent, real_t p_priority);

	/// Creates the obstacle.
	virtual RID obstacle_create();
	virtual void obstacle_set_avoidance_enabled(RID p_obstacle, bool p_enabled);
	virtual bool obstacle_get_avoidance_enabled(RID p_obstacle) const;
	virtual void obstacle_set_map(RID p_obstacle, RID p_map);
	virtual RID obstacle_get_map(RID p_obstacle) const;
	virtual void obstacle_set_radius(RID p_obstacle, real_t p_radius);
	virtual void obstacle_set_velocity(RID p_obstacle, Vector2 p_velocity);
	virtual void obstacle_set_paused(RID p_obstacle, bool p_paused);
	virtual bool obstacle_get_paused(RID p_obstacle) const;
	virtual void obstacle_set_position(RID p_obstacle, Vector2 p_position);
	virtual void obstacle_set_vertices(RID p_obstacle, const Vector<Vector2> &p_vertices);
	virtual void obstacle_set_avoidance_layers(RID p_obstacle, uint32_t p_layers);

	/// Destroy the `RID`
	virtual void free(RID p_object);

	virtual NavigationUtilities::PathQueryResult2D _query_path(const NavigationUtilities::PathQueryParameters2D &p_parameters) const;

	PandemoniumNavigation2DServer();
	virtual ~PandemoniumNavigation2DServer();
};

#endif
