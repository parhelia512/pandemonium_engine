#ifndef MESH_INSTANCE_H
#define MESH_INSTANCE_H

/*************************************************************************/
/*  mesh_instance.h                                                      */
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

#include "scene/3d/visual_instance.h"

#include "core/containers/local_vector.h"
#include "core/object/reference.h"

#include "modules/modules_enabled.gen.h"

#ifdef MODULE_SKELETON_3D_ENABLED
class Skin;
class SkinReference;
#endif

class Mesh;
class NodePath;

class MeshInstance : public GeometryInstance {
	GDCLASS(MeshInstance, GeometryInstance);

protected:
	Ref<Mesh> mesh;

#ifdef MODULE_SKELETON_3D_ENABLED
	Ref<Skin> skin;
	Ref<Skin> skin_internal;
	Ref<SkinReference> skin_ref;
	NodePath skeleton_path;

	struct SoftwareSkinning {
		enum Flags {
			// Data flags.
			FLAG_TRANSFORM_NORMALS = 1 << 0,

			// Runtime flags.
			FLAG_BONES_READY = 1 << 1,
		};

		struct SurfaceData {
			PoolByteArray source_buffer;
			uint32_t source_format;
			PoolByteArray buffer;
			PoolByteArray::Write buffer_write;
			bool transform_tangents;
			bool ensure_correct_normals;
		};

		Ref<Mesh> mesh_instance;
		LocalVector<SurfaceData> surface_data;
	};

	SoftwareSkinning *software_skinning;
	uint32_t software_skinning_flags;
#endif

	struct BlendShapeTrack {
		int idx;
		float value;
		BlendShapeTrack() {
			idx = 0;
			value = 0;
		}
	};

	RBMap<StringName, BlendShapeTrack> blend_shape_tracks;
	Vector<Ref<Material>> materials;

	void _mesh_changed();

#ifdef MODULE_SKELETON_3D_ENABLED
	void _resolve_skeleton_path();

	bool _is_software_skinning_enabled() const;
	static bool _is_global_software_skinning_enabled();

	void _initialize_skinning(bool p_force_reset = false, bool p_call_attach_skeleton = true);
	void _update_skinning();
#endif

private:
	// merging
	bool _merge_meshes(Vector<MeshInstance *> p_list, bool p_use_global_space, bool p_check_compatibility);
	bool _is_mergeable_with(const MeshInstance &p_other) const;
	void _merge_into_mesh_data(const MeshInstance &p_mi, const Transform &p_dest_tr_inv, int p_surface_id, LocalVector<Vector3> &r_verts, LocalVector<Vector3> &r_norms, LocalVector<real_t> &r_tangents, LocalVector<Color> &r_colors, LocalVector<Vector2> &r_uvs, LocalVector<Vector2> &r_uv2s, LocalVector<int> &r_inds);
	bool _ensure_indices_valid(LocalVector<int> &r_indices, const PoolVector<Vector3> &p_verts) const;
	bool _check_for_valid_indices(const LocalVector<int> &p_inds, const PoolVector<Vector3> &p_verts, LocalVector<int> *r_inds) const;
	bool _triangle_is_degenerate(const Vector3 &p_a, const Vector3 &p_b, const Vector3 &p_c, real_t p_epsilon) const;
	void _merge_log(String p_string) const;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

#ifdef MODULE_SKELETON_3D_ENABLED
	void set_skin(const Ref<Skin> &p_skin);
	Ref<Skin> get_skin() const;

	void set_skeleton_path(const NodePath &p_skeleton);
	NodePath get_skeleton_path();
#endif

	int get_surface_material_count() const;
	void set_surface_material(int p_surface, const Ref<Material> &p_material);
	Ref<Material> get_surface_material(int p_surface) const;
	Ref<Material> get_active_material(int p_surface) const;

	virtual void set_material_override(const Ref<Material> &p_material);

	virtual void set_material_overlay(const Ref<Material> &p_material);

	void set_software_skinning_transform_normals(bool p_enabled);
	bool is_software_skinning_transform_normals_enabled() const;

	Node *create_trimesh_collision_node();
	void create_trimesh_collision();

	Node *create_multiple_convex_collisions_node();
	void create_multiple_convex_collisions();

	Node *create_convex_collision_node(bool p_clean = true, bool p_simplify = false);
	void create_convex_collision(bool p_clean = true, bool p_simplify = false);

	void create_debug_tangents();

	// merging
	bool is_mergeable_with(Node *p_other) const;
	bool merge_meshes(Vector<Variant> p_list, bool p_use_global_space, bool p_check_compatibility);

	virtual AABB get_aabb() const;
	virtual PoolVector<Face3> get_faces(uint32_t p_usage_flags) const;

	MeshInstance();
	~MeshInstance();
};

#endif
