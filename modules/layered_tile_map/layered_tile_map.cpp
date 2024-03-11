/*************************************************************************/
/*  layered_tile_map.cpp                                                 */
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

#include "layered_tile_map.h"

#include "core/config/engine.h"
#include "core/core_string_names.h"
#include "layered_tile_map_layer.h"
#include "scene/main/control.h"

#define TILEMAP_CALL_FOR_LAYER(layer, function, ...) \
	if (layer < 0) {                                 \
		layer = layers.size() + layer;               \
	};                                               \
	ERR_FAIL_INDEX(layer, (int)layers.size());       \
	layers[layer]->function(__VA_ARGS__);

#define TILEMAP_CALL_FOR_LAYER_V(layer, err_value, function, ...) \
	if (layer < 0) {                                              \
		layer = layers.size() + layer;                            \
	};                                                            \
	ERR_FAIL_INDEX_V(layer, (int)layers.size(), err_value);       \
	return layers[layer]->function(__VA_ARGS__);

void LayeredTileMap::_emit_changed() {
	emit_signal(CoreStringNames::get_singleton()->changed);
}

void LayeredTileMap::_notification(int p_what) {
	switch (p_what) {
		case LayeredTileMap::NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			// This is only executed when collision_animatable is enabled.

			bool in_editor = false;
#ifdef TOOLS_ENABLED
			in_editor = Engine::get_singleton()->is_editor_hint();
#endif
			if (is_inside_tree() && collision_animatable && !in_editor) {
				// Update transform on the physics tick when in animatable mode.
				last_valid_transform = new_transform;
				set_notify_local_transform(false);
				set_global_transform(new_transform);
				set_notify_local_transform(true);
			}
		} break;

		case LayeredTileMap::NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			// This is only executed when collision_animatable is enabled.

			bool in_editor = false;
#ifdef TOOLS_ENABLED
			in_editor = Engine::get_singleton()->is_editor_hint();
#endif

			if (is_inside_tree() && collision_animatable && !in_editor) {
				// Store last valid transform.
				new_transform = get_global_transform();

				// ... but then revert changes.
				set_notify_local_transform(false);
				set_global_transform(last_valid_transform);
				set_notify_local_transform(true);
			}
		} break;
	}
}

#ifndef DISABLE_DEPRECATED
// Deprecated methods.
void LayeredTileMap::force_update(int p_layer) {
	notify_runtime_tile_data_update(p_layer);
	update_internals();
}
#endif

void LayeredTileMap::set_rendering_quadrant_size(int p_size) {
	ERR_FAIL_COND_MSG(p_size < 1, "LayeredTileMapQuadrant size cannot be smaller than 1.");

	rendering_quadrant_size = p_size;

	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];

		layer->set_rendering_quadrant_size(p_size);
	}
	_emit_changed();
}

int LayeredTileMap::get_rendering_quadrant_size() const {
	return rendering_quadrant_size;
}

void LayeredTileMap::draw_tile(RID p_canvas_item, const Vector2 &p_position, const Ref<LayeredTileSet> p_tile_set, int p_atlas_source_id, const Vector2i &p_atlas_coords, int p_alternative_tile, int p_frame, Color p_modulation, const LayeredTileData *p_tile_data_override, real_t p_normalized_animation_offset) {
	ERR_FAIL_COND(!p_tile_set.is_valid());
	ERR_FAIL_COND(!p_tile_set->has_source(p_atlas_source_id));
	ERR_FAIL_COND(!p_tile_set->get_source(p_atlas_source_id)->has_tile(p_atlas_coords));
	ERR_FAIL_COND(!p_tile_set->get_source(p_atlas_source_id)->has_alternative_tile(p_atlas_coords, p_alternative_tile));
	LayeredTileSetSource *source = *p_tile_set->get_source(p_atlas_source_id);
	LayeredTileSetAtlasSource *atlas_source = Object::cast_to<LayeredTileSetAtlasSource>(source);
	if (atlas_source) {
		// Check for the frame.
		if (p_frame >= 0) {
			ERR_FAIL_INDEX(p_frame, atlas_source->get_tile_animation_frames_count(p_atlas_coords));
		}

		// Get the texture.
		Ref<Texture> tex = atlas_source->get_runtime_texture();
		if (!tex.is_valid()) {
			return;
		}

		// Check if we are in the texture, return otherwise.
		Vector2i grid_size = atlas_source->get_atlas_grid_size();
		if (p_atlas_coords.x >= grid_size.x || p_atlas_coords.y >= grid_size.y) {
			return;
		}

		// Get tile data.
		const LayeredTileData *tile_data = p_tile_data_override ? p_tile_data_override : atlas_source->get_tile_data(p_atlas_coords, p_alternative_tile);

		// Get the tile modulation.
		Color modulate = tile_data->get_modulate() * p_modulation;

		// Compute the offset.
		Vector2 tile_offset = tile_data->get_texture_origin();

		// Get destination rect.
		Rect2 dest_rect;
		dest_rect.size = atlas_source->get_runtime_tile_texture_region(p_atlas_coords).size;
		dest_rect.size.x += FP_ADJUST;
		dest_rect.size.y += FP_ADJUST;

		bool transpose = tile_data->get_transpose() ^ bool(p_alternative_tile & LayeredTileSetAtlasSource::TRANSFORM_TRANSPOSE);
		if (transpose) {
			dest_rect.position = (p_position - Vector2(dest_rect.size.y, dest_rect.size.x) / 2 - tile_offset);
		} else {
			dest_rect.position = (p_position - dest_rect.size / 2 - tile_offset);
		}

		if (tile_data->get_flip_h() ^ bool(p_alternative_tile & LayeredTileSetAtlasSource::TRANSFORM_FLIP_H)) {
			dest_rect.size.x = -dest_rect.size.x;
		}

		if (tile_data->get_flip_v() ^ bool(p_alternative_tile & LayeredTileSetAtlasSource::TRANSFORM_FLIP_V)) {
			dest_rect.size.y = -dest_rect.size.y;
		}

		// Draw the tile.
		if (p_frame >= 0) {
			Rect2i source_rect = atlas_source->get_runtime_tile_texture_region(p_atlas_coords, p_frame);
			tex->draw_rect_region(p_canvas_item, dest_rect, source_rect, modulate, transpose, Ref<Texture>(), p_tile_set->is_uv_clipping());
		} else if (atlas_source->get_tile_animation_frames_count(p_atlas_coords) == 1) {
			Rect2i source_rect = atlas_source->get_runtime_tile_texture_region(p_atlas_coords, 0);
			tex->draw_rect_region(p_canvas_item, dest_rect, source_rect, modulate, transpose, Ref<Texture>(), p_tile_set->is_uv_clipping());
		} else {
			real_t speed = atlas_source->get_tile_animation_speed(p_atlas_coords);

			Array anim_data;
			
			for (int frame = 0; frame < atlas_source->get_tile_animation_frames_count(p_atlas_coords); frame++) {
				real_t frame_duration_scaled = atlas_source->get_tile_animation_frame_duration(p_atlas_coords, frame) * speed;
				
				RID tex_rid = tex.is_valid() ? tex->get_rid() : RID();
				//RID normal_rid = p_normal_map.is_valid() ? p_normal_map->get_rid() : RID();
				RID normal_rid = RID();
				
				Rect2i source_rect = atlas_source->get_runtime_tile_texture_region(p_atlas_coords, frame);
				
				//tex->draw_rect_region(p_canvas_item, dest_rect, source_rect, modulate, transpose, Ref<Texture>(), p_tile_set->is_uv_clipping());
				
				Array d;
				d.resize(8);
				
				//real_t frame_time = d[0];
				//Rect2 tex_rect = d[1]; //const Rect2 &p_rect
				//rect->texture = d[2]; //RID p_texture
				//Rect2 tex_src_rect = d[3]; //const Rect2 &p_src_rect
				//rect->modulate = d[4]; //const Color &p_modulate = Color(1, 1, 1)
				//bool transpose = d[5]; //bool p_transpose = false
				//rect->normal_map = d[6]; //RID p_normal_map = RID()
				//bool clip_uv = d[7]; //bool p_clip_uv = false
				
				d[0] = frame_duration_scaled;
				d[1] = dest_rect;
				d[2] = tex_rid;
				d[3] = source_rect;
				d[4] = modulate;
				d[5] = transpose;
				d[6] = normal_rid;
				d[7] = p_tile_set->is_uv_clipping();

				anim_data.push_back(d);
			}
			
			bool random_start_time = atlas_source->get_tile_animation_mode(p_atlas_coords) == LayeredTileSetAtlasSource::TILE_ANIMATION_MODE_RANDOM_START_TIMES;
			
			RenderingServer::get_singleton()->canvas_item_add_texture_rect_animation(p_canvas_item, anim_data, random_start_time);
		}
	}
}

int LayeredTileMap::get_layers_count() const {
	return layers.size();
}

void LayeredTileMap::add_layer(int p_to_pos) {
	if (p_to_pos < 0) {
		p_to_pos = layers.size() + p_to_pos + 1;
	}

	ERR_FAIL_INDEX(p_to_pos, (int)layers.size() + 1);

	// Must clear before adding the layer.
	LayeredTileMapLayer *new_layer = memnew(LayeredTileMapLayer);
	layers.insert(p_to_pos, new_layer);
	add_child(new_layer, false);
	move_child(new_layer, 0);
	new_layer->set_name(vformat("Layer%d", p_to_pos));
	move_child(new_layer, p_to_pos);
	for (uint32_t i = 0; i < layers.size(); i++) {
		layers[i]->set_as_tile_map_internal_node(i);
	}

	new_layer->connect(CoreStringNames::get_singleton()->changed, this, "_emit_changed");

	property_list_changed_notify();

	_emit_changed();

	update_configuration_warning();
}

void LayeredTileMap::move_layer(int p_layer, int p_to_pos) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());
	ERR_FAIL_INDEX(p_to_pos, (int)layers.size() + 1);

	// Clear before shuffling layers.
	LayeredTileMapLayer *layer = layers[p_layer];
	layers.insert(p_to_pos, layer);
	layers.remove(p_to_pos < p_layer ? p_layer + 1 : p_layer);
	for (uint32_t i = 0; i < layers.size(); i++) {
		move_child(layers[i], i);
		layers[i]->set_as_tile_map_internal_node(i);
	}
	property_list_changed_notify();

	_emit_changed();

	update_configuration_warning();
}

void LayeredTileMap::remove_layer(int p_layer) {
	ERR_FAIL_INDEX(p_layer, (int)layers.size());

	// Clear before removing the layer.
	layers[p_layer]->queue_delete();

	layers.remove(p_layer);
	for (uint32_t i = 0; i < layers.size(); i++) {
		layers[i]->set_as_tile_map_internal_node(i);
	}
	property_list_changed_notify();

	_emit_changed();

	update_configuration_warning();
}

void LayeredTileMap::set_layer_name(int p_layer, String p_name) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_name, p_name);
}

String LayeredTileMap::get_layer_name(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, "", get_name);
}

void LayeredTileMap::set_layer_enabled(int p_layer, bool p_enabled) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_enabled, p_enabled);
}

bool LayeredTileMap::is_layer_enabled(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, false, is_enabled);
}

void LayeredTileMap::set_layer_modulate(int p_layer, Color p_modulate) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_modulate, p_modulate);
}

Color LayeredTileMap::get_layer_modulate(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, Color(), get_modulate);
}

void LayeredTileMap::set_layer_y_sort_enabled(int p_layer, bool p_y_sort_enabled) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_y_sort_enabled, p_y_sort_enabled);
}

bool LayeredTileMap::is_layer_y_sort_enabled(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, false, is_sort_enabled);
}

void LayeredTileMap::set_layer_y_sort_origin(int p_layer, int p_y_sort_origin) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_y_sort_origin, p_y_sort_origin);
}

int LayeredTileMap::get_layer_y_sort_origin(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, 0, get_y_sort_origin);
}

void LayeredTileMap::set_layer_z_index(int p_layer, int p_z_index) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_z_index, p_z_index);
}

int LayeredTileMap::get_layer_z_index(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, 0, get_z_index);
}

void LayeredTileMap::set_layer_navigation_enabled(int p_layer, bool p_enabled) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_navigation_enabled, p_enabled);
}

bool LayeredTileMap::is_layer_navigation_enabled(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, false, is_navigation_enabled);
}

void LayeredTileMap::set_layer_navigation_map(int p_layer, RID p_map) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_navigation_map, p_map);
}

RID LayeredTileMap::get_layer_navigation_map(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, RID(), get_navigation_map);
}

void LayeredTileMap::set_collision_animatable(bool p_collision_animatable) {
	if (collision_animatable == p_collision_animatable) {
		return;
	}
	collision_animatable = p_collision_animatable;
	set_notify_local_transform(p_collision_animatable);
	set_physics_process_internal(p_collision_animatable);

	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];

		layer->set_use_kinematic_bodies(layer);
	}
}

bool LayeredTileMap::is_collision_animatable() const {
	return collision_animatable;
}

void LayeredTileMap::set_collision_visibility_mode(LayeredTileMap::VisibilityMode p_show_collision) {
	if (collision_visibility_mode == p_show_collision) {
		return;
	}
	collision_visibility_mode = p_show_collision;

	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];

		layer->set_collision_visibility_mode(LayeredTileMapLayer::VisibilityMode(p_show_collision));
	}

	_emit_changed();
}

LayeredTileMap::VisibilityMode LayeredTileMap::get_collision_visibility_mode() const {
	return collision_visibility_mode;
}

void LayeredTileMap::set_navigation_visibility_mode(LayeredTileMap::VisibilityMode p_show_navigation) {
	if (navigation_visibility_mode == p_show_navigation) {
		return;
	}
	navigation_visibility_mode = p_show_navigation;

	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];
		layer->set_navigation_visibility_mode(LayeredTileMapLayer::VisibilityMode(p_show_navigation));
	}

	_emit_changed();
}

LayeredTileMap::VisibilityMode LayeredTileMap::get_navigation_visibility_mode() const {
	return navigation_visibility_mode;
}

void LayeredTileMap::set_y_sort_enabled(bool p_enable) {
	if (is_sort_enabled() == p_enable) {
		return;
	}

	YSort::set_sort_enabled(p_enable);

	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];

		layer->set_y_sort_enabled(p_enable);
	}

	_emit_changed();
	update_configuration_warning();
}

void LayeredTileMap::set_cell(int p_layer, const Vector2i &p_coords, int p_source_id, const Vector2i p_atlas_coords, int p_alternative_tile) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_cell, p_coords, p_source_id, p_atlas_coords, p_alternative_tile);
}

void LayeredTileMap::erase_cell(int p_layer, const Vector2i &p_coords) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_cell, p_coords, LayeredTileSet::INVALID_SOURCE, LayeredTileSetSource::INVALID_ATLAS_COORDS, LayeredTileSetSource::INVALID_TILE_ALTERNATIVE);
}

int LayeredTileMap::get_cell_source_id(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, LayeredTileSet::INVALID_SOURCE, get_cell_source_id, p_coords, p_use_proxies);
}

Vector2i LayeredTileMap::get_cell_atlas_coords(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, LayeredTileSetSource::INVALID_ATLAS_COORDS, get_cell_atlas_coords, p_coords, p_use_proxies);
}

int LayeredTileMap::get_cell_alternative_tile(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, LayeredTileSetSource::INVALID_TILE_ALTERNATIVE, get_cell_alternative_tile, p_coords, p_use_proxies);
}

LayeredTileData *LayeredTileMap::get_cell_tile_data(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, nullptr, get_cell_tile_data, p_coords, p_use_proxies);
}

Ref<LayeredTileMapPattern> LayeredTileMap::get_pattern(int p_layer, PoolVector2iArray p_coords_array) {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, Ref<LayeredTileMapPattern>(), get_pattern, p_coords_array);
}

Vector2i LayeredTileMap::map_pattern(const Vector2i &p_position_in_tilemap, const Vector2i &p_coords_in_pattern, Ref<LayeredTileMapPattern> p_pattern) {
	ERR_FAIL_COND_V(!tile_set.is_valid(), Vector2i());
	return tile_set->map_pattern(p_position_in_tilemap, p_coords_in_pattern, p_pattern);
}

void LayeredTileMap::set_pattern(int p_layer, const Vector2i &p_position, const Ref<LayeredTileMapPattern> p_pattern) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_pattern, p_position, p_pattern);
}

HashMap<Vector2i, LayeredTileSet::TerrainsPattern> LayeredTileMap::terrain_fill_constraints(int p_layer, const Vector<Vector2i> &p_to_replace, int p_terrain_set, const RBSet<TerrainConstraint> &p_constraints) {
	HashMap<Vector2i, LayeredTileSet::TerrainsPattern> err_value;
	TILEMAP_CALL_FOR_LAYER_V(p_layer, err_value, terrain_fill_constraints, p_to_replace, p_terrain_set, p_constraints);
}

HashMap<Vector2i, LayeredTileSet::TerrainsPattern> LayeredTileMap::terrain_fill_connect(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	HashMap<Vector2i, LayeredTileSet::TerrainsPattern> err_value;
	TILEMAP_CALL_FOR_LAYER_V(p_layer, err_value, terrain_fill_connect, p_coords_array, p_terrain_set, p_terrain, p_ignore_empty_terrains);
}

HashMap<Vector2i, LayeredTileSet::TerrainsPattern> LayeredTileMap::terrain_fill_path(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	HashMap<Vector2i, LayeredTileSet::TerrainsPattern> err_value;
	TILEMAP_CALL_FOR_LAYER_V(p_layer, err_value, terrain_fill_path, p_coords_array, p_terrain_set, p_terrain, p_ignore_empty_terrains);
}

HashMap<Vector2i, LayeredTileSet::TerrainsPattern> LayeredTileMap::terrain_fill_pattern(int p_layer, const Vector<Vector2i> &p_coords_array, int p_terrain_set, LayeredTileSet::TerrainsPattern p_terrains_pattern, bool p_ignore_empty_terrains) {
	HashMap<Vector2i, LayeredTileSet::TerrainsPattern> err_value;
	TILEMAP_CALL_FOR_LAYER_V(p_layer, err_value, terrain_fill_pattern, p_coords_array, p_terrain_set, p_terrains_pattern, p_ignore_empty_terrains);
}

void LayeredTileMap::set_cells_terrain_connect(int p_layer, PoolVector2iArray p_cells, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_cells_terrain_connect, p_cells, p_terrain_set, p_terrain, p_ignore_empty_terrains);
}

void LayeredTileMap::set_cells_terrain_path(int p_layer, PoolVector2iArray p_path, int p_terrain_set, int p_terrain, bool p_ignore_empty_terrains) {
	TILEMAP_CALL_FOR_LAYER(p_layer, set_cells_terrain_path, p_path, p_terrain_set, p_terrain, p_ignore_empty_terrains);
}

LayeredTileMapCell LayeredTileMap::get_cell(int p_layer, const Vector2i &p_coords, bool p_use_proxies) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, LayeredTileMapCell(), get_cell, p_coords, p_use_proxies);
}

Vector2i LayeredTileMap::get_coords_for_body_rid(RID p_physics_body) {
	for (uint32_t i = 0; i < layers.size(); ++i) {
		const LayeredTileMapLayer *layer = layers[i];

		if (layer->has_body_rid(p_physics_body)) {
			return layer->get_coords_for_body_rid(p_physics_body);
		}
	}

	ERR_FAIL_V_MSG(Vector2i(), vformat("No tiles for the given body RID %d.", p_physics_body.get_id()));
}

int LayeredTileMap::get_layer_for_body_rid(RID p_physics_body) {
	for (uint32_t i = 0; i < layers.size(); i++) {
		if (layers[i]->has_body_rid(p_physics_body)) {
			return i;
		}
	}
	ERR_FAIL_V_MSG(-1, vformat("No tiles for the given body RID %d.", p_physics_body.get_id()));
}

void LayeredTileMap::fix_invalid_tiles() {
	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];
		layer->fix_invalid_tiles();
	}
}

void LayeredTileMap::clear_layer(int p_layer) {
	TILEMAP_CALL_FOR_LAYER(p_layer, clear)
}

void LayeredTileMap::clear() {
	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];
		layer->clear();
	}
}

void LayeredTileMap::update_internals() {
	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];
		layer->update_internals();
	}
}

void LayeredTileMap::notify_runtime_tile_data_update(int p_layer) {
	if (p_layer >= 0) {
		TILEMAP_CALL_FOR_LAYER(p_layer, notify_runtime_tile_data_update);
	} else {
		for (uint32_t i = 0; i < layers.size(); ++i) {
			LayeredTileMapLayer *layer = layers[i];
			layer->notify_runtime_tile_data_update();
		}
	}
}

#ifdef TOOLS_ENABLED
Rect2 LayeredTileMap::_edit_get_rect() const {
	// Return the visible rect of the tilemap.
	if (layers.empty()) {
		return Rect2();
	}

	bool any_changed = false;
	bool changed = false;
	Rect2 rect = layers[0]->get_rect(changed);
	any_changed |= changed;
	for (uint32_t i = 1; i < layers.size(); i++) {
		rect = rect.merge(layers[i]->get_rect(changed));
		any_changed |= changed;
	}
	const_cast<LayeredTileMap *>(this)->item_rect_changed(any_changed);
	return rect;
}
#endif

bool LayeredTileMap::_set(const StringName &p_name, const Variant &p_value) {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (p_name == "format") {
		if (p_value.get_type() == Variant::INT) {
			format = (LayeredTileMapDataFormat)(p_value.operator int64_t()); // Set format used for loading.
			return true;
		}
	}
#ifndef DISABLE_DEPRECATED
	else if (p_name == "tile_data") { // Kept for compatibility reasons.
		if (p_value.is_array()) {
			if (layers.size() == 0) {
				LayeredTileMapLayer *new_layer = memnew(LayeredTileMapLayer);
				add_child(new_layer);
				move_child(new_layer, 0);
				new_layer->set_as_tile_map_internal_node(0);
				new_layer->set_name("Layer0");
				new_layer->connect(CoreStringNames::get_singleton()->changed, this, "_emit_changed");
				layers.push_back(new_layer);
			}
			layers[0]->set_tile_data(format, p_value);
			_emit_changed();
			return true;
		}
		return false;
	} else if (p_name == "cell_quadrant_size") {
		set_rendering_quadrant_size(p_value);
		return true;
	}
#endif // DISABLE_DEPRECATED
	else if (components.size() == 2 && components[0].begins_with("layer_") && components[0].trim_prefix("layer_").is_valid_integer()) {
		int index = components[0].trim_prefix("layer_").to_int();
		if (index < 0) {
			return false;
		}

		if (index >= (int)layers.size()) {
			while (index >= (int)layers.size()) {
				LayeredTileMapLayer *new_layer = memnew(LayeredTileMapLayer);
				add_child(new_layer);
				move_child(new_layer, 0);
				new_layer->set_as_tile_map_internal_node(index);
				new_layer->set_name(vformat("Layer%d", index));
				new_layer->connect(CoreStringNames::get_singleton()->changed, this, "_emit_changed");
				layers.push_back(new_layer);
			}

			property_list_changed_notify();
			_emit_changed();
			update_configuration_warning();
		}

		if (components[1] == "name") {
			set_layer_name(index, p_value);
			return true;
		} else if (components[1] == "enabled") {
			set_layer_enabled(index, p_value);
			return true;
		} else if (components[1] == "modulate") {
			set_layer_modulate(index, p_value);
			return true;
		} else if (components[1] == "y_sort_enabled") {
			set_layer_y_sort_enabled(index, p_value);
			return true;
		} else if (components[1] == "y_sort_origin") {
			set_layer_y_sort_origin(index, p_value);
			return true;
		} else if (components[1] == "z_index") {
			set_layer_z_index(index, p_value);
			return true;
		} else if (components[1] == "navigation_enabled") {
			set_layer_navigation_enabled(index, p_value);
			return true;
		} else if (components[1] == "tile_data") {
			layers[index]->set_tile_data(format, p_value);
			_emit_changed();
			return true;
		} else {
			return false;
		}
	}
	return false;
}

bool LayeredTileMap::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (p_name == "format") {
		r_ret = LayeredTileMapDataFormat::FORMAT_MAX - 1; // When saving, always save highest format.
		return true;
	}
#ifndef DISABLE_DEPRECATED
	else if (p_name == "cell_quadrant_size") { // Kept for compatibility reasons.
		r_ret = get_rendering_quadrant_size();
		return true;
	}
#endif
	else if (components.size() == 2 && components[0].begins_with("layer_") && components[0].trim_prefix("layer_").is_valid_integer()) {
		int index = components[0].trim_prefix("layer_").to_int();
		if (index < 0 || index >= (int)layers.size()) {
			return false;
		}

		if (components[1] == "name") {
			r_ret = get_layer_name(index);
			return true;
		} else if (components[1] == "enabled") {
			r_ret = is_layer_enabled(index);
			return true;
		} else if (components[1] == "modulate") {
			r_ret = get_layer_modulate(index);
			return true;
		} else if (components[1] == "y_sort_enabled") {
			r_ret = is_layer_y_sort_enabled(index);
			return true;
		} else if (components[1] == "y_sort_origin") {
			r_ret = get_layer_y_sort_origin(index);
			return true;
		} else if (components[1] == "z_index") {
			r_ret = get_layer_z_index(index);
			return true;
		} else if (components[1] == "navigation_enabled") {
			r_ret = is_layer_navigation_enabled(index);
			return true;
		} else if (components[1] == "tile_data") {
			r_ret = layers[index]->get_tile_data();
			return true;
		} else {
			return false;
		}
	}
	return false;
}

void LayeredTileMap::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::INT, "format", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL));
	p_list->push_back(PropertyInfo(Variant::NIL, "Layers", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_GROUP));

#define MAKE_LAYER_PROPERTY(m_type, m_name, m_hint)                                                                 \
	{                                                                                                               \
		const String property_name = vformat("layer_%d/" m_name, i);                                                \
		p_list->push_back(PropertyInfo(m_type, property_name, PROPERTY_HINT_NONE, m_hint, PROPERTY_USAGE_DEFAULT)); \
	}

	for (uint32_t i = 0; i < layers.size(); i++) {
		MAKE_LAYER_PROPERTY(Variant::STRING, "name", "");
		MAKE_LAYER_PROPERTY(Variant::BOOL, "enabled", "");
		MAKE_LAYER_PROPERTY(Variant::COLOR, "modulate", "");
		MAKE_LAYER_PROPERTY(Variant::BOOL, "y_sort_enabled", "");
		MAKE_LAYER_PROPERTY(Variant::INT, "y_sort_origin", "suffix:px");
		MAKE_LAYER_PROPERTY(Variant::INT, "z_index", "");
		MAKE_LAYER_PROPERTY(Variant::BOOL, "navigation_enabled", "");
		p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("layer_%d/tile_data", i), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR));
	}

#undef MAKE_LAYER_PROPERTY
}

bool LayeredTileMap::_property_can_revert(const StringName &p_name) const {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (components.size() == 2 && components[0].begins_with("layer_")) {
		int index = components[0].trim_prefix("layer_").to_int();
		if (index <= 0 || index >= (int)layers.size()) {
			return false;
		}

		if (components[1] == "name") {
			return layers[index]->get_name() != default_layer->get_name();
		} else if (components[1] == "enabled") {
			return layers[index]->is_enabled() != default_layer->is_enabled();
		} else if (components[1] == "modulate") {
			return layers[index]->get_modulate() != default_layer->get_modulate();
		} else if (components[1] == "y_sort_enabled") {
			return layers[index]->is_sort_enabled() != default_layer->is_sort_enabled();
		} else if (components[1] == "y_sort_origin") {
			return layers[index]->get_y_sort_origin() != default_layer->get_y_sort_origin();
		} else if (components[1] == "z_index") {
			return layers[index]->get_z_index() != default_layer->get_z_index();
		} else if (components[1] == "navigation_enabled") {
			return layers[index]->is_navigation_enabled() != default_layer->is_navigation_enabled();
		}
	}

	return false;
}

bool LayeredTileMap::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	Vector<String> components = String(p_name).split("/", true, 2);
	if (components.size() == 2 && components[0].begins_with("layer_")) {
		int index = components[0].trim_prefix("layer_").to_int();
		if (index <= 0 || index >= (int)layers.size()) {
			return false;
		}

		if (components[1] == "name") {
			r_property = default_layer->get_name();
			return true;
		} else if (components[1] == "enabled") {
			r_property = default_layer->is_enabled();
			return true;
		} else if (components[1] == "modulate") {
			r_property = default_layer->get_modulate();
			return true;
		} else if (components[1] == "y_sort_enabled") {
			r_property = default_layer->is_sort_enabled();
			return true;
		} else if (components[1] == "y_sort_origin") {
			r_property = default_layer->get_y_sort_origin();
			return true;
		} else if (components[1] == "z_index") {
			r_property = default_layer->get_z_index();
			return true;
		} else if (components[1] == "navigation_enabled") {
			r_property = default_layer->is_navigation_enabled();
			return true;
		}
	}

	return false;
}

Vector2 LayeredTileMap::map_to_local(const Vector2i &p_pos) const {
	ERR_FAIL_COND_V(!tile_set.is_valid(), Vector2());
	return tile_set->map_to_local(p_pos);
}

Vector2i LayeredTileMap::local_to_map(const Vector2 &p_pos) const {
	ERR_FAIL_COND_V(!tile_set.is_valid(), Vector2i());
	return tile_set->local_to_map(p_pos);
}

bool LayeredTileMap::is_existing_neighbor(LayeredTileSet::CellNeighbor p_cell_neighbor) const {
	ERR_FAIL_COND_V(!tile_set.is_valid(), false);
	return tile_set->is_existing_neighbor(p_cell_neighbor);
}

Vector2i LayeredTileMap::get_neighbor_cell(const Vector2i &p_coords, LayeredTileSet::CellNeighbor p_cell_neighbor) const {
	ERR_FAIL_COND_V(!tile_set.is_valid(), Vector2i());
	return tile_set->get_neighbor_cell(p_coords, p_cell_neighbor);
}

PoolVector2iArray LayeredTileMap::get_used_cells(int p_layer) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, PoolVector2iArray(), get_used_cells);
}

PoolVector2iArray LayeredTileMap::get_used_cells_by_id(int p_layer, int p_source_id, const Vector2i p_atlas_coords, int p_alternative_tile) const {
	TILEMAP_CALL_FOR_LAYER_V(p_layer, PoolVector2iArray(), get_used_cells_by_id, p_source_id, p_atlas_coords, p_alternative_tile);
}

Rect2i LayeredTileMap::get_used_rect() const {
	// Return the visible rect of the tilemap.
	bool first = true;
	Rect2i rect = Rect2i();

	for (uint32_t i = 0; i < layers.size(); ++i) {
		const LayeredTileMapLayer *layer = layers[i];

		Rect2i layer_rect = layer->get_used_rect();
		if (layer_rect == Rect2i()) {
			continue;
		}
		if (first) {
			rect = layer_rect;
			first = false;
		} else {
			rect = rect.merge(layer_rect);
		}
	}
	return rect;
}

// --- Override some methods of the CanvasItem class to pass the changes to the quadrants CanvasItems ---

void LayeredTileMap::set_light_mask(int p_light_mask) {
	// Set light mask for occlusion and applies it to all layers too.
	CanvasItem::set_light_mask(p_light_mask);
	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];
		layer->set_light_mask(p_light_mask);
	}
}

void LayeredTileMap::set_self_modulate(const Color &p_self_modulate) {
	// Set self_modulation and applies it to all layers too.
	CanvasItem::set_self_modulate(p_self_modulate);
	for (uint32_t i = 0; i < layers.size(); ++i) {
		LayeredTileMapLayer *layer = layers[i];
		layer->set_self_modulate(p_self_modulate);
	}
}

PoolVector2iArray LayeredTileMap::get_surrounding_cells(const Vector2i &p_coords) {
	if (!tile_set.is_valid()) {
		return PoolVector2iArray();
	}

	return tile_set->get_surrounding_cells(p_coords);
}

bool LayeredTileMap::use_tile_data_runtime_update(const int p_layer, const Vector2i &p_coords) {
	return call("_use_tile_data_runtime_update", p_layer, p_coords);
}
void LayeredTileMap::tile_data_runtime_update(const int p_layer, const Vector2i &p_coords, LayeredTileData *p_tile_data) {
	call("_tile_data_runtime_update", p_layer, p_coords, p_tile_data);
}

String LayeredTileMap::get_configuration_warning() const {
	String warnings = Node::get_configuration_warning();

	// Retrieve the set of Z index values with a Y-sorted layer.
	RBSet<int> y_sorted_z_index;
	for (uint32_t i = 0; i < layers.size(); ++i) {
		const LayeredTileMapLayer *layer = layers[i];

		if (layer->is_sort_enabled()) {
			y_sorted_z_index.insert(layer->get_z_index());
		}
	}

	// Check if we have a non-sorted layer in a Z-index with a Y-sorted layer.
	for (uint32_t i = 0; i < layers.size(); ++i) {
		const LayeredTileMapLayer *layer = layers[i];

		if (!layer->is_sort_enabled() && y_sorted_z_index.has(layer->get_z_index())) {
			warnings += RTR("A Y-sorted layer has the same Z-index value as a not Y-sorted layer.\nThis may lead to unwanted behaviors, as a layer that is not Y-sorted will be Y-sorted as a whole with tiles from Y-sorted layers.") + "\n";
			break;
		}
	}

	if (!is_sort_enabled()) {
		// Check if Y-sort is enabled on a layer but not on the node.
		for (uint32_t i = 0; i < layers.size(); ++i) {
			const LayeredTileMapLayer *layer = layers[i];

			if (layer->is_sort_enabled()) {
				warnings += RTR("A LayeredTileMap layer is set as Y-sorted, but Y-sort is not enabled on the LayeredTileMap node itself.") + "\n";
				break;
			}
		}
	} else {
		// Check if Y-sort is enabled on the node, but not on any of the layers.
		bool need_warning = true;
		for (uint32_t i = 0; i < layers.size(); ++i) {
			const LayeredTileMapLayer *layer = layers[i];

			if (layer->is_sort_enabled()) {
				need_warning = false;
				break;
			}
		}
		if (need_warning) {
			warnings += RTR("The LayeredTileMap node is set as Y-sorted, but Y-sort is not enabled on any of the LayeredTileMap's layers.\nThis may lead to unwanted behaviors, as a layer that is not Y-sorted will be Y-sorted as a whole.") + "\n";
		}
	}

	// Check if we are in isometric mode without Y-sort enabled.
	if (tile_set.is_valid() && tile_set->get_tile_shape() == LayeredTileSet::TILE_SHAPE_ISOMETRIC) {
		bool warn = !is_sort_enabled();
		if (!warn) {
			for (uint32_t i = 0; i < layers.size(); ++i) {
				const LayeredTileMapLayer *layer = layers[i];

				if (!layer->is_sort_enabled()) {
					warn = true;
					break;
				}
			}
		}

		if (warn) {
			warnings += RTR("Isometric LayeredTileSet will likely not look as intended without Y-sort enabled for the LayeredTileMap and all of its layers.") + "\n";
		}
	}

	return warnings;
}

void LayeredTileMap::_bind_methods() {
#ifndef DISABLE_DEPRECATED
	ClassDB::bind_method(D_METHOD("set_navigation_map", "layer", "map"), &LayeredTileMap::set_layer_navigation_map);
	ClassDB::bind_method(D_METHOD("get_navigation_map", "layer"), &LayeredTileMap::get_layer_navigation_map);
	ClassDB::bind_method(D_METHOD("force_update", "layer"), &LayeredTileMap::force_update, DEFVAL(-1));
#endif // DISABLE_DEPRECATED

	ClassDB::bind_method(D_METHOD("set_rendering_quadrant_size", "size"), &LayeredTileMap::set_rendering_quadrant_size);
	ClassDB::bind_method(D_METHOD("get_rendering_quadrant_size"), &LayeredTileMap::get_rendering_quadrant_size);

	ClassDB::bind_method(D_METHOD("get_layers_count"), &LayeredTileMap::get_layers_count);
	ClassDB::bind_method(D_METHOD("add_layer", "to_position"), &LayeredTileMap::add_layer);
	ClassDB::bind_method(D_METHOD("move_layer", "layer", "to_position"), &LayeredTileMap::move_layer);
	ClassDB::bind_method(D_METHOD("remove_layer", "layer"), &LayeredTileMap::remove_layer);
	ClassDB::bind_method(D_METHOD("set_layer_name", "layer", "name"), &LayeredTileMap::set_layer_name);
	ClassDB::bind_method(D_METHOD("get_layer_name", "layer"), &LayeredTileMap::get_layer_name);
	ClassDB::bind_method(D_METHOD("set_layer_enabled", "layer", "enabled"), &LayeredTileMap::set_layer_enabled);
	ClassDB::bind_method(D_METHOD("is_layer_enabled", "layer"), &LayeredTileMap::is_layer_enabled);
	ClassDB::bind_method(D_METHOD("set_layer_modulate", "layer", "modulate"), &LayeredTileMap::set_layer_modulate);
	ClassDB::bind_method(D_METHOD("get_layer_modulate", "layer"), &LayeredTileMap::get_layer_modulate);
	ClassDB::bind_method(D_METHOD("set_layer_y_sort_enabled", "layer", "y_sort_enabled"), &LayeredTileMap::set_layer_y_sort_enabled);
	ClassDB::bind_method(D_METHOD("is_layer_y_sort_enabled", "layer"), &LayeredTileMap::is_layer_y_sort_enabled);
	ClassDB::bind_method(D_METHOD("set_layer_y_sort_origin", "layer", "y_sort_origin"), &LayeredTileMap::set_layer_y_sort_origin);
	ClassDB::bind_method(D_METHOD("get_layer_y_sort_origin", "layer"), &LayeredTileMap::get_layer_y_sort_origin);
	ClassDB::bind_method(D_METHOD("set_layer_z_index", "layer", "z_index"), &LayeredTileMap::set_layer_z_index);
	ClassDB::bind_method(D_METHOD("get_layer_z_index", "layer"), &LayeredTileMap::get_layer_z_index);
	ClassDB::bind_method(D_METHOD("set_layer_navigation_enabled", "layer", "enabled"), &LayeredTileMap::set_layer_navigation_enabled);
	ClassDB::bind_method(D_METHOD("is_layer_navigation_enabled", "layer"), &LayeredTileMap::is_layer_navigation_enabled);
	ClassDB::bind_method(D_METHOD("set_layer_navigation_map", "layer", "map"), &LayeredTileMap::set_layer_navigation_map);
	ClassDB::bind_method(D_METHOD("get_layer_navigation_map", "layer"), &LayeredTileMap::get_layer_navigation_map);

	ClassDB::bind_method(D_METHOD("set_collision_animatable", "enabled"), &LayeredTileMap::set_collision_animatable);
	ClassDB::bind_method(D_METHOD("is_collision_animatable"), &LayeredTileMap::is_collision_animatable);
	ClassDB::bind_method(D_METHOD("set_collision_visibility_mode", "collision_visibility_mode"), &LayeredTileMap::set_collision_visibility_mode);
	ClassDB::bind_method(D_METHOD("get_collision_visibility_mode"), &LayeredTileMap::get_collision_visibility_mode);

	ClassDB::bind_method(D_METHOD("set_navigation_visibility_mode", "navigation_visibility_mode"), &LayeredTileMap::set_navigation_visibility_mode);
	ClassDB::bind_method(D_METHOD("get_navigation_visibility_mode"), &LayeredTileMap::get_navigation_visibility_mode);

	ClassDB::bind_method(D_METHOD("set_cell", "layer", "coords", "source_id", "atlas_coords", "alternative_tile"), &LayeredTileMap::set_cell, DEFVAL(LayeredTileSet::INVALID_SOURCE), DEFVAL(LayeredTileSetSource::INVALID_ATLAS_COORDS), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("erase_cell", "layer", "coords"), &LayeredTileMap::erase_cell);
	ClassDB::bind_method(D_METHOD("get_cell_source_id", "layer", "coords", "use_proxies"), &LayeredTileMap::get_cell_source_id, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_atlas_coords", "layer", "coords", "use_proxies"), &LayeredTileMap::get_cell_atlas_coords, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_alternative_tile", "layer", "coords", "use_proxies"), &LayeredTileMap::get_cell_alternative_tile, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_cell_tile_data", "layer", "coords", "use_proxies"), &LayeredTileMap::get_cell_tile_data, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("get_coords_for_body_rid", "body"), &LayeredTileMap::get_coords_for_body_rid);
	ClassDB::bind_method(D_METHOD("get_layer_for_body_rid", "body"), &LayeredTileMap::get_layer_for_body_rid);

	ClassDB::bind_method(D_METHOD("get_pattern", "layer", "coords_array"), &LayeredTileMap::get_pattern);
	ClassDB::bind_method(D_METHOD("map_pattern", "position_in_tilemap", "coords_in_pattern", "pattern"), &LayeredTileMap::map_pattern);
	ClassDB::bind_method(D_METHOD("set_pattern", "layer", "position", "pattern"), &LayeredTileMap::set_pattern);

	ClassDB::bind_method(D_METHOD("set_cells_terrain_connect", "layer", "cells", "terrain_set", "terrain", "ignore_empty_terrains"), &LayeredTileMap::set_cells_terrain_connect, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("set_cells_terrain_path", "layer", "path", "terrain_set", "terrain", "ignore_empty_terrains"), &LayeredTileMap::set_cells_terrain_path, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("fix_invalid_tiles"), &LayeredTileMap::fix_invalid_tiles);
	ClassDB::bind_method(D_METHOD("clear_layer", "layer"), &LayeredTileMap::clear_layer);
	ClassDB::bind_method(D_METHOD("clear"), &LayeredTileMap::clear);

	ClassDB::bind_method(D_METHOD("update_internals"), &LayeredTileMap::update_internals);
	ClassDB::bind_method(D_METHOD("notify_runtime_tile_data_update", "layer"), &LayeredTileMap::notify_runtime_tile_data_update, DEFVAL(-1));

	ClassDB::bind_method(D_METHOD("get_surrounding_cells", "coords"), &LayeredTileMap::get_surrounding_cells);

	ClassDB::bind_method(D_METHOD("get_used_cells", "layer"), &LayeredTileMap::get_used_cells);
	ClassDB::bind_method(D_METHOD("get_used_cells_by_id", "layer", "source_id", "atlas_coords", "alternative_tile"), &LayeredTileMap::get_used_cells_by_id, DEFVAL(LayeredTileSet::INVALID_SOURCE), DEFVAL(LayeredTileSetSource::INVALID_ATLAS_COORDS), DEFVAL(LayeredTileSetSource::INVALID_TILE_ALTERNATIVE));
	ClassDB::bind_method(D_METHOD("get_used_rect"), &LayeredTileMap::get_used_rect);

	ClassDB::bind_method(D_METHOD("map_to_local", "map_position"), &LayeredTileMap::map_to_local);
	ClassDB::bind_method(D_METHOD("local_to_map", "local_position"), &LayeredTileMap::local_to_map);

	ClassDB::bind_method(D_METHOD("get_neighbor_cell", "coords", "neighbor"), &LayeredTileMap::get_neighbor_cell);

	BIND_VMETHOD(MethodInfo(Variant::BOOL, "_use_tile_data_runtime_update",
			PropertyInfo(Variant::INT, "layer"),
			PropertyInfo(Variant::VECTOR2I, "coords")));

	BIND_VMETHOD(MethodInfo("_tile_data_runtime_update",
			PropertyInfo(Variant::INT, "layer"),
			PropertyInfo(Variant::VECTOR2I, "coords"),
			PropertyInfo(Variant::OBJECT, "tile_data", PROPERTY_HINT_RESOURCE_TYPE, "LayeredTileData")));

	//ClassDB::bind_method(D_METHOD("use_tile_data_runtime_update", "layer", "coords"), &LayeredTileMap::use_tile_data_runtime_update);
	//ClassDB::bind_method(D_METHOD("tile_data_runtime_update", "layer", "coords", "tile_data"), &LayeredTileMap::tile_data_runtime_update);

	ClassDB::bind_method(D_METHOD("_emit_changed"), &LayeredTileMap::_emit_changed);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "rendering_quadrant_size", PROPERTY_HINT_RANGE, "1,128,1"), "set_rendering_quadrant_size", "get_rendering_quadrant_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_animatable"), "set_collision_animatable", "is_collision_animatable");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_visibility_mode", PROPERTY_HINT_ENUM, "Default,Force Show,Force Hide"), "set_collision_visibility_mode", "get_collision_visibility_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "navigation_visibility_mode", PROPERTY_HINT_ENUM, "Default,Force Show,Force Hide"), "set_navigation_visibility_mode", "get_navigation_visibility_mode");

	ADD_ARRAY("layers", "layer_");

	ADD_PROPERTY_DEFAULT("format", LayeredTileMapDataFormat::FORMAT_1);

	ADD_SIGNAL(MethodInfo(CoreStringNames::get_singleton()->changed));

	BIND_ENUM_CONSTANT(VISIBILITY_MODE_DEFAULT);
	BIND_ENUM_CONSTANT(VISIBILITY_MODE_FORCE_HIDE);
	BIND_ENUM_CONSTANT(VISIBILITY_MODE_FORCE_SHOW);
}

LayeredTileMap::LayeredTileMap() {
	LayeredTileMapLayer *new_layer = memnew(LayeredTileMapLayer);
	add_child(new_layer);
	move_child(new_layer, 0);
	new_layer->set_as_tile_map_internal_node(0);
	new_layer->set_name("Layer0");
	new_layer->connect(CoreStringNames::get_singleton()->changed, this, "_emit_changed");
	layers.push_back(new_layer);
	default_layer = memnew(LayeredTileMapLayer);
}

LayeredTileMap::~LayeredTileMap() {
	memdelete(default_layer);
}

#undef TILEMAP_CALL_FOR_LAYER
#undef TILEMAP_CALL_FOR_LAYER_V
