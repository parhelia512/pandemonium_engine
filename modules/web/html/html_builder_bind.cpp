#include "html_builder_bind.h"
#include "core/print_string.h"
#include "core/ustring.h"

//#include "web/http/request.h"

Ref<_HTMLTag> _HTMLTag::str(const String &str) {
	result += " " + str;

	return this;
}

Ref<_HTMLTag> _HTMLTag::style(const String &val) {
	attrib("style", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::href(const String &val) {
	attrib("href", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::cls(const String &val) {
	attrib("class", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::clsse(const String &val) {
	if (val == "") {
		return this;
	}

	attrib("class", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::id(const String &val) {
	attrib("id", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::name(const String &val) {
	attrib("name", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::content(const String &val) {
	attrib("content", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::value(const String &val) {
	attrib("value", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::accept(const String &val) {
	attrib("accept", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::src(const String &val) {
	attrib("src", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::alt(const String &val) {
	attrib("alt", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::autocomplete(const String &val) {
	attrib("autocomplete", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::autocomplete_off() {
	attrib("autocomplete", "off");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_on() {
	attrib("autocomplete", "on");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_name() {
	attrib("autocomplete", "name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_name_honorific_prefix() {
	attrib("autocomplete", "honorific-prefix");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_name_given_name() {
	attrib("autocomplete", "given-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_name_additional_name() {
	attrib("autocomplete", "additional-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_name_family_name() {
	attrib("autocomplete", "family-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_name_honorific_suffix() {
	attrib("autocomplete", "honorific-suffix");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_name_nickname() {
	attrib("autocomplete", "nickname");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_email() {
	attrib("autocomplete", "email");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_username() {
	attrib("autocomplete", "username");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_new_password() {
	attrib("autocomplete", "new-password");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_current_password() {
	attrib("autocomplete", "current-password");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_one_time_code() {
	attrib("autocomplete", "one-time-code");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_organization_title() {
	attrib("autocomplete", "organization-title");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_organization() {
	attrib("autocomplete", "organization");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_street_address() {
	attrib("autocomplete", "street-address");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_address_line1() {
	attrib("autocomplete", "address-line1");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_address_line2() {
	attrib("autocomplete", "address-line2");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_address_line3() {
	attrib("autocomplete", "address-line3");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_address_level_1() {
	attrib("autocomplete", "address-level1");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_address_level_2() {
	attrib("autocomplete", "address-level2");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_address_level_3() {
	attrib("autocomplete", "address-level3");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_address_level_4() {
	attrib("autocomplete", "address-level4");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_country() {
	attrib("autocomplete", "country");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_country_name() {
	attrib("autocomplete", "country-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_postal_code() {
	attrib("autocomplete", "postal-code");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_name() {
	attrib("autocomplete", "cc-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_given_name() {
	attrib("autocomplete", "cc-given-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_additional_name() {
	attrib("autocomplete", "cc-additional-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_family_name() {
	attrib("autocomplete", "cc-family-name");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_number() {
	attrib("autocomplete", "cc-number");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_exp() {
	attrib("autocomplete", "cc-exp");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_exp_month() {
	attrib("autocomplete", "cc-exp-month");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_exp_year() {
	attrib("autocomplete", "cc-exp-year");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_csc() {
	attrib("autocomplete", "cc-csc");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_cc_type() {
	attrib("autocomplete", "cc-type");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_transaction_currency() {
	attrib("autocomplete", "transaction-currency");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_transaction_amount() {
	attrib("autocomplete", "transaction-amount");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_language() {
	attrib("autocomplete", "language");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_bday() {
	attrib("autocomplete", "bday");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_bday_day() {
	attrib("autocomplete", "bday-day");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_bday_month() {
	attrib("autocomplete", "bday-month");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_bday_year() {
	attrib("autocomplete", "bday-year");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_sex() {
	attrib("autocomplete", "sex");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_tel() {
	attrib("autocomplete", "tel");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_tel_country_code() {
	attrib("autocomplete", "tel-country-code");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_tel_national() {
	attrib("autocomplete", "tel-national");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_tel_area_code() {
	attrib("autocomplete", "tel-area-code");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_tel_local() {
	attrib("autocomplete", "tel-local");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_tel_extension() {
	attrib("autocomplete", "tel-extension");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_impp() {
	attrib("autocomplete", "impp");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_url() {
	attrib("autocomplete", "url");

	return this;
}
Ref<_HTMLTag> _HTMLTag::autocomplete_photo() {
	attrib("autocomplete", "photo");

	return this;
}

Ref<_HTMLTag> _HTMLTag::onclick(const String &val) {
	attrib("onclick", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::inputmode(const String &val) {
	attrib("inputmode", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::list(const String &val) {
	attrib("list", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::checked(const bool val) {
	if (val) {
		result += " checked";
	}

	return this;
}

Ref<_HTMLTag> _HTMLTag::selected(const bool val) {
	if (val) {
		result += " selected";
	}

	return this;
}

Ref<_HTMLTag> _HTMLTag::autofocus(const bool val) {
	if (val) {
		result += " autofocus";
	}

	return this;
}

Ref<_HTMLTag> _HTMLTag::disabled(const bool val) {
	if (val) {
		result += " disabled";
	}

	return this;
}

Ref<_HTMLTag> _HTMLTag::multiple(const bool val) {
	if (val) {
		result += " multiple";
	}

	return this;
}

Ref<_HTMLTag> _HTMLTag::required(const bool val) {
	if (val) {
		result += " required";
	}

	return this;
}

Ref<_HTMLTag> _HTMLTag::spellcheck(const bool val) {
	if (val) {
		attrib("spellcheck", "true");
	} else {
		attrib("spellcheck", "false");
	}

	return this;
}

Ref<_HTMLTag> _HTMLTag::max(const String &val) {
	attrib("max", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::min(const String &val) {
	attrib("min", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::step(const String &val) {
	attrib("step", val);

	return this;
}
Ref<_HTMLTag> _HTMLTag::step_any() {
	attrib("step", "any");

	return this;
}

Ref<_HTMLTag> _HTMLTag::minlength(const int val) {
	attrib("minlength", String::num(val));

	return this;
}
Ref<_HTMLTag> _HTMLTag::minlength(const String &val) {
	attrib("minlength", val);

	return this;
}
Ref<_HTMLTag> _HTMLTag::maxlength(const int val) {
	attrib("maxlength", String::num(val));

	return this;
}
Ref<_HTMLTag> _HTMLTag::maxlength(const String &val) {
	attrib("maxlength", val);

	return this;
}
Ref<_HTMLTag> _HTMLTag::size(const int val) {
	attrib("size", String::num(val));

	return this;
}
Ref<_HTMLTag> _HTMLTag::size(const String &val) {
	attrib("size", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::width(const int val) {
	attrib("width", String::num(val));

	return this;
}

Ref<_HTMLTag> _HTMLTag::width(const String &val) {
	attrib("width", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::height(const int val) {
	attrib("height", String::num(val));

	return this;
}

Ref<_HTMLTag> _HTMLTag::height(const String &val) {
	attrib("height", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::pattern(const String &val) {
	attrib("pattern", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::method(const String &val) {
	attrib("method", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::method_get() {
	attrib("method", "get");

	return this;
}
Ref<_HTMLTag> _HTMLTag::method_post() {
	attrib("method", "post");

	return this;
}

Ref<_HTMLTag> _HTMLTag::action(const String &val) {
	attrib("action", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::type(const String &val) {
	attrib("type", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::placeholder(const String &val) {
	attrib("placeholder", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::fora(const String &val) {
	attrib("for", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel(const String &val) {
	attrib("rel", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_stylesheet() {
	attrib("rel", "stylesheet");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_alternate() {
	attrib("rel", "alternate");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_author() {
	attrib("rel", "author");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_bookmark() {
	attrib("rel", "bookmark");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_external() {
	attrib("rel", "external");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_help() {
	attrib("rel", "help");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_next() {
	attrib("rel", "next");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_nofollow() {
	attrib("rel", "nofollow");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_noopener() {
	attrib("rel", "noopener");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_noreferrer() {
	attrib("rel", "noreferrer");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_prev() {
	attrib("rel", "prev");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_search() {
	attrib("rel", "search");

	return this;
}

Ref<_HTMLTag> _HTMLTag::rel_tag() {
	attrib("rel", "tag");

	return this;
}

Ref<_HTMLTag> _HTMLTag::charset(const String &val) {
	attrib("charset", val);

	return this;
}

Ref<_HTMLTag> _HTMLTag::charset_utf_8() {
	attrib("charset", "utf-8");

	return this;
}

Ref<_HTMLTag> _HTMLTag::itbutton() {
	attrib("type", "button");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itcheckbox() {
	attrib("type", "checkbox");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itcolor() {
	attrib("type", "color");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itdate() {
	attrib("type", "date");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itdatetime_local() {
	attrib("type", "datetime_local");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itemail() {
	attrib("type", "email");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itfile() {
	attrib("type", "file");

	return this;
}
Ref<_HTMLTag> _HTMLTag::ithidden() {
	attrib("type", "hidden");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itimage() {
	attrib("type", "image");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itmonth() {
	attrib("type", "month");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itnumber() {
	attrib("type", "number");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itpassword() {
	attrib("type", "password");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itradio() {
	attrib("type", "radio");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itrange() {
	attrib("type", "range");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itreset() {
	attrib("type", "reset");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itsearch() {
	attrib("type", "search");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itsubmit() {
	attrib("type", "submit");

	return this;
}
Ref<_HTMLTag> _HTMLTag::ittel() {
	attrib("type", "tel");

	return this;
}
Ref<_HTMLTag> _HTMLTag::ittext() {
	attrib("type", "text");

	return this;
}
Ref<_HTMLTag> _HTMLTag::ittime() {
	attrib("type", "time");

	return this;
}
Ref<_HTMLTag> _HTMLTag::iturl() {
	attrib("type", "url");

	return this;
}
Ref<_HTMLTag> _HTMLTag::itweek() {
	attrib("type", "week");

	return this;
}

Ref<_HTMLTag> _HTMLTag::inputmode_none() {
	attrib("inputmode", "none");

	return this;
}
Ref<_HTMLTag> _HTMLTag::inputmode_text() {
	attrib("inputmode", "text");

	return this;
}
Ref<_HTMLTag> _HTMLTag::inputmode_decimal() {
	attrib("inputmode", "decimal");

	return this;
}
Ref<_HTMLTag> _HTMLTag::inputmode_numeric() {
	attrib("inputmode", "numeric");

	return this;
}
Ref<_HTMLTag> _HTMLTag::inputmode_tel() {
	attrib("inputmode", "tel");

	return this;
}
Ref<_HTMLTag> _HTMLTag::inputmode_search() {
	attrib("inputmode", "search");

	return this;
}
Ref<_HTMLTag> _HTMLTag::inputmode_email() {
	attrib("inputmode", "email");

	return this;
}
Ref<_HTMLTag> _HTMLTag::inputmode_url() {
	attrib("inputmode", "url");

	return this;
}

Ref<_HTMLTag> _HTMLTag::attrib(const String &attr, const String &val) {
	result += " " + attr + "=\"" + val + "\"";

	return this;
}

Ref<_HTMLTag> _HTMLTag::start(const String &p_tag, const bool p_simple) {
	simple = p_simple;

	result = "<" + p_tag;

	return this;
}

Ref<_HTMLTag> _HTMLTag::reset() {
	result.clear();

	return this;
}

Ref<_HTMLTag> _HTMLTag::close() {
	if (simple) {
		result += "/>";
	} else {
		result += ">";
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLTag::f() {
	return owner;
}

bool _HTMLTag::has_data() {
	return result.size() > 0;
}

_HTMLTag::_HTMLTag() {
	simple = true;
}

void _HTMLTag::_bind_methods() {
	ClassDB::bind_method(D_METHOD("style", "val"), &_HTMLTag::style);
}

Ref<_HTMLBuilder> _HTMLBuilder::comment(const String &val) {
	write_tag();

	result += "<!--" + val + "-->";

	return this;
}

Ref<_HTMLTag> _HTMLBuilder::doctype() {
	write_tag();

	return tag->start("!DOCTYPE");
}

Ref<_HTMLBuilder> _HTMLBuilder::doctype(const String &val) {
	write_tag();

	result += "<!DOCTYPE " + val + ">";

	return this;
}

Ref<_HTMLTag> _HTMLBuilder::a() {
	write_tag();

	return tag->start("a");
}
Ref<_HTMLTag> _HTMLBuilder::abbr() {
	write_tag();

	return tag->start("abbr");
}

Ref<_HTMLTag> _HTMLBuilder::acronym() { // Not supported in HTML5. Use <abbr> instead. Defines an acronym
	write_tag();

	return tag->start("acronym");
}

Ref<_HTMLTag> _HTMLBuilder::address() {
	write_tag();

	return tag->start("address");
}

Ref<_HTMLTag> _HTMLBuilder::applet() { // Not supported in HTML5. Use <embed> or <object> instead. Defines an embedded applet
	write_tag();

	return tag->start("applet");
}

Ref<_HTMLTag> _HTMLBuilder::area() {
	write_tag();

	return tag->start("area");
}

Ref<_HTMLTag> _HTMLBuilder::article() {
	write_tag();

	return tag->start("article");
}

Ref<_HTMLTag> _HTMLBuilder::aside() {
	write_tag();

	return tag->start("aside");
}

Ref<_HTMLTag> _HTMLBuilder::audio() {
	write_tag();

	return tag->start("audio");
}

Ref<_HTMLTag> _HTMLBuilder::b() {
	write_tag();

	return tag->start("b");
}

Ref<_HTMLTag> _HTMLBuilder::basefont() { // Not supported in HTML5. Use CSS instead. Specifies a default color, size, and font for all text in a document
	write_tag();

	return tag->start("basefont");
}

Ref<_HTMLTag> _HTMLBuilder::bdi() {
	write_tag();

	return tag->start("bdi");
}

Ref<_HTMLTag> _HTMLBuilder::bdo() {
	write_tag();

	return tag->start("bdo");
}

Ref<_HTMLTag> _HTMLBuilder::big() { // Not supported in HTML5. Use CSS instead. Defines big text
	write_tag();

	return tag->start("big");
}

Ref<_HTMLTag> _HTMLBuilder::blockquote() {
	write_tag();

	return tag->start("blockquote");
}

Ref<_HTMLTag> _HTMLBuilder::body() {
	write_tag();

	return tag->start("body");
}

Ref<_HTMLTag> _HTMLBuilder::br() {
	write_tag();

	return tag->start("br", true);
}

Ref<_HTMLTag> _HTMLBuilder::button() {
	write_tag();

	return tag->start("button");
}

Ref<_HTMLTag> _HTMLBuilder::canvas() {
	write_tag();

	return tag->start("canvas");
}

Ref<_HTMLTag> _HTMLBuilder::caption() {
	write_tag();

	return tag->start("caption");
}

Ref<_HTMLTag> _HTMLBuilder::center() { // Not supported in HTML5. Use CSS instead. Defines centered text
	write_tag();

	return tag->start("center");
}

Ref<_HTMLTag> _HTMLBuilder::cite() {
	write_tag();

	return tag->start("cite");
}

Ref<_HTMLTag> _HTMLBuilder::code() {
	write_tag();

	return tag->start("code");
}

Ref<_HTMLTag> _HTMLBuilder::col() {
	write_tag();

	return tag->start("col");
}

Ref<_HTMLTag> _HTMLBuilder::colgroup() {
	write_tag();

	return tag->start("colgroup");
}

Ref<_HTMLTag> _HTMLBuilder::data() {
	write_tag();

	return tag->start("cite");
}

Ref<_HTMLTag> _HTMLBuilder::datalist() {
	write_tag();

	return tag->start("datalist");
}

Ref<_HTMLTag> _HTMLBuilder::dd() {
	write_tag();

	return tag->start("dd");
}

Ref<_HTMLTag> _HTMLBuilder::del() {
	write_tag();

	return tag->start("del");
}

Ref<_HTMLTag> _HTMLBuilder::details() {
	write_tag();

	return tag->start("details");
}

Ref<_HTMLTag> _HTMLBuilder::dfn() {
	write_tag();

	return tag->start("dfn");
}

Ref<_HTMLTag> _HTMLBuilder::dialog() {
	write_tag();

	return tag->start("dialog");
}

Ref<_HTMLTag> _HTMLBuilder::dir() { // Not supported in HTML5. Use <ul> instead.
	write_tag();

	return tag->start("dir");
}

Ref<_HTMLTag> _HTMLBuilder::div() {
	write_tag();

	return tag->start("div");
}

Ref<_HTMLTag> _HTMLBuilder::dl() {
	write_tag();

	return tag->start("dl");
}

Ref<_HTMLTag> _HTMLBuilder::dt() {
	write_tag();

	return tag->start("dt");
}

Ref<_HTMLTag> _HTMLBuilder::em() {
	write_tag();

	return tag->start("em");
}

Ref<_HTMLTag> _HTMLBuilder::embed() {
	write_tag();

	return tag->start("embed");
}

Ref<_HTMLTag> _HTMLBuilder::fieldset() {
	write_tag();

	return tag->start("fieldset");
}
Ref<_HTMLTag> _HTMLBuilder::figcaption() {
	write_tag();

	return tag->start("figcaption");
}

Ref<_HTMLTag> _HTMLBuilder::figure() {
	write_tag();

	return tag->start("figure");
}

Ref<_HTMLTag> _HTMLBuilder::font() { // Not supported in HTML5.
	write_tag();

	return tag->start("font");
}

Ref<_HTMLTag> _HTMLBuilder::footer() {
	write_tag();

	return tag->start("footer");
}

Ref<_HTMLTag> _HTMLBuilder::form() {
	write_tag();

	return tag->start("form");
}

Ref<_HTMLTag> _HTMLBuilder::frame() { // Not supported in HTML5.
	write_tag();

	return tag->start("frame");
}

Ref<_HTMLTag> _HTMLBuilder::frameset() { // Not supported in HTML5.
	write_tag();

	return tag->start("frameset");
}

Ref<_HTMLTag> _HTMLBuilder::h1() {
	write_tag();

	return tag->start("h1");
}

Ref<_HTMLTag> _HTMLBuilder::h2() {
	write_tag();

	return tag->start("h2");
}

Ref<_HTMLTag> _HTMLBuilder::h3() {
	write_tag();

	return tag->start("h3");
}

Ref<_HTMLTag> _HTMLBuilder::h4() {
	write_tag();

	return tag->start("h4");
}

Ref<_HTMLTag> _HTMLBuilder::h5() {
	write_tag();

	return tag->start("h5");
}

Ref<_HTMLTag> _HTMLBuilder::h6() {
	write_tag();

	return tag->start("h6");
}

Ref<_HTMLTag> _HTMLBuilder::head() {
	write_tag();

	return tag->start("head");
}

Ref<_HTMLTag> _HTMLBuilder::header() {
	write_tag();

	return tag->start("header");
}

Ref<_HTMLTag> _HTMLBuilder::hr() {
	write_tag();

	return tag->start("hr");
}

Ref<_HTMLTag> _HTMLBuilder::html() {
	write_tag();

	return tag->start("html");
}

Ref<_HTMLTag> _HTMLBuilder::i() {
	write_tag();

	return tag->start("i");
}

Ref<_HTMLTag> _HTMLBuilder::iframe() {
	write_tag();

	return tag->start("iframe");
}

Ref<_HTMLTag> _HTMLBuilder::img() {
	write_tag();

	return tag->start("img");
}

Ref<_HTMLTag> _HTMLBuilder::input() {
	write_tag();

	return tag->start("input");
}

Ref<_HTMLTag> _HTMLBuilder::ins() {
	write_tag();

	return tag->start("ins");
}

Ref<_HTMLTag> _HTMLBuilder::kbd() {
	write_tag();

	return tag->start("kbd");
}

Ref<_HTMLTag> _HTMLBuilder::label() {
	write_tag();

	return tag->start("label");
}

Ref<_HTMLTag> _HTMLBuilder::legend() {
	write_tag();

	return tag->start("legend");
}

Ref<_HTMLTag> _HTMLBuilder::li() {
	write_tag();

	return tag->start("li");
}

Ref<_HTMLTag> _HTMLBuilder::link() {
	write_tag();

	return tag->start("link");
}

Ref<_HTMLTag> _HTMLBuilder::main() {
	write_tag();

	return tag->start("main");
}

Ref<_HTMLTag> _HTMLBuilder::map() {
	write_tag();

	return tag->start("map");
}
Ref<_HTMLTag> _HTMLBuilder::mark() {
	write_tag();

	return tag->start("mark");
}

Ref<_HTMLTag> _HTMLBuilder::meta() {
	write_tag();

	return tag->start("meta");
}

Ref<_HTMLTag> _HTMLBuilder::meter() {
	write_tag();

	return tag->start("meter");
}

Ref<_HTMLTag> _HTMLBuilder::nav() {
	write_tag();

	return tag->start("nav");
}

Ref<_HTMLTag> _HTMLBuilder::noframes() { // Not supported in HTML5.
	write_tag();

	return tag->start("noframes");
}

Ref<_HTMLTag> _HTMLBuilder::noscript() {
	write_tag();

	return tag->start("noscript");
}

Ref<_HTMLTag> _HTMLBuilder::object() {
	write_tag();

	return tag->start("object");
}

Ref<_HTMLTag> _HTMLBuilder::ol() {
	write_tag();

	return tag->start("ol");
}

Ref<_HTMLTag> _HTMLBuilder::optgroup() {
	write_tag();

	return tag->start("optgroup");
}

Ref<_HTMLTag> _HTMLBuilder::option() {
	write_tag();

	return tag->start("option");
}

Ref<_HTMLTag> _HTMLBuilder::output() {
	write_tag();

	return tag->start("output");
}

Ref<_HTMLTag> _HTMLBuilder::p() {
	write_tag();

	return tag->start("p");
}

Ref<_HTMLTag> _HTMLBuilder::param() {
	write_tag();

	return tag->start("param");
}

Ref<_HTMLTag> _HTMLBuilder::picture() {
	write_tag();

	return tag->start("picture");
}

Ref<_HTMLTag> _HTMLBuilder::pre() {
	write_tag();

	return tag->start("pre");
}

Ref<_HTMLTag> _HTMLBuilder::progress() {
	write_tag();

	return tag->start("progress");
}

Ref<_HTMLTag> _HTMLBuilder::q() {
	write_tag();

	return tag->start("q");
}

Ref<_HTMLTag> _HTMLBuilder::rp() {
	write_tag();

	return tag->start("rp");
}

Ref<_HTMLTag> _HTMLBuilder::rt() {
	write_tag();

	return tag->start("rt");
}

Ref<_HTMLTag> _HTMLBuilder::ruby() {
	write_tag();

	return tag->start("ruby");
}

Ref<_HTMLTag> _HTMLBuilder::s() {
	write_tag();

	return tag->start("s");
}

Ref<_HTMLTag> _HTMLBuilder::samp() {
	write_tag();

	return tag->start("samp");
}

Ref<_HTMLTag> _HTMLBuilder::script() {
	write_tag();

	return tag->start("script");
}

Ref<_HTMLTag> _HTMLBuilder::section() {
	write_tag();

	return tag->start("section");
}

Ref<_HTMLTag> _HTMLBuilder::select() {
	write_tag();

	return tag->start("select");
}

Ref<_HTMLTag> _HTMLBuilder::small() {
	write_tag();

	return tag->start("small");
}

Ref<_HTMLTag> _HTMLBuilder::source() {
	write_tag();

	return tag->start("source");
}

Ref<_HTMLTag> _HTMLBuilder::span() {
	write_tag();

	return tag->start("span");
}

Ref<_HTMLTag> _HTMLBuilder::strike() { // Not supported in HTML5
	write_tag();

	return tag->start("strike");
}

Ref<_HTMLTag> _HTMLBuilder::strong() {
	write_tag();

	return tag->start("strong");
}

Ref<_HTMLTag> _HTMLBuilder::style() {
	write_tag();

	return tag->start("style");
}

Ref<_HTMLTag> _HTMLBuilder::sub() {
	write_tag();

	return tag->start("sub");
}

Ref<_HTMLTag> _HTMLBuilder::summary() {
	write_tag();

	return tag->start("summary");
}

Ref<_HTMLTag> _HTMLBuilder::sup() {
	write_tag();

	return tag->start("sup");
}

Ref<_HTMLTag> _HTMLBuilder::svg() {
	write_tag();

	return tag->start("svg");
}

Ref<_HTMLTag> _HTMLBuilder::table() {
	write_tag();

	return tag->start("table");
}

Ref<_HTMLTag> _HTMLBuilder::tbody() {
	write_tag();

	return tag->start("tbody");
}

Ref<_HTMLTag> _HTMLBuilder::td() {
	write_tag();

	return tag->start("td");
}

Ref<_HTMLTag> _HTMLBuilder::templateh() {
	write_tag();

	return tag->start("template");
}

Ref<_HTMLTag> _HTMLBuilder::textarea() {
	write_tag();

	return tag->start("textarea");
}

Ref<_HTMLTag> _HTMLBuilder::tfoot() {
	write_tag();

	return tag->start("tfoot");
}

Ref<_HTMLTag> _HTMLBuilder::th() {
	write_tag();

	return tag->start("th");
}

Ref<_HTMLTag> _HTMLBuilder::thead() {
	write_tag();

	return tag->start("thead");
}

Ref<_HTMLTag> _HTMLBuilder::time() {
	write_tag();

	return tag->start("time");
}

Ref<_HTMLTag> _HTMLBuilder::title() {
	write_tag();

	return tag->start("title");
}

Ref<_HTMLTag> _HTMLBuilder::tr() {
	write_tag();

	return tag->start("tr");
}

Ref<_HTMLTag> _HTMLBuilder::track() {
	write_tag();

	return tag->start("track");
}

Ref<_HTMLTag> _HTMLBuilder::tt() { // Not supported in HTML5.
	write_tag();

	return tag->start("tt");
}

Ref<_HTMLTag> _HTMLBuilder::u() {
	write_tag();

	return tag->start("u");
}

Ref<_HTMLTag> _HTMLBuilder::ul() {
	write_tag();

	return tag->start("ul");
}

Ref<_HTMLTag> _HTMLBuilder::var() {
	write_tag();

	return tag->start("var");
}

Ref<_HTMLTag> _HTMLBuilder::video() {
	write_tag();

	return tag->start("video");
}

Ref<_HTMLTag> _HTMLBuilder::wbr() {
	write_tag();

	return tag->start("wbr");
}

//Ref<_HTMLBuilder> _HTMLBuilder::a(const String &href, const String &cls, const String &id) {
/*
  Ref<_HTMLTag> t = a();

  t->href(href);

  if (cls != "") {
	  t->cls(cls);
  }

  if (id != "") {
	  t->id(id);
  }
*/
//	return this;
//}

Ref<_HTMLBuilder> _HTMLBuilder::fa(const String &href, const String &body, const String &cls, const String &id) {
	//a(href, cls, id);
	w(body);
	ca();

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::div(const String &cls, const String &id) {
	Ref<_HTMLTag> t = div();

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::fdiv(const String &body, const String &cls, const String &id) {
	div(cls, id);
	w(body);
	cdiv();

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::textarea(const String &name, const String &cls, const String &id) {
	Ref<_HTMLTag> t = textarea();

	t->name(name);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}
Ref<_HTMLBuilder> _HTMLBuilder::ftextarea(const String &name, const String &body, const String &cls, const String &id) {
	textarea(name, cls, id);
	w(body);
	ctextarea();

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::select(const String &name, const String &cls, const String &id) {
	Ref<_HTMLTag> t = select();

	t->name(name);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLTag> _HTMLBuilder::option(const String &value) {
	Ref<_HTMLTag> t = option();

	t->value(value);

	return t;
}
Ref<_HTMLBuilder> _HTMLBuilder::foption(const String &value, const String &body, const bool selected) {
	option(value)->selected(selected);
	w(body);
	coption();

	return this;
}

// Closing tags

Ref<_HTMLBuilder> _HTMLBuilder::ca() {
	write_tag();
	result += "</a>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cabbr() {
	write_tag();
	result += "</abbr>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cacronym() {
	write_tag();
	result += "</acronym>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::caddress() {
	write_tag();
	result += "</address>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::capplet() {
	write_tag();
	result += "</applet>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::carea() {
	write_tag();
	result += "</area>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::carticle() {
	write_tag();
	result += "</article>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::caside() {
	write_tag();
	result += "</aside>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::caudio() {
	write_tag();
	result += "</audio>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cb() {
	write_tag();
	result += "</b>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cbasefont() {
	write_tag();
	result += "</basefont>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cbdi() {
	write_tag();
	result += "</bdi>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cbdo() {
	write_tag();
	result += "</bdo>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cbig() {
	write_tag();
	result += "</big>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cblockquote() {
	write_tag();
	result += "</blockquote>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cbody() {
	write_tag();
	result += "</body>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cbutton() {
	write_tag();
	result += "</button>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ccanvas() {
	write_tag();
	result += "</canvas>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ccaption() {
	write_tag();
	result += "</caption>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ccenter() {
	write_tag();
	result += "</center>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ccite() {
	write_tag();
	result += "</cite>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ccode() {
	write_tag();
	result += "</code>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ccol() {
	write_tag();
	result += "</col>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ccolgroup() {
	write_tag();
	result += "</colgroup>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdata() {
	write_tag();
	result += "</data>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdatalist() {
	write_tag();
	result += "</datalist>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdd() {
	write_tag();
	result += "</dd>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdel() {
	write_tag();
	result += "</del>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdetails() {
	write_tag();
	result += "</details>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdfn() {
	write_tag();
	result += "</dfn>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdialog() {
	write_tag();
	result += "</dialog>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdir() {
	write_tag();
	result += "</dir>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdiv() {
	write_tag();
	result += "</div>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdl() {
	write_tag();
	result += "</dl>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cdt() {
	write_tag();
	result += "</dt>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cem() {
	write_tag();
	result += "</em>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cembed() {
	write_tag();
	result += "</embed>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cfieldset() {
	write_tag();
	result += "</fieldset>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cfigcaption() {
	write_tag();
	result += "</figcaption>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cfigure() {
	write_tag();
	result += "</figure>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cfont() {
	write_tag();
	result += "</font>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cfooter() {
	write_tag();
	result += "</footer>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cform() {
	write_tag();
	result += "</form>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cframe() {
	write_tag();
	result += "</frame>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cframeset() {
	write_tag();
	result += "</frameset>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ch1() {
	write_tag();
	result += "</h1>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ch2() {
	write_tag();
	result += "</h2>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ch3() {
	write_tag();
	result += "</h3>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ch4() {
	write_tag();
	result += "</h4>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ch5() {
	write_tag();
	result += "</h5>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ch6() {
	write_tag();
	result += "</h6>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::chead() {
	write_tag();
	result += "</head>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cheader() {
	write_tag();
	result += "</header>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::chr() {
	write_tag();
	result += "</hr>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::chtml() {
	write_tag();
	result += "</html>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ci() {
	write_tag();
	result += "</i>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ciframe() {
	write_tag();
	result += "</iframe>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cimg() {
	write_tag();
	result += "</img>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cinput() {
	write_tag();
	result += "</input>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cins() {
	write_tag();
	result += "</ins>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ckbd() {
	write_tag();
	result += "</kbd>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::clabel() {
	write_tag();
	result += "</label>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::clegend() {
	write_tag();
	result += "</legend>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cli() {
	write_tag();
	result += "</li>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::clink() {
	write_tag();
	result += "</link>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cmain() {
	write_tag();
	result += "</main>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cmap() {
	write_tag();
	result += "</map>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cmark() {
	write_tag();
	result += "</mark>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cmeta() {
	write_tag();
	result += "</meta>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cmeter() {
	write_tag();
	result += "</meter>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cnav() {
	write_tag();
	result += "</nav>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cnoframes() {
	write_tag();
	result += "</noframes>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cnoscript() {
	write_tag();
	result += "</noscript>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cobject() {
	write_tag();
	result += "</object>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::c_ol() {
	write_tag();
	result += "</ol>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::coptgroup() {
	write_tag();
	result += "</optgroup>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::coption() {
	write_tag();
	result += "</option>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::coutput() {
	write_tag();
	result += "</output>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cp() {
	write_tag();
	result += "</p>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cparam() {
	write_tag();
	result += "</param>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cpicture() {
	write_tag();
	result += "</picture>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cpre() {
	write_tag();
	result += "</pre>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cprogress() {
	write_tag();
	result += "</progress>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cq() {
	write_tag();
	result += "</a>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::crp() {
	write_tag();
	result += "</rp>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::crt() {
	write_tag();
	result += "</rt>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cruby() {
	write_tag();
	result += "</ruby>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cs() {
	write_tag();
	result += "</s>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csamp() {
	write_tag();
	result += "</samp>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cscript() {
	write_tag();
	result += "</script>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csection() {
	write_tag();
	result += "</section>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cselect() {
	write_tag();
	result += "</select>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csmall() {
	write_tag();
	result += "</small>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csource() {
	write_tag();
	result += "</source>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cspan() {
	write_tag();
	result += "</span>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cstrike() {
	write_tag();
	result += "</strike>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cstrong() {
	write_tag();
	result += "</strong>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cstyle() {
	write_tag();
	result += "</style>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csub() {
	write_tag();
	result += "</sub>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csummary() {
	write_tag();
	result += "</summary>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csup() {
	write_tag();
	result += "</sup>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csvg() {
	write_tag();
	result += "</svg>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctable() {
	write_tag();
	result += "</table>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctbody() {
	write_tag();
	result += "</tbody>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctd() {
	write_tag();
	result += "</td>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctemplateh() {
	write_tag();
	result += "</templateh>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctextarea() {
	write_tag();
	result += "</textarea>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctfoot() {
	write_tag();
	result += "</tfoot>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cth() {
	write_tag();
	result += "</th>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cthead() {
	write_tag();
	result += "</thead>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctime() {
	write_tag();
	result += "</time>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctitle() {
	write_tag();
	result += "</title>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctr() {
	write_tag();
	result += "</tr>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctrack() {
	write_tag();
	result += "</track>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::ctt() {
	write_tag();
	result += "</tt>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cu() {
	write_tag();
	result += "</u>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cul() {
	write_tag();
	result += "</ul>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cvar() {
	write_tag();
	result += "</var>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cvideo() {
	write_tag();
	result += "</video>";

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::cwbr() {
	write_tag();
	result += "</wbr>";

	return this;
}

Ref<_HTMLTag> _HTMLBuilder::form_get() {
	write_tag();

	return tag->start("form")->method_get();
}
Ref<_HTMLTag> _HTMLBuilder::form_post() {
	write_tag();

	return tag->start("form")->method_post();
}
Ref<_HTMLBuilder> _HTMLBuilder::form_get(const String &action, const String &cls, const String &id) {
	Ref<_HTMLTag> t = form_get();

	t->fora(action);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}
Ref<_HTMLBuilder> _HTMLBuilder::form_post(const String &action, const String &cls, const String &id) {
	Ref<_HTMLTag> t = form_post();

	t->action(action);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

/*
Ref<_HTMLBuilder> _HTMLBuilder::form_post(const String &action, Request *request, const String &cls, const String &id) {
	form_post(action, cls, id);
	csrf_token(request);

	return this;
}
*/

Ref<_HTMLTag> _HTMLBuilder::input_button() {
	write_tag();

	return tag->start("input")->itbutton();
}

Ref<_HTMLTag> _HTMLBuilder::input_checkbox() {
	write_tag();

	return tag->start("input")->itcheckbox();
}

Ref<_HTMLTag> _HTMLBuilder::input_color() {
	write_tag();

	return tag->start("input")->itcolor();
}

Ref<_HTMLTag> _HTMLBuilder::input_date() {
	write_tag();

	return tag->start("input")->itdate();
}

Ref<_HTMLTag> _HTMLBuilder::input_datetime_local() {
	write_tag();

	return tag->start("input")->itdatetime_local();
}

Ref<_HTMLTag> _HTMLBuilder::input_email() {
	write_tag();

	return tag->start("input")->itemail();
}

Ref<_HTMLTag> _HTMLBuilder::input_file() {
	write_tag();

	return tag->start("input")->itfile();
}

Ref<_HTMLTag> _HTMLBuilder::input_hidden() {
	write_tag();

	return tag->start("input")->ithidden();
}

Ref<_HTMLTag> _HTMLBuilder::input_image() {
	write_tag();

	return tag->start("input")->itimage();
}

Ref<_HTMLTag> _HTMLBuilder::input_month() {
	write_tag();

	return tag->start("input")->itmonth();
}

Ref<_HTMLTag> _HTMLBuilder::input_number() {
	write_tag();

	return tag->start("input")->itnumber();
}

Ref<_HTMLTag> _HTMLBuilder::input_password() {
	write_tag();

	return tag->start("input")->itpassword();
}

Ref<_HTMLTag> _HTMLBuilder::input_radio() {
	write_tag();

	return tag->start("input")->itradio();
}

Ref<_HTMLTag> _HTMLBuilder::input_range() {
	write_tag();

	return tag->start("input")->itrange();
}

Ref<_HTMLTag> _HTMLBuilder::input_reset() {
	write_tag();

	return tag->start("input")->itreset();
}

Ref<_HTMLTag> _HTMLBuilder::input_search() {
	write_tag();

	return tag->start("input")->itsearch();
}

Ref<_HTMLTag> _HTMLBuilder::input_submit() {
	write_tag();

	return tag->start("input")->itsubmit();
}

Ref<_HTMLTag> _HTMLBuilder::input_tel() {
	write_tag();

	return tag->start("input")->ittel();
}

Ref<_HTMLTag> _HTMLBuilder::input_text() {
	write_tag();

	return tag->start("input")->ittext();
}

Ref<_HTMLTag> _HTMLBuilder::input_time() {
	write_tag();

	return tag->start("input")->ittime();
}

Ref<_HTMLTag> _HTMLBuilder::input_url() {
	write_tag();

	return tag->start("input")->iturl();
}

Ref<_HTMLTag> _HTMLBuilder::input_week() {
	write_tag();

	return tag->start("input")->itweek();
}

Ref<_HTMLBuilder> _HTMLBuilder::label(const String &pfor, const String &plabel, const String &cls, const String &id) {
	Ref<_HTMLTag> t = label();

	t->fora(pfor);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	w(plabel);

	clabel();

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_button(const String &name, const String &value, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_button();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_checkbox(const String &name, const String &value, const bool checked, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_checkbox();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	t->checked(checked);

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_color(const String &name, const String &value, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_color();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_date(const String &name, const String &value, const String &cls, const String &id, const String &date_min, const String &date_max, const String &date_step) {
	Ref<_HTMLTag> t = input_date();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (date_min != "") {
		t->min(date_min);
	}

	if (date_max != "") {
		t->max(date_max);
	}

	if (date_step != "") {
		t->step(date_step);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_datetime_local(const String &name, const String &value, const String &cls, const String &id, const String &date_min, const String &date_max, const String &date_step) {
	Ref<_HTMLTag> t = input_datetime_local();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (date_min != "") {
		t->min(date_min);
	}

	if (date_max != "") {
		t->max(date_max);
	}

	if (date_step != "") {
		t->step(date_step);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_email(const String &name, const String &value, const String &placeholder, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_email();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (placeholder != "") {
		t->placeholder(placeholder);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_file(const String &name, const String &accept, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_file();

	t->name(name);

	if (accept != "") {
		t->accept(accept);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_image(const String &name, const String &src, const String &alt, const String &cls, const String &id, const int width, const int height) {
	Ref<_HTMLTag> t = input_image();

	t->name(name);

	if (src != "") {
		t->src(src);
	}

	if (alt != "") {
		t->alt(alt);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (width != 0) {
		t->width(width);
	}

	if (height != 0) {
		t->height(height);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_month(const String &name, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_month();

	t->name(name);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_number(const String &name, const String &vmin, const String &vmax, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_number();

	t->name(name);

	if (vmin != "") {
		t->min(vmin);
	}

	if (vmax != "") {
		t->max(vmax);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_password(const String &name, const String &value, const String &placeholder, const String &cls, const String &id, const String &minlength, const String &maxlength, const String &size) {
	Ref<_HTMLTag> t = input_password();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (placeholder != "") {
		t->placeholder(placeholder);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (minlength != "") {
		t->minlength(minlength);
	}

	if (maxlength != "") {
		t->maxlength(maxlength);
	}

	if (size != "") {
		t->size(size);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_radio(const String &name, const String &value, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_password();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_range(const String &name, const String &value, const String &vmin, const String &vmax, const String &vstep, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_range();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (vmin != "") {
		t->min(vmin);
	}

	if (vmax != "") {
		t->max(vmax);
	}

	if (vstep != "") {
		t->step(vstep);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_reset(const String &name, const String &value, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_reset();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_search(const String &name, const String &value, const String &placeholder, const String &cls, const String &id, const String &minlength, const String &maxlength, const String &size, const String &pattern) {
	Ref<_HTMLTag> t = input_search();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (placeholder != "") {
		t->placeholder(placeholder);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (minlength != "") {
		t->minlength(minlength);
	}

	if (maxlength != "") {
		t->maxlength(maxlength);
	}

	if (size != "") {
		t->size(size);
	}

	if (pattern != "") {
		t->pattern(pattern);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_submit(const String &value, const String &cls, const String &id) {
	Ref<_HTMLTag> t = input_submit();

	t->value(value);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_tel(const String &name, const String &value, const String &placeholder, const String &cls, const String &id, const String &minlength, const String &maxlength, const String &size, const String &pattern) {
	Ref<_HTMLTag> t = input_tel();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (placeholder != "") {
		t->placeholder(placeholder);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (minlength != "") {
		t->minlength(minlength);
	}

	if (maxlength != "") {
		t->maxlength(maxlength);
	}

	if (size != "") {
		t->size(size);
	}

	if (pattern != "") {
		t->pattern(pattern);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_text(const String &name, const String &value, const String &placeholder, const String &cls, const String &id, const String &minlength, const String &maxlength, const String &size) {
	Ref<_HTMLTag> t = input_text();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (placeholder != "") {
		t->placeholder(placeholder);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (minlength != "") {
		t->minlength(minlength);
	}

	if (maxlength != "") {
		t->maxlength(maxlength);
	}

	if (size != "") {
		t->size(size);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_time(const String &name, const String &cls, const String &id, const String &vmin, const String &vmax, const String &vstep) {
	Ref<_HTMLTag> t = input_time();

	t->name(name);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (vmin != "") {
		t->min(vmin);
	}

	if (vmax != "") {
		t->max(vmax);
	}

	if (vstep != "") {
		t->step(vstep);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_url(const String &name, const String &value, const String &placeholder, const String &cls, const String &id, const String &minlength, const String &maxlength, const String &size) {
	Ref<_HTMLTag> t = input_url();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	if (placeholder != "") {
		t->placeholder(placeholder);
	}

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (minlength != "") {
		t->minlength(minlength);
	}

	if (maxlength != "") {
		t->maxlength(maxlength);
	}

	if (size != "") {
		t->size(size);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_week(const String &name, const String &cls, const String &id, const String &vmin, const String &vmax) {
	Ref<_HTMLTag> t = input_week();

	t->name(name);

	if (cls != "") {
		t->cls(cls);
	}

	if (id != "") {
		t->id(id);
	}

	if (vmin != "") {
		t->min(vmin);
	}

	if (vmax != "") {
		t->max(vmax);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::input_hidden(const String &name, const String &value) {
	Ref<_HTMLTag> t = input_hidden();

	t->name(name);

	if (value != "") {
		t->value(value);
	}

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::csrf_token(const String &token) {
	if (token == "") {
		// don't waste html characters if it's an empty string anyway
		return this;
	}

	input_hidden("csrf_token", token);

	return this;
}
/*
Ref<_HTMLBuilder> _HTMLBuilder::csrf_token(Request *request) {
	return csrf_token(request->get_csrf_token());
}
*/

void _HTMLBuilder::f() {
	write_tag();
}

Ref<_HTMLBuilder> _HTMLBuilder::w(const String &val) {
	write_tag();

	result += val;

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::wn(const double val, int p_decimals) {
	write_tag();

	result += String::num(val, p_decimals);

	return this;
}
Ref<_HTMLBuilder> _HTMLBuilder::wns(const double val) {
	write_tag();

	result += String::num_scientific(val);

	return this;
}
Ref<_HTMLBuilder> _HTMLBuilder::wr(const double val, const bool p_trailing) {
	write_tag();

	//TODO
	//result += String::num_real(val, p_trailing);

	return this;
}
Ref<_HTMLBuilder> _HTMLBuilder::wi(const int64_t val, const int base, const bool capitalize_hex) {
	write_tag();

	result += String::num_int64(val, base, capitalize_hex);

	return this;
}
Ref<_HTMLBuilder> _HTMLBuilder::wui(const uint64_t val, const int base, const bool capitalize_hex) {
	write_tag();

	result += String::num_uint64(val, base, capitalize_hex);

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::wbn(const bool val) {
	write_tag();

	//TODO
	//result += String::bool_num(val);

	return this;
}
Ref<_HTMLBuilder> _HTMLBuilder::wbs(const bool val) {
	write_tag();

	//TODO
	//result += String::bool_str(val);

	return this;
}

// TODO!
Ref<_HTMLBuilder> _HTMLBuilder::we(const String &val) {
	//print_error("_HTMLBuilder::write_excaped NYI!");

	write_tag();

	result += val;

	return this;
}

Ref<_HTMLBuilder> _HTMLBuilder::write_tag() {
	if (tag->has_data()) {
		tag->close();
		result += tag->result;
		tag->reset();
	}

	return this;
}

_HTMLBuilder::_HTMLBuilder() {
	tag.instance();
	tag->owner.reference_ptr(this);
}

_HTMLBuilder::~_HTMLBuilder() {
}

void _HTMLBuilder::_bind_methods() {
	ClassDB::bind_method(D_METHOD("a"), &_HTMLBuilder::a);
	//Ref<_HTMLTag> a();
}
