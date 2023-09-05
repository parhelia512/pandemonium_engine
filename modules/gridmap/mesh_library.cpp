/*************************************************************************/
/*  mesh_library.cpp                                                     */
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

#include "mesh_library.h"

#include "scene/resources/box_shape.h"
#include "scene/resources/navigation_mesh.h"

bool MeshLibrary::_set(const StringName &p_name, const Variant &p_value) {
	String name = p_name;
	if (name.begins_with("item/")) {
		int idx = name.get_slicec('/', 1).to_int();
		String what = name.get_slicec('/', 2);
		if (!item_map.has(idx)) {
			create_item(idx);
		}

		if (what == "name") {
			set_item_name(idx, p_value);
		} else if (what == "mesh") {
			set_item_mesh(idx, p_value);
		} else if (what == "mesh_transform") {
			set_item_mesh_transform(idx, p_value);
		} else if (what == "shape") {
			Vector<ShapeData> shapes;
			ShapeData sd;
			sd.shape = p_value;
			shapes.push_back(sd);
			set_item_shapes(idx, shapes);
		} else if (what == "shapes") {
			_set_item_shapes(idx, p_value);
		} else if (what == "preview") {
			set_item_preview(idx, p_value);
		} else if (what == "navigation_mesh") {
			set_item_navigation_mesh(idx, p_value);
		} else if (what == "navigation_mesh_transform") {
			set_item_navigation_mesh_transform(idx, p_value);
#ifndef DISABLE_DEPRECATED
		} else if (what == "navmesh") { // Renamed after 4.0
			set_item_navigation_mesh(idx, p_value);
		} else if (what == "navmesh_transform") { // Renamed after 4.0
			set_item_navigation_mesh_transform(idx, p_value);
#endif // DISABLE_DEPRECATED
		} else {
			return false;
		}

		return true;
	}

	return false;
}

bool MeshLibrary::_get(const StringName &p_name, Variant &r_ret) const {
	String name = p_name;
	int idx = name.get_slicec('/', 1).to_int();
	ERR_FAIL_COND_V(!item_map.has(idx), false);
	String what = name.get_slicec('/', 2);

	if (what == "name") {
		r_ret = get_item_name(idx);
	} else if (what == "mesh") {
		r_ret = get_item_mesh(idx);
	} else if (what == "mesh_transform") {
		r_ret = get_item_mesh_transform(idx);
	} else if (what == "shapes") {
		r_ret = _get_item_shapes(idx);
	} else if (what == "navigation_mesh") {
		r_ret = get_item_navigation_mesh(idx);
	} else if (what == "navigation_mesh_transform") {
		r_ret = get_item_navigation_mesh_transform(idx);
#ifndef DISABLE_DEPRECATED
	} else if (what == "navmesh") { // Renamed after 4.0
		r_ret = get_item_navigation_mesh(idx);
	} else if (what == "navmesh_transform") { // Renamed after 4.0
		r_ret = get_item_navigation_mesh_transform(idx);
#endif // DISABLE_DEPRECATED
	} else if (what == "preview") {
		r_ret = get_item_preview(idx);
	} else {
		return false;
	}

	return true;
}

void MeshLibrary::_get_property_list(List<PropertyInfo> *p_list) const {
	for (RBMap<int, Item>::Element *E = item_map.front(); E; E = E->next()) {
		String name = vformat("%s/%d/", "item", E->key());
		p_list->push_back(PropertyInfo(Variant::STRING, name + ("name")));
		p_list->push_back(PropertyInfo(Variant::OBJECT, name + ("mesh"), PROPERTY_HINT_RESOURCE_TYPE, "Mesh"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, name + ("mesh_transform")));
		p_list->push_back(PropertyInfo(Variant::ARRAY, name + ("shapes")));
		p_list->push_back(PropertyInfo(Variant::OBJECT, name + ("navigation_mesh"), PROPERTY_HINT_RESOURCE_TYPE, "NavigationMesh"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, name + ("navigation_mesh_transform")));
		p_list->push_back(PropertyInfo(Variant::OBJECT, name + ("preview"), PROPERTY_HINT_RESOURCE_TYPE, "Texture", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_HELPER));
	}
}

void MeshLibrary::create_item(int p_item) {
	ERR_FAIL_COND(p_item < 0);
	ERR_FAIL_COND(item_map.has(p_item));
	item_map[p_item] = Item();
	_change_notify();
}

void MeshLibrary::set_item_name(int p_item, const String &p_name) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map[p_item].name = p_name;
	emit_changed();
	_change_notify();
}

void MeshLibrary::set_item_mesh(int p_item, const Ref<Mesh> &p_mesh) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map[p_item].mesh = p_mesh;
	notify_change_to_owners();
	emit_changed();
	_change_notify();
}

void MeshLibrary::set_item_mesh_transform(int p_item, const Transform &p_transform) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map[p_item].mesh_transform = p_transform;
	notify_change_to_owners();
	emit_changed();
	_change_notify();
}

void MeshLibrary::set_item_shapes(int p_item, const Vector<ShapeData> &p_shapes) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map[p_item].shapes = p_shapes;
	_change_notify();
	notify_change_to_owners();
	emit_changed();
	_change_notify();
}

void MeshLibrary::set_item_navigation_mesh(int p_item, const Ref<NavigationMesh> &p_navigation_mesh) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map[p_item].navigation_mesh = p_navigation_mesh;
	_change_notify();
	notify_change_to_owners();
	emit_changed();
	_change_notify();
}

void MeshLibrary::set_item_navigation_mesh_transform(int p_item, const Transform &p_transform) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map[p_item].navigation_mesh_transform = p_transform;
	notify_change_to_owners();
	emit_changed();
	_change_notify();
}

void MeshLibrary::set_item_preview(int p_item, const Ref<Texture> &p_preview) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map[p_item].preview = p_preview;
	emit_changed();
	_change_notify();
}

String MeshLibrary::get_item_name(int p_item) const {
	ERR_FAIL_COND_V_MSG(!item_map.has(p_item), "", "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	return item_map[p_item].name;
}

Ref<Mesh> MeshLibrary::get_item_mesh(int p_item) const {
	ERR_FAIL_COND_V_MSG(!item_map.has(p_item), Ref<Mesh>(), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	return item_map[p_item].mesh;
}

Transform MeshLibrary::get_item_mesh_transform(int p_item) const {
	ERR_FAIL_COND_V_MSG(!item_map.has(p_item), Transform(), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	return item_map[p_item].mesh_transform;
}

Vector<MeshLibrary::ShapeData> MeshLibrary::get_item_shapes(int p_item) const {
	ERR_FAIL_COND_V_MSG(!item_map.has(p_item), Vector<ShapeData>(), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	return item_map[p_item].shapes;
}

Ref<NavigationMesh> MeshLibrary::get_item_navigation_mesh(int p_item) const {
	ERR_FAIL_COND_V_MSG(!item_map.has(p_item), Ref<NavigationMesh>(), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	return item_map[p_item].navigation_mesh;
}

Transform MeshLibrary::get_item_navigation_mesh_transform(int p_item) const {
	ERR_FAIL_COND_V_MSG(!item_map.has(p_item), Transform(), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	return item_map[p_item].navigation_mesh_transform;
}

Ref<Texture> MeshLibrary::get_item_preview(int p_item) const {
	ERR_FAIL_COND_V_MSG(!item_map.has(p_item), Ref<Texture>(), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	return item_map[p_item].preview;
}

bool MeshLibrary::has_item(int p_item) const {
	return item_map.has(p_item);
}
void MeshLibrary::remove_item(int p_item) {
	ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
	item_map.erase(p_item);
	notify_change_to_owners();
	_change_notify();
	emit_changed();
}

void MeshLibrary::clear() {
	item_map.clear();
	notify_change_to_owners();
	_change_notify();
	emit_changed();
}

Vector<int> MeshLibrary::get_item_list() const {
	Vector<int> ret;
	ret.resize(item_map.size());
	int idx = 0;
	for (RBMap<int, Item>::Element *E = item_map.front(); E; E = E->next()) {
		ret.write[idx++] = E->key();
	}

	return ret;
}

int MeshLibrary::find_item_by_name(const String &p_name) const {
	for (RBMap<int, Item>::Element *E = item_map.front(); E; E = E->next()) {
		if (E->get().name == p_name) {
			return E->key();
		}
	}
	return -1;
}

int MeshLibrary::get_last_unused_item_id() const {
	if (!item_map.size()) {
		return 0;
	} else {
		return item_map.back()->key() + 1;
	}
}

void MeshLibrary::_set_item_shapes(int p_item, const Array &p_shapes) {
	Array arr_shapes = p_shapes;
	int size = p_shapes.size();
	if (size & 1) {
		ERR_FAIL_COND_MSG(!item_map.has(p_item), "Requested for nonexistent MeshLibrary item '" + itos(p_item) + "'.");
		int prev_size = item_map[p_item].shapes.size() * 2;

		if (prev_size < size) {
			// Check if last element is a shape.
			Ref<Shape> shape = arr_shapes[size - 1];
			if (shape.is_null()) {
				Ref<BoxShape> box_shape;
				box_shape.instance();
				arr_shapes[size - 1] = box_shape;
			}

			// Make sure the added element is a Transform.
			arr_shapes.push_back(Transform());
			size++;
		} else {
			size--;
			arr_shapes.resize(size);
		}
	}

	Vector<ShapeData> shapes;
	for (int i = 0; i < size; i += 2) {
		ShapeData sd;
		sd.shape = arr_shapes[i + 0];
		sd.local_transform = arr_shapes[i + 1];

		if (sd.shape.is_valid()) {
			shapes.push_back(sd);
		}
	}

	set_item_shapes(p_item, shapes);
}

Array MeshLibrary::_get_item_shapes(int p_item) const {
	Vector<ShapeData> shapes = get_item_shapes(p_item);
	Array ret;
	for (int i = 0; i < shapes.size(); i++) {
		ret.push_back(shapes[i].shape);
		ret.push_back(shapes[i].local_transform);
	}

	return ret;
}

void MeshLibrary::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_item", "id"), &MeshLibrary::create_item);
	ClassDB::bind_method(D_METHOD("set_item_name", "id", "name"), &MeshLibrary::set_item_name);
	ClassDB::bind_method(D_METHOD("set_item_mesh", "id", "mesh"), &MeshLibrary::set_item_mesh);
	ClassDB::bind_method(D_METHOD("set_item_mesh_transform", "id", "mesh_transform"), &MeshLibrary::set_item_mesh_transform);
	ClassDB::bind_method(D_METHOD("set_item_navigation_mesh", "id", "navigation_mesh"), &MeshLibrary::set_item_navigation_mesh);
	ClassDB::bind_method(D_METHOD("set_item_navigation_mesh_transform", "id", "navigation_mesh"), &MeshLibrary::set_item_navigation_mesh_transform);
	ClassDB::bind_method(D_METHOD("set_item_shapes", "id", "shapes"), &MeshLibrary::_set_item_shapes);
	ClassDB::bind_method(D_METHOD("set_item_preview", "id", "texture"), &MeshLibrary::set_item_preview);
	ClassDB::bind_method(D_METHOD("get_item_name", "id"), &MeshLibrary::get_item_name);
	ClassDB::bind_method(D_METHOD("get_item_mesh", "id"), &MeshLibrary::get_item_mesh);
	ClassDB::bind_method(D_METHOD("get_item_mesh_transform", "id"), &MeshLibrary::get_item_mesh_transform);
	ClassDB::bind_method(D_METHOD("get_item_navigation_mesh", "id"), &MeshLibrary::get_item_navigation_mesh);
	ClassDB::bind_method(D_METHOD("get_item_navigation_mesh_transform", "id"), &MeshLibrary::get_item_navigation_mesh_transform);
	ClassDB::bind_method(D_METHOD("get_item_shapes", "id"), &MeshLibrary::_get_item_shapes);
	ClassDB::bind_method(D_METHOD("get_item_preview", "id"), &MeshLibrary::get_item_preview);
	ClassDB::bind_method(D_METHOD("remove_item", "id"), &MeshLibrary::remove_item);
	ClassDB::bind_method(D_METHOD("find_item_by_name", "name"), &MeshLibrary::find_item_by_name);

	ClassDB::bind_method(D_METHOD("clear"), &MeshLibrary::clear);
	ClassDB::bind_method(D_METHOD("get_item_list"), &MeshLibrary::get_item_list);
	ClassDB::bind_method(D_METHOD("get_last_unused_item_id"), &MeshLibrary::get_last_unused_item_id);
}

MeshLibrary::MeshLibrary() {
}
MeshLibrary::~MeshLibrary() {
}
