#include <iostream>
#include "gdi_implement.h"

static bool radio_value = false;
static float slider_value = 0;
static float vscollbar_value = 0;
static float hscollbar_value = 0;

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
	auto window = imp.create_window( "XUI", icon, { 500, 540, 600, 600 } );

	imp.update( [&]()
	{
		ctx.begin();
		{
			ctx.push_font_id( font ); ctx.push_style( &style ); ctx.push_window_id( window );
			{
				ctx.push_window_rect( imp.get_window_rect( window ) );
				{
					ctx.begin_window( "超级UI", icon, xui::window_flag::WINDOW_NO_MINIMIZEBOX );
					{
						static xui::menubar_model menubar_m = []()
						{
							xui::menubar_model m;

							if ( auto menu1 = m.add_menu( "menu1" ) )
							{
								menu1->add_item( "item1-item1" );
								menu1->beg_menu("menu1-menu11" );
								{
									menu1->add_item( "menu1-menu11-item1" );
								}
								menu1->end_menu();
							}

							if( auto menu2 = m.add_menu( "menu2" ) )
							{
								menu2->add_item( "item2-item2" );
							}

							return m;
						}( );
						ctx.menubar( &menubar_m );

						ctx.push_window_rect( { 100, 100, 100, 100 } );
						ctx.label( "奋斗精神鼓励" );
						ctx.pop_window_rect();

						ctx.push_window_rect( { 200, 200, 100, 100 } );
						ctx.image( icon );
						ctx.pop_window_rect();

						ctx.push_window_rect( { 300, 200, 50, 70 } );
						if ( ctx.button( "购房价款" ) )
						{
							std::cout << "购房价款 clicked" << std::endl;
						}
						ctx.pop_window_rect();

						ctx.push_window_rect( { 100, 200, 100, 100 } );
						ctx.slider( slider_value, 0, 1 );
						ctx.pop_window_rect();

						ctx.push_window_rect( { 100, 300, 100, 100 } );
						ctx.process( slider_value, 0, 1, std::to_string( (int)( slider_value * 100 ) ) + "%" );
						ctx.pop_window_rect();

						ctx.push_window_rect( { 100, 420, 20, 20 } );
						ctx.radio( radio_value );
						ctx.pop_window_rect();

						ctx.push_window_rect( { 100, 440, 20, 20 } );
						ctx.check( radio_value );
						ctx.pop_window_rect();

						auto rect = ctx.current_window_rect();

						ctx.push_window_rect( { rect.x + rect.w - 20, rect.y, 20, rect.h - 20 } );
						ctx.scrollbar( vscollbar_value, 0.1f, 0, 1, xui::direction::TOP_BOTTOM );
						ctx.pop_window_rect();

						ctx.push_window_rect( { rect.x, rect.y + rect.h - 20, rect.w - 20, 20 } );
						ctx.scrollbar( hscollbar_value, 0.1f, 0, 1, xui::direction::LEFT_RIGHT );
						ctx.pop_window_rect();


					}
					ctx.end_window();
				}
				ctx.pop_window_rect();
			}
			ctx.pop_window_id(); ctx.pop_style(); ctx.pop_font_id();
		}
		return ctx.end();
	} );

	ctx.release();
	imp.release();

	return 0;
}
