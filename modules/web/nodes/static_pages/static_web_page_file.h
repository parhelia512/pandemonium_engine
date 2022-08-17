#ifndef STATIC_WEB_PAGE_FILE_H
#define STATIC_WEB_PAGE_FILE_H

#include "core/string/ustring.h"

#include "static_web_page.h"

class StaticWebPageFile : public StaticWebPage {
	GDCLASS(StaticWebPageFile, StaticWebPage);

public:
	String get_file_path();
	void set_file_path(const String &val);

	bool get_process_if_can();
	void set_process_if_can(const bool &val);

	void load();

	StaticWebPageFile();
	~StaticWebPageFile();

protected:
	void _notification(const int what);
	static void _bind_methods();

	String _file_path;
	bool _process_if_can;
};

#endif
