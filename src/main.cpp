#include <iostream>
#include "win_context.h"

int main()
{
	win_context ctx;

	ctx.init();

	auto font = ctx.create_font( xui::system_resource::FONT_DEFAULT );
	auto icon = ctx.create_texture( xui::system_resource::ICON_APPLICATION );
	auto window = ctx.create_window( "XUI", icon, { 2260, 540, 600, 600 } );

	ctx.push_font( font );

	ctx.update( [&]()
	{
		return ctx.draw( [&]( xui::draw_context & ctx )
		{
			ctx.push_window( window );
			
			ctx.push_rect( ctx.get_window_rect( window ) );
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
				}
				ctx.end_window();
			}
			ctx.pop_rect();

			ctx.pop_window();
		} );
	} );

	ctx.release();

	return 0;
}
