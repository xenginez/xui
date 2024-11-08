#pragma once

#include "xui.h"

namespace Gdiplus
{
	class Pen;
	class Brush;
}

namespace system_resource
{
	static constexpr xui::control_id FONT_DEFAULT = "font://default";

	static constexpr xui::control_id ICON_APPLICATION = "icon://application";
	static constexpr xui::control_id ICON_ERROR = "icon://error";
	static constexpr xui::control_id ICON_WARNING = "icon://warning";
	static constexpr xui::control_id ICON_INFORMATION = "icon://information";
}

class gdi_implement : public xui::implement
{
private:
	struct private_p;

public:
	gdi_implement();
	~gdi_implement();

public:
	void init();
	void update( const std::function<std::span<xui::drawcmd>()> & paint );
	void release();

public:
	xui::window_id create_window( std::string_view title, xui::texture_id icon, const xui::rect & rect, xui::window_id parent = xui::invalid_window_id ) override;
	xui::window_id get_window_parent( xui::window_id id ) const override;
	void set_window_parent( xui::window_id id, xui::window_id parent ) override;
	xui::window_status get_window_status( xui::window_id id ) const override;
	void set_window_status( xui::window_id id, xui::window_status show ) override;
	xui::rect get_window_rect( xui::window_id id ) const override;
	void set_window_rect( xui::window_id id, const xui::rect & rect ) override;
	std::string get_window_title( xui::window_id id ) const override;
	void set_window_title( xui::window_id id, std::string_view title ) override;
	void remove_window( xui::window_id id ) override;

public:
	bool load_font_file( std::string_view filename ) override;
	xui::font_id create_font( std::string_view family, int size, xui::font_flag flag ) override;
	xui::size font_size( xui::font_id id, std::string_view text ) const override;
	void remove_font( xui::font_id id ) override;

public:
	xui::texture_id create_texture( std::string_view filename ) override;
	xui::size texture_size( xui::texture_id id ) const override;
	void remove_texture( xui::texture_id id ) override;

public:
	xui::vec2 get_cursor_dt( xui::window_id id ) const override;
	xui::vec2 get_cursor_pos( xui::window_id id ) const override;
	xui::vec2 get_cusor_wheel( xui::window_id id ) const override;
	std::string get_unicodes( xui::window_id id ) const override;
	int get_event( xui::window_id id, xui::event key ) const override;
	std::span<xui::vec2> get_touchs( xui::window_id id ) const override;
	std::string get_clipboard_data( xui::window_id id, std::string_view mime ) const override;
	bool set_clipboard_data( xui::window_id id, std::string_view mime, std::string_view data ) override;

private:
	void present();
	void render( std::span<xui::drawcmd> cmds );

private:
	std::shared_ptr<Gdiplus::Pen> create_pen( const xui::stroke & stroke ) const;
	std::shared_ptr<Gdiplus::Brush> create_brush( const xui::filled & filled ) const;

private:
	void set_unicode( xui::window_id id, wchar_t unicode );
	void set_wheel( xui::window_id id, const xui::vec2 & dt );
	void set_cursor( xui::window_id id, const xui::vec2 & pos );
	void set_touchs( xui::window_id id, std::span<xui::vec2> touchs );
	void set_event( xui::window_id id, xui::event key, int val );

private:
	private_p * _p;
};