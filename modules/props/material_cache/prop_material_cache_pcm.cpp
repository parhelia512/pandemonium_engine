/*
Copyright (c) 2019-2023 Péter Magyar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "prop_material_cache_pcm.h"

#include "../../texture_packer/texture_packer.h"
#include "../singleton/prop_cache.h"
#include "scene/resources/texture.h"

int PropMaterialCachePCM::get_texture_flags() const {
	return _packer->get_texture_flags();
}
void PropMaterialCachePCM::set_texture_flags(const int flags) {
	_packer->set_texture_flags(flags);
}

int PropMaterialCachePCM::get_max_atlas_size() const {
	return _packer->get_max_atlas_size();
}
void PropMaterialCachePCM::set_max_atlas_size(const int size) {
	_packer->set_max_atlas_size(size);
}

bool PropMaterialCachePCM::get_keep_original_atlases() const {
	return _packer->get_keep_original_atlases();
}
void PropMaterialCachePCM::set_keep_original_atlases(const bool value) {
	_packer->set_keep_original_atlases(value);
}

Color PropMaterialCachePCM::get_background_color() const {
	return _packer->get_background_color();
}
void PropMaterialCachePCM::set_background_color(const Color &color) {
	_packer->set_background_color(color);
}

int PropMaterialCachePCM::get_margin() const {
	return _packer->get_margin();
}
void PropMaterialCachePCM::set_margin(const int margin) {
	_packer->set_margin(margin);
}

Ref<AtlasTexture> PropMaterialCachePCM::texture_get_atlas_tex(const Ref<Texture> &texture) {
	if (!_packer->contains_texture(texture)) {
		return Ref<AtlasTexture>();
	}

	return _packer->get_texture(texture);
}
Rect2 PropMaterialCachePCM::texture_get_uv_rect(const Ref<Texture> &texture) {
	if (!texture.is_valid()) {
		return Rect2(0, 0, 1, 1);
	}

	Ref<AtlasTexture> at = _packer->get_texture(texture);

	if (!at.is_valid()) {
		return Rect2(0, 0, 1, 1);
	}

	Rect2 region = at->get_region();

	Ref<Texture> tex = at->get_atlas();

	if (!tex.is_valid()) {
		return Rect2(0, 0, 1, 1);
	}

	Ref<Image> image = tex->get_data();

	if (!image.is_valid()) {
		return Rect2(0, 0, 1, 1);
	}

	float w = tex->get_width();
	float h = tex->get_height();

	region.position = Size2(region.position.x / w, region.position.y / h);
	region.size = Size2(region.size.x / w, region.size.y / h);

	return region;
}

void PropMaterialCachePCM::refresh_rects() {
	bool texture_added = false;

	for (int i = 0; i < _textures.size(); i++) {
		Ref<Texture> tex = _textures.get(i);

		ERR_CONTINUE(!tex.is_valid());

		if (!_packer->contains_texture(tex)) {
			_packer->add_texture(tex);
			texture_added = true;
		}
	}

	if (texture_added) {
		_packer->merge();

		ERR_FAIL_COND(_packer->get_texture_count() == 0);

		Ref<Texture> tex = _packer->get_generated_texture(0);

		setup_material_albedo(tex);
	}

	_initialized = true;
}

void PropMaterialCachePCM::initial_setup_default() {
	PropMaterialCache::initial_setup_default();

	PropCache *pc = PropCache::get_singleton();

	set_texture_flags(pc->get_texture_flags());
	set_max_atlas_size(pc->get_max_atlas_size());
	set_keep_original_atlases(pc->get_keep_original_atlases());
	set_background_color(pc->get_background_color());
	set_margin(pc->get_margin());
}

void PropMaterialCachePCM::_setup_material_albedo(Ref<Texture> texture) {
	int count = material_get_num();

	for (int i = 0; i < count; ++i) {
		Ref<Material> m = material_get(i);

		Ref<SpatialMaterial> spmat = m;

		if (spmat.is_valid()) {
			spmat->set_texture(SpatialMaterial::TEXTURE_ALBEDO, texture);
			continue;
		}

		Ref<ShaderMaterial> shmat = m;

		if (shmat.is_valid()) {
			shmat->set_shader_param("texture_albedo", texture);
		}
	}
}

PropMaterialCachePCM::PropMaterialCachePCM() {
	_packer.instance();

	_packer->set_texture_flags(Texture::FLAG_MIPMAPS | Texture::FLAG_FILTER);

	_packer->set_max_atlas_size(1024);
	_packer->set_keep_original_atlases(false);
	_packer->set_margin(0);
}

PropMaterialCachePCM::~PropMaterialCachePCM() {
	_packer->clear();
	_packer.unref();
}

void PropMaterialCachePCM::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_texture_flags"), &PropMaterialCachePCM::get_texture_flags);
	ClassDB::bind_method(D_METHOD("set_texture_flags", "flags"), &PropMaterialCachePCM::set_texture_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_flags", PROPERTY_HINT_FLAGS, "Mipmaps,Repeat,Filter,Anisotropic Linear,Convert to Linear,Mirrored Repeat,Video Surface"), "set_texture_flags", "get_texture_flags");

	ClassDB::bind_method(D_METHOD("get_max_atlas_size"), &PropMaterialCachePCM::get_max_atlas_size);
	ClassDB::bind_method(D_METHOD("set_max_atlas_size", "size"), &PropMaterialCachePCM::set_max_atlas_size);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_atlas_size"), "set_max_atlas_size", "get_max_atlas_size");

	ClassDB::bind_method(D_METHOD("get_keep_original_atlases"), &PropMaterialCachePCM::get_keep_original_atlases);
	ClassDB::bind_method(D_METHOD("set_keep_original_atlases", "value"), &PropMaterialCachePCM::set_keep_original_atlases);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "keep_original_atlases"), "set_keep_original_atlases", "get_keep_original_atlases");

	ClassDB::bind_method(D_METHOD("get_background_color"), &PropMaterialCachePCM::get_background_color);
	ClassDB::bind_method(D_METHOD("set_background_color", "color"), &PropMaterialCachePCM::set_background_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");

	ClassDB::bind_method(D_METHOD("get_margin"), &PropMaterialCachePCM::get_margin);
	ClassDB::bind_method(D_METHOD("set_margin", "size"), &PropMaterialCachePCM::set_margin);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "margin"), "set_margin", "get_margin");

	ClassDB::bind_method(D_METHOD("_setup_material_albedo", "texture"), &PropMaterialCachePCM::_setup_material_albedo);
}
