#include "win_context.h"

#include <iomanip>
#include <iostream>
#include <Windows.h>
#include <Windowsx.h>
#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")

#define RGBA(r, g, b, a) ( RGB( r, g, b ) | ((DWORD)(BYTE)(a))<<24 )

namespace
{
    struct font
    {
        std::string name;
        Gdiplus::Font * font;
    };
    struct window
    {
        HWND hwnd = nullptr;
        xui::rect rect;
        xui::rect rrect;
        HBITMAP frame_buffer = nullptr;
    };
    struct texture
    {
        std::string name;
        Gdiplus::Image * image;
    };

    template<typename ... Ts> struct overload : Ts ... { using Ts::operator() ...; };
    template<class... Ts> overload( Ts... ) -> overload<Ts...>;

    std::wstring ansi_wide( std::string_view str )
    {
        int len = MultiByteToWideChar( CP_ACP, 0, str.data(), str.size(), NULL, 0);

        std::wstring wstr( len, 0 );

        len = MultiByteToWideChar( CP_ACP, 0, str.data(), str.size(), wstr.data(), len );

        return wstr;
    }
}

struct win_context::private_p
{
    HDC _hdc;
    ULONG_PTR _gditoken;
    std::vector<font> _fonts;
    std::vector<window> _windows;
    std::vector<texture> _textures;
    Gdiplus::PrivateFontCollection font_collection;
};

win_context::win_context( std::pmr::memory_resource * res )
    : xui::draw_context( res ), _p( new private_p )
{
}

win_context::~win_context()
{
    delete _p;
}

void win_context::init()
{
    WNDCLASSEXA wc;
    wc.cbSize = sizeof( wc );
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProcA;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandleA( nullptr );
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wc.hbrBackground = static_cast<HBRUSH>( GetStockObject( COLOR_WINDOW ) );
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = "XUIClass";
    wc.hIconSm = nullptr;
    RegisterClassExA( &wc );

    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup( &_p->_gditoken, &input, nullptr );

    _p->_hdc = CreateCompatibleDC( nullptr );

    xui::draw_context::init();
}

void win_context::update( const std::function<std::span<xui::drawcommand>()> & paint )
{
    MSG msg;
    while ( 1 )
    {
        GetMessageA( &msg, nullptr, 0, 0 );

        xui::window_id id = xui::invalid_id;
        auto it = std::find_if( _p->_windows.begin(), _p->_windows.end(), [&](const auto & val )
        {
            return val.hwnd == msg.hwnd;
        } );
        if ( it != _p->_windows.end() )
            id = std::distance( _p->_windows.begin(), it );

        if ( msg.message == WM_DESTROY )
        {
            std::cout << "WM_DESTROY" << std::endl;
            break;
        }
        else if ( msg.message == WM_QUIT )
        {
            std::cout << "WM_QUIT" << std::endl;
            break;
        }
        else if ( msg.message == WM_LBUTTONDOWN )
        {
            set_event( id, xui::KEY_MOUSE_LEFT, 1 );

            std::cout << "WM_LBUTTONDOWN" << std::endl;
        }
        else if ( msg.message == WM_RBUTTONDOWN )
        {
            set_event( id, xui::KEY_MOUSE_RIGHT, 1 );

            std::cout << "WM_RBUTTONDOWN" << std::endl;
        }
        else if ( msg.message == WM_MOUSEWHEEL )
        {
            set_event( id, xui::MOUSE_WHEEL, ( int( msg.wParam ) / WHEEL_DELTA ) );
            
            std::cout << "WM_MOUSEWHEEL: " << std::dec << ( int( msg.wParam ) / WHEEL_DELTA ) << std::endl;
        }
        else if ( msg.message == WM_LBUTTONUP )
        {
            set_event( id, xui::KEY_MOUSE_LEFT, 0 );

            std::cout << "WM_LBUTTONUP" << std::endl;
        }
        else if ( msg.message == WM_RBUTTONUP )
        {
            set_event( id, xui::KEY_MOUSE_RIGHT, 0 );

            std::cout << "WM_RBUTTONUP" << std::endl;
        }
        else if ( msg.message == WM_MOUSEMOVE )
        {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof( tme );
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = HOVER_DEFAULT;
            tme.hwndTrack = msg.hwnd;
            TrackMouseEvent( &tme );

            set_event( id, xui::MOUSE_ENTER, 1 );
            set_event( id, xui::MOUSE_LEAVE, 0 );
            set_event( id, xui::MOUSE_MOVE, msg.lParam );

            std::cout << "WM_MOUSEMOVE: " << std::dec << GET_X_LPARAM( msg.lParam ) << ":" << GET_Y_LPARAM( msg.lParam ) << std::endl;
        }
        else if ( msg.message == WM_MOUSELEAVE )
        {
            set_event( id, xui::MOUSE_ENTER, 0 );
            set_event( id, xui::MOUSE_LEAVE, 1 );

            std::cout << "WM_MOUSELEAVE" << std::endl;
        }
        else if ( msg.message == WM_SETFOCUS )
        {
            std::cout << "WM_SETFOCUS" << std::endl;
        }
        else if ( msg.message == WM_ACTIVATE )
        {
            std::cout << "WM_ACTIVATE" << std::endl;
        }
        else if ( msg.message == WA_ACTIVE )
        {
            std::cout << "WA_ACTIVE" << std::endl;
        }
        else if ( msg.message == WM_CLOSE )
        {
            set_event( id, xui::WINDOW_CLOSE, 1 );

            DestroyWindow( msg.hwnd );
            
            if ( std::find_if( _p->_windows.begin(), _p->_windows.end(), []( const auto & val ) { return val.hwnd != nullptr; } ) == _p->_windows.end() )
            {
                PostQuitMessage( 0 );
            }

            std::cout << "WM_CLOSE" << std::endl;
        }
        else if ( msg.message == WM_TIMER )
        {
            std::cout << "WM_TIMER" << std::endl;
        }
        else if ( msg.message == WM_SIZE )
        {
            switch ( msg.wParam )
            {
            case SIZE_MAXHIDE:
                break;
            case SIZE_MAXIMIZED:
                break;
            case SIZE_MAXSHOW:
                break;
            case SIZE_MINIMIZED:
                break;
            case SIZE_RESTORED:
                break;
            }

            std::cout << "WM_SIZE" << std::endl;
        }
        else
        {
            std::cout << "0x" << std::hex << std::setfill( '0' ) << std::setw( 4 ) << msg.message << std::endl;
        }

        TranslateMessage( &msg );

        DispatchMessageA( &msg );

        render( paint() );
    }
}

void win_context::release()
{
    xui::draw_context::release();

    for ( size_t i = 0; i < _p->_windows.size(); i++ )
        remove_window( i );
    _p->_windows.clear();

    for ( size_t i = 0; i < _p->_textures.size(); i++ )
        remove_texture( i );
    _p->_textures.clear();

    for ( size_t i = 0; i < _p->_fonts.size(); i++ )
        remove_font( i );
    _p->_fonts.clear();

    DeleteDC( _p->_hdc );
    Gdiplus::GdiplusShutdown( _p->_gditoken );
    UnregisterClassA( "XUIClass", GetModuleHandleA( nullptr ) );
}

xui::window_id win_context::create_window( std::string_view title, xui::texture_id icon, const xui::rect & rect, xui::window_id parent )
{
    xui::window_id id = 0;
    for ( size_t i = 0; i < _p->_windows.size(); i++ )
    {
        if ( _p->_windows[i].hwnd == nullptr )
        {
            id = i;
            break;
        }
    }
    if ( id == 0 )
    {
        _p->_windows.push_back( {} );
        id = _p->_windows.size() - 1;
    }

    window w;

    HWND phwnd = nullptr;
    if ( parent < _p->_windows.size() )
    {
        phwnd = _p->_windows[parent].hwnd;
    }

    w.hwnd = CreateWindowExA( WS_EX_LAYERED, "XUIClass", title.data(), WS_POPUP, rect.x, rect.y, rect.w, rect.h, phwnd, nullptr, GetModuleHandleA( nullptr ), nullptr );
    w.rect = rect;
    w.frame_buffer = CreateBitmap( rect.w, rect.h, 1, 32, nullptr );

    SetWindowLongPtrA( w.hwnd, GWLP_USERDATA, (LONG_PTR)this );
    SetWindowLongPtrA( w.hwnd, GWLP_USERDATA - 1, (LONG_PTR)id );

    ShowWindow( w.hwnd, SW_SHOW );
    SetActiveWindow( w.hwnd );

    _p->_windows[id] = w;

    return id;
}

xui::window_id win_context::get_window_parent( xui::window_id id ) const
{
    if ( id >= _p->_windows.size() )
        return xui::invalid_id;

    auto p = GetParent( _p->_windows[id].hwnd );
    if ( p != nullptr )
    {
        auto it = std::find_if( _p->_windows.begin(), _p->_windows.end(), [&p]( const auto & val )
        {
            return val.hwnd == p;
        } );
        if ( it != _p->_windows.end() )
        {
            return std::distance( _p->_windows.begin(), it );
        }
    }

    return xui::invalid_id;
}

void win_context::set_window_parent( xui::window_id id, xui::window_id parent )
{
    if ( id >= _p->_windows.size() )
        return;

    SetParent( _p->_windows[id].hwnd, parent == xui::invalid_id ? nullptr : _p->_windows[parent].hwnd );
}

xui::windowstatus win_context::get_window_status( xui::window_id id ) const
{
    int status = 0;

    if ( id < _p->_windows.size() )
    {
        if ( ( ::GetWindowLongA( _p->_windows[id].hwnd, GWL_STYLE ) & WS_VISIBLE ) != 0 )
        {
            status |= xui::windowstatus::SHOW;

            if ( IsIconic( _p->_windows[id].hwnd ) )
                status |= xui::windowstatus::MINIMIZE;
            else if ( IsZoomed( _p->_windows[id].hwnd ) )
                status |= xui::windowstatus::MAXIMIZE;
        }
        else
        {
            status |= xui::windowstatus::HIDE;
        }
    }

    return xui::windowstatus( status );
}

void win_context::set_window_status( xui::window_id id, xui::windowstatus show )
{
    if ( id >= _p->_windows.size() )
        return;

    switch ( show )
    {
    case xui::SHOW:
        ShowWindow( _p->_windows[id].hwnd, SW_SHOW );
        break;
    case xui::HIDE:
        ShowWindow( _p->_windows[id].hwnd, SW_HIDE );
        break;
    case xui::RESTORE:
    {
        ShowWindow( _p->_windows[id].hwnd, SW_RESTORE );

        _p->_windows[id].rect = _p->_windows[id].rrect;

        SetWindowPos( _p->_windows[id].hwnd, HWND_BOTTOM, _p->_windows[id].rect.x, _p->_windows[id].rect.y, _p->_windows[id].rect.w, _p->_windows[id].rect.h, SWP_NOZORDER );

        DeleteObject( _p->_windows[id].frame_buffer );

        _p->_windows[id].frame_buffer = CreateBitmap( _p->_windows[id].rect.w, _p->_windows[id].rect.h, 1, 32, nullptr );

        PostMessageA( _p->_windows[id].hwnd, WM_TIMER, 0, 0 );
    }
        break;
    case xui::MINIMIZE:
        ShowWindow( _p->_windows[id].hwnd, SW_MINIMIZE );
        break;
    case xui::MAXIMIZE:
    {
        ShowWindow( _p->_windows[id].hwnd, SW_MAXIMIZE );
        _p->_windows[id].rrect = _p->_windows[id].rect;

        RECT rect;
        GetWindowRect( _p->_windows[id].hwnd, &rect );

        _p->_windows[id].rect.x = rect.left;
        _p->_windows[id].rect.y = rect.top;
        _p->_windows[id].rect.w = rect.right - rect.left;
        _p->_windows[id].rect.h = rect.bottom - rect.top;

        DeleteObject( _p->_windows[id].frame_buffer );

        _p->_windows[id].frame_buffer = CreateBitmap( _p->_windows[id].rect.w, _p->_windows[id].rect.h, 1, 32, nullptr );

        PostMessageA( _p->_windows[id].hwnd, WM_TIMER, 0, 0 );
    }
        break;
    }
}

xui::rect win_context::get_window_rect( xui::window_id id ) const
{
    if ( id >= _p->_windows.size() )
        return {};

    return _p->_windows[id].rect;
}

void win_context::set_window_rect( xui::window_id id, const xui::rect & rect )
{
    if ( id >= _p->_windows.size() )
        return;

    DeleteObject( _p->_windows[id].frame_buffer );

    _p->_windows[id].rect = rect;
    _p->_windows[id].frame_buffer = CreateBitmap( rect.w, rect.h, 1, 32, nullptr );
    
    SetWindowPos( _p->_windows[id].hwnd, HWND_BOTTOM, _p->_windows[id].rect.x, _p->_windows[id].rect.y, _p->_windows[id].rect.w, _p->_windows[id].rect.h, SWP_NOZORDER );
}

std::string win_context::get_window_title( xui::window_id id ) const
{
    if ( id >= _p->_windows.size() )
        return {};

    char buf[512]; memset( buf, 0, 512 );
    if ( GetWindowTextA( _p->_windows[id].hwnd, buf, 512 ) > 0 )
    {
        return buf;
    }
    return {};
}

void win_context::set_window_title( xui::window_id id, std::string_view title )
{
    if ( id >= _p->_windows.size() )
        return;

    char buf[512]; memset( buf, 0, 512 );
    strncpy_s( buf, 512, title.data(), title.size() );

    SetWindowTextA( _p->_windows[id].hwnd, buf );
}

void win_context::remove_window( xui::window_id id )
{
    if ( id >= _p->_windows.size() )
        return;

    if ( _p->_windows[id].frame_buffer != nullptr ) DeleteObject( _p->_windows[id].frame_buffer );
    if ( _p->_windows[id].hwnd != nullptr ) PostMessageA( _p->_windows[id].hwnd, WM_CLOSE, 0, 0 );

    _p->_windows[id].hwnd = nullptr;
}

xui::font_id win_context::create_font( std::string_view filename )
{
    xui::font_id id = xui::invalid_id;

    for ( size_t i = 0; i < _p->_fonts.size(); i++ )
    {
        if ( _p->_fonts[i].font == nullptr )
        {
            id = i;
            break;
        }
    }
    if ( id == xui::invalid_id )
    {
        _p->_fonts.push_back( {} );
        id = _p->_fonts.size() - 1;
    }

    font fnt;

    fnt.name = filename;
    if ( filename == xui::system_resource::FONT_DEFAULT )
    {
        Gdiplus::FontCollection c;
        fnt.font = new Gdiplus::Font( L"ËÎÌו", 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel );
    }
    else
    {
        auto path = ansi_wide( filename );
        Gdiplus::Status status = _p->font_collection.AddFontFile( path.c_str() );
        if ( status == Gdiplus::Status::Ok )
        {
            auto count = _p->font_collection.GetFamilyCount();
            INT num;
            Gdiplus::FontFamily family;
            status = _p->font_collection.GetFamilies( count - 1, &family, &num );
            if ( status == Gdiplus::Status::Ok )
            {
                // https://learn.microsoft.com/zh-cn/windows/win32/gdiplus/-gdiplus-creating-a-private-font-collection-use
            }
        }

        return xui::invalid_id;
        Gdiplus::Font;
        // fnt.font = CreateFontA();
    }

    _p->_fonts[id] = fnt;

    return id;
}

void win_context::remove_font( xui::font_id id )
{
    if ( id >= _p->_fonts.size() )
        return;

    if ( _p->_fonts[id].font ) delete _p->_fonts[id].font;

    _p->_fonts[id].font = nullptr;
}

xui::texture_id win_context::create_texture( std::string_view filename )
{
    auto it = std::find_if( _p->_textures.begin(), _p->_textures.end(), [&]( const texture & val )
    {
        return val.name == filename;
    } );
    if ( it != _p->_textures.end() )
        return std::distance( _p->_textures.begin(), it );

    xui::texture_id id = xui::invalid_id;

    for ( size_t i = 0; i < _p->_textures.size(); i++ )
    {
        if ( _p->_textures[i].image == nullptr )
        {
            id = i;
            break;
        }
    }
    if ( id == xui::invalid_id )
    {
        _p->_textures.push_back( {} );
        id = _p->_textures.size() - 1;
    }

    texture tex;

    tex.name = filename;
    if ( filename.find( "icon" ) == 0 )
    {
        LPSTR icon_id;

        if ( filename == xui::system_resource::ICON_APPLICATION ) icon_id = IDI_APPLICATION;
        else if ( filename == xui::system_resource::ICON_ERROR ) icon_id = IDI_ERROR;
        else if ( filename == xui::system_resource::ICON_WARNING ) icon_id = IDI_WARNING;
        else if ( filename == xui::system_resource::ICON_INFORMATION ) icon_id = IDI_INFORMATION;
        
        tex.image = Gdiplus::Bitmap::FromHICON( LoadIconA( nullptr, icon_id ) );
    }
    else
    {
        auto wfile = ansi_wide( filename );
        tex.image = Gdiplus::Image::FromFile( wfile.c_str() );
    }

    _p->_textures[id] = tex;

    return id;
}

xui::size win_context::texture_size( xui::texture_id id ) const
{
    if ( id >= _p->_textures.size() )
        return {};

    return { (float)_p->_textures[id].image->GetWidth(), (float)_p->_textures[id].image->GetHeight() };
}

void win_context::remove_texture( xui::texture_id id )
{
    if ( id >= _p->_textures.size() )
        return;

    if ( _p->_textures[id].image ) delete _p->_textures[id].image;

    _p->_textures[id].image = nullptr;
}

void win_context::present()
{
    for ( size_t id = 0; id < _p->_windows.size(); id++ )
    {
        HGDIOBJ old_bitmap = SelectObject( _p->_hdc, (HGDIOBJ)_p->_windows[id].frame_buffer );
        {
            POINT point = { 0, 0 };
            SIZE size = { _p->_windows[id].rect.w, _p->_windows[id].rect.h };

            BLENDFUNCTION blend = {};
            blend.BlendOp = AC_SRC_OVER;
            blend.BlendFlags = 0;
            blend.SourceConstantAlpha = 255;
            blend.AlphaFormat = AC_SRC_ALPHA;

            UpdateLayeredWindow( _p->_windows[id].hwnd, nullptr, nullptr, &size, _p->_hdc, &point, 0, &blend, ULW_ALPHA );
        }
        SelectObject( _p->_hdc, old_bitmap );
    }
}

void win_context::render( std::span<xui::drawcommand> cmds )
{
    HGDIOBJ old_obj = nullptr;
    xui::window_id id = xui::invalid_id;

    for ( const auto & cmd : cmds )
    {
        if ( id != cmd.id )
        {
            if( old_obj != nullptr ) SelectObject( _p->_hdc, old_obj );

            old_obj = SelectObject( _p->_hdc, (HGDIOBJ)_p->_windows[cmd.id].frame_buffer );
        }

        if ( cmd.id < _p->_windows.size() && _p->_windows[cmd.id].hwnd != nullptr )
        {
            std::visit( overload(
            [&]( std::monostate )
            {

            },
            [&]( const xui::drawcommand::text_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                Gdiplus::SolidBrush brush( Gdiplus::Color( element.color.a, element.color.r, element.color.g, element.color.b ) );

                auto wtext = ansi_wide( element.text );

                Gdiplus::StringFormat fmt;

                if ( ( element.align & xui::alignment_flag::LEFT ) != 0 )
                    fmt.SetAlignment( Gdiplus::StringAlignment::StringAlignmentNear );
                else if ( ( element.align & xui::alignment_flag::RIGHT ) != 0 )
                    fmt.SetAlignment( Gdiplus::StringAlignment::StringAlignmentFar );
                if ( ( element.align & xui::alignment_flag::HCENTER ) != 0 )
                    fmt.SetAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );

                if ( ( element.align & xui::alignment_flag::TOP ) != 0 )
                    fmt.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentNear );
                else if ( ( element.align & xui::alignment_flag::BOTTOM ) != 0 )
                    fmt.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentFar );
                if ( ( element.align & xui::alignment_flag::VCENTER ) != 0 )
                    fmt.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );

                g.DrawString( wtext.c_str(), wtext.size(), _p->_fonts[element.font].font, Gdiplus::RectF( element.rect.x, element.rect.y, element.rect.w, element.rect.h ), &fmt, &brush );
            },
            [&]( const xui::drawcommand::line_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                Gdiplus::Pen pen( Gdiplus::Color( element.stroke.color.a, element.stroke.color.r, element.stroke.color.g, element.stroke.color.b ), element.stroke.width );

                g.DrawLine( &pen, Gdiplus::PointF{ element.p1.x, element.p1.y }, Gdiplus::PointF{ element.p2.x, element.p2.y } );
            },
            [&]( const xui::drawcommand::rect_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                Gdiplus::GraphicsPath path;

                float radius = 0;

                radius = 2 * element.border.radius.x;
                path.AddArc( element.rect.x, element.rect.y, radius, radius, 180, 90 );
                path.AddLine( element.rect.x + element.border.radius.x, element.rect.y, element.rect.x + element.rect.w - element.border.radius.y, element.rect.y );

                radius = 2 * element.border.radius.y;
                path.AddArc( element.rect.x + element.rect.w - radius, element.rect.y, radius, radius, 270, 90 );
                path.AddLine( element.rect.x + element.rect.w, element.rect.y + element.border.radius.y, element.rect.x + element.rect.w, element.rect.y + element.rect.h - element.border.radius.z );

                radius = 2 * element.border.radius.z;
                path.AddArc( element.rect.x + element.rect.w - radius, element.rect.y + element.rect.h - radius, radius, radius, 0, 90 );
                path.AddLine( element.rect.x + element.rect.w - element.border.radius.z, element.rect.y + element.rect.h, element.rect.x + element.border.radius.w, element.rect.y + element.rect.h );

                radius = 2 * element.border.radius.w;
                path.AddArc( element.rect.x, element.rect.y + element.rect.h - radius, radius, radius, 90, 90 );
                path.AddLine( element.rect.x, element.rect.y + element.rect.h - element.border.radius.w, element.rect.x, element.rect.y + element.border.radius.x );

                if ( element.filled.color.a != 0 )
                {
                    Gdiplus::SolidBrush brush( Gdiplus::Color( element.filled.color.a, element.filled.color.r, element.filled.color.g, element.filled.color.b ) );
                    g.FillPath( &brush, &path );
                }

                Gdiplus::Pen pen( Gdiplus::Color( element.border.color.a, element.border.color.r, element.border.color.g, element.border.color.b ), element.border.width );
                g.DrawPath( &pen, &path );
            },
            [&]( const xui::drawcommand::path_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                Gdiplus::GraphicsPath path;
                Gdiplus::Pen pen( Gdiplus::Color( element.stroke.color.a, element.stroke.color.r, element.stroke.color.g, element.stroke.color.b ), element.stroke.width );

                auto it = element.data.begin();
                auto ppoint = []( auto & it ) -> xui::point
                {
                    xui::point p;

                    auto beg = it;
                    while ( std::isdigit( *it ) ) it++;
                    p.x = std::stoi( std::string( beg, it ) );

                    it++;

                    beg = it;
                    while ( std::isdigit( *it ) ) it++;
                    p.y = std::stoi( std::string( beg, it ) );

                    if ( *it == ' ' )
                        it++;

                    return p;
                };
                xui::point m;

                while ( it != element.data.end() )
                {
                    switch ( *it )
                    {
                    case 'M':
                    {
                        it++;
                        m = ppoint( it );
                    }
                    break;
                    case 'L':
                    {
                        it++;
                        auto p = ppoint( it );
                        path.AddLine( Gdiplus::PointF( m.x, m.y ), Gdiplus::PointF( p.x, p.y ) );
                        m = p;
                    }
                    break;
                    case 'C':
                        //PolyBezier()
                        break;
                    case 'S':
                        //PolyBezier()
                        break;
                    case 'Q':
                        break;
                    case 'T':
                        break;
                    case 'A':
                        //AngleArc 
                        break;
                    case 'Z':
                    {
                        it++;
                        path.CloseFigure();
                        path.StartFigure();
                    }
                    break;
                    case ' ':
                        it++;
                        break;
                    default:
                        throw "";
                        break;
                    }
                }

                g.DrawPath( &pen, &path );
            },
            [&]( const xui::drawcommand::image_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                g.DrawImage( _p->_textures[element.id].image, Gdiplus::RectF( element.rect.x, element.rect.y, element.rect.w, element.rect.h ) );
            },
            [&]( const xui::drawcommand::circle_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                if ( element.filled.color.a != 0 )
                {
                    Gdiplus::SolidBrush brush( Gdiplus::Color( element.filled.color.a, element.filled.color.r, element.filled.color.g, element.filled.color.b ) );
                    g.FillEllipse( &brush, element.center.x - element.radius, element.center.y - element.radius, element.radius * 2, element.radius * 2 );
                }

                Gdiplus::Pen pen( Gdiplus::Color( element.border.color.a, element.border.color.r, element.border.color.g, element.border.color.b ), element.border.width );
                g.DrawEllipse( &pen, element.center.x - element.radius, element.center.y - element.radius, element.radius * 2, element.radius * 2 );
            },
            [&]( const xui::drawcommand::ellipse_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                if ( element.filled.color.a != 0 )
                {
                    Gdiplus::SolidBrush brush( Gdiplus::Color( element.filled.color.a, element.filled.color.r, element.filled.color.g, element.filled.color.b ) );
                    g.FillEllipse( &brush, element.center.x - element.radius.x, element.center.y - element.radius.y, element.radius.x * 2, element.radius.y * 2 );
                }

                Gdiplus::Pen pen( Gdiplus::Color( element.border.color.a, element.border.color.r, element.border.color.g, element.border.color.b ), element.border.width );
                g.DrawEllipse( &pen, element.center.x - element.radius.x, element.center.y - element.radius.y, element.radius.x * 2, element.radius.y * 2 );
            },
            [&]( const xui::drawcommand::polygon_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                std::vector<Gdiplus::PointF> points;
                for ( const auto & it : element.points )
                {
                    points.push_back( { it.x, it.y } );
                }

                if ( element.filled.color.a != 0 )
                {
                    Gdiplus::SolidBrush brush( Gdiplus::Color( element.filled.color.a, element.filled.color.r, element.filled.color.g, element.filled.color.b ) );
                    g.FillPolygon( &brush, points.data(), points.size() );
                }

                Gdiplus::Pen pen( Gdiplus::Color( element.border.color.a, element.border.color.r, element.border.color.g, element.border.color.b ), element.border.width );
                g.DrawPolygon( &pen, points.data(), points.size() );
            }
            ), cmd.element );
        }
    }
    
    if ( old_obj != nullptr ) SelectObject( _p->_hdc, old_obj );

    present();
}
