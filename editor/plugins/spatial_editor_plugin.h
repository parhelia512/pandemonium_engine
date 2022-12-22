#ifndef SPATIAL_EDITOR_PLUGIN_H
#define SPATIAL_EDITOR_PLUGIN_H
/*************************************************************************/
/*  spatial_editor_plugin.h                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           PANDEMONIUM ENGINE                                */
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

#include "editor/editor_plugin.h"
#include "scene/3d/spatial.h"
#include "scene/gui/box_container.h"
#include "scene/gui/container.h"
#include "scene/gui/control.h"

#include "scene/gui/spin_box.h"

#include "core/containers/hash_map.h"
#include "core/containers/list.h"
#include "core/containers/rid.h"
#include "core/containers/vector.h"
#include "core/error/error_macros.h"
#include "core/math/aabb.h"
#include "core/math/color.h"
#include "core/math/math_defs.h"
#include "core/math/transform.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/object/object.h"
#include "core/object/object_id.h"
#include "core/object/reference.h"
#include "core/object/resource.h"
#include "core/string/ustring.h"
#include "core/typedefs.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "editor/editor_scale.h"

#include "editor/spatial_editor_gizmos.h"

#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

class Camera;
class SpatialEditor;
class EditorSpatialGizmoPlugin;
class EditorSpatialGizmo;
class ViewportContainer;
class SpatialEditorViewport;
class Environment3D;
class AcceptDialog;
class CheckBox;
class ConfirmationDialog;
class EditorData;
class EditorNode;
class EditorSelection;
class Gradient;
class HSplitContainer;
class InputEvent;
class InputEventMouseButton;
class InputEventMouseMotion;
class InputEventWithModifiers;
class Label;
class LineEdit;
class MenuButton;
class OptionButton;
class PanelContainer;
struct Plane;
class PopupMenu;
class SkinReference;
class TriangleMesh;
class UndoRedo;
class VSplitContainer;
class Viewport;
class SpinBox;
class ViewportNavigationControl;

class ViewportRotationControl : public Control {
	GDCLASS(ViewportRotationControl, Control);

	struct Axis2D {
		Vector2 screen_point;
		float z_axis = -99.0;
		int axis = -1;
	};

	struct Axis2DCompare {
		_FORCE_INLINE_ bool operator()(const Axis2D &l, const Axis2D &r) const {
			return l.z_axis < r.z_axis;
		}
	};

	SpatialEditorViewport *viewport = nullptr;
	Vector<Color> axis_colors;
	Vector<int> axis_menu_options;
	Vector2i orbiting_mouse_start;
	int orbiting_index = -1;
	int focused_axis = -2;

	const float AXIS_CIRCLE_RADIUS = 8.0f * EDSCALE;

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void _gui_input(Ref<InputEvent> p_event);
	void _draw();
	void _draw_axis(const Axis2D &p_axis);
	void _get_sorted_axis(Vector<Axis2D> &r_axis);
	void _update_focus();
	void _on_mouse_exited();
	void _process_click(int p_index, Vector2 p_position, bool p_pressed);
	void _process_drag(Ref<InputEventWithModifiers> p_event, int p_index, Vector2 p_position, Vector2 p_relative_position);

public:
	void set_viewport(SpatialEditorViewport *p_viewport);
};

class SpatialEditorViewport : public Control {
	GDCLASS(SpatialEditorViewport, Control);
	friend class SpatialEditor;
	friend class ViewportNavigationControl;
	friend class ViewportRotationControl;

	enum {
		VIEW_TOP,
		VIEW_BOTTOM,
		VIEW_LEFT,
		VIEW_RIGHT,
		VIEW_FRONT,
		VIEW_REAR,
		VIEW_CENTER_TO_ORIGIN,
		VIEW_CENTER_TO_SELECTION,
		VIEW_ALIGN_TRANSFORM_WITH_VIEW,
		VIEW_ALIGN_ROTATION_WITH_VIEW,
		VIEW_PERSPECTIVE,
		VIEW_ENVIRONMENT,
		VIEW_ORTHOGONAL,
		VIEW_HALF_RESOLUTION,
		VIEW_AUDIO_LISTENER,
		VIEW_AUDIO_DOPPLER,
		VIEW_GIZMOS,
		VIEW_INFORMATION,
		VIEW_FPS,
		VIEW_DISPLAY_NORMAL,
		VIEW_DISPLAY_WIREFRAME,
		VIEW_DISPLAY_OVERDRAW,
		VIEW_DISPLAY_SHADELESS,
		VIEW_LOCK_ROTATION,
		VIEW_CINEMATIC_PREVIEW,
		VIEW_AUTO_ORTHOGONAL,
		VIEW_PORTAL_CULLING,
	};

	enum ViewType {
		VIEW_TYPE_USER,
		VIEW_TYPE_TOP,
		VIEW_TYPE_BOTTOM,
		VIEW_TYPE_LEFT,
		VIEW_TYPE_RIGHT,
		VIEW_TYPE_FRONT,
		VIEW_TYPE_REAR,
	};

public:
	enum {
		GIZMO_BASE_LAYER = 27,
		GIZMO_EDIT_LAYER = 26,
		GIZMO_GRID_LAYER = 25,
		MISC_TOOL_LAYER = 24
	};

	enum NavigationScheme {
		NAVIGATION_PANDEMONIUM,
		NAVIGATION_MAYA,
		NAVIGATION_MODO,
	};

	enum FreelookNavigationScheme {
		FREELOOK_DEFAULT,
		FREELOOK_PARTIALLY_AXIS_LOCKED,
		FREELOOK_FULLY_AXIS_LOCKED,
	};

private:
	int index;
	bool _project_settings_change_pending;
	ViewType view_type;
	void _menu_option(int p_option);
	void _set_auto_orthogonal();
	Spatial *preview_node;
	AABB *preview_bounds;
	Vector<String> selected_files;
	AcceptDialog *accept;

	Node *target_node;
	Point2 drop_pos;

	EditorNode *editor;
	EditorData *editor_data;
	EditorSelection *editor_selection;
	UndoRedo *undo_redo;

	CheckBox *preview_camera;
	ViewportContainer *viewport_container;

	MenuButton *view_menu;

	Control *surface;
	Viewport *viewport;
	Camera *camera;
	bool transforming;
	bool orthogonal;
	bool auto_orthogonal;
	bool lock_rotation;
	float gizmo_scale;

	bool freelook_active;
	real_t freelook_speed;
	Vector2 previous_mouse_position;

	Label *info_label;
	Label *cinema_label;
	Label *locked_label;
	Label *zoom_limit_label;

	VBoxContainer *top_right_vbox;
	VBoxContainer *bottom_center_vbox;
	ViewportNavigationControl *position_control;
	ViewportNavigationControl *look_control;
	ViewportRotationControl *rotation_control;
	Gradient *frame_time_gradient;
	Label *fps_label;

	struct _RayResult {
		Spatial *item;
		float depth;
		_FORCE_INLINE_ bool operator<(const _RayResult &p_rr) const { return depth < p_rr.depth; }
	};

	void _update_name();
	void _compute_edit(const Point2 &p_point);
	void _clear_selected();
	void _select_clicked(bool p_allow_locked);
	ObjectID _select_ray(const Point2 &p_pos);
	void _find_items_at_pos(const Point2 &p_pos, Vector<_RayResult> &r_results, bool p_include_locked);
	Vector3 _get_ray_pos(const Vector2 &p_pos) const;
	Vector3 _get_ray(const Vector2 &p_pos) const;
	Point2 _point_to_screen(const Vector3 &p_point);
	Transform _get_camera_transform() const;
	int get_selected_count() const;

	Vector3 _get_camera_position() const;
	Vector3 _get_camera_normal() const;
	Vector3 _get_screen_to_space(const Vector3 &p_vector3);

	void _select_region();
	bool _transform_gizmo_select(const Vector2 &p_screenpos, bool p_highlight_only = false);
	void _transform_gizmo_apply(Spatial *p_node, const Transform &p_transform, bool p_local);

	void _nav_pan(Ref<InputEventWithModifiers> p_event, const Vector2 &p_relative);
	void _nav_zoom(Ref<InputEventWithModifiers> p_event, const Vector2 &p_relative);
	void _nav_orbit(Ref<InputEventWithModifiers> p_event, const Vector2 &p_relative);
	void _nav_look(Ref<InputEventWithModifiers> p_event, const Vector2 &p_relative);

	float get_znear() const;
	float get_zfar() const;
	float get_fov() const;

	ObjectID clicked;
	Vector<_RayResult> selection_results;
	bool clicked_wants_append;
	bool selection_in_progress = false;

	PopupMenu *selection_menu;

	enum NavigationZoomStyle {
		NAVIGATION_ZOOM_VERTICAL,
		NAVIGATION_ZOOM_HORIZONTAL
	};

	enum NavigationMode {
		NAVIGATION_NONE,
		NAVIGATION_PAN,
		NAVIGATION_ZOOM,
		NAVIGATION_ORBIT,
		NAVIGATION_LOOK,
		NAVIGATION_MOVE
	};

public:
	enum TransformMode {
		TRANSFORM_NONE,
		TRANSFORM_ROTATE,
		TRANSFORM_TRANSLATE,
		TRANSFORM_SCALE

	};
	enum TransformPlane {
		TRANSFORM_VIEW,
		TRANSFORM_X_AXIS,
		TRANSFORM_Y_AXIS,
		TRANSFORM_Z_AXIS,
		TRANSFORM_YZ,
		TRANSFORM_XZ,
		TRANSFORM_XY,
	};

	struct EditData {
		TransformMode mode;
		TransformPlane plane;
		Transform original;
		Vector3 click_ray;
		Vector3 click_ray_pos;
		Vector3 center;
		Point2 mouse_pos;
		Point2 original_mouse_pos;
		bool snap;
		Ref<EditorSpatialGizmo> gizmo;
		int gizmo_handle;
		bool gizmo_handle_secondary;
		Variant gizmo_initial_value;
	} _edit;

private:
	struct Cursor {
		Vector3 pos;
		float x_rot, y_rot, distance, fov_scale;
		Vector3 eye_pos; // Used in freelook mode
		bool region_select;
		Point2 region_begin, region_end;

		Cursor() {
			// These rotations place the camera in +X +Y +Z, aka south east, facing north west.
			x_rot = 0.5;
			y_rot = -0.5;
			distance = 4;
			fov_scale = 1.0;
			region_select = false;
		}
	};
	// Viewport camera supports movement smoothing,
	// so one cursor is the real cursor, while the other can be an interpolated version.
	Cursor cursor; // Immediate cursor
	Cursor camera_cursor; // That one may be interpolated (don't modify this one except for smoothing purposes)

	void scale_fov(real_t p_fov_offset);
	void reset_fov();
	void scale_cursor_distance(real_t scale);

	void set_freelook_active(bool active_now);
	void scale_freelook_speed(real_t scale);

	real_t zoom_indicator_delay;
	int zoom_failed_attempts_count = 0;

	RID move_gizmo_instance[3], move_plane_gizmo_instance[3], rotate_gizmo_instance[4], scale_gizmo_instance[3], scale_plane_gizmo_instance[3];

	String last_message;
	String message;
	float message_time;

	void set_message(String p_message, float p_time = 5);

	void _view_settings_confirmed(float p_interp_delta);
	void _update_camera(float p_interp_delta);
	void _update_navigation_controls_visibility();
	Transform to_camera_transform(const Cursor &p_cursor) const;
	void _draw();

	void _surface_mouse_enter();
	void _surface_mouse_exit();
	void _surface_focus_enter();
	void _surface_focus_exit();

	void _sinput(const Ref<InputEvent> &p_event);
	void _update_freelook(real_t delta);
	SpatialEditor *spatial_editor;

	Camera *previewing;
	Camera *preview;

	bool previewing_camera;
	bool previewing_cinema;
	bool _is_node_locked(const Node *p_node);
	void _preview_exited_scene();
	void _toggle_camera_preview(bool);
	void _toggle_cinema_preview(bool);
	void _init_gizmo_instance(int p_idx);
	void _finish_gizmo_instances();
	void _selection_result_pressed(int);
	void _selection_menu_hide();
	void _list_select(Ref<InputEventMouseButton> b);
	Point2i _get_warped_mouse_motion(const Ref<InputEventMouseMotion> &p_ev_mouse_motion) const;

	Vector3 _get_instance_position(const Point2 &p_pos) const;
	static AABB _calculate_spatial_bounds(const Spatial *p_parent, bool p_exclude_toplevel_transform = true);

	Node *_sanitize_preview_node(Node *p_node) const;

	void _create_preview(const Vector<String> &files) const;
	void _remove_preview();
	bool _cyclical_dependency_exists(const String &p_target_scene_path, Node *p_desired_node);
	bool _create_instance(Node *parent, String &path, const Point2 &p_point);
	void _perform_drop_data();

	bool can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const;
	void drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from);

	void _project_settings_changed();

	Transform _compute_transform(TransformMode p_mode, const Transform &p_original, const Transform &p_original_local, Vector3 p_motion, double p_extra, bool p_local);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void update_surface() { surface->update(); }
	void update_transform_gizmo_view();

	void set_can_preview(Camera *p_preview);
	void set_state(const Dictionary &p_state);
	Dictionary get_state() const;
	void reset();
	bool is_freelook_active() const { return freelook_active; }

	void focus_selection();

	void assign_pending_data_pointers(
			Spatial *p_preview_node,
			AABB *p_preview_bounds,
			AcceptDialog *p_accept);

	Viewport *get_viewport_node() { return viewport; }
	Camera *get_camera() { return camera; } // return the default camera object.

	SpatialEditorViewport(SpatialEditor *p_spatial_editor, EditorNode *p_editor, int p_index);
	~SpatialEditorViewport();

public:
	ViewportContainer *get_viewport_container() const { return viewport_container; }
	float get_gizmo_scale() { return gizmo_scale; };

	Vector3 get_camera_normal() const;
	Vector3 get_ray_pos(const Vector2 &p_pos) const;
	Vector3 get_ray(const Vector2 &p_pos) const;
};

class SpatialEditorSelectedItem : public Object {
	GDCLASS(SpatialEditorSelectedItem, Object);

public:
	AABB aabb;
	Transform original; // original location when moving
	Transform original_local;
	Transform last_xform; // last transform
	bool last_xform_dirty;
	Spatial *sp;
	RID sbox_instance;
	RID sbox_instance_offset;
	RID sbox_instance_xray;
	RID sbox_instance_xray_offset;
	Ref<EditorSpatialGizmo> gizmo;
	Map<int, Transform> subgizmos; // map ID -> initial transform

	SpatialEditorSelectedItem() {
		sp = nullptr;
		last_xform_dirty = true;
	}
	~SpatialEditorSelectedItem();
};

class SpatialEditorViewportContainer : public Container {
	GDCLASS(SpatialEditorViewportContainer, Container);

public:
	enum View {
		VIEW_USE_1_VIEWPORT,
		VIEW_USE_2_VIEWPORTS,
		VIEW_USE_2_VIEWPORTS_ALT,
		VIEW_USE_3_VIEWPORTS,
		VIEW_USE_3_VIEWPORTS_ALT,
		VIEW_USE_4_VIEWPORTS,
	};

private:
	View view;
	bool mouseover;
	float ratio_h;
	float ratio_v;

	bool hovering_v;
	bool hovering_h;

	bool dragging_v;
	bool dragging_h;
	Vector2 drag_begin_pos;
	Vector2 drag_begin_ratio;

	void _gui_input(const Ref<InputEvent> &p_event);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_view(View p_view);
	View get_view();

	SpatialEditorViewportContainer();
};

class SpatialEditor : public VBoxContainer {
	GDCLASS(SpatialEditor, VBoxContainer);

public:
	static const unsigned int VIEWPORTS_COUNT = 4;

	//Make sure the entries has the same order as in the MenuOption enum!
	enum ToolMode {
		TOOL_MODE_SELECT = 0,
		TOOL_MODE_MOVE,
		TOOL_MODE_ROTATE,
		TOOL_MODE_SCALE,
		TOOL_MODE_LIST_SELECT,
		TOOL_LOCK_SELECTED,
		TOOL_UNLOCK_SELECTED,
		TOOL_GROUP_SELECTED,
		TOOL_UNGROUP_SELECTED,
		TOOL_CONVERT_ROOMS,

		TOOL_MAX
	};

	enum ToolOptions {

		TOOL_OPT_LOCAL_COORDS,
		TOOL_OPT_USE_SNAP,
		TOOL_OPT_OVERRIDE_CAMERA,
		TOOL_OPT_MAX

	};

private:
	EditorNode *editor;
	EditorSelection *editor_selection;

	SpatialEditorViewportContainer *viewport_base;
	SpatialEditorViewport *viewports[VIEWPORTS_COUNT];
	VSplitContainer *shader_split;
	HSplitContainer *left_panel_split;
	HSplitContainer *right_panel_split;

	/////

	ToolMode tool_mode;

	RenderingServer::ScenarioDebugMode scenario_debug;

	RID origin;
	RID origin_instance;
	bool origin_enabled;
	RID grid[3];
	RID grid_instance[3];
	bool grid_visible[3]; //currently visible
	bool grid_enable[3]; //should be always visible if true
	bool grid_enabled;

	Ref<ArrayMesh> move_gizmo[3], move_plane_gizmo[3], rotate_gizmo[4], scale_gizmo[3], scale_plane_gizmo[3];
	Ref<SpatialMaterial> gizmo_color[3];
	Ref<SpatialMaterial> plane_gizmo_color[3];
	Ref<ShaderMaterial> rotate_gizmo_color[3];
	Ref<SpatialMaterial> gizmo_color_hl[3];
	Ref<SpatialMaterial> plane_gizmo_color_hl[3];
	Ref<ShaderMaterial> rotate_gizmo_color_hl[3];

	Ref<SpatialGizmo> current_hover_gizmo;
	int current_hover_gizmo_handle;
	bool current_hover_gizmo_handle_secondary;

	float snap_translate_value;
	float snap_rotate_value;
	float snap_scale_value;

	Ref<ArrayMesh> selection_box_xray;
	Ref<ArrayMesh> selection_box;
	RID indicators;
	RID indicators_instance;
	RID cursor_mesh;
	RID cursor_instance;
	Ref<SpatialMaterial> indicator_mat;
	Ref<ShaderMaterial> grid_mat[3];
	Ref<SpatialMaterial> cursor_material;

	// Scene drag and drop support
	Spatial *preview_node;
	AABB preview_bounds;

	struct Gizmo {
		bool visible;
		float scale;
		Transform transform;
	} gizmo;

	//Make sure the entries has the same order as in the ToolMode enum!
	enum MenuOption {
		MENU_TOOL_SELECT = 0,
		MENU_TOOL_MOVE,
		MENU_TOOL_ROTATE,
		MENU_TOOL_SCALE,
		MENU_TOOL_LIST_SELECT,
		MENU_LOCK_SELECTED,
		MENU_UNLOCK_SELECTED,
		MENU_GROUP_SELECTED,
		MENU_UNGROUP_SELECTED,
		MENU_TOOL_CONVERT_ROOMS,
		MENU_TOOL_LOCAL_COORDS,
		MENU_TOOL_USE_SNAP,
		MENU_TOOL_OVERRIDE_CAMERA,
		MENU_TRANSFORM_CONFIGURE_SNAP,
		MENU_TRANSFORM_DIALOG,
		MENU_VIEW_USE_1_VIEWPORT,
		MENU_VIEW_USE_2_VIEWPORTS,
		MENU_VIEW_USE_2_VIEWPORTS_ALT,
		MENU_VIEW_USE_3_VIEWPORTS,
		MENU_VIEW_USE_3_VIEWPORTS_ALT,
		MENU_VIEW_USE_4_VIEWPORTS,
		MENU_VIEW_ORIGIN,
		MENU_VIEW_GRID,
		MENU_VIEW_PORTAL_CULLING,
		MENU_VIEW_OCCLUSION_CULLING,
		MENU_VIEW_GIZMOS_3D_ICONS,
		MENU_VIEW_CAMERA_SETTINGS,
		MENU_SNAP_TO_FLOOR,
	};

	Button *tool_button[TOOL_MAX];
	Button *tool_option_button[TOOL_OPT_MAX];

	MenuButton *transform_menu;
	PopupMenu *gizmos_menu;
	MenuButton *view_menu = nullptr;

	AcceptDialog *accept;

	ConfirmationDialog *snap_dialog;
	ConfirmationDialog *xform_dialog;
	ConfirmationDialog *settings_dialog;

	bool snap_enabled;
	bool snap_key_enabled;
	LineEdit *snap_translate;
	LineEdit *snap_rotate;
	LineEdit *snap_scale;

	LineEdit *xform_translate[3];
	LineEdit *xform_rotate[3];
	LineEdit *xform_scale[3];
	OptionButton *xform_type;

	VBoxContainer *settings_vbc;
	SpinBox *settings_fov;
	SpinBox *settings_znear;
	SpinBox *settings_zfar;

	void _snap_changed();
	void _snap_update();
	void _xform_dialog_action();
	void _menu_item_pressed(int p_option);
	void _menu_item_toggled(bool pressed, int p_option);
	void _menu_gizmo_toggled(int p_option);
	void _update_camera_override_button(bool p_game_running);
	void _update_camera_override_viewport(Object *p_viewport);

	// Used for secondary menu items which are displayed depending on the currently selected node
	// (such as MeshInstance's "Mesh" menu).
	PanelContainer *context_menu_panel;
	HBoxContainer *context_menu_hbox;

	void _generate_selection_boxes();
	UndoRedo *undo_redo;

	int camera_override_viewport_id;

	void _init_indicators();
	void _update_context_menu_stylebox();
	void _update_gizmos_menu();
	void _update_gizmos_menu_theme();
	void _init_grid();
	void _finish_indicators();
	void _finish_grid();

	void _toggle_maximize_view(Object *p_viewport);

	Node *custom_camera;

	Object *_get_editor_data(Object *p_what);

	Ref<Environment3D> viewport_environment;

	Spatial *selected;

	void _request_gizmo(Object *p_obj);
	void _set_subgizmo_selection(Object *p_obj, Ref<SpatialGizmo> p_gizmo, int p_id, Transform p_transform = Transform());
	void _clear_subgizmo_selection(Object *p_obj = nullptr);

	static SpatialEditor *singleton;

	void _node_removed(Node *p_node);
	Vector<Ref<EditorSpatialGizmoPlugin>> gizmo_plugins_by_priority;
	Vector<Ref<EditorSpatialGizmoPlugin>> gizmo_plugins_by_name;

	void _register_all_gizmos();

	void _selection_changed();
	void _refresh_menu_icons();

protected:
	void _notification(int p_what);
	//void _gui_input(InputEvent p_event);
	void _unhandled_key_input(Ref<InputEvent> p_event);

	static void _bind_methods();

public:
	static SpatialEditor *get_singleton() { return singleton; }
	void snap_cursor_to_plane(const Plane &p_plane);

	Vector3 snap_point(Vector3 p_target, Vector3 p_start = Vector3(0, 0, 0)) const;

	float get_znear() const { return settings_znear->get_value(); }
	float get_zfar() const { return settings_zfar->get_value(); }
	float get_fov() const { return settings_fov->get_value(); }

	Transform get_gizmo_transform() const { return gizmo.transform; }
	bool is_gizmo_visible() const;

	ToolMode get_tool_mode() const { return tool_mode; }
	bool are_local_coords_enabled() const { return tool_option_button[SpatialEditor::TOOL_OPT_LOCAL_COORDS]->is_pressed(); }
	bool is_snap_enabled() const { return snap_enabled ^ snap_key_enabled; }
	float get_translate_snap() const;
	float get_rotate_snap() const;
	float get_scale_snap() const;

	Ref<ArrayMesh> get_move_gizmo(int idx) const { return move_gizmo[idx]; }
	Ref<ArrayMesh> get_move_plane_gizmo(int idx) const { return move_plane_gizmo[idx]; }
	Ref<ArrayMesh> get_rotate_gizmo(int idx) const { return rotate_gizmo[idx]; }
	Ref<ArrayMesh> get_scale_gizmo(int idx) const { return scale_gizmo[idx]; }
	Ref<ArrayMesh> get_scale_plane_gizmo(int idx) const { return scale_plane_gizmo[idx]; }

	void update_grid();
	void update_transform_gizmo();
	void update_portal_tools();
	void show_advanced_portal_tools(bool p_show);
	void update_all_gizmos(Node *p_node = nullptr);
	void snap_selected_nodes_to_floor();
	void select_gizmo_highlight_axis(int p_axis);
	void set_custom_camera(Node *p_camera) { custom_camera = p_camera; }

	void set_undo_redo(UndoRedo *p_undo_redo) { undo_redo = p_undo_redo; }
	Dictionary get_state() const;
	void set_state(const Dictionary &p_state);

	Ref<Environment3D> get_viewport_environment();

	UndoRedo *get_undo_redo() { return undo_redo; }

	void add_control_to_menu_panel(Control *p_control);
	void remove_control_from_menu_panel(Control *p_control);

	void add_control_to_left_panel(Control *p_control);
	void remove_control_from_left_panel(Control *p_control);

	void add_control_to_right_panel(Control *p_control);
	void remove_control_from_right_panel(Control *p_control);

	void move_control_to_left_panel(Control *p_control);
	void move_control_to_right_panel(Control *p_control);

	VSplitContainer *get_shader_split();

	Spatial *get_single_selected_node() { return selected; }
	bool is_current_selected_gizmo(const EditorSpatialGizmo *p_gizmo);
	bool is_subgizmo_selected(int p_id);
	Vector<int> get_subgizmo_selection();

	Ref<EditorSpatialGizmo> get_current_hover_gizmo() const { return current_hover_gizmo; }
	void set_current_hover_gizmo(Ref<EditorSpatialGizmo> p_gizmo) { current_hover_gizmo = p_gizmo; }

	void set_current_hover_gizmo_handle(int p_id, bool p_secondary) {
		current_hover_gizmo_handle = p_id;
		current_hover_gizmo_handle_secondary = p_secondary;
	}

	int get_current_hover_gizmo_handle(bool &r_secondary) const {
		r_secondary = current_hover_gizmo_handle_secondary;
		return current_hover_gizmo_handle;
	}

	void set_can_preview(Camera *p_preview);
	void set_message(String p_message, float p_time = 5);

	SpatialEditorViewport *get_editor_viewport(int p_idx) {
		ERR_FAIL_INDEX_V(p_idx, static_cast<int>(VIEWPORTS_COUNT), nullptr);
		return viewports[p_idx];
	}

	void add_gizmo_plugin(Ref<EditorSpatialGizmoPlugin> p_plugin);
	void remove_gizmo_plugin(Ref<EditorSpatialGizmoPlugin> p_plugin);

	void edit(Spatial *p_spatial);
	void clear();

	SpatialEditor(EditorNode *p_editor);
	~SpatialEditor();

public:
	Ref<EditorSpatialGizmoPlugin> get_gizmo_plugin(const String &p_plugin);
};

class SpatialEditorPlugin : public EditorPlugin {
	GDCLASS(SpatialEditorPlugin, EditorPlugin);

	SpatialEditor *spatial_editor;
	EditorNode *editor;

protected:
	static void _bind_methods();

public:
	void snap_cursor_to_plane(const Plane &p_plane);

	SpatialEditor *get_spatial_editor() { return spatial_editor; }
	virtual String get_name() const { return "3D"; }
	bool has_main_screen() const { return true; }
	virtual void make_visible(bool p_visible);
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;

	virtual Dictionary get_state() const;
	virtual void set_state(const Dictionary &p_state);
	virtual void clear() { spatial_editor->clear(); }

	virtual void edited_scene_changed();

	SpatialEditorPlugin(EditorNode *p_node);
	~SpatialEditorPlugin();
};

class ViewportNavigationControl : public Control {
	GDCLASS(ViewportNavigationControl, Control);

public:
	void set_navigation_mode(SpatialEditorViewport::NavigationMode p_nav_mode);
	void set_viewport(SpatialEditorViewport *p_viewport);

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void _gui_input(Ref<InputEvent> p_event);
	void _draw();
	void _on_mouse_entered();
	void _on_mouse_exited();
	void _process_click(int p_index, Vector2 p_position, bool p_pressed);
	void _process_drag(int p_index, Vector2 p_position, Vector2 p_relative_position);
	void _update_navigation();

private:
	SpatialEditorViewport *viewport = nullptr;
	Vector2i focused_mouse_start;
	Vector2 focused_pos;
	bool hovered = false;
	int focused_index = -1;
	SpatialEditorViewport::NavigationMode nav_mode = SpatialEditorViewport::NavigationMode::NAVIGATION_NONE;

	const float AXIS_CIRCLE_RADIUS = 30.0f * EDSCALE;
};

#endif
