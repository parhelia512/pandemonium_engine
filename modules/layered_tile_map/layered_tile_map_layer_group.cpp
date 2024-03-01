/*************************************************************************/
/*  layered_tile_map_layer_group.cpp                                     */
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

#include "tile_map_layer_group.h"

#include "core/core_string_names.h"
#include "scene/2d/tile_map_layer.h"
#include "scene/resources/2d/tile_set.h"

#ifdef TOOLS_ENABLED

void LayeredTileMapLayerGroup::_cleanup_selected_layers() {
	for (int i = 0; i < selected_layers.size(); i++) {
		const String name = selected_layers[i];
		LayeredTileMapLayer *layer = Object::cast_to<LayeredTileMapLayer>(get_node_or_null(name));
		if (!layer) {
			selected_layers.remove_at(i);
			i--;
		}
	}
}

#endif // TOOLS_ENABLED

void LayeredTileMapLayerGroup::_tile_set_changed() {
	for (int i = 0; i < get_child_count(); i++) {
		LayeredTileMapLayer *layer = Object::cast_to<LayeredTileMapLayer>(get_child(i));
		if (layer) {
			layer->notify_tile_map_layer_group_change(LayeredTileMapLayer::DIRTY_FLAGS_LAYER_GROUP_TILE_SET);
		}
	}

	update_configuration_warnings();
}

#ifdef TOOLS_ENABLED

void LayeredTileMapLayerGroup::set_selected_layers(Vector<StringName> p_layer_names) {
	selected_layers = p_layer_names;
	_cleanup_selected_layers();

	// Update the layers modulation.
	for (int i = 0; i < get_child_count(); i++) {
		LayeredTileMapLayer *layer = Object::cast_to<LayeredTileMapLayer>(get_child(i));
		if (layer) {
			layer->notify_tile_map_layer_group_change(LayeredTileMapLayer::DIRTY_FLAGS_LAYER_GROUP_SELECTED_LAYERS);
		}
	}
}

Vector<StringName> LayeredTileMapLayerGroup::get_selected_layers() const {
	return selected_layers;
}

void LayeredTileMapLayerGroup::set_highlight_selected_layer(bool p_highlight_selected_layer) {
	if (highlight_selected_layer == p_highlight_selected_layer) {
		return;
	}

	highlight_selected_layer = p_highlight_selected_layer;

	for (int i = 0; i < get_child_count(); i++) {
		LayeredTileMapLayer *layer = Object::cast_to<LayeredTileMapLayer>(get_child(i));
		if (layer) {
			layer->notify_tile_map_layer_group_change(LayeredTileMapLayer::DIRTY_FLAGS_LAYER_GROUP_HIGHLIGHT_SELECTED);
		}
	}
}

bool LayeredTileMapLayerGroup::is_highlighting_selected_layer() const {
	return highlight_selected_layer;
}

#endif // TOOLS_ENABLED

void LayeredTileMapLayerGroup::remove_child_notify(Node *p_child) {
#ifdef TOOLS_ENABLED
	_cleanup_selected_layers();
#endif // TOOLS_ENABLED
}

void LayeredTileMapLayerGroup::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tileset", "tileset"), &LayeredTileMapLayerGroup::set_tileset);
	ClassDB::bind_method(D_METHOD("get_tileset"), &LayeredTileMapLayerGroup::get_tileset);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tile_set", PROPERTY_HINT_RESOURCE_TYPE, "LayeredTileSet"), "set_tileset", "get_tileset");
}

void LayeredTileMapLayerGroup::set_tileset(const Ref<LayeredTileSet> &p_tileset) {
	if (p_tileset == tile_set) {
		return;
	}

	// Set the tileset, registering to its changes.
	if (tile_set.is_valid()) {
		tile_set->disconnect_changed(callable_mp(this, &LayeredTileMapLayerGroup::_tile_set_changed));
	}

	tile_set = p_tileset;

	if (tile_set.is_valid()) {
		tile_set->connect_changed(callable_mp(this, &LayeredTileMapLayerGroup::_tile_set_changed));
	}

	for (int i = 0; i < get_child_count(); i++) {
		LayeredTileMapLayer *layer = Object::cast_to<LayeredTileMapLayer>(get_child(i));
		if (layer) {
			layer->notify_tile_map_layer_group_change(LayeredTileMapLayer::DIRTY_FLAGS_LAYER_GROUP_TILE_SET);
		}
	}
}

Ref<LayeredTileSet> LayeredTileMapLayerGroup::get_tileset() const {
	return tile_set;
}

LayeredTileMapLayerGroup::~LayeredTileMapLayerGroup() {
	if (tile_set.is_valid()) {
		tile_set->disconnect_changed(callable_mp(this, &LayeredTileMapLayerGroup::_tile_set_changed));
	}
}
