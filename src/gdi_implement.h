#pragma once

#include "xui.h"

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
	xui::window_id create_window( std::string_view title, xui::texture_id icon, const xui::rect & rect, xui::window_id parent = xui::invalid_id ) override;
	xui::window_id get_window_parent( xui::window_id id ) const override;
	void set_window_parent( xui::window_id id, xui::window_id parent ) override;
	xui::windowstatus get_window_status( xui::window_id id ) const override;
	void set_window_status( xui::window_id id, xui::windowstatus show ) override;
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
	void set_event( xui::window_id id, xui::event key, int val );
	int get_key( xui::window_id id, xui::event key ) const override;
	xui::point get_cursor_pos( xui::window_id id ) const override;

private:
	void present();
	void render( std::span<xui::drawcmd> cmds );

private:
	private_p * _p;
};