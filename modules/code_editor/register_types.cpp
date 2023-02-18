
#include "register_types.h"

#include "script_editor_plugin.h"

#include "script_text_editor.h"
#include "text_editor.h"

void register_code_editor_types(ModuleRegistrationLevel p_level) {
	if (p_level == MODULE_REGISTRATION_LEVEL_SCENE) {
		//ClassDB::register_class<>();
		ClassDB::register_virtual_class<ScriptEditor>();
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_REGISTRATION_LEVEL_EDITOR) {
		EditorPlugins::add_by_type<ScriptEditorPlugin>();
	}
#endif
}

void unregister_code_editor_types(ModuleRegistrationLevel p_level) {
}
