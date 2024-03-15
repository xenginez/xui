#include <iostream>
#include "gdi_implement.h"

static float slider_value = 0;
static bool radio_value = false;
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

	ctx.push_font( font );
	ctx.push_style( &style );

	imp.update( [&]()
	{
		ctx.begin();
		{
			ctx.push_window_id( window );
			{
				ctx.push_rect( imp.get_window_rect( window ) );
				{
					ctx.begin_window( "超级UI", icon );// , xui::WINDOW_NO_BACKGROUND );
					{
						ctx.push_rect( { 100, 100, 100, 100 } );
						ctx.label( "奋斗精神鼓励" );
						ctx.pop_rect();

						ctx.push_rect( { 200, 200, 100, 100 } );
						ctx.image( icon );
						ctx.pop_rect();

						ctx.push_rect( { 300, 200, 50, 70 } );
						if ( ctx.button( "购房价款" ) )
						{
							std::cout << "购房价款 clicked" << std::endl;
						}
						ctx.pop_rect();

						ctx.push_rect( { 100, 200, 100, 100 } );
						ctx.slider( slider_value, 0, 1 );
						ctx.pop_rect();

						ctx.push_rect( { 100, 300, 100, 100 } );
						ctx.process( slider_value, 0, 1, std::to_string( (int)( slider_value * 100 ) ) + "%" );
						ctx.pop_rect();

						ctx.push_rect( { 100, 420, 20, 20 } );
						ctx.radio( radio_value );
						ctx.pop_rect();

						ctx.push_rect( { 100, 440, 20, 20 } );
						ctx.check( radio_value );
						ctx.pop_rect();

						auto rect = ctx.currrent_rect();

						ctx.push_rect( { rect.x + rect.w - 20, rect.y, 20, rect.h - 20 } );
						ctx.scrollbar( vscollbar_value, 0.1f, 0, 1, xui::direction::TOP_BOTTOM );
						ctx.pop_rect();

						ctx.push_rect( { rect.x, rect.y + rect.h - 20, rect.w - 20, 20 } );
						ctx.scrollbar( hscollbar_value, 0.1f, 0, 1, xui::direction::LEFT_RIGHT );
						ctx.pop_rect();

					}
					ctx.end_window();
				}
				ctx.pop_rect();
			}
			ctx.pop_window_id();
		}
		return ctx.end();
	} );

	ctx.release();
	imp.release();

	return 0;
}
