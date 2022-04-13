/*
Copyright (c) 2019-2022 Péter Magyar

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

#include "mdi_ed.h"

#include "core/os/keyboard.h"
#include "mdi_ed_plugin.h"

void MDIEd::_enter_tree() {
	/*
	uv_preview = get_node(uv_preview_path)
	uv_editor = get_node(uv_editor_path)

	if _plugin && uv_editor:
		uv_editor.set_plugin(_plugin)
	*/
}
void MDIEd::set_plugin(EditorPlugin *plugin) {
	/*
	_plugin = plugin

	if uv_editor:
		uv_editor.set_plugin(plugin)
	*/
}
void MDIEd::set_mesh_data_resource(Ref<MeshDataResource> a) {
	/*
	if uv_preview:
		uv_preview.set_mesh_data_resource(a)

	if uv_editor:
		uv_editor.set_mesh_data_resource(a)
	*/
}
void MDIEd::set_mesh_data_instance(MeshDataInstance *a) {
	/*
	if uv_preview:
		uv_preview.set_mesh_data_instance(a)

	if uv_editor:
		uv_editor.set_mesh_data_instance(a)
	*/
}
void MDIEd::_unhandled_key_input(Ref<InputEventKey> event) {
	if (event->is_echo()) {
		return;
	}

	if (event->get_alt() || event->get_shift() || event->get_control() || event->get_metakey() || event->get_command()) {
		return;
	}

	uint32_t scancode = event->get_scancode();

	if (scancode == KEY_G) {
		set_edit_mode_translate();
	} else if (scancode == KEY_H) {
		set_edit_mode_rotate();
	} else if (scancode == KEY_J) {
		set_edit_mode_scale();
	} else if (scancode == KEY_V) {
		set_axis_x(!get_axis_x());
	} else if (scancode == KEY_B) {
		set_axis_y(!get_axis_y());
	} else if (scancode == KEY_N) {
		set_axis_z(!get_axis_z());
	} else if (scancode == KEY_K) {
		set_selection_mode_vertex();
	} else if (scancode == KEY_L) {
		set_selection_mode_edge();
	} else if (scancode == KEY_SEMICOLON) {
		set_selection_mode_face();
	}
}

//Edit modes
void MDIEd::set_edit_mode_translate() {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer/Translate.pressed = true
	*/
}
void MDIEd::set_edit_mode_rotate() {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer/Rotate.pressed = true
	*/
}
void MDIEd::set_edit_mode_scale() {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer/Scale.pressed = true
	*/
}

void MDIEd::on_edit_mode_translate_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_translate();
		}
	}
}
void MDIEd::on_edit_mode_rotate_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_rotate();
		}
	}
}
void MDIEd::on_edit_mode_scale_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_scale();
		}
	}
}

//axis locks
bool MDIEd::get_axis_x() {
	/*
	return $VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisX.pressed
	*/
	return false;
}
bool MDIEd::get_axis_y() {
	/*
	return $VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisY.pressed
	*/
	return false;
}
bool MDIEd::get_axis_z() {
	/*
	return $VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisZ.pressed
	*/
	return false;
}
void MDIEd::set_axis_x(bool on) {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisX.pressed = on
	*/
}
void MDIEd::set_axis_y(bool on) {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisY.pressed = on
	*/
}
void MDIEd::set_axis_z(bool on) {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisZ.pressed = on
	*/
}

void MDIEd::on_axis_x_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_axis_x(on);
		}
	}
}
void MDIEd::on_axis_y_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_axis_y(on);
		}
	}
}
void MDIEd::on_axis_z_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_axis_z(on);
		}
	}
}

//selection modes
void MDIEd::on_selection_mode_vertex_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_selection_mode_vertex();
		}
	}
}
void MDIEd::on_selection_mode_edge_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_selection_mode_edge();
		}
	}
}
void MDIEd::on_selection_mode_face_toggled(bool on) {
	if (on) {
		if (_plugin) {
			_plugin->set_selection_mode_face();
		}
	}
}

void MDIEd::set_selection_mode_vertex() {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Vertex.pressed = true
	*/
}
void MDIEd::set_selection_mode_edge() {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Edge.pressed = true
	*/
}
void MDIEd::set_selection_mode_face() {
	/*
	$VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Face.pressed = true
	*/
}

void MDIEd::_on_Extrude_pressed() {
	_plugin->extrude();
}
void MDIEd::_on_AddBox_pressed() {
	_plugin->add_box();
}
void MDIEd::_on_UnwrapButton_pressed() {
	_plugin->uv_unwrap();
}
void MDIEd::_on_add_triangle_pressed() {
	_plugin->add_triangle();
}
void MDIEd::_on_add_quad_pressed() {
	_plugin->add_quad();
}
void MDIEd::_on_split_pressed() {
	_plugin->split();
}
void MDIEd::_on_connect_to_first_selected_pressed() {
	_plugin->connect_to_first_selected();
}
void MDIEd::_on_connect_to_avg_pressed() {
	_plugin->connect_to_avg();
}
void MDIEd::_on_connect_to_last_selected_pressed() {
	_plugin->connect_to_last_selected();
}
void MDIEd::_on_disconnect_pressed() {
	_plugin->disconnect_action();
}
void MDIEd::_on_add_triangle_at_pressed() {
	_plugin->add_triangle_at();
}
void MDIEd::_on_add_auad_at_pressed() {
	_plugin->add_quad_at();
}
void MDIEd::_oncreate_face_pressed() {
	_plugin->create_face();
}
void MDIEd::_on_delete_pressed() {
	_plugin->delete_selected();
}
void MDIEd::_on_GenNormals_pressed() {
	_plugin->generate_normals();
}
void MDIEd::_on_RemDoubles_pressed() {
	_plugin->remove_doubles();
}
void MDIEd::_on_MergeOptimize_pressed() {
	_plugin->merge_optimize();
}
void MDIEd::_on_GenTangents_pressed() {
	_plugin->generate_tangents();
}
void MDIEd::_on_mark_seam_pressed() {
	_plugin->mark_seam();
}
void MDIEd::_on_unmark_seam_pressed() {
	_plugin->unmark_seam();
}
void MDIEd::_on_apply_seams_pressed() {
	_plugin->apply_seam();
}
void MDIEd::_on_uv_edit_pressed() {
	/*
	$Popups/UVEditorPopup.popup_centered()
	*/
}

void MDIEd::on_pivot_average_toggled(bool on) {
	if (on) {
		_plugin->set_pivot_averaged();
	}
}
void MDIEd::on_pivot_mdi_origin_toggled(bool on) {
	if (on) {
		_plugin->set_pivot_mdi_origin();
	}
}
void MDIEd::on_pivot_world_origin_toggled(bool on) {
	if (on) {
		_plugin->set_pivot_world_origin();
	}
	/*
	if on:
		_plugin.set_pivot_world_origin()
	*/
}
void MDIEd::on_visual_indicator_outline_toggled(bool on) {
	_plugin->visual_indicator_outline_set(on);
}
void MDIEd::on_visual_indicator_seam_toggled(bool on) {
	_plugin->visual_indicator_seam_set(on);
}
void MDIEd::on_visual_indicator_handle_toggled(bool on) {
	_plugin->visual_indicator_handle_set(on);
}
void MDIEd::_on_select_all_pressed() {
	_plugin->select_all();
}

void MDIEd::onhandle_selection_type_front_toggled(bool on) {
	_plugin->handle_selection_type_front();
}
void MDIEd::onhandle_selection_type_back_toggled(bool on) {
	_plugin->handle_selection_type_back();
}
void MDIEd::onhandle_selection_type_all_toggled(bool on) {
	_plugin->handle_selection_type_all();
}

void MDIEd::_on_clean_mesh_pressed() {
	_plugin->clean_mesh();
}
void MDIEd::_on_flip_face_pressed() {
	_plugin->flip_selected_faces();
}

MDIEd::MDIEd() {
	/*
[gd_scene load_steps=9 format=2]

[ext_resource path="res://addons/mesh_data_resource_editor/MDIEd.gd" type="Script" id=1]
[ext_resource path="res://addons/mesh_data_resource_editor/button_groups/vertex_position_operation_bg.tres" type="ButtonGroup" id=2]
[ext_resource path="res://addons/mesh_data_resource_editor/UVEditor.gd" type="Script" id=3]
[ext_resource path="res://addons/mesh_data_resource_editor/uv_editor/UVEditor.tscn" type="PackedScene" id=4]
[ext_resource path="res://addons/mesh_data_resource_editor/button_groups/edit_mode_button_group.tres" type="ButtonGroup" id=5]
[ext_resource path="res://addons/mesh_data_resource_editor/uv_editor/UVEditorPopup.gd" type="Script" id=6]
[ext_resource path="res://addons/mesh_data_resource_editor/button_groups/pivot_button_group.tres" type="ButtonGroup" id=7]
[ext_resource path="res://addons/mesh_data_resource_editor/button_groups/handle_selection_typen_group.tres" type="ButtonGroup" id=8]

[node name="MDIEd" type="PanelContainer"]
anchor_right = 1.0
anchor_bottom = 1.0
script = ExtResource( 1 )
__meta__ = {
"_edit_use_anchors_": false
}
uv_preview_path = NodePath("VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/UVDisplay")
uv_editor_path = NodePath("Popups/UVEditorPopup/UVEditor")

[node name="VBoxContainer" type="VBoxContainer" parent="."]
margin_left = 7.0
margin_top = 7.0
margin_right = 1017.0
margin_bottom = 593.0
__meta__ = {
"_edit_use_anchors_": false
}

[node name="Actions" type="VBoxContainer" parent="VBoxContainer"]
margin_right = 1010.0
margin_bottom = 68.0

[node name="Actions" type="HBoxContainer" parent="VBoxContainer/Actions"]
margin_right = 1010.0
margin_bottom = 68.0
alignment = 1

[node name="VBoxContainer" type="VBoxContainer" parent="VBoxContainer/Actions/Actions"]
margin_left = 457.0
margin_right = 466.0
margin_bottom = 68.0

[node name="Label" type="Label" parent="VBoxContainer/Actions/Actions/VBoxContainer"]
margin_right = 9.0
margin_bottom = 20.0
hint_tooltip = "Edit Mode"
mouse_filter = 0
size_flags_vertical = 7
text = "E"
align = 1
valign = 1

[node name="Label2" type="Label" parent="VBoxContainer/Actions/Actions/VBoxContainer"]
margin_top = 24.0
margin_right = 9.0
margin_bottom = 44.0
hint_tooltip = "Active Axis"
mouse_filter = 0
size_flags_vertical = 7
text = "A"
align = 1
valign = 1

[node name="Label3" type="Label" parent="VBoxContainer/Actions/Actions/VBoxContainer"]
margin_top = 48.0
margin_right = 9.0
margin_bottom = 68.0
hint_tooltip = "Selection Mode"
mouse_filter = 0
size_flags_vertical = 7
text = "S"
align = 1
valign = 1

[node name="VBoxContainer2" type="VBoxContainer" parent="VBoxContainer/Actions/Actions"]
margin_left = 470.0
margin_right = 553.0
margin_bottom = 68.0

[node name="HBoxContainer" type="HBoxContainer" parent="VBoxContainer/Actions/Actions/VBoxContainer2"]
margin_right = 83.0
margin_bottom = 20.0

[node name="Translate" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer"]
margin_right = 25.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Translate"
size_flags_horizontal = 3
toggle_mode = true
pressed = true
group = ExtResource( 2 )
text = "T"

[node name="Rotate" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer"]
margin_left = 29.0
margin_right = 54.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Rotate"
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 2 )
text = "R"

[node name="Scale" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer"]
margin_left = 58.0
margin_right = 83.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Scale"
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 2 )
text = "S"

[node name="HBoxContainer2" type="HBoxContainer" parent="VBoxContainer/Actions/Actions/VBoxContainer2"]
margin_top = 24.0
margin_right = 83.0
margin_bottom = 44.0

[node name="AxisX" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2"]
margin_right = 25.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
size_flags_horizontal = 3
toggle_mode = true
pressed = true
text = "X"

[node name="AxisY" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2"]
margin_left = 29.0
margin_right = 54.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
size_flags_horizontal = 3
toggle_mode = true
pressed = true
text = "Y"

[node name="AxisZ" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2"]
margin_left = 58.0
margin_right = 83.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
size_flags_horizontal = 3
toggle_mode = true
pressed = true
text = "Z"

[node name="HBoxContainer3" type="HBoxContainer" parent="VBoxContainer/Actions/Actions/VBoxContainer2"]
margin_top = 48.0
margin_right = 83.0
margin_bottom = 68.0

[node name="Vertex" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3"]
margin_right = 25.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Vertex"
size_flags_horizontal = 3
toggle_mode = true
pressed = true
group = ExtResource( 5 )
text = "V"

[node name="Edge" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3"]
margin_left = 29.0
margin_right = 54.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Edge"
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 5 )
text = "E"

[node name="Face" type="Button" parent="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3"]
margin_left = 58.0
margin_right = 83.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Face"
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 5 )
text = "F"

[node name="HSeparator" type="HSeparator" parent="VBoxContainer"]
margin_top = 72.0
margin_right = 1010.0
margin_bottom = 76.0

[node name="ScrollContainer" type="ScrollContainer" parent="VBoxContainer"]
margin_top = 80.0
margin_right = 1010.0
margin_bottom = 586.0
size_flags_horizontal = 3
size_flags_vertical = 3
scroll_horizontal_enabled = false

[node name="VBoxContainer2" type="VBoxContainer" parent="VBoxContainer/ScrollContainer"]
margin_right = 998.0
margin_bottom = 674.0
size_flags_horizontal = 3

[node name="HBoxContainer6" type="HBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_right = 998.0
margin_bottom = 20.0

[node name="Label4" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer6"]
margin_right = 10.0
margin_bottom = 20.0
hint_tooltip = "Handle Selection type."
mouse_filter = 0
size_flags_vertical = 7
text = "H"
align = 1
valign = 1

[node name="Front" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer6"]
margin_left = 14.0
margin_right = 339.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "
"
size_flags_horizontal = 3
toggle_mode = true
pressed = true
group = ExtResource( 8 )
text = "F"

[node name="Back" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer6"]
margin_left = 343.0
margin_right = 668.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Only select handles that face away
the camera."
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 8 )
text = "B"

[node name="All" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer6"]
margin_left = 672.0
margin_right = 998.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Ignore camera facing when selecting handles.
"
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 8 )
text = "A"

[node name="HBoxContainer4" type="HBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 24.0
margin_right = 998.0
margin_bottom = 44.0

[node name="Label4" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer4"]
margin_right = 8.0
margin_bottom = 20.0
hint_tooltip = "Pivot"
mouse_filter = 0
size_flags_vertical = 7
text = "P"
align = 1
valign = 1

[node name="Average" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer4"]
margin_left = 12.0
margin_right = 338.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Average"
size_flags_horizontal = 3
toggle_mode = true
pressed = true
group = ExtResource( 7 )
text = "A"

[node name="MDIOrigin" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer4"]
margin_left = 342.0
margin_right = 668.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Mesh Data Instance Origin"
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 7 )
text = "M"

[node name="WorldOrigin" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer4"]
margin_left = 672.0
margin_right = 998.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "World Origin"
size_flags_horizontal = 3
toggle_mode = true
group = ExtResource( 7 )
text = "w"

[node name="HBoxContainer5" type="HBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 48.0
margin_right = 998.0
margin_bottom = 68.0

[node name="Label4" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer5"]
margin_right = 8.0
margin_bottom = 20.0
hint_tooltip = "Visual indicators"
mouse_filter = 0
size_flags_vertical = 7
text = "V"
align = 1
valign = 1

[node name="Outline" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer5"]
margin_left = 12.0
margin_right = 338.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Outline"
size_flags_horizontal = 3
toggle_mode = true
pressed = true
text = "O"

[node name="Seam" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer5"]
margin_left = 342.0
margin_right = 668.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Seam"
size_flags_horizontal = 3
toggle_mode = true
pressed = true
text = "S"

[node name="Handle" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer5"]
margin_left = 672.0
margin_right = 998.0
margin_bottom = 20.0
rect_min_size = Vector2( 25, 20 )
hint_tooltip = "Handle"
size_flags_horizontal = 3
toggle_mode = true
pressed = true
text = "H"

[node name="HSeparator" type="HSeparator" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 72.0
margin_right = 998.0
margin_bottom = 76.0
__meta__ = {
"_edit_use_anchors_": false
}

[node name="Select All" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 80.0
margin_right = 998.0
margin_bottom = 100.0
text = "Select All"

[node name="HSeparator5" type="HSeparator" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 104.0
margin_right = 998.0
margin_bottom = 108.0
__meta__ = {
"_edit_use_anchors_": false
}

[node name="VertexOps" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 112.0
margin_right = 998.0
margin_bottom = 192.0

[node name="OperationsLabel" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps"]
margin_right = 998.0
margin_bottom = 14.0
text = "= Vertex ="
align = 1
valign = 1

[node name="Operations" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps"]
margin_top = 18.0
margin_right = 998.0
margin_bottom = 80.0

[node name="AddFace" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations"]
margin_right = 998.0
margin_bottom = 20.0
text = "Create Face"

[node name="Split" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations"]
visible = false
margin_top = 24.0
margin_right = 998.0
margin_bottom = 44.0
text = "Split"

[node name="Label" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations"]
margin_top = 24.0
margin_right = 998.0
margin_bottom = 38.0
text = "Connect"
align = 1
valign = 1

[node name="HBoxContainer" type="HBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations"]
margin_top = 42.0
margin_right = 998.0
margin_bottom = 62.0
size_flags_horizontal = 3

[node name="ConnectToFirst" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/HBoxContainer"]
margin_right = 330.0
margin_bottom = 20.0
hint_tooltip = "Move all vertices to the first one that was selected."
size_flags_horizontal = 3
text = "x<"

[node name="ConnectMed" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/HBoxContainer"]
margin_left = 334.0
margin_right = 664.0
margin_bottom = 20.0
hint_tooltip = "Move all selected vertices to their average."
size_flags_horizontal = 3
text = ">x<"

[node name="ConnectToLast" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/HBoxContainer"]
margin_left = 668.0
margin_right = 998.0
margin_bottom = 20.0
hint_tooltip = "Move all selected vertices to the last that was  selected."
size_flags_horizontal = 3
text = ">x"

[node name="EdgeOps" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
visible = false
margin_top = 108.0
margin_right = 998.0
margin_bottom = 206.0

[node name="OperationsLabel" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps"]
margin_right = 998.0
margin_bottom = 14.0
text = "= Edge ="
align = 1
valign = 1

[node name="Operations" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps"]
margin_top = 18.0
margin_right = 998.0
margin_bottom = 98.0

[node name="Extrude" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations"]
margin_right = 1010.0
margin_bottom = 20.0
text = "Extrude"

[node name="Label" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations"]
margin_right = 998.0
margin_bottom = 14.0
text = "Append"
align = 1
valign = 1

[node name="HBoxContainer2" type="HBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations"]
margin_top = 18.0
margin_right = 998.0
margin_bottom = 38.0

[node name="TriAt" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer2"]
margin_right = 497.0
margin_bottom = 20.0
size_flags_horizontal = 3
size_flags_vertical = 3
text = "Tri"

[node name="QuadAt" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer2"]
margin_left = 501.0
margin_right = 998.0
margin_bottom = 20.0
size_flags_horizontal = 3
text = "Quad"

[node name="SeamLabel" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations"]
margin_top = 42.0
margin_right = 998.0
margin_bottom = 56.0
text = "Seam"
align = 1
valign = 1

[node name="HBoxContainer" type="HBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations"]
margin_top = 60.0
margin_right = 998.0
margin_bottom = 80.0
size_flags_horizontal = 3

[node name="Mark" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer"]
margin_right = 497.0
margin_bottom = 20.0
size_flags_horizontal = 3
text = "Mark"

[node name="Unmark" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer"]
margin_left = 501.0
margin_right = 998.0
margin_bottom = 20.0
size_flags_horizontal = 3
text = "Unmark"

[node name="FaceOps" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
visible = false
margin_right = 55.0
margin_bottom = 38.0

[node name="OperationsLabel" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/FaceOps"]
margin_right = 1010.0
margin_bottom = 14.0
text = "= Face ="
align = 1
valign = 1

[node name="Operations" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/FaceOps"]
margin_top = 18.0
margin_right = 1010.0
margin_bottom = 110.0

[node name="Delete" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/FaceOps/Operations"]
margin_right = 1010.0
margin_bottom = 20.0
text = "Delete"

[node name="Flip" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/FaceOps/Operations"]
margin_right = 1010.0
margin_bottom = 20.0
text = "Flip"

[node name="HSeparator4" type="HSeparator" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 196.0
margin_right = 998.0
margin_bottom = 200.0

[node name="Operations" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 204.0
margin_right = 998.0
margin_bottom = 568.0

[node name="OperationsLabel" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations"]
margin_right = 998.0
margin_bottom = 14.0
text = "= Operations ="
align = 1
valign = 1

[node name="Operations" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations"]
margin_top = 18.0
margin_right = 998.0
margin_bottom = 364.0

[node name="Label2" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_right = 998.0
margin_bottom = 14.0
text = "Generate"
align = 1
valign = 1

[node name="GenNormals" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 18.0
margin_right = 998.0
margin_bottom = 38.0
text = "Normals"

[node name="GenTangents" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 42.0
margin_right = 998.0
margin_bottom = 62.0
text = "Tangents"

[node name="Label" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 66.0
margin_right = 998.0
margin_bottom = 80.0
text = "Optimizations"
align = 1
valign = 1

[node name="RemDoubles" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 84.0
margin_right = 998.0
margin_bottom = 104.0
text = "Rem Doubles"

[node name="MergeOptimize" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 108.0
margin_right = 998.0
margin_bottom = 128.0
text = "Full Merge"

[node name="Clean" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 132.0
margin_right = 998.0
margin_bottom = 152.0
hint_tooltip = "Clean mesh for example for unused verts."
text = "Clean"

[node name="Label3" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 156.0
margin_right = 998.0
margin_bottom = 170.0
text = "UV"
align = 1
valign = 1

[node name="UVDisplay" type="Control" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 174.0
margin_right = 998.0
margin_bottom = 274.0
rect_min_size = Vector2( 100, 100 )
script = ExtResource( 3 )

[node name="UVEditButton" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 278.0
margin_right = 998.0
margin_bottom = 298.0
text = "Edit"

[node name="ApplySeams" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 302.0
margin_right = 998.0
margin_bottom = 322.0
text = "Apply seams"

[node name="UnwrapButton2" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations"]
margin_top = 326.0
margin_right = 998.0
margin_bottom = 346.0
text = "Unwrap"

[node name="HSeparator3" type="HSeparator" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 572.0
margin_right = 998.0
margin_bottom = 576.0

[node name="Add" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 580.0
margin_right = 998.0
margin_bottom = 666.0

[node name="AddLabel" type="Label" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Add"]
margin_right = 998.0
margin_bottom = 14.0
text = "= Add ="
align = 1
valign = 1

[node name="Add" type="VBoxContainer" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Add"]
margin_top = 18.0
margin_right = 998.0
margin_bottom = 86.0

[node name="AddBox" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Add/Add"]
margin_right = 998.0
margin_bottom = 20.0
text = "Box"

[node name="AddTriangle" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Add/Add"]
margin_top = 24.0
margin_right = 998.0
margin_bottom = 44.0
size_flags_horizontal = 3
text = "Tri"

[node name="AddQuad" type="Button" parent="VBoxContainer/ScrollContainer/VBoxContainer2/Add/Add"]
margin_top = 48.0
margin_right = 998.0
margin_bottom = 68.0
size_flags_horizontal = 3
text = "Quad"

[node name="HSeparator2" type="HSeparator" parent="VBoxContainer/ScrollContainer/VBoxContainer2"]
margin_top = 670.0
margin_right = 998.0
margin_bottom = 674.0

[node name="Popups" type="Control" parent="."]
margin_left = 7.0
margin_top = 7.0
margin_right = 1017.0
margin_bottom = 593.0
mouse_filter = 2
__meta__ = {
"_edit_lock_": true
}

[node name="UVEditorPopup" type="ConfirmationDialog" parent="Popups"]
margin_left = 149.0
margin_top = 35.0
margin_right = 901.0
margin_bottom = 803.0
window_title = "UV Editor"
resizable = true
script = ExtResource( 6 )
__meta__ = {
"_edit_use_anchors_": false
}

[node name="UVEditor" parent="Popups/UVEditorPopup" instance=ExtResource( 4 )]
margin_left = 8.0
margin_top = 8.0
margin_right = -8.0
margin_bottom = -36.0
size_flags_horizontal = 3
size_flags_vertical = 3

[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer/Translate" to="." method="on_edit_mode_translate_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer/Rotate" to="." method="on_edit_mode_rotate_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer/Scale" to="." method="on_edit_mode_scale_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisX" to="." method="on_axis_x_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisY" to="." method="on_axis_y_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer2/AxisZ" to="." method="on_axis_z_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Vertex" to="." method="on_selection_mode_vertex_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Vertex" to="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps" method="set_visible"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Edge" to="." method="on_selection_mode_edge_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Edge" to="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps" method="set_visible"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Face" to="." method="on_selection_mode_face_toggled"]
[connection signal="toggled" from="VBoxContainer/Actions/Actions/VBoxContainer2/HBoxContainer3/Face" to="VBoxContainer/ScrollContainer/VBoxContainer2/FaceOps" method="set_visible"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer6/Front" to="." method="onhandle_selection_type_front_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer6/Back" to="." method="onhandle_selection_type_back_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer6/All" to="." method="onhandle_selection_type_all_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer4/Average" to="." method="on_pivot_average_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer4/MDIOrigin" to="." method="on_pivot_mdi_origin_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer4/WorldOrigin" to="." method="on_pivot_world_origin_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer5/Outline" to="." method="on_visual_indicator_outline_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer5/Seam" to="." method="on_visual_indicator_seam_toggled"]
[connection signal="toggled" from="VBoxContainer/ScrollContainer/VBoxContainer2/HBoxContainer5/Handle" to="." method="on_visual_indicator_handle_toggled"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Select All" to="." method="_on_select_all_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/AddFace" to="." method="_oncreate_face_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/Split" to="." method="_on_split_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/HBoxContainer/ConnectToFirst" to="." method="_on_connect_to_first_selected_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/HBoxContainer/ConnectMed" to="." method="_on_connect_to_avg_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/VertexOps/Operations/HBoxContainer/ConnectToLast" to="." method="_on_connect_to_last_selected_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/Extrude" to="." method="_on_Extrude_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer2/TriAt" to="." method="_on_add_triangle_at_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer2/QuadAt" to="." method="_on_add_auad_at_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer/Mark" to="." method="_on_mark_seam_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/EdgeOps/Operations/HBoxContainer/Unmark" to="." method="_on_unmark_seam_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/FaceOps/Operations/Delete" to="." method="_on_delete_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/FaceOps/Operations/Flip" to="." method="_on_flip_face_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/GenNormals" to="." method="_on_GenNormals_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/GenTangents" to="." method="_on_GenTangents_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/RemDoubles" to="." method="_on_RemDoubles_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/MergeOptimize" to="." method="_on_MergeOptimize_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/Clean" to="." method="_on_clean_mesh_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/UVEditButton" to="." method="_on_uv_edit_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/ApplySeams" to="." method="_on_apply_seams_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Operations/Operations/UnwrapButton2" to="." method="_on_UnwrapButton_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Add/Add/AddBox" to="." method="_on_AddBox_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Add/Add/AddTriangle" to="." method="_on_add_triangle_pressed"]
[connection signal="pressed" from="VBoxContainer/ScrollContainer/VBoxContainer2/Add/Add/AddQuad" to="." method="_on_add_quad_pressed"]


	*/
}

MDIEd::~MDIEd() {
}

void MDIEd::_bind_methods() {
}
