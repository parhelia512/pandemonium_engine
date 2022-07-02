#ifndef STATIC_PAGE_FILE_H
#define STATIC_PAGE_FILE_H

#include "core/ustring.h"

#include "static_page.h"

class StaticPageFile : public StaticPage {
	GDCLASS(StaticPageFile, StaticPage);

public:
	String get_file_path();
	void set_file_path(const String &val);

	bool get_process_if_can();
	void set_process_if_can(const bool &val);

	void load();

	StaticPageFile();
	~StaticPageFile();

protected:
	void _notification(const int what);
	static void _bind_methods();

	String _file_path;
	bool _process_if_can;
};

#endif
