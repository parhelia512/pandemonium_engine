
#include "curve_edit.h"

#include "curve_dialog.h"
#include "curve_view.h"

Variant CurveEdit::get_value() {
	return value;
}

void CurveEdit::set_value(const Variant &v) {
	value = v;
	_curve_view->set_curve(value);
	_curve_view->update();
}

void CurveEdit::_on_CurveEdit_pressed() {
	CurveDialog *dialog = memnew(CurveDialog);
	add_child(dialog);
	dialog->connect("curve_changed", self, "on_value_changed");
	dialog->edit_curve(value);
}

void CurveEdit::on_value_changed(const Variant &v) {
	//set_value(v);
	emit_signal("updated", v);
	_curve_view->update();
}

CurveEdit::CurveEdit() {
	set_custom_minimum_size(Vector2(60, 20));
	set_focus_mode(FOCUS_CLICK);

	_curve_view = memnew(CurveView);
	_curve_view->set_name("CurveView");
	add_child(_curve_view);
	_curve_view->set_mouse_filter(MOUSE_FILTER_IGNORE);
}

CurveEdit::~CurveEdit() {
}

void CurveEdit::_bind_methods() {
	ADD_SIGNAL(MethodInfo("updated", PropertyInfo(Variant::OBJECT, "curve", PROPERTY_HINT_RESOURCE_TYPE, "CurveBase")));

	//ClassDB::bind_method(D_METHOD("get_Variant"), &CurveEdit::get_Variant);
	//ClassDB::bind_method(D_METHOD("set_Variant", "value"), &CurveEdit::set_Variant);
	//ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "Variant", PROPERTY_HINT_RESOURCE_TYPE, "Variant"), "set_Variant", "get_Variant");

	ClassDB::bind_method(D_METHOD("set_value", "v"), &CurveEdit::set_value);
	ClassDB::bind_method(D_METHOD("_on_CurveEdit_pressed"), &CurveEdit::_on_CurveEdit_pressed);
	ClassDB::bind_method(D_METHOD("on_value_changed", "v"), &CurveEdit::on_value_changed);
}
