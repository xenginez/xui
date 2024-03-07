#include <iostream>
#include "gdi_implement.h"

int main()
{
	xui::context ctx;
	gdi_implement imp;
	xui::style style;
	style.load( xui::context::dark_style() );

	std::vector<xui::url> urls;
	style.get_values<xui::url>( urls );

	imp.init();
	ctx.init( &imp );

	auto font = imp.create_font( xui::system_resource::FONT_DEFAULT, 16, xui::font_flag::FONT_NONE );
	auto icon = imp.create_texture( xui::system_resource::ICON_APPLICATION );
	auto window = imp.create_window( "XUI", icon, { 500, 540, 600, 600 } );

	ctx.push_font( font );
	ctx.push_style( &style );

	imp.update( [&]()
	{
		return ctx.draw( [&]()
		{
			ctx.push_window( window );
			
			ctx.push_rect( imp.get_window_rect( window ) );
			{
				ctx.begin_window( "超级UI", icon );
				{
					ctx.push_rect( { 100, 100, 100, 100 } );
					ctx.label( "奋斗精神鼓励" );
					ctx.pop_rect();

					ctx.push_rect( { 200, 200, 100, 100 } );
					ctx.image( icon );
					ctx.pop_rect();

					ctx.push_rect( { 300, 200, 100, 150 } );
					if ( ctx.button( "购房价款" ) )
					{
						std::cout << "购房价款 clicked" << std::endl;
					}
					ctx.pop_rect();

					static int cur = 0;
					cur = ( cur + 1 ) % 100;

					ctx.push_rect( { 100, 200, 100, 30 } );
					ctx.process( cur / 100.0f );
					ctx.pop_rect();

					ctx.push_rect( { 100, 240, 100, 30 } );
					ctx.slider_int( &cur, 1, 0, 100 );
					ctx.pop_rect();
				}
				ctx.end_window();
			}
			ctx.pop_rect();

			ctx.pop_window();
		} );
	} );

	ctx.release();
	imp.release();

	return 0;
}
