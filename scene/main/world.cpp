#include "world.h"

#include "core/core_string_names.h"
#include "scene/3d/spatial.h"
#include "scene/3d/world_environment_3d.h"
#include "scene/resources/world_2d.h"
#include "scene/resources/world_3d.h"
#include "viewport.h"

Ref<World2D> World::get_world_2d() const {
	return world_2d;
}

void World::set_world_2d(const Ref<World2D> &p_world_2d) {
	if (world_2d == p_world_2d) {
		return;
	}

	if (_parent_world && _parent_world->find_world_2d() == p_world_2d) {
		WARN_PRINT("Unable to use parent world as world_2d");
		return;
	}

	Ref<World2D> old_world;

	if (is_inside_tree()) {
		old_world = find_world_2d();
	}

	if (p_world_2d.is_valid()) {
		world_2d = p_world_2d;
	} else {
		WARN_PRINT("Invalid world");
		world_2d = Ref<World2D>(memnew(World2D));
	}

	_on_set_world_2d(old_world);
}

Ref<World3D> World::get_world_3d() const {
	return world_3d;
}

void World::set_world_3d(const Ref<World3D> &p_world_3d) {
	if (world_3d == p_world_3d) {
		return;
	}

	if (is_inside_tree()) {
		_propagate_exit_world(this);
	}

	Ref<World3D> old_world = world_3d;

	if (own_world_3d.is_valid() && world_3d.is_valid()) {
		world_3d->disconnect(CoreStringNames::get_singleton()->changed, this, "_own_world_3d_changed");
	}

	world_3d = p_world_3d;

	if (own_world_3d.is_valid()) {
		if (world_3d.is_valid()) {
			own_world_3d = world_3d->duplicate();
			world_3d->connect(CoreStringNames::get_singleton()->changed, this, "_own_world_3d_changed");
		} else {
			own_world_3d = Ref<World3D>(memnew(World3D));
		}
	}

	if (is_inside_tree()) {
		_propagate_enter_world(this);
	}

	_on_set_world_3d(old_world);
}

bool World::is_using_own_world_3d() const {
	return own_world_3d.is_valid();
}

void World::set_use_own_world_3d(bool p_use_own_world_3d) {
	if (p_use_own_world_3d == own_world_3d.is_valid()) {
		return;
	}

	if (is_inside_tree()) {
		_propagate_exit_world(this);
	}

	if (p_use_own_world_3d) {
		if (world_3d.is_valid()) {
			own_world_3d = world_3d->duplicate();
			world_3d->connect(CoreStringNames::get_singleton()->changed, this, "_own_world_3d_changed");
		} else {
			own_world_3d = Ref<World3D>(memnew(World3D));
		}
	} else {
		own_world_3d = Ref<World3D>();
		if (world_3d.is_valid()) {
			world_3d->disconnect(CoreStringNames::get_singleton()->changed, this, "_own_world_3d_changed");
		}
	}

	if (is_inside_tree()) {
		_propagate_enter_world(this);
	}

	_on_set_use_own_world_3d(p_use_own_world_3d);
}

bool World::get_override_in_parent_viewport() {
	return _override_in_parent_viewport;
}
void World::set_override_in_parent_viewport(const bool value) {
	_override_in_parent_viewport = value;
}

Ref<World2D> World::find_world_2d() const {
	if (_override_world) {
		return _override_world->find_world_2d();
	}

	if (world_2d.is_valid()) {
		return world_2d;
	} else if (_parent_world) {
		return _parent_world->find_world_2d();
	} else {
		return Ref<World2D>();
	}
}

Ref<World3D> World::find_world_3d() const {
	if (_override_world) {
		return _override_world->find_world_3d();
	}

	if (own_world_3d.is_valid()) {
		return own_world_3d;
	} else if (world_3d.is_valid()) {
		return world_3d;
	} else if (_parent_world) {
		return _parent_world->find_world_3d();
	} else {
		return Ref<World3D>();
	}
}

World *World::get_override_world() {
	return _override_world;
}
World *World::get_override_world_or_this() {
	if (!_override_world) {
		return this;
	}

	return _override_world;
}
void World::set_override_world(World *p_world) {
	if (p_world == _override_world) {
		return;
	}

	World *old_world = _override_world;

	if (old_world) {
		old_world->_remove_overridden_world(this);
	}

	_override_world = p_world;

	if (_override_world) {
		_override_world->_add_overridden_world(this);
	}

	_on_world_override_changed(old_world);
}
void World::set_override_world_bind(Node *p_world) {
	World *w = Object::cast_to<World>(p_world);

	ERR_FAIL_COND(p_world && !w);

	set_override_world(w);
}

World::World() {
	world_2d = Ref<World2D>(memnew(World2D));
	_override_world = NULL;
	_override_in_parent_viewport = false;
}
World::~World() {
}

void World::_propagate_enter_world(Node *p_node) {
	if (p_node != this) {
		if (!p_node->is_inside_tree()) { //may not have entered scene yet
			return;
		}

		if (Object::cast_to<Spatial>(p_node) || Object::cast_to<WorldEnvironment3D>(p_node)) {
			p_node->notification(Spatial::NOTIFICATION_ENTER_WORLD);
		} else {
			World *v = Object::cast_to<World>(p_node);
			if (v) {
				if (v->world_3d.is_valid() || v->own_world_3d.is_valid()) {
					return;
				}
			}
		}
	}

	for (int i = 0; i < p_node->get_child_count(); i++) {
		_propagate_enter_world(p_node->get_child(i));
	}
}

void World::_propagate_exit_world(Node *p_node) {
	if (p_node != this) {
		if (!p_node->is_inside_tree()) { //may have exited scene already
			return;
		}

		if (Object::cast_to<Spatial>(p_node) || Object::cast_to<WorldEnvironment3D>(p_node)) {
			p_node->notification(Spatial::NOTIFICATION_EXIT_WORLD);
		} else {
			World *v = Object::cast_to<World>(p_node);
			if (v) {
				if (v->world_3d.is_valid() || v->own_world_3d.is_valid()) {
					return;
				}
			}
		}
	}

	for (int i = 0; i < p_node->get_child_count(); i++) {
		_propagate_exit_world(p_node->get_child(i));
	}
}

void World::_add_overridden_world(World *p_world) {
	ERR_FAIL_COND(!p_world);

	_overriding_worlds.push_back(p_world);
}
void World::_remove_overridden_world(World *p_world) {
	ERR_FAIL_COND(!p_world);

	for (int i = 0; i < _overriding_worlds.size(); ++i) {
		World *w = _overriding_worlds[i];

		if (w == p_world) {
			_overriding_worlds.remove(i);
			return;
		}
	}

	ERR_FAIL();
}

void World::_own_world_3d_changed() {
	ERR_FAIL_COND(world_3d.is_null());
	ERR_FAIL_COND(own_world_3d.is_null());

	if (is_inside_tree()) {
		_propagate_exit_world(this);
	}

	own_world_3d = world_3d->duplicate();

	if (is_inside_tree()) {
		_propagate_enter_world(this);
	}
}

void World::_on_set_use_own_world_3d(bool p_use_own_world_3d) {
}

void World::_on_set_world_3d(const Ref<World3D> &p_old_world) {
}
void World::_on_set_world_2d(const Ref<World2D> &p_old_world_2d) {
}

void World::_on_world_override_changed(World *p_old_world) {
}

void World::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (get_parent()) {
				_parent_world = get_parent()->get_world();
			} else {
				_parent_world = nullptr;
			}
		} break;
		case NOTIFICATION_READY: {
			if (_override_in_parent_viewport) {
				if (get_parent()) {
					World *w = get_parent()->get_viewport();

					if (w) {
						w->set_override_world(this);
					}
				}
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			for (int i = 0; i < _overriding_worlds.size(); ++i) {
				World *w = _overriding_worlds[i];

				w->set_override_world(NULL);
			}

			_override_world = NULL;

		} break;
	}
}

void World::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_world_2d"), &World::get_world_2d);
	ClassDB::bind_method(D_METHOD("set_world_2d", "world_2d"), &World::set_world_2d);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world_2d", PROPERTY_HINT_RESOURCE_TYPE, "World2D", 0), "set_world_2d", "get_world_2d");

	ClassDB::bind_method(D_METHOD("get_world_3d"), &World::get_world_3d);
	ClassDB::bind_method(D_METHOD("set_world_3d", "world"), &World::set_world_3d);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world", PROPERTY_HINT_RESOURCE_TYPE, "World3D"), "set_world_3d", "get_world_3d");

	ClassDB::bind_method(D_METHOD("is_using_own_world_3d"), &World::is_using_own_world_3d);
	ClassDB::bind_method(D_METHOD("set_use_own_world_3d", "enable"), &World::set_use_own_world_3d);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "own_world_3d"), "set_use_own_world_3d", "is_using_own_world_3d");

	ClassDB::bind_method(D_METHOD("get_override_in_parent_viewport"), &World::get_override_in_parent_viewport);
	ClassDB::bind_method(D_METHOD("set_override_in_parent_viewport", "enable"), &World::set_override_in_parent_viewport);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "override_in_parent_viewport"), "set_override_in_parent_viewport", "get_override_in_parent_viewport");

	ClassDB::bind_method(D_METHOD("find_world_2d"), &World::find_world_2d);
	ClassDB::bind_method(D_METHOD("find_world_3d"), &World::find_world_3d);

	ClassDB::bind_method(D_METHOD("get_override_world"), &World::get_override_world);
	ClassDB::bind_method(D_METHOD("get_override_world_or_this"), &World::get_override_world_or_this);
	ClassDB::bind_method(D_METHOD("set_override_world", "world"), &World::set_override_world_bind);

	ClassDB::bind_method(D_METHOD("_own_world_3d_changed"), &World::_own_world_3d_changed);
}
