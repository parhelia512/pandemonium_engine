
#include "gsaispecializedagent.h"


bool GSAISpecializedAgent::get_calculate_velocities() const {
 return calculate_velocities;
}

void GSAISpecializedAgent::set_calculate_velocities(const bool val) {
calculate_velocities = val;
}


bool GSAISpecializedAgent::get_apply_linear_drag() const {
 return apply_linear_drag;
}

void GSAISpecializedAgent::set_apply_linear_drag(const bool val) {
apply_linear_drag = val;
}


bool GSAISpecializedAgent::get_apply_angular_drag() const {
 return apply_angular_drag;
}

void GSAISpecializedAgent::set_apply_angular_drag(const bool val) {
apply_angular_drag = val;
}


float GSAISpecializedAgent::get_linear_drag_percentage() const {
 return linear_drag_percentage;
}

void GSAISpecializedAgent::set_linear_drag_percentage(const float val) {
linear_drag_percentage = val;
}


float GSAISpecializedAgent::get_angular_drag_percentage() const {
 return angular_drag_percentage;
}

void GSAISpecializedAgent::set_angular_drag_percentage(const float val) {
angular_drag_percentage = val;
}


float GSAISpecializedAgent::get_last_orientation() const {
 return last_orientation;
}

void GSAISpecializedAgent::set_last_orientation(const float val) {
last_orientation = val;
}


bool GSAISpecializedAgent::get_applied_steering() const {
 return applied_steering;
}

void GSAISpecializedAgent::set_applied_steering(const bool val) {
applied_steering = val;
}



 // A base class for a specialized steering agent that updates itself every frame;
 // so the user does not have to. All other specialized agents derive from this.;
 // @category - Specialized agents;
 // @tags - abstract;
 // If `true`, calculates linear and angular velocities based on the previous;
 // frame. When `false`, the user must keep those values updated.;
  bool calculate_velocities = true;
 // If `true`, interpolates the current linear velocity towards 0 by the;
 // `linear_drag_percentage` value.;
 // Does not apply to `RigidBody` and `RigidBody2D` nodes.;
  bool apply_linear_drag = true;
 // If `true`, interpolates the current angular velocity towards 0 by the;
 // `angular_drag_percentage` value.;
 // Does not apply to `RigidBody` and `RigidBody2D` nodes.;
  bool apply_angular_drag = true;
 // The percentage between the current linear velocity and 0 to interpolate by if;
 // `apply_linear_drag` is true.;
 // Does not apply to `RigidBody` and `RigidBody2D` nodes.;
  float linear_drag_percentage = 0.0;
 // The percentage between the current angular velocity and 0 to interpolate by if;
 // `apply_angular_drag` is true.;
 // Does not apply to `RigidBody` and `RigidBody2D` nodes.;
  float angular_drag_percentage = 0.0;
  float last_orientation = 0.0;
  bool applied_steering = false;

 void GSAISpecializedAgent::apply_steering(const GSAITargetAcceleration &_acceleration, const float _delta) {
  call("_apply_steering", _acceleration, _delta);
}

 // Moves the agent's body by target `acceleration`.;
 // @tags - virtual;

 void GSAISpecializedAgent::_apply_steering(const GSAITargetAcceleration &_acceleration, const float _delta) {
  pass;
}

}

 GSAISpecializedAgent::GSAISpecializedAgent() {
  calculate_velocities = true;
  apply_linear_drag = true;
  apply_angular_drag = true;
  linear_drag_percentage = 0.0;
  angular_drag_percentage = 0.0;
  last_orientation = 0.0;
  applied_steering = false;
 }

 GSAISpecializedAgent::~GSAISpecializedAgent() {
 }


 static void GSAISpecializedAgent::_bind_methods() {
   ClassDB::bind_method(D_METHOD("get_calculate_velocities"), &GSAISpecializedAgent::get_calculate_velocities);
   ClassDB::bind_method(D_METHOD("set_calculate_velocities", "value"), &GSAISpecializedAgent::set_calculate_velocities);
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "calculate_velocities"), "set_calculate_velocities", "get_calculate_velocities");


   ClassDB::bind_method(D_METHOD("get_apply_linear_drag"), &GSAISpecializedAgent::get_apply_linear_drag);
   ClassDB::bind_method(D_METHOD("set_apply_linear_drag", "value"), &GSAISpecializedAgent::set_apply_linear_drag);
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "apply_linear_drag"), "set_apply_linear_drag", "get_apply_linear_drag");


   ClassDB::bind_method(D_METHOD("get_apply_angular_drag"), &GSAISpecializedAgent::get_apply_angular_drag);
   ClassDB::bind_method(D_METHOD("set_apply_angular_drag", "value"), &GSAISpecializedAgent::set_apply_angular_drag);
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "apply_angular_drag"), "set_apply_angular_drag", "get_apply_angular_drag");


   ClassDB::bind_method(D_METHOD("get_linear_drag_percentage"), &GSAISpecializedAgent::get_linear_drag_percentage);
   ClassDB::bind_method(D_METHOD("set_linear_drag_percentage", "value"), &GSAISpecializedAgent::set_linear_drag_percentage);
   ADD_PROPERTY(PropertyInfo(Variant::REAL, "linear_drag_percentage"), "set_linear_drag_percentage", "get_linear_drag_percentage");


   ClassDB::bind_method(D_METHOD("get_angular_drag_percentage"), &GSAISpecializedAgent::get_angular_drag_percentage);
   ClassDB::bind_method(D_METHOD("set_angular_drag_percentage", "value"), &GSAISpecializedAgent::set_angular_drag_percentage);
   ADD_PROPERTY(PropertyInfo(Variant::REAL, "angular_drag_percentage"), "set_angular_drag_percentage", "get_angular_drag_percentage");


   ClassDB::bind_method(D_METHOD("get_last_orientation"), &GSAISpecializedAgent::get_last_orientation);
   ClassDB::bind_method(D_METHOD("set_last_orientation", "value"), &GSAISpecializedAgent::set_last_orientation);
   ADD_PROPERTY(PropertyInfo(Variant::REAL, "last_orientation"), "set_last_orientation", "get_last_orientation");


   ClassDB::bind_method(D_METHOD("get_applied_steering"), &GSAISpecializedAgent::get_applied_steering);
   ClassDB::bind_method(D_METHOD("set_applied_steering", "value"), &GSAISpecializedAgent::set_applied_steering);
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "applied_steering"), "set_applied_steering", "get_applied_steering");


  ClassDB::bind_method(D_METHOD("apply_steering", "_acceleration", "_delta"), &GSAISpecializedAgent::apply_steering);
  ClassDB::bind_method(D_METHOD("_apply_steering", "_acceleration", "_delta"), &GSAISpecializedAgent::_apply_steering);

 }



