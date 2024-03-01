#pragma once

#include "xui.h"

class win_context : public xui::draw_context
{
private:
	struct private_p;

public:
	win_context( std::pmr::memory_resource * res = std::pmr::get_default_resource() );
	~win_context();

public:
	void init() override;
	void update( const std::function<std::span<xui::drawcommand>()> & paint );
	void release() override;

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
	xui::font_id create_font( std::string_view filename ) override;
	void remove_font( xui::font_id id ) override;

public:
	xui::texture_id create_texture( std::string_view filename ) override;
	xui::size texture_size( xui::texture_id id ) const override;
	void remove_texture( xui::texture_id id ) override;

private:
	void present();
	void render( std::span<xui::drawcommand> cmds );

private:
	private_p * _p;
};