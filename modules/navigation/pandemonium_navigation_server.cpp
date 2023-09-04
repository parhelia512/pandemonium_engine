/*************************************************************************/
/*  pandemonium_navigation_server.cpp                                          */
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

#include "pandemonium_navigation_server.h"

#include "core/os/mutex.h"

#ifndef _3D_DISABLED
//#include "navigation_mesh_generator.h"
#include "scene/resources/mesh.h"
#include "scene/resources/navigation_mesh.h"
#endif

using namespace NavigationUtilities;

/// Creates a struct for each function and a function that once called creates
/// an instance of that struct with the submitted parameters.
/// Then, that struct is stored in an array; the `sync` function consume that array.

#define COMMAND_1(F_NAME, T_0, D_0)                              \
	struct MERGE(F_NAME, _command) : public SetCommand {         \
		T_0 d_0;                                                 \
		MERGE(F_NAME, _command)                                  \
		(T_0 p_d_0) :                                            \
				d_0(p_d_0) {}                                    \
		virtual void exec(PandemoniumNavigationServer *server) { \
			server->MERGE(_cmd_, F_NAME)(d_0);                   \
		}                                                        \
	};                                                           \
	void PandemoniumNavigationServer::F_NAME(T_0 D_0) {          \
		auto cmd = memnew(MERGE(F_NAME, _command)(               \
				D_0));                                           \
		add_command(cmd);                                        \
	}                                                            \
	void PandemoniumNavigationServer::MERGE(_cmd_, F_NAME)(T_0 D_0)

#define COMMAND_2(F_NAME, T_0, D_0, T_1, D_1)                    \
	struct MERGE(F_NAME, _command) : public SetCommand {         \
		T_0 d_0;                                                 \
		T_1 d_1;                                                 \
		MERGE(F_NAME, _command)                                  \
		(                                                        \
				T_0 p_d_0,                                       \
				T_1 p_d_1) :                                     \
				d_0(p_d_0),                                      \
				d_1(p_d_1) {}                                    \
		virtual void exec(PandemoniumNavigationServer *server) { \
			server->MERGE(_cmd_, F_NAME)(d_0, d_1);              \
		}                                                        \
	};                                                           \
	void PandemoniumNavigationServer::F_NAME(T_0 D_0, T_1 D_1) { \
		auto cmd = memnew(MERGE(F_NAME, _command)(               \
				D_0,                                             \
				D_1));                                           \
		add_command(cmd);                                        \
	}                                                            \
	void PandemoniumNavigationServer::MERGE(_cmd_, F_NAME)(T_0 D_0, T_1 D_1)

#define COMMAND_4(F_NAME, T_0, D_0, T_1, D_1, T_2, D_2, T_3, D_3)                  \
	struct MERGE(F_NAME, _command) : public SetCommand {                           \
		T_0 d_0;                                                                   \
		T_1 d_1;                                                                   \
		T_2 d_2;                                                                   \
		T_3 d_3;                                                                   \
		MERGE(F_NAME, _command)                                                    \
		(                                                                          \
				T_0 p_d_0,                                                         \
				T_1 p_d_1,                                                         \
				T_2 p_d_2,                                                         \
				T_3 p_d_3) :                                                       \
				d_0(p_d_0),                                                        \
				d_1(p_d_1),                                                        \
				d_2(p_d_2),                                                        \
				d_3(p_d_3) {}                                                      \
		virtual void exec(PandemoniumNavigationServer *server) {                   \
			server->MERGE(_cmd_, F_NAME)(d_0, d_1, d_2, d_3);                      \
		}                                                                          \
	};                                                                             \
	void PandemoniumNavigationServer::F_NAME(T_0 D_0, T_1 D_1, T_2 D_2, T_3 D_3) { \
		auto cmd = memnew(MERGE(F_NAME, _command)(                                 \
				D_0,                                                               \
				D_1,                                                               \
				D_2,                                                               \
				D_3));                                                             \
		add_command(cmd);                                                          \
	}                                                                              \
	void PandemoniumNavigationServer::MERGE(_cmd_, F_NAME)(T_0 D_0, T_1 D_1, T_2 D_2, T_3 D_3)

PandemoniumNavigationServer::PandemoniumNavigationServer() {
	active = true;

	pm_region_count = 0;
	pm_agent_count = 0;
	pm_link_count = 0;
	pm_polygon_count = 0;
	pm_edge_count = 0;
	pm_edge_merge_count = 0;
	pm_edge_connection_count = 0;
	pm_edge_free_count = 0;
}

PandemoniumNavigationServer::~PandemoniumNavigationServer() {
	flush_queries();
}

void PandemoniumNavigationServer::add_command(SetCommand *command) const {
	PandemoniumNavigationServer *mut_this = const_cast<PandemoniumNavigationServer *>(this);
	{
		MutexLock lock(commands_mutex);
		mut_this->commands.push_back(command);
	}
}

Array PandemoniumNavigationServer::get_maps() const {
	Array all_map_rids;
	List<RID> maps_owned;
	map_owner.get_owned_list(&maps_owned);
	if (maps_owned.size()) {
		for (List<RID>::Element *E = maps_owned.front(); E; E = E->next()) {
			all_map_rids.push_back(E->get());
		}
	}
	return all_map_rids;
}

RID PandemoniumNavigationServer::map_create() {
	PandemoniumNavigationServer *mut_this = const_cast<PandemoniumNavigationServer *>(this);
	MutexLock lock(mut_this->operations_mutex);
	NavMap *space = memnew(NavMap);
	RID rid = map_owner.make_rid(space);
	space->set_self(rid);
	return rid;
}

COMMAND_2(map_set_active, RID, p_map, bool, p_active) {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND(map == nullptr);

	if (p_active) {
		if (!map_is_active(p_map)) {
			active_maps.push_back(map);
			active_maps_update_id.push_back(map->get_map_update_id());
		}
	} else {
		int map_index = active_maps.find(map);
		ERR_FAIL_COND(map_index < 0);
		active_maps.remove(map_index);
		active_maps_update_id.remove(map_index);
	}
}

bool PandemoniumNavigationServer::map_is_active(RID p_map) const {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, false);

	return active_maps.find(map) >= 0;
}

COMMAND_2(map_set_up, RID, p_map, Vector3, p_up) {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND(map == nullptr);

	map->set_up(p_up);
}

Vector3 PandemoniumNavigationServer::map_get_up(RID p_map) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, Vector3());

	return map->get_up();
}

COMMAND_2(map_set_cell_size, RID, p_map, real_t, p_cell_size) {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND(map == nullptr);

	map->set_cell_size(p_cell_size);
}

real_t PandemoniumNavigationServer::map_get_cell_size(RID p_map) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, 0);

	return map->get_cell_size();
}

COMMAND_2(map_set_cell_height, RID, p_map, real_t, p_cell_height) {
	NavMap *map = map_owner.get(p_map);
	ERR_FAIL_COND(map == nullptr);

	map->set_cell_height(p_cell_height);
}

real_t PandemoniumNavigationServer::map_get_cell_height(RID p_map) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, 0);

	return map->get_cell_height();
}

COMMAND_2(map_set_use_edge_connections, RID, p_map, bool, p_enabled) {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND(map == nullptr);

	map->set_use_edge_connections(p_enabled);
}

bool PandemoniumNavigationServer::map_get_use_edge_connections(RID p_map) const {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, false);

	return map->get_use_edge_connections();
}

COMMAND_2(map_set_edge_connection_margin, RID, p_map, real_t, p_connection_margin) {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND(map == nullptr);

	map->set_edge_connection_margin(p_connection_margin);
}

real_t PandemoniumNavigationServer::map_get_edge_connection_margin(RID p_map) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, 0);

	return map->get_edge_connection_margin();
}

COMMAND_2(map_set_link_connection_radius, RID, p_map, real_t, p_connection_radius) {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND(map == nullptr);

	map->set_link_connection_radius(p_connection_radius);
}

real_t PandemoniumNavigationServer::map_get_link_connection_radius(RID p_map) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, 0);

	return map->get_link_connection_radius();
}

Vector<Vector3> PandemoniumNavigationServer::map_get_path(RID p_map, Vector3 p_origin, Vector3 p_destination, bool p_optimize, uint32_t p_layers) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, Vector<Vector3>());

	return map->get_path(p_origin, p_destination, p_optimize, p_layers, nullptr, nullptr, nullptr);
}

Vector3 PandemoniumNavigationServer::map_get_closest_point_to_segment(RID p_map, const Vector3 &p_from, const Vector3 &p_to, const bool p_use_collision) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, Vector3());

	return map->get_closest_point_to_segment(p_from, p_to, p_use_collision);
}

Vector3 PandemoniumNavigationServer::map_get_closest_point(RID p_map, const Vector3 &p_point) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, Vector3());

	return map->get_closest_point(p_point);
}

Vector3 PandemoniumNavigationServer::map_get_closest_point_normal(RID p_map, const Vector3 &p_point) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, Vector3());

	return map->get_closest_point_normal(p_point);
}

RID PandemoniumNavigationServer::map_get_closest_point_owner(RID p_map, const Vector3 &p_point) const {
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, RID());

	return map->get_closest_point_owner(p_point);
}

Array PandemoniumNavigationServer::map_get_links(RID p_map) const {
	Array link_rids;
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, link_rids);

	const LocalVector<NavLink *> links = map->get_links();
	link_rids.resize(links.size());

	for (uint32_t i = 0; i < links.size(); i++) {
		link_rids[i] = links[i]->get_self();
	}
	return link_rids;
}

Array PandemoniumNavigationServer::map_get_regions(RID p_map) const {
	Array regions_rids;
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, regions_rids);

	const LocalVector<NavRegion *> regions = map->get_regions();
	regions_rids.resize(regions.size());

	for (uint32_t i = 0; i < regions.size(); i++) {
		regions_rids[i] = regions[i]->get_self();
	}
	return regions_rids;
}

Array PandemoniumNavigationServer::map_get_agents(RID p_map) const {
	Array agents_rids;
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, agents_rids);

	const LocalVector<NavAgent *> agents = map->get_agents();
	agents_rids.resize(agents.size());

	for (uint32_t i = 0; i < agents.size(); i++) {
		agents_rids[i] = agents[i]->get_self();
	}

	return agents_rids;
}

Array PandemoniumNavigationServer::map_get_obstacles(RID p_map) const {
	Array obstacles_rids;
	const NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND_V(map == nullptr, obstacles_rids);
	const LocalVector<NavObstacle *> obstacles = map->get_obstacles();
	obstacles_rids.resize(obstacles.size());
	for (uint32_t i = 0; i < obstacles.size(); i++) {
		obstacles_rids[i] = obstacles[i]->get_self();
	}
	return obstacles_rids;
}

RID PandemoniumNavigationServer::region_get_map(RID p_region) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(region == nullptr, RID());

	if (region->get_map()) {
		return region->get_map()->get_self();
	}

	return RID();
}

RID PandemoniumNavigationServer::agent_get_map(RID p_agent) const {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND_V(agent == nullptr, RID());

	if (agent->get_map()) {
		return agent->get_map()->get_self();
	}

	return RID();
}

RID PandemoniumNavigationServer::region_create() {
	PandemoniumNavigationServer *mut_this = const_cast<PandemoniumNavigationServer *>(this);
	MutexLock lock(mut_this->operations_mutex);
	NavRegion *reg = memnew(NavRegion);
	RID rid = region_owner.make_rid(reg);
	reg->set_self(rid);
	return rid;
}

COMMAND_2(region_set_use_edge_connections, RID, p_region, bool, p_enabled) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);

	region->set_use_edge_connections(p_enabled);
}

bool PandemoniumNavigationServer::region_get_use_edge_connections(RID p_region) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(region == nullptr, false);

	return region->get_use_edge_connections();
}

COMMAND_2(region_set_map, RID, p_region, RID, p_map) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);

	if (region->get_map() != nullptr) {
		if (region->get_map()->get_self() == p_map) {
			return; // Pointless
		}

		region->get_map()->remove_region(region);
		region->set_map(nullptr);
	}

	if (p_map.is_valid()) {
		NavMap *map = map_owner.getornull(p_map);
		ERR_FAIL_COND(map == nullptr);

		map->add_region(region);
		region->set_map(map);
	}
}

COMMAND_2(region_set_transform, RID, p_region, Transform, p_transform) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);

	region->set_transform(p_transform);
}

COMMAND_2(region_set_enter_cost, RID, p_region, real_t, p_enter_cost) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);
	ERR_FAIL_COND(p_enter_cost < 0.0);

	region->set_enter_cost(p_enter_cost);
}

real_t PandemoniumNavigationServer::region_get_enter_cost(RID p_region) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(region == nullptr, 0);

	return region->get_enter_cost();
}

COMMAND_2(region_set_travel_cost, RID, p_region, real_t, p_travel_cost) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);
	ERR_FAIL_COND(p_travel_cost < 0.0);

	region->set_travel_cost(p_travel_cost);
}

real_t PandemoniumNavigationServer::region_get_travel_cost(RID p_region) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(region == nullptr, 0);

	return region->get_travel_cost();
}

COMMAND_2(region_set_owner_id, RID, p_region, ObjectID, p_owner_id) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);

	region->set_owner_id(p_owner_id);
}

ObjectID PandemoniumNavigationServer::region_get_owner_id(RID p_region) const {
	const NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(region == nullptr, ObjectID());

	return region->get_owner_id();
}

bool PandemoniumNavigationServer::region_owns_point(RID p_region, const Vector3 &p_point) const {
	const NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(region == nullptr, false);
	if (region->get_map()) {
		RID closest_point_owner = map_get_closest_point_owner(region->get_map()->get_self(), p_point);
		return closest_point_owner == region->get_self();
	}
	return false;
}

COMMAND_2(region_set_navigation_layers, RID, p_region, uint32_t, p_navigation_layers) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);

	region->set_navigation_layers(p_navigation_layers);
}

uint32_t PandemoniumNavigationServer::region_get_navigation_layers(RID p_region) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(region == nullptr, 0);

	return region->get_navigation_layers();
}

COMMAND_2(region_set_navigation_mesh, RID, p_region, Ref<NavigationMesh>, p_navigation_mesh) {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND(region == nullptr);

	region->set_mesh(p_navigation_mesh);
}

int PandemoniumNavigationServer::region_get_connections_count(RID p_region) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(!region, 0);

	return region->get_connections_count();
}

Vector3 PandemoniumNavigationServer::region_get_connection_pathway_start(RID p_region, int p_connection_id) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(!region, Vector3());

	return region->get_connection_pathway_start(p_connection_id);
}

Vector3 PandemoniumNavigationServer::region_get_connection_pathway_end(RID p_region, int p_connection_id) const {
	NavRegion *region = region_owner.getornull(p_region);
	ERR_FAIL_COND_V(!region, Vector3());

	return region->get_connection_pathway_end(p_connection_id);
}

RID PandemoniumNavigationServer::link_create() {
	PandemoniumNavigationServer *mut_this = const_cast<PandemoniumNavigationServer *>(this);
	MutexLock lock(mut_this->operations_mutex);
	NavLink *link = memnew(NavLink);
	RID rid = link_owner.make_rid(link);
	link->set_self(rid);
	return rid;
}

COMMAND_2(link_set_map, RID, p_link, RID, p_map) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	if (link->get_map() != nullptr) {
		if (link->get_map()->get_self() == p_map) {
			return; // Pointless
		}

		link->get_map()->remove_link(link);
		link->set_map(nullptr);
	}

	if (p_map.is_valid()) {
		NavMap *map = map_owner.getornull(p_map);
		ERR_FAIL_COND(map == nullptr);

		map->add_link(link);
		link->set_map(map);
	}
}

RID PandemoniumNavigationServer::link_get_map(const RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, RID());

	if (link->get_map()) {
		return link->get_map()->get_self();
	}
	return RID();
}

COMMAND_2(link_set_bidirectional, RID, p_link, bool, p_bidirectional) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	link->set_bidirectional(p_bidirectional);
}

bool PandemoniumNavigationServer::link_is_bidirectional(RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, false);

	return link->is_bidirectional();
}

COMMAND_2(link_set_navigation_layers, RID, p_link, uint32_t, p_navigation_layers) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	link->set_navigation_layers(p_navigation_layers);
}

uint32_t PandemoniumNavigationServer::link_get_navigation_layers(const RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, 0);

	return link->get_navigation_layers();
}

COMMAND_2(link_set_start_position, RID, p_link, Vector3, p_position) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	link->set_start_position(p_position);
}

Vector3 PandemoniumNavigationServer::link_get_start_position(RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, Vector3());

	return link->get_start_position();
}

COMMAND_2(link_set_end_position, RID, p_link, Vector3, p_position) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	link->set_end_position(p_position);
}

Vector3 PandemoniumNavigationServer::link_get_end_position(RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, Vector3());

	return link->get_end_position();
}

COMMAND_2(link_set_enter_cost, RID, p_link, real_t, p_enter_cost) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	link->set_enter_cost(p_enter_cost);
}

real_t PandemoniumNavigationServer::link_get_enter_cost(const RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, 0);

	return link->get_enter_cost();
}

COMMAND_2(link_set_travel_cost, RID, p_link, real_t, p_travel_cost) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	link->set_travel_cost(p_travel_cost);
}

real_t PandemoniumNavigationServer::link_get_travel_cost(const RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, 0);

	return link->get_travel_cost();
}

COMMAND_2(link_set_owner_id, RID, p_link, ObjectID, p_owner_id) {
	NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND(link == nullptr);

	link->set_owner_id(p_owner_id);
}

ObjectID PandemoniumNavigationServer::link_get_owner_id(RID p_link) const {
	const NavLink *link = link_owner.getornull(p_link);
	ERR_FAIL_COND_V(link == nullptr, ObjectID());

	return link->get_owner_id();
}

RID PandemoniumNavigationServer::agent_create() {
	PandemoniumNavigationServer *mut_this = const_cast<PandemoniumNavigationServer *>(this);
	MutexLock lock(mut_this->operations_mutex);
	NavAgent *agent = memnew(NavAgent());
	RID rid = agent_owner.make_rid(agent);
	agent->set_self(rid);
	return rid;
}

COMMAND_2(agent_set_avoidance_enabled, RID, p_agent, bool, p_enabled) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_avoidance_enabled(p_enabled);
}

bool PandemoniumNavigationServer::agent_get_avoidance_enabled(RID p_agent) const {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND_V(agent == nullptr, false);

	return agent->is_avoidance_enabled();
}

COMMAND_2(agent_set_use_3d_avoidance, RID, p_agent, bool, p_enabled) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_use_3d_avoidance(p_enabled);
}

bool PandemoniumNavigationServer::agent_get_use_3d_avoidance(RID p_agent) const {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND_V(agent == nullptr, false);

	return agent->get_use_3d_avoidance();
}

COMMAND_2(agent_set_map, RID, p_agent, RID, p_map) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	if (agent->get_map()) {
		if (agent->get_map()->get_self() == p_map) {
			return; // Pointless
		}

		agent->get_map()->remove_agent(agent);
	}

	agent->set_map(nullptr);

	if (p_map.is_valid()) {
		NavMap *map = map_owner.getornull(p_map);
		ERR_FAIL_COND(map == nullptr);

		agent->set_map(map);
		map->add_agent(agent);

		if (agent->has_avoidance_callback()) {
			map->set_agent_as_controlled(agent);
		}
	}
}

COMMAND_2(agent_set_paused, RID, p_agent, bool, p_paused) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_paused(p_paused);
}

bool PandemoniumNavigationServer::agent_get_paused(RID p_agent) const {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND_V(agent == nullptr, false);

	return agent->get_paused();
}

COMMAND_2(agent_set_neighbor_distance, RID, p_agent, real_t, p_dist) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_neighbor_distance(p_dist);
}

COMMAND_2(agent_set_max_neighbors, RID, p_agent, int, p_count) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_max_neighbors(p_count);
}

COMMAND_2(agent_set_time_horizon_agents, RID, p_agent, real_t, p_time_horizon) {
	ERR_FAIL_COND_MSG(p_time_horizon < 0.0, "Time horizion must be positive.");
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_time_horizon_agents(p_time_horizon);
}

COMMAND_2(agent_set_time_horizon_obstacles, RID, p_agent, real_t, p_time_horizon) {
	ERR_FAIL_COND_MSG(p_time_horizon < 0.0, "Time horizion must be positive.");
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_time_horizon_obstacles(p_time_horizon);
}

COMMAND_2(agent_set_radius, RID, p_agent, real_t, p_radius) {
	ERR_FAIL_COND_MSG(p_radius < 0.0, "Radius must be positive.");
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_radius(p_radius);
}

COMMAND_2(agent_set_height, RID, p_agent, real_t, p_height) {
	ERR_FAIL_COND_MSG(p_height < 0.0, "Height must be positive.");
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_height(p_height);
}

COMMAND_2(agent_set_max_speed, RID, p_agent, real_t, p_max_speed) {
	ERR_FAIL_COND_MSG(p_max_speed < 0.0, "Max speed must be positive.");
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_max_speed(p_max_speed);
}

COMMAND_2(agent_set_velocity, RID, p_agent, Vector3, p_velocity) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_velocity(p_velocity);
}

COMMAND_2(agent_set_velocity_forced, RID, p_agent, Vector3, p_velocity) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_velocity_forced(p_velocity);
}

COMMAND_2(agent_set_position, RID, p_agent, Vector3, p_position) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_position(p_position);
}

bool PandemoniumNavigationServer::agent_is_map_changed(RID p_agent) const {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND_V(agent == nullptr, false);

	return agent->is_map_changed();
}

COMMAND_4(agent_set_avoidance_callback, RID, p_agent, ObjectID, p_object_id, StringName, p_method, Variant, p_udata) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);

	agent->set_avoidance_callback(p_object_id, p_method, p_udata);

	if (agent->get_map()) {
		if (p_object_id == ObjectID()) {
			agent->get_map()->remove_agent_as_controlled(agent);
		} else {
			agent->get_map()->set_agent_as_controlled(agent);
		}
	}
}

COMMAND_2(agent_set_avoidance_layers, RID, p_agent, uint32_t, p_layers) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);
	agent->set_avoidance_layers(p_layers);
}

COMMAND_2(agent_set_avoidance_mask, RID, p_agent, uint32_t, p_mask) {
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);
	agent->set_avoidance_mask(p_mask);
}

COMMAND_2(agent_set_avoidance_priority, RID, p_agent, real_t, p_priority) {
	ERR_FAIL_COND_MSG(p_priority < 0.0, "Avoidance priority must be between 0.0 and 1.0 inclusive.");
	ERR_FAIL_COND_MSG(p_priority > 1.0, "Avoidance priority must be between 0.0 and 1.0 inclusive.");
	NavAgent *agent = agent_owner.getornull(p_agent);
	ERR_FAIL_COND(agent == nullptr);
	agent->set_avoidance_priority(p_priority);
}

RID PandemoniumNavigationServer::obstacle_create() {
	PandemoniumNavigationServer *mut_this = const_cast<PandemoniumNavigationServer *>(this);
	MutexLock lock(mut_this->operations_mutex);
	NavObstacle *obstacle = memnew(NavObstacle());
	RID rid = obstacle_owner.make_rid(obstacle);
	obstacle->set_self(rid);
	return rid;
}

COMMAND_2(obstacle_set_map, RID, p_obstacle, RID, p_map) {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND(obstacle == nullptr);

	if (obstacle->get_map()) {
		if (obstacle->get_map()->get_self() == p_map) {
			return; // Pointless
		}

		obstacle->get_map()->remove_obstacle(obstacle);
	}

	obstacle->set_map(nullptr);

	if (p_map.is_valid()) {
		NavMap *map = map_owner.getornull(p_map);
		ERR_FAIL_COND(map == nullptr);

		obstacle->set_map(map);
		map->add_obstacle(obstacle);
	}
}

RID PandemoniumNavigationServer::obstacle_get_map(RID p_obstacle) const {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND_V(obstacle == nullptr, RID());
	if (obstacle->get_map()) {
		return obstacle->get_map()->get_self();
	}
	return RID();
}

COMMAND_2(obstacle_set_paused, RID, p_obstacle, bool, p_paused) {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND(obstacle == nullptr);

	obstacle->set_paused(p_paused);
}

bool PandemoniumNavigationServer::obstacle_get_paused(RID p_obstacle) const {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND_V(obstacle == nullptr, false);

	return obstacle->get_paused();
}

COMMAND_2(obstacle_set_height, RID, p_obstacle, real_t, p_height) {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND(obstacle == nullptr);
	obstacle->set_height(p_height);
}

COMMAND_2(obstacle_set_position, RID, p_obstacle, Vector3, p_position) {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND(obstacle == nullptr);
	obstacle->set_position(p_position);
}

void PandemoniumNavigationServer::obstacle_set_vertices(RID p_obstacle, const Vector<Vector3> &p_vertices) {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND(obstacle == nullptr);
	obstacle->set_vertices(p_vertices);
}

COMMAND_2(obstacle_set_avoidance_layers, RID, p_obstacle, uint32_t, p_layers) {
	NavObstacle *obstacle = obstacle_owner.getornull(p_obstacle);
	ERR_FAIL_COND(obstacle == nullptr);
	obstacle->set_avoidance_layers(p_layers);
}

COMMAND_1(free, RID, p_object) {
	if (!p_object.is_valid()) {
		ERR_FAIL_MSG("Invalid RID.");
		return;
	}

	if (map_owner.owns(p_object)) {
		NavMap *map = map_owner.getornull(p_object);

		// Removes any assigned region
		LocalVector<NavRegion *> regions = map->get_regions();
		for (uint32_t i = 0; i < regions.size(); i++) {
			map->remove_region(regions[i]);
			regions[i]->set_map(nullptr);
		}

		// Removes any assigned links
		LocalVector<NavLink *> links = map->get_links();
		for (uint32_t i = 0; i < links.size(); i++) {
			map->remove_link(links[i]);
			links[i]->set_map(nullptr);
		}

		// Remove any assigned agent
		LocalVector<NavAgent *> agents = map->get_agents();
		for (uint32_t i = 0; i < agents.size(); i++) {
			map->remove_agent(agents[i]);
			agents[i]->set_map(nullptr);
		}

		// Remove any assigned obstacles
		LocalVector<NavObstacle *> obstacles = map->get_obstacles();
		for (uint32_t i = 0; i < obstacles.size(); i++) {
			map->remove_obstacle(obstacles[i]);
			obstacles[i]->set_map(nullptr);
		}

		int map_index = active_maps.find(map);
		active_maps.remove(map_index);
		active_maps_update_id.remove(map_index);
		map_owner.free(p_object);
		memdelete(map);

	} else if (region_owner.owns(p_object)) {
		NavRegion *region = region_owner.getornull(p_object);

		// Removes this region from the map if assigned
		if (region->get_map() != nullptr) {
			region->get_map()->remove_region(region);
			region->set_map(nullptr);
		}

		region_owner.free(p_object);
		memdelete(region);

	} else if (link_owner.owns(p_object)) {
		NavLink *link = link_owner.getornull(p_object);

		// Removes this link from the map if assigned
		if (link->get_map() != nullptr) {
			link->get_map()->remove_link(link);
			link->set_map(nullptr);
		}

		link_owner.free(p_object);

	} else if (agent_owner.owns(p_object)) {
		NavAgent *agent = agent_owner.getornull(p_object);

		// Removes this agent from the map if assigned
		if (agent->get_map() != nullptr) {
			agent->get_map()->remove_agent(agent);
			agent->set_map(nullptr);
		}

		agent_owner.free(p_object);
		memdelete(agent);

	} else if (obstacle_owner.owns(p_object)) {
		NavObstacle *obstacle = obstacle_owner.getornull(p_object);

		// Removes this agent from the map if assigned
		if (obstacle->get_map() != nullptr) {
			obstacle->get_map()->remove_obstacle(obstacle);
			obstacle->set_map(nullptr);
		}

		obstacle_owner.free(p_object);
	} else {
		ERR_PRINT("Attempted to free a NavigationServer RID that did not exist (or was already freed).");
	}
}

void PandemoniumNavigationServer::set_active(bool p_active) {
	PandemoniumNavigationServer *mut_this = const_cast<PandemoniumNavigationServer *>(this);
	MutexLock lock(mut_this->operations_mutex);
	mut_this->active = p_active;
}

void PandemoniumNavigationServer::flush_queries() {
	// In C++ we can't be sure that this is performed in the main thread
	// even with mutable functions.
	MutexLock lock(commands_mutex);
	MutexLock lock2(operations_mutex);
	for (size_t i(0); i < commands.size(); i++) {
		commands[i]->exec(this);
		memdelete(commands[i]);
	}
	commands.clear();
}

void PandemoniumNavigationServer::map_force_update(RID p_map) {
	NavMap *map = map_owner.getornull(p_map);
	ERR_FAIL_COND(map == nullptr);

	flush_queries();

	map->sync();
}

void PandemoniumNavigationServer::process(real_t p_delta_time) {
	flush_queries();

	if (!active) {
		return;
	}

	int _new_pm_region_count = 0;
	int _new_pm_agent_count = 0;
	int _new_pm_link_count = 0;
	int _new_pm_polygon_count = 0;
	int _new_pm_edge_count = 0;
	int _new_pm_edge_merge_count = 0;
	int _new_pm_edge_connection_count = 0;
	int _new_pm_edge_free_count = 0;

	// In c++ we can't be sure that this is performed in the main thread
	// even with mutable functions.
	MutexLock lock(operations_mutex);
	for (uint32_t i(0); i < active_maps.size(); i++) {
		active_maps[i]->sync();
		active_maps[i]->step(p_delta_time);
		active_maps[i]->dispatch_callbacks();

		_new_pm_region_count += active_maps[i]->get_pm_region_count();
		_new_pm_agent_count += active_maps[i]->get_pm_agent_count();
		_new_pm_link_count += active_maps[i]->get_pm_link_count();
		_new_pm_polygon_count += active_maps[i]->get_pm_polygon_count();
		_new_pm_edge_count += active_maps[i]->get_pm_edge_count();
		_new_pm_edge_merge_count += active_maps[i]->get_pm_edge_merge_count();
		_new_pm_edge_connection_count += active_maps[i]->get_pm_edge_connection_count();
		_new_pm_edge_free_count += active_maps[i]->get_pm_edge_free_count();

		// Emit a signal if a map changed.
		const uint32_t new_map_update_id = active_maps[i]->get_map_update_id();
		if (new_map_update_id != active_maps_update_id[i]) {
			emit_signal("map_changed", active_maps[i]->get_self());
			active_maps_update_id[i] = new_map_update_id;
		}
	}

	pm_region_count = _new_pm_region_count;
	pm_agent_count = _new_pm_agent_count;
	pm_link_count = _new_pm_link_count;
	pm_polygon_count = _new_pm_polygon_count;
	pm_edge_count = _new_pm_edge_count;
	pm_edge_merge_count = _new_pm_edge_merge_count;
	pm_edge_connection_count = _new_pm_edge_connection_count;
	pm_edge_free_count = _new_pm_edge_free_count;
}

PathQueryResult PandemoniumNavigationServer::_query_path(const PathQueryParameters &p_parameters) const {
	PathQueryResult r_query_result;

	const NavMap *map = map_owner.getornull(p_parameters.map);
	ERR_FAIL_COND_V(map == nullptr, r_query_result);

	// run the pathfinding

	if (p_parameters.pathfinding_algorithm == PathfindingAlgorithm::PATHFINDING_ALGORITHM_ASTAR) {
		Vector<Vector3> path;
		// while postprocessing is still part of map.get_path() need to check and route it here for the correct "optimize" post-processing
		if (p_parameters.path_postprocessing == PathPostProcessing::PATH_POSTPROCESSING_CORRIDORFUNNEL) {
			r_query_result.path = map->get_path(
					p_parameters.start_position,
					p_parameters.target_position,
					true,
					p_parameters.navigation_layers,
					((p_parameters.metadata_flags & PathMetadataFlags::PATH_INCLUDE_TYPES) != 0) ? &r_query_result.path_types : nullptr,
					((p_parameters.metadata_flags & PathMetadataFlags::PATH_INCLUDE_RIDS) != 0) ? &r_query_result.path_rids : nullptr,
					((p_parameters.metadata_flags & PathMetadataFlags::PATH_INCLUDE_OWNERS) != 0) ? &r_query_result.path_owner_ids : nullptr);
		} else if (p_parameters.path_postprocessing == PathPostProcessing::PATH_POSTPROCESSING_EDGECENTERED) {
			r_query_result.path = map->get_path(
					p_parameters.start_position,
					p_parameters.target_position,
					false,
					p_parameters.navigation_layers,
					((p_parameters.metadata_flags & PathMetadataFlags::PATH_INCLUDE_TYPES) != 0) ? &r_query_result.path_types : nullptr,
					((p_parameters.metadata_flags & PathMetadataFlags::PATH_INCLUDE_RIDS) != 0) ? &r_query_result.path_rids : nullptr,
					((p_parameters.metadata_flags & PathMetadataFlags::PATH_INCLUDE_OWNERS) != 0) ? &r_query_result.path_owner_ids : nullptr);
		}

		r_query_result.path.resize(path.size());

		Vector3 *wptr = r_query_result.path.ptrw();
		const Vector3 *pptr = path.ptr();

		for (int i = 0; i < path.size(); ++i) {
			wptr[i] = pptr[i];
		}
	} else {
		return r_query_result;
	}

	// add path postprocessing

	// add path stats

	return r_query_result;
}

int PandemoniumNavigationServer::get_process_info(ProcessInfo p_info) const {
	switch (p_info) {
		case INFO_ACTIVE_MAPS: {
			return active_maps.size();
		} break;
		case INFO_REGION_COUNT: {
			return pm_region_count;
		} break;
		case INFO_AGENT_COUNT: {
			return pm_agent_count;
		} break;
		case INFO_LINK_COUNT: {
			return pm_link_count;
		} break;
		case INFO_POLYGON_COUNT: {
			return pm_polygon_count;
		} break;
		case INFO_EDGE_COUNT: {
			return pm_edge_count;
		} break;
		case INFO_EDGE_MERGE_COUNT: {
			return pm_edge_merge_count;
		} break;
		case INFO_EDGE_CONNECTION_COUNT: {
			return pm_edge_connection_count;
		} break;
		case INFO_EDGE_FREE_COUNT: {
			return pm_edge_free_count;
		} break;
	}

	return 0;
}

#undef COMMAND_1
#undef COMMAND_2
#undef COMMAND_2N
#undef COMMAND_4
