#ifndef TERRAIN_WORLD_EDITOR_PLUGIN_H
#define TERRAIN_WORLD_EDITOR_PLUGIN_H


#include "editor/editor_node.h"
#include "editor/editor_plugin.h"

#include "scene/gui/panel_container.h"

class TerrainWorld;
class SpatialEditorPlugin;
class HSlider;

class TerrainWorldEditor : public PanelContainer {
	GDCLASS(TerrainWorldEditor, PanelContainer);

public:
	enum TerrainWorldEditorToolMode {
		TOOL_MODE_ADD = 0,
		TOOL_MODE_REMOVE,
	};

public:
	EditorPlugin::AfterGUIInput forward_spatial_input_event(Camera *p_camera, const Ref<InputEvent> &p_event);

	void edit(TerrainWorld *p_world);
	EditorPlugin::AfterGUIInput do_input_action(Camera *p_camera, const Point2 &p_point, bool p_click);

	TerrainWorldEditor();
	TerrainWorldEditor(EditorNode *p_editor);
	~TerrainWorldEditor();

	HBoxContainer *spatial_editor_hb;

protected:
	static void _bind_methods();
	void _node_removed(Node *p_node);
	void _on_surface_button_pressed();
	void _on_tool_button_pressed();
	void _on_insert_block_at_camera_button_pressed();
	void _on_isolevel_slider_value_changed(float value);

private:
	VBoxContainer *_surfaces_vbox_container;
	Ref<ButtonGroup> _surfaces_button_group;

	Ref<ButtonGroup> _tool_button_group;

	TerrainWorldEditorToolMode _tool_mode;
	TerrainWorld *_world;

	HSlider *_isolevel_slider;

	int _selected_type;
	int _current_isolevel;

	SpatialEditorPlugin *spatial_editor;
	EditorNode *_editor;

	int _channel_type;
	int _channel_isolevel;
};

class TerrainWorldEditorPlugin : public EditorPlugin {
	GDCLASS(TerrainWorldEditorPlugin, EditorPlugin);

	TerrainWorldEditor *voxel_world_editor;
	EditorNode *editor;

protected:
	void _notification(int p_what);

public:
	bool forward_spatial_input_event(Camera *p_camera, const Ref<InputEvent> &p_event) { return voxel_world_editor->forward_spatial_input_event(p_camera, p_event); }
	virtual EditorPlugin::AfterGUIInput forward_spatial_gui_input(Camera *p_camera, const Ref<InputEvent> &p_event) { return voxel_world_editor->forward_spatial_input_event(p_camera, p_event); }
	virtual String get_name() const { return "TerrainWorldEditor"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;
	virtual void make_visible(bool p_visible);

	TerrainWorldEditorPlugin(EditorNode *p_node);
	~TerrainWorldEditorPlugin();
};

#endif
