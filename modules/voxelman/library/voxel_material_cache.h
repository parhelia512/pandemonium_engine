#ifndef VOXEL_MATERIAL_CACHE_H
#define VOXEL_MATERIAL_CACHE_H


#include "core/math/color.h"
#include "core/object/resource.h"
#include "core/containers/vector.h"

#include "core/math/rect2.h"
#include "scene/resources/material/material.h"

#include "voxel_library.h"

#include "../defines.h"

#include "modules/modules_enabled.gen.h"

class VoxelLibrary;

class VoxelMaterialCache : public Resource {
	GDCLASS(VoxelMaterialCache, Resource)

public:
	bool get_initialized();
	void set_initialized(const bool value);

	int get_ref_count();
	void set_ref_count(const int value);
	void inc_ref_count();
	void dec_ref_count();

	Ref<Material> material_get(const int index);
	Ref<Material> material_lod_get(const int index);
	void material_add(const Ref<Material> &value);
	void material_set(const int index, const Ref<Material> &value);
	void material_remove(const int index);
	int material_get_num() const;
	void materials_clear();

	Vector<Variant> materials_get();
	void materials_set(const Vector<Variant> &materials);

	virtual Ref<VoxelSurface> surface_get(const int index);
	virtual Ref<VoxelSurface> surface_id_get(const int id);
	virtual void surface_add(Ref<VoxelSurface> value);
	virtual void surface_set(const int index, Ref<VoxelSurface> value);
	virtual void surface_remove(const int index);
	virtual int surface_get_num() const;
	virtual void surfaces_clear();

	virtual void additional_texture_add(const Ref<Texture> &texture);
	virtual void additional_texture_remove(const Ref<Texture> &texture);
	virtual void additional_texture_remove_index(const int index);
	virtual void additional_textures_clear();
	virtual int additional_texture_count();
	virtual Ref<Texture> additional_texture_get(const int index);
	virtual Ref<AtlasTexture> additional_texture_get_atlas(const int index);
	virtual Ref<AtlasTexture> additional_texture_get_atlas_tex(const Ref<Texture> &texture);
	virtual Rect2 additional_texture_get_uv_rect(const Ref<Texture> &texture);

#ifdef MODULE_PROPS_ENABLED
	void prop_add_textures(const Ref<PropData> &prop);
	void prop_remove_textures(const Ref<PropData> &prop);
#endif

	virtual void refresh_rects();

	void setup_material_albedo(Ref<Texture> texture);

	VoxelMaterialCache();
	~VoxelMaterialCache();

protected:
	static void _bind_methods();

	bool _initialized;

	Vector<Ref<VoxelSurface>> _surfaces;
	Vector<Ref<Material>> _materials;
	Vector<Ref<Texture>> _additional_textures;

	int _ref_count;
};

#endif
