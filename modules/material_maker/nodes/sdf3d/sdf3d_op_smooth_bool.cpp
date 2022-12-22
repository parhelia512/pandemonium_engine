
#include "sdf3d_op_smooth_bool.h"

#include "../../algos/mm_algos.h"
#include "../../editor/mm_graph_node.h"
#include "../mm_material.h"

Ref<MMNodeUniversalProperty> MMSdf3dOpSmoothBool::get_input1() {
	return input1;
}

void MMSdf3dOpSmoothBool::set_input1(const Ref<MMNodeUniversalProperty> &val) {
	input1 = val;
}

Ref<MMNodeUniversalProperty> MMSdf3dOpSmoothBool::get_input2() {
	return input2;
}

void MMSdf3dOpSmoothBool::set_input2(const Ref<MMNodeUniversalProperty> &val) {
	input2 = val;
}

Ref<MMNodeUniversalProperty> MMSdf3dOpSmoothBool::get_output() {
	return output;
}

void MMSdf3dOpSmoothBool::set_output(const Ref<MMNodeUniversalProperty> &val) {
	output = val;
}

int MMSdf3dOpSmoothBool::get_operation() const {
	return operation;
}

void MMSdf3dOpSmoothBool::set_operation(const int val) {
	operation = val;
	emit_changed();
	output->do_emit_changed();
}

float MMSdf3dOpSmoothBool::get_smoothness() const {
	return smoothness;
}

void MMSdf3dOpSmoothBool::set_smoothness(const float val) {
	smoothness = val;
	emit_changed();
	output->do_emit_changed();
}

void MMSdf3dOpSmoothBool::_init_properties() {
	if (!input1.is_valid()) {
		input1.instance();
		input1->set_default_type(MMNodeUniversalProperty::DEFAULT_TYPE_VECTOR2);
	}

	input1->set_input_slot_type(MMNodeUniversalProperty::SLOT_TYPE_UNIVERSAL);
	//	input1.input_slot_type = MMNodeUniversalProperty.SLOT_TYPE_VECTOR2;
	input1->set_slot_name(">>>   Input 1        ");

	if (!input1->is_connected("changed", this, "on_input_changed")) {
		input1->connect("changed", this, "on_input_changed");
	}

	if (!input2.is_valid()) {
		input2.instance();
		input2->set_default_type(MMNodeUniversalProperty::DEFAULT_TYPE_VECTOR2);
	}

	input2->set_input_slot_type(MMNodeUniversalProperty::SLOT_TYPE_UNIVERSAL);
	//	input2.input_slot_type = MMNodeUniversalProperty.SLOT_TYPE_VECTOR2;
	input2->set_slot_name(">>>   Input 2        ");

	if (!input2->is_connected("changed", this, "on_input_changed")) {
		input2->connect("changed", this, "on_input_changed");
	}

	if (!output.is_valid()) {
		output.instance();
		output->set_default_type(MMNodeUniversalProperty::DEFAULT_TYPE_VECTOR2);
	}

	output->set_output_slot_type(MMNodeUniversalProperty::SLOT_TYPE_FLOAT);
	output->set_slot_name(">>>   Output    >>>");
	output->set_get_value_from_owner(true);

	register_input_property(input1);
	register_input_property(input2);
	register_output_property(output);
}

void MMSdf3dOpSmoothBool::_register_methods(MMGraphNode *mm_graph_node) {
	mm_graph_node->add_slot_label_universal(input1);
	mm_graph_node->add_slot_label_universal(input2);
	mm_graph_node->add_slot_label_universal(output);

	Array arr;
	arr.push_back("Union");
	arr.push_back("Subtraction");
	arr.push_back("Intersection");

	mm_graph_node->add_slot_enum("get_operation", "set_operation", "Operation", arr);
	mm_graph_node->add_slot_float("get_smoothness", "set_smoothness", "Smoothness", 0.01);
}

Vector2 MMSdf3dOpSmoothBool::_get_property_value_sdf3d(const Vector3 &uv3) {
	Vector2 s1 = input1->get_value_sdf3d(uv3);
	Vector2 s2 = input2->get_value_sdf3d(uv3);

	if (operation == 0) {
		return MMAlgos::sdf3d_smooth_union(s1, s2, smoothness);
	} else if (operation == 1) {
		return MMAlgos::sdf3d_smooth_subtraction(s1, s2, smoothness);
	} else if (operation == 2) {
		return MMAlgos::sdf3d_smooth_intersection(s1, s2, smoothness);
	}

	return Vector2();
}

void MMSdf3dOpSmoothBool::on_input_changed() {
	emit_changed();
	output->do_emit_changed();
}

MMSdf3dOpSmoothBool::MMSdf3dOpSmoothBool() {
	operation = 0;
	smoothness = 0.15;
}

MMSdf3dOpSmoothBool::~MMSdf3dOpSmoothBool() {
}

void MMSdf3dOpSmoothBool::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_input1"), &MMSdf3dOpSmoothBool::get_input1);
	ClassDB::bind_method(D_METHOD("set_input1", "value"), &MMSdf3dOpSmoothBool::set_input1);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "input1", PROPERTY_HINT_RESOURCE_TYPE, "MMNodeUniversalProperty"), "set_input1", "get_input1");

	ClassDB::bind_method(D_METHOD("get_input2"), &MMSdf3dOpSmoothBool::get_input2);
	ClassDB::bind_method(D_METHOD("set_input2", "value"), &MMSdf3dOpSmoothBool::set_input2);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "input2", PROPERTY_HINT_RESOURCE_TYPE, "MMNodeUniversalProperty"), "set_input2", "get_input2");

	ClassDB::bind_method(D_METHOD("get_output"), &MMSdf3dOpSmoothBool::get_output);
	ClassDB::bind_method(D_METHOD("set_output", "value"), &MMSdf3dOpSmoothBool::set_output);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "output", PROPERTY_HINT_RESOURCE_TYPE, "MMNodeUniversalProperty"), "set_output", "get_output");

	ClassDB::bind_method(D_METHOD("get_operation"), &MMSdf3dOpSmoothBool::get_operation);
	ClassDB::bind_method(D_METHOD("set_operation", "value"), &MMSdf3dOpSmoothBool::set_operation);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "operation"), "set_operation", "get_operation");

	ClassDB::bind_method(D_METHOD("get_smoothness"), &MMSdf3dOpSmoothBool::get_smoothness);
	ClassDB::bind_method(D_METHOD("set_smoothness", "value"), &MMSdf3dOpSmoothBool::set_smoothness);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "smoothness"), "set_smoothness", "get_smoothness");

	ClassDB::bind_method(D_METHOD("on_input_changed"), &MMSdf3dOpSmoothBool::on_input_changed);
}
