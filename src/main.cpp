#include <iostream>
#include "gdi_implement.h"

static bool radio_value = false;
static float slider_value = 0;
static float vscollbar_value = 0;
static float hscollbar_value = 0;
static xui::control_id menubar_value;

int main()
{
	xui::context ctx;
	gdi_implement imp;
	xui::style style;
	style.parse( xui::context::dark_style() );

	std::vector<xui::url> urls;
	style.get_values<xui::url>( urls );

	imp.init();
	ctx.init( &imp );

	auto font = imp.create_font( system_resource::FONT_DEFAULT, 16, xui::font_flag::FONT_NONE );
	auto icon = imp.create_texture( system_resource::ICON_APPLICATION );
	auto window = imp.create_window( "XUI", icon, { 500, 300, 600, 600 } );

	imp.update( [&]()
	{
		ctx.begin();
		{
			auto rect = imp.get_window_rect( window );
			ctx.push_style( &style );
			ctx.push_font_id( font );
			ctx.push_window_id( window );
			ctx.push_window_rect( rect );
			ctx.push_viewport( { 0, 0, rect.w,  rect.h } );
			{
				ctx.begin_window( "超级UI", icon, xui::window_flag::WINDOW_NO_MINIMIZEBOX );
				{
					static xui::menubar_model menubar_m = []()
					{
						xui::menubar_model m( "menubar" );

						if ( auto menu1 = m.add_menu( "menu1" ) )
						{
							menu1->add_item( "item1-item1" );
							menu1->beg_menu( "menu1-menu11" );
							{
								menu1->add_item( "menu1-menu11-item1" );
								menu1->beg_menu( "menu1-menu11-menu111" );
								{
									menu1->add_item( "menu1-menu11-menu111-item1" );
								}
								menu1->end_menu();
							}
							menu1->end_menu();
						}

						if ( auto menu2 = m.add_menu( "menu2" ) )
						{
							menu2->add_item( "item2-item2" );
						}

						return m;
					}( );

					menubar_value = xui::invalid_control_id;
					if ( ctx.menubar( &menubar_m, menubar_value ) )
					{
						std::cout << "menubar action " << menubar_value << std::endl;
					}

					ctx.push_viewport( { 100, 100, 100, 100 } );
					ctx.label( "奋斗精神鼓励" );
					ctx.pop_viewport();

					ctx.push_viewport( { 200, 200, 100, 100 } );
					ctx.image( icon );
					ctx.pop_viewport();

					ctx.push_viewport( { 300, 200, 50, 70 } );
					if ( ctx.button( "购房价款" ) )
					{
						std::cout << "购房价款 clicked" << std::endl;
					}
					ctx.pop_viewport();

					ctx.push_viewport( { 100, 50, 100, 100 } );
					ctx.slider( slider_value, 0, 1 );
					ctx.pop_viewport();

					ctx.push_viewport( { 100, 300, 100, 100 } );
					ctx.process( slider_value, 0, 1, std::to_string( (int)( slider_value * 100 ) ) + "%" );
					ctx.pop_viewport();

					ctx.push_viewport( { 100, 420, 20, 20 } );
					ctx.radio( radio_value );
					ctx.pop_viewport();

					ctx.push_viewport( { 100, 440, 20, 20 } );
					ctx.check( radio_value );
					ctx.pop_viewport();

					auto rect = ctx.current_viewport();

					ctx.push_viewport( { rect.x + rect.w - 20, rect.y, 20, rect.h - 20 } );
					ctx.scrollbar( vscollbar_value, 0.1f, 0, 1, xui::direction::TOP_BOTTOM );
					ctx.pop_viewport();

					ctx.push_viewport( { rect.x, rect.y + rect.h - 20, rect.w - 20, 20 } );
					ctx.scrollbar( hscollbar_value, 0.1f, 0, 1, xui::direction::LEFT_RIGHT );
					ctx.pop_viewport();


				}
				ctx.end_window();
			}
			ctx.pop_viewport();
			ctx.pop_window_rect();
			ctx.pop_window_id();
			ctx.pop_font_id();
			ctx.pop_style();
		}
		return ctx.end();
	} );

	ctx.release();
	imp.release();

	return 0;
}
