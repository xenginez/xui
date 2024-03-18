#include "gdi_implement.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <Windows.h>
#include <Windowsx.h>
#include <gdiplus.h>

#pragma comment(lib, "Gdiplus.lib")

#define RGBA(r, g, b, a) ( RGB( r, g, b ) | ((DWORD)(BYTE)(a))<<24 )

namespace
{
    struct eventmap
    {
    public:
        xui::vec2 dt() const
        {
            return _cursorpos - _cursorold;
        }
        const xui::vec2 & pos() const
        {
            return _cursorpos;
        }
        const xui::vec2 & wheel() const
        {
            return _cursorold;
        }
        const xui::vec2 & oldpos() const
        {
            return _cursorold;
        }
        int operator[]( int idx ) const
        {
            return _events[idx];
        }

    public:
        xui::vec2 _cursorpos = {};
        xui::vec2 _cursorold = {};
        xui::vec2 _cursorwheel = {};
        std::wstring _unicodes = {};
        std::vector<xui::vec2> _touchs;
        std::array<int, (size_t)xui::event::EVENT_MAX_COUNT> _events = { 0 };
    };
    struct texture
    {
        std::string name;
        Gdiplus::Image * image;
    };
    struct window
    {
        HWND hwnd = nullptr;
        xui::rect rect;
        xui::rect rrect;
        eventmap events;
        HBITMAP frame_buffer = nullptr;
    };
    struct font
    {
        int size;
        std::string family;
        xui::font_flag flag;
        Gdiplus::Font * font;
    };

    std::wstring ansi_wide( std::string_view str )
    {
        int len = MultiByteToWideChar( CP_ACP, 0, str.data(), str.size(), NULL, 0);

        std::wstring wstr( len, 0 );

        len = MultiByteToWideChar( CP_ACP, 0, str.data(), str.size(), wstr.data(), len );

        return wstr;
    }
    std::string wide_ansi( std::wstring_view wstr )
    {
        int len = WideCharToMultiByte( CP_ACP, 0, wstr.data(), wstr.size(), NULL, 0, 0, 0 );

        std::string str( len, 0 );

        len = WideCharToMultiByte( CP_ACP, 0, wstr.data(), wstr.size(), str.data(), len, 0, 0 );

        return str;
    }
    xui::event key_event( WPARAM wParam )
    {
        switch ( wParam )
        {
        case VK_TAB: return xui::event::KEY_TAB;
        case VK_LEFT: return xui::event::KEY_LEFT_ARROW;
        case VK_RIGHT: return xui::event::KEY_RIGHT_ARROW;
        case VK_UP: return xui::event::KEY_UP_ARROW;
        case VK_DOWN: return xui::event::KEY_DOWN_ARROW;
        case VK_PRIOR: return xui::event::KEY_PAGE_UP;
        case VK_NEXT: return xui::event::KEY_PAGE_DOWN;
        case VK_HOME: return xui::event::KEY_HOME;
        case VK_END: return xui::event::KEY_END;
        case VK_INSERT: return xui::event::KEY_INSERT;
        case VK_DELETE: return xui::event::KEY_DELETE;
        case VK_BACK: return xui::event::KEY_BACKSPACE;
        case VK_SPACE: return xui::event::KEY_SPACE;
        case VK_RETURN: return xui::event::KEY_ENTER;
        case VK_ESCAPE: return xui::event::KEY_ESCAPE;
        case VK_OEM_7: return xui::event::KEY_APOSTROPHE;
        case VK_OEM_COMMA: return xui::event::KEY_COMMA;
        case VK_OEM_MINUS: return xui::event::KEY_MINUS;
        case VK_OEM_PERIOD: return xui::event::KEY_PERIOD;
        case VK_OEM_2: return xui::event::KEY_SLASH;
        case VK_OEM_1: return xui::event::KEY_SEMICOLON;
        case VK_OEM_PLUS: return xui::event::KEY_EQUAL;
        case VK_OEM_4: return xui::event::KEY_LEFT_BRACKET;
        case VK_OEM_5: return xui::event::KEY_BACKSLASH;
        case VK_OEM_6: return xui::event::KEY_RIGHT_BRACKET;
        case VK_OEM_3: return xui::event::KEY_GRAVE_ACCENT;
        case VK_CAPITAL: return xui::event::KEY_CAPS_LOCK;
        case VK_SCROLL: return xui::event::KEY_SCROLL_LOCK;
        case VK_NUMLOCK: return xui::event::KEY_NUM_LOCK;
        case VK_SNAPSHOT: return xui::event::KEY_PRINT_SCREEN;
        case VK_PAUSE: return xui::event::KEY_PAUSE;
        case VK_NUMPAD0: return xui::event::KEY_KEYPAD_0;
        case VK_NUMPAD1: return xui::event::KEY_KEYPAD_1;
        case VK_NUMPAD2: return xui::event::KEY_KEYPAD_2;
        case VK_NUMPAD3: return xui::event::KEY_KEYPAD_3;
        case VK_NUMPAD4: return xui::event::KEY_KEYPAD_4;
        case VK_NUMPAD5: return xui::event::KEY_KEYPAD_5;
        case VK_NUMPAD6: return xui::event::KEY_KEYPAD_6;
        case VK_NUMPAD7: return xui::event::KEY_KEYPAD_7;
        case VK_NUMPAD8: return xui::event::KEY_KEYPAD_8;
        case VK_NUMPAD9: return xui::event::KEY_KEYPAD_9;
        case VK_DECIMAL: return xui::event::KEY_KEYPAD_DECIMAL;
        case VK_DIVIDE: return xui::event::KEY_KEYPAD_DIVIDE;
        case VK_MULTIPLY: return xui::event::KEY_KEYPAD_MULTIPLY;
        case VK_SUBTRACT: return xui::event::KEY_KEYPAD_SUBTRACT;
        case VK_ADD: return xui::event::KEY_KEYPAD_ADD;
//        case IM_VK_KEYPAD_ENTER: return xui::EVENT::KEY_KEYPAD_ENTER;
        case VK_LSHIFT: return xui::event::KEY_LEFT_SHIFT;
        case VK_LCONTROL: return xui::event::KEY_LEFT_CTRL;
        case VK_LMENU: return xui::event::KEY_LEFT_ALT;
        case VK_LWIN: return xui::event::KEY_LEFT_SUPER;
        case VK_RSHIFT: return xui::event::KEY_RIGHT_SHIFT;
        case VK_RCONTROL: return xui::event::KEY_RIGHT_CTRL;
        case VK_RMENU: return xui::event::KEY_RIGHT_ALT;
        case VK_RWIN: return xui::event::KEY_RIGHT_SUPER;
        case VK_APPS: return xui::event::KEY_MENU;
        case '0': return xui::event::KEY_0;
        case '1': return xui::event::KEY_1;
        case '2': return xui::event::KEY_2;
        case '3': return xui::event::KEY_3;
        case '4': return xui::event::KEY_4;
        case '5': return xui::event::KEY_5;
        case '6': return xui::event::KEY_6;
        case '7': return xui::event::KEY_7;
        case '8': return xui::event::KEY_8;
        case '9': return xui::event::KEY_9;
        case 'A': return xui::event::KEY_A;
        case 'B': return xui::event::KEY_B;
        case 'C': return xui::event::KEY_C;
        case 'D': return xui::event::KEY_D;
        case 'E': return xui::event::KEY_E;
        case 'F': return xui::event::KEY_F;
        case 'G': return xui::event::KEY_G;
        case 'H': return xui::event::KEY_H;
        case 'I': return xui::event::KEY_I;
        case 'J': return xui::event::KEY_J;
        case 'K': return xui::event::KEY_K;
        case 'L': return xui::event::KEY_L;
        case 'M': return xui::event::KEY_M;
        case 'N': return xui::event::KEY_N;
        case 'O': return xui::event::KEY_O;
        case 'P': return xui::event::KEY_P;
        case 'Q': return xui::event::KEY_Q;
        case 'R': return xui::event::KEY_R;
        case 'S': return xui::event::KEY_S;
        case 'T': return xui::event::KEY_T;
        case 'U': return xui::event::KEY_U;
        case 'V': return xui::event::KEY_V;
        case 'W': return xui::event::KEY_W;
        case 'X': return xui::event::KEY_X;
        case 'Y': return xui::event::KEY_Y;
        case 'Z': return xui::event::KEY_Z;
        case VK_F1: return xui::event::KEY_F1;
        case VK_F2: return xui::event::KEY_F2;
        case VK_F3: return xui::event::KEY_F3;
        case VK_F4: return xui::event::KEY_F4;
        case VK_F5: return xui::event::KEY_F5;
        case VK_F6: return xui::event::KEY_F6;
        case VK_F7: return xui::event::KEY_F7;
        case VK_F8: return xui::event::KEY_F8;
        case VK_F9: return xui::event::KEY_F9;
        case VK_F10: return xui::event::KEY_F10;
        case VK_F11: return xui::event::KEY_F11;
        case VK_F12: return xui::event::KEY_F12;
        case VK_F13: return xui::event::KEY_F13;
        case VK_F14: return xui::event::KEY_F14;
        case VK_F15: return xui::event::KEY_F15;
        case VK_F16: return xui::event::KEY_F16;
        case VK_F17: return xui::event::KEY_F17;
        case VK_F18: return xui::event::KEY_F18;
        case VK_F19: return xui::event::KEY_F19;
        case VK_F20: return xui::event::KEY_F20;
        case VK_F21: return xui::event::KEY_F21;
        case VK_F22: return xui::event::KEY_F22;
        case VK_F23: return xui::event::KEY_F23;
        case VK_F24: return xui::event::KEY_F24;
        case VK_BROWSER_BACK: return xui::event::KEY_APP_BACK;
        case VK_BROWSER_FORWARD: return xui::event::KEY_APP_FORWARD;
        }

        return xui::event::EVENT_MAX_COUNT;
    }
}

struct gdi_implement::private_p
{
    HDC _hdc;
    ULONG_PTR _gditoken;
    std::vector<font> _fonts;
    std::vector<window> _windows;
    std::vector<texture> _textures;
    Gdiplus::PrivateFontCollection _collection;
    std::array<Gdiplus::FontFamily, 100> _familys;
};

gdi_implement::gdi_implement()
    : _p( new private_p )
{
}

gdi_implement::~gdi_implement()
{
    delete _p;
}

void gdi_implement::init()
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
}

void gdi_implement::update( const std::function<std::span<xui::drawcmd>()> & paint )
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

        if ( msg.message == WM_QUIT )
        {
            std::cout << "WM_QUIT" << std::endl;
            break;
        }
        else if ( msg.message == WM_DESTROY )
        {
            std::cout << "WM_DESTROY" << std::endl;
            break;
        }
        else if ( msg.message == WM_LBUTTONDOWN )
        {
            SetCapture( msg.hwnd );

            set_event( id, xui::event::KEY_MOUSE_LEFT, 1 );

            std::cout << "WM_LBUTTONDOWN" << std::endl;
        }
        else if ( msg.message == WM_RBUTTONDOWN )
        {
            set_event( id, xui::event::KEY_MOUSE_RIGHT, 1 );

            std::cout << "WM_RBUTTONDOWN" << std::endl;
        }
        else if ( msg.message == WM_MOUSEWHEEL )
        {
            set_wheel( id, { 0.0f, float( int( msg.wParam ) / WHEEL_DELTA ) } );

            std::cout << "WM_MOUSEWHEEL: " << std::dec << ( int( msg.wParam ) / WHEEL_DELTA ) << std::endl;
        }
        else if ( msg.message == WM_LBUTTONUP )
        {
            ReleaseCapture();

            set_event( id, xui::event::KEY_MOUSE_LEFT, 1, xui::action::RELEASE );

            std::cout << "WM_LBUTTONUP" << std::endl;
        }
        else if ( msg.message == WM_RBUTTONUP )
        {
            set_event( id, xui::event::KEY_MOUSE_RIGHT, 1, xui::action::RELEASE );

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

            set_event( id, xui::event::MOUSE_ACTIVE, 1 );

            auto x = (float)(short)LOWORD( msg.lParam );
            auto y = (float)(short)HIWORD( msg.lParam );

            set_cursor( id, { _p->_windows[id].rect.x + x, _p->_windows[id].rect.y + y } );
        }
        else if ( msg.message == WM_MOUSELEAVE )
        {
            set_event( id, xui::event::MOUSE_ACTIVE, 1, xui::action::RELEASE );

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
        else if ( msg.message == WM_TOUCH )
        {
            _p->_windows[id].events._touchs.clear();

            TOUCHINPUT inputs[15];
            UINT count = LOWORD( msg.wParam );

            std::vector<xui::vec2> touchs;
            if ( GetTouchInputInfo( (HTOUCHINPUT)msg.lParam, count, inputs, sizeof( TOUCHINPUT ) ) )
            {
                for ( size_t i = 0; i < count; i++ )
                {
                    touchs.push_back( { (float)inputs[i].x, (float)inputs[i].y } );
                }
            }
            set_touchs( id, touchs );
        }
        else if ( msg.message == WM_TIMER )
        {
            std::cout << "WM_TIMER" << std::endl;
        }
        else if ( msg.message == WM_CLOSE )
        {
            UnregisterTouchWindow( msg.hwnd );
            DestroyWindow( msg.hwnd );

            if ( std::find_if( _p->_windows.begin(), _p->_windows.end(), []( const auto & val )
            {
                return val.hwnd != nullptr;
            } ) == _p->_windows.end() )
            {
                PostQuitMessage( 0 );
            }

            std::cout << "WM_CLOSE" << std::endl;
        }
        else if ( msg.message == WM_KEYDOWN )
        {
            auto key = key_event( msg.wParam );
            if ( key != xui::event::EVENT_MAX_COUNT )
            {
                set_event( id, key, 1 );
            }

            std::cout << "WM_KEYDOWN" << std::endl;
        }
        else if ( msg.message == WM_KEYUP )
        {
            auto key = key_event( msg.wParam );
            if ( key != xui::event::EVENT_MAX_COUNT )
            {
                set_event( id, key, 1, xui::action::RELEASE );
            }

            std::cout << "WM_KEYUP" << std::endl;
        }
        else if ( msg.message == WM_CHAR )
        {
            set_unicode( id, (wchar_t)msg.wParam );

            std::wcout << L"WM_CHAR: \'" << (wchar_t)msg.wParam << L"\'" << std::endl;
        }
        else
        {
            std::cout << "0x" << std::hex << std::setfill( '0' ) << std::setw( 4 ) << msg.message << std::endl;
        }

        TranslateMessage( &msg );

        DispatchMessageA( &msg );

        render( paint() );

        present();

        DefWindowProcA( msg.hwnd, msg.message, msg.wParam, msg.lParam );
    }
}

void gdi_implement::release()
{
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

xui::window_id gdi_implement::create_window( std::string_view title, xui::texture_id icon, const xui::rect & rect, xui::window_id parent )
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
    RegisterTouchWindow( w.hwnd, 0 );

    _p->_windows[id] = w;

    return id;
}

xui::window_id gdi_implement::get_window_parent( xui::window_id id ) const
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

void gdi_implement::set_window_parent( xui::window_id id, xui::window_id parent )
{
    if ( id >= _p->_windows.size() )
        return;

    SetParent( _p->_windows[id].hwnd, parent == xui::invalid_id ? nullptr : _p->_windows[parent].hwnd );
}

xui::window_status gdi_implement::get_window_status( xui::window_id id ) const
{
    int status = 0;

    if ( id < _p->_windows.size() )
    {
        if ( ( ::GetWindowLongA( _p->_windows[id].hwnd, GWL_STYLE ) & WS_VISIBLE ) != 0 )
        {
            status |= xui::window_status::WINDOW_SHOW;

            if ( IsIconic( _p->_windows[id].hwnd ) )
                status |= xui::window_status::WINDOW_MINIMIZE;
            else if ( IsZoomed( _p->_windows[id].hwnd ) )
                status |= xui::window_status::WINDOW_MAXIMIZE;
        }
        else
        {
            status |= xui::window_status::WINDOW_HIDE;
        }
    }

    return xui::window_status( status );
}

void gdi_implement::set_window_status( xui::window_id id, xui::window_status show )
{
    if ( id >= _p->_windows.size() )
        return;

    switch ( show )
    {
    case xui::window_status::WINDOW_SHOW:
        ShowWindow( _p->_windows[id].hwnd, SW_SHOW );
        break;
    case xui::window_status::WINDOW_HIDE:
        ShowWindow( _p->_windows[id].hwnd, SW_HIDE );
        break;
    case xui::window_status::WINDOW_RESTORE:
    {
        ShowWindow( _p->_windows[id].hwnd, SW_RESTORE );
        _p->_windows[id].rect = _p->_windows[id].rrect;
        SetWindowPos( _p->_windows[id].hwnd, HWND_BOTTOM, _p->_windows[id].rect.x, _p->_windows[id].rect.y, _p->_windows[id].rect.w, _p->_windows[id].rect.h, SWP_NOZORDER );
    }
        break;
    case xui::window_status::WINDOW_MINIMIZE:
        ShowWindow( _p->_windows[id].hwnd, SW_MINIMIZE );
        break;
    case xui::window_status::WINDOW_MAXIMIZE:
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
    }
        break;
    }
}

xui::rect gdi_implement::get_window_rect( xui::window_id id ) const
{
    if ( id >= _p->_windows.size() )
        return {};

    return _p->_windows[id].rect;
}

void gdi_implement::set_window_rect( xui::window_id id, const xui::rect & rect )
{
    if ( id >= _p->_windows.size() )
        return;
    
    _p->_windows[id].rect = rect;
    SetWindowPos( _p->_windows[id].hwnd, HWND_BOTTOM, rect.x, rect.y, rect.w, rect.h, SWP_NOZORDER );

    DeleteObject( _p->_windows[id].frame_buffer );
    _p->_windows[id].frame_buffer = CreateBitmap( _p->_windows[id].rect.w, _p->_windows[id].rect.h, 1, 32, nullptr );
}

std::string gdi_implement::get_window_title( xui::window_id id ) const
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

void gdi_implement::set_window_title( xui::window_id id, std::string_view title )
{
    if ( id >= _p->_windows.size() )
        return;

    char buf[512]; memset( buf, 0, 512 );
    strncpy_s( buf, 512, title.data(), title.size() );

    SetWindowTextA( _p->_windows[id].hwnd, buf );
}

void gdi_implement::remove_window( xui::window_id id )
{
    if ( id >= _p->_windows.size() )
        return;

    if ( _p->_windows[id].frame_buffer != nullptr ) DeleteObject( _p->_windows[id].frame_buffer );
    if ( _p->_windows[id].hwnd != nullptr ) PostMessageA( _p->_windows[id].hwnd, WM_CLOSE, 0, 0 );

    _p->_windows[id].hwnd = nullptr;
}

bool gdi_implement::load_font_file( std::string_view filename )
{
    auto path = ansi_wide( filename );
    if ( _p->_collection.AddFontFile( path.c_str() ) == Gdiplus::Status::Ok )
    {
        INT found = 0;
        INT count = min( 100, _p->_collection.GetFamilyCount() );
        return _p->_collection.GetFamilies( count, _p->_familys.data(), &found ) == Gdiplus::Status::Ok;
    }
    return false;
}

xui::font_id gdi_implement::create_font( std::string_view family, int size, xui::font_flag flag )
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

    fnt.size = size;
    fnt.flag = flag;
    fnt.family = family;
    if ( family == system_resource::FONT_DEFAULT )
    {
        fnt.font = new Gdiplus::Font( L"ËÎÌו", size, flag, Gdiplus::UnitPixel );
        _p->_fonts[id] = fnt;
        return id;
    }

    auto wfamily = ansi_wide( family );

    for ( auto & it : _p->_familys )
    {
        if ( it.IsAvailable() )
        {
            wchar_t buf[64];
            memset( buf, 0, sizeof( wchar_t ) * 64 );
            if ( it.GetFamilyName( buf ) == Gdiplus::Ok )
            {
                if ( wfamily == buf && it.IsStyleAvailable( flag ) )
                {
                    fnt.font = new Gdiplus::Font( wfamily.c_str(), size, flag, Gdiplus::UnitPixel, &_p->_collection );
                    _p->_fonts[id] = fnt;
                    return id;
                }
            }
        }
    }

    return xui::invalid_id;
}

xui::size gdi_implement::font_size( xui::font_id id, std::string_view text ) const
{
    Gdiplus::StringFormat fmt;
    fmt.SetAlignment( Gdiplus::StringAlignment::StringAlignmentNear );
    fmt.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentNear );

    Gdiplus::RectF stringRect;
    Gdiplus::RectF layoutRect( 0, 0, 600, 100 );

    auto str = ansi_wide( text );
    Gdiplus::Graphics g( _p->_hdc );
    g.MeasureString( str.c_str(), str.size(), _p->_fonts[id].font, layoutRect, &fmt, &stringRect );

    return { stringRect.Width, stringRect.Height };
}

void gdi_implement::remove_font( xui::font_id id )
{
    if ( id >= _p->_fonts.size() )
        return;

    if ( _p->_fonts[id].font ) delete _p->_fonts[id].font;

    _p->_fonts[id].font = nullptr;
}

xui::texture_id gdi_implement::create_texture( std::string_view filename )
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

        if ( filename == system_resource::ICON_APPLICATION ) icon_id = IDI_APPLICATION;
        else if ( filename == system_resource::ICON_ERROR ) icon_id = IDI_ERROR;
        else if ( filename == system_resource::ICON_WARNING ) icon_id = IDI_WARNING;
        else if ( filename == system_resource::ICON_INFORMATION ) icon_id = IDI_INFORMATION;
        
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

xui::size gdi_implement::texture_size( xui::texture_id id ) const
{
    if ( id >= _p->_textures.size() )
        return {};

    return { (float)_p->_textures[id].image->GetWidth(), (float)_p->_textures[id].image->GetHeight() };
}

void gdi_implement::remove_texture( xui::texture_id id )
{
    if ( id >= _p->_textures.size() )
        return;

    if ( _p->_textures[id].image ) delete _p->_textures[id].image;

    _p->_textures[id].image = nullptr;
}

std::string gdi_implement::get_clipboard_data( xui::window_id id, std::string_view mime ) const
{
    std::string result;

    if ( OpenClipboard( _p->_windows[id].hwnd ) )
    {
        auto fmt = RegisterClipboardFormatA( mime.data() );
        if ( fmt != 0 )
        {
            HANDLE h_data = GetClipboardData( fmt );
            if ( h_data != NULL )
            {
                auto p_data = (const char *)GlobalLock( h_data );
                if ( p_data != nullptr )
                {
                    result = p_data;
                    GlobalUnlock( h_data );
                }
            }
        }
        CloseClipboard();
    }

    return result;
}

bool gdi_implement::set_clipboard_data( xui::window_id id, std::string_view mime, std::string_view data )
{
    bool result = false;

    if ( OpenClipboard( _p->_windows[id].hwnd ) )
    {
        EmptyClipboard();

        auto fmt = RegisterClipboardFormatA( mime.data() );
        if ( fmt != 0 )
        {
            HANDLE h_data = GlobalAlloc( GMEM_MOVEABLE, data.size() + 1 );
            if ( h_data != nullptr )
            {
                auto p_data = (char *)GlobalLock( h_data );
                if ( p_data != nullptr )
                {
                    memset( p_data, 0, data.size() + 1 );
                    memcpy( p_data, data.data(), data.size() );
                    GlobalUnlock( h_data );
                    result = true;
                }
                SetClipboardData( fmt, h_data );
            }
        }
        CloseClipboard();
    }

    return result;
}

xui::vec2 gdi_implement::get_cursor_dt( xui::window_id id ) const
{
    return _p->_windows[id].events.dt();
}

xui::vec2 gdi_implement::get_cursor_pos( xui::window_id id ) const
{
    auto pos = _p->_windows[id].events.pos();
    return pos - xui::vec2{ _p->_windows[id].rect.x, _p->_windows[id].rect.y };
}

xui::vec2 gdi_implement::get_cusor_wheel( xui::window_id id ) const
{
    return _p->_windows[id].events.wheel();
}

std::string gdi_implement::get_unicodes( xui::window_id id ) const
{
    return wide_ansi( _p->_windows[id].events._unicodes );
}

int gdi_implement::get_event( xui::window_id id, xui::event key ) const
{
    return _p->_windows[id].events[(size_t)key];
}

std::span<xui::vec2> gdi_implement::get_touchs( xui::window_id id ) const
{
    return _p->_windows[id].events._touchs;
}

void gdi_implement::present()
{
    for ( auto & it : _p->_windows )
    {
        HGDIOBJ old_bitmap = SelectObject( _p->_hdc, (HGDIOBJ)it.frame_buffer );
        {
            POINT point = { 0, 0 };
            SIZE size = { it.rect.w, it.rect.h };

            BLENDFUNCTION blend = {};
            blend.BlendOp = AC_SRC_OVER;
            blend.BlendFlags = 0;
            blend.SourceConstantAlpha = 255;
            blend.AlphaFormat = AC_SRC_ALPHA;

            UpdateLayeredWindow( it.hwnd, nullptr, nullptr, &size, _p->_hdc, &point, 0, &blend, ULW_ALPHA );
        }
        SelectObject( _p->_hdc, old_bitmap );
    }
}

void gdi_implement::render( std::span<xui::drawcmd> cmds )
{
    HGDIOBJ old_obj = nullptr;
    xui::window_id id = xui::invalid_id;

    for ( const auto & cmd : cmds )
    {
        if ( id != cmd.id )
        {
            id = cmd.id;

            if( old_obj != nullptr ) SelectObject( _p->_hdc, old_obj );

            old_obj = SelectObject( _p->_hdc, (HGDIOBJ)_p->_windows[cmd.id].frame_buffer );
        }

        if ( cmd.id < _p->_windows.size() && _p->_windows[cmd.id].hwnd != nullptr )
        {
            std::visit( xui::overload(
            [&]( std::monostate )
            {

            },
            [&]( const xui::drawcmd::text_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                Gdiplus::SolidBrush brush( Gdiplus::Color( element.color.a, element.color.r, element.color.g, element.color.b ) );

                auto wtext = ansi_wide( element.text );

                Gdiplus::StringFormat fmt;

                if ( ( element.align & xui::alignment_flag::ALIGN_LEFT ) != 0 )
                    fmt.SetAlignment( Gdiplus::StringAlignment::StringAlignmentNear );
                else if ( ( element.align & xui::alignment_flag::ALIGN_RIGHT ) != 0 )
                    fmt.SetAlignment( Gdiplus::StringAlignment::StringAlignmentFar );
                if ( ( element.align & xui::alignment_flag::ALIGN_HCENTER ) != 0 )
                    fmt.SetAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );

                if ( ( element.align & xui::alignment_flag::ALIGN_TOP ) != 0 )
                    fmt.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentNear );
                else if ( ( element.align & xui::alignment_flag::ALIGN_BOTTOM ) != 0 )
                    fmt.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentFar );
                if ( ( element.align & xui::alignment_flag::ALIGN_VCENTER ) != 0 )
                    fmt.SetLineAlignment( Gdiplus::StringAlignment::StringAlignmentCenter );

                g.DrawString( wtext.c_str(), wtext.size(), _p->_fonts[element.font].font, Gdiplus::RectF( element.rect.x, element.rect.y, element.rect.w, element.rect.h ), &fmt, &brush );
            },
            [&]( const xui::drawcmd::line_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                auto pen = create_pen( element.stroke );
                g.DrawLine( pen.get(), Gdiplus::PointF{ element.p1.x, element.p1.y }, Gdiplus::PointF{ element.p2.x, element.p2.y } );
            },
            [&]( const xui::drawcmd::rect_element & element )
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

                if ( element.filled.colors.index() != 0 )
                {
                    g.FillPath( create_brush( element.filled ).get(), &path );
                }

                g.DrawPath( create_pen( element.border ).get(), &path );
            },
            [&]( const xui::drawcmd::path_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                Gdiplus::GraphicsPath path;

                auto it = element.data.begin();
                auto f = []( auto & it )
                {
                    auto beg = it;
                    if ( *it == '-' || *it == '+' ) it++;
                    while ( std::isdigit( *it ) || *it == '.' ) it++;

                    float value = 0;
                    std::from_chars( beg.operator->(), it.operator->(), value );
                    return value;
                };
                auto ppoint = [f]( auto & it ) -> xui::vec2
                {
                    xui::vec2 p;

                    p.x = f( it );
                    it++;
                    p.y = f( it );

                    if ( *it == ' ' )
                        it++;

                    return p;
                };
                xui::vec2 m, c;

                while ( it != element.data.end() )
                {
                    if ( element.data.find( '-' ) != std::string::npos )
                    {
                        int i = 0;
                    }
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
                    {
                        it++;

                        auto c1 = ppoint( it );
                        auto c2 = ppoint( it );
                        auto e = ppoint( it );

                        Gdiplus::PointF points[4];
                        points[0] = { m.x, m.y };
                        points[1] = { c1.x, c1.y };
                        points[2] = { c2.x, c2.y };
                        points[3] = { e.x, e.y };

                        path.AddBeziers( points, 4 );

                        m = e;
                        c = c2;
                    }
                        break;
                    case 'S':
                    {
                        it++;

                        auto c1 = c;
                        auto c2 = ppoint( it );
                        auto e = ppoint( it );

                        Gdiplus::PointF points[4];
                        points[0] = { m.x, m.y };
                        points[1] = { c1.x, c1.y };
                        points[2] = { c2.x, c2.y };
                        points[3] = { e.x, e.y };

                        path.AddBeziers( points, 4 );

                        m = e;
                        c = {};
                    }
                        break;
                    case 'Q':
                    {
                        it++;

                        c = ppoint( it );
                        auto e = ppoint( it );

                        Gdiplus::PointF points[4];
                        points[0] = { m.x, m.y };
                        points[1] = { ( m.x + c.x ) / 2, ( m.y + c.y ) / 2 };
                        points[2] = { ( e.x + c.x ) / 2, ( e.y + c.y ) / 2 };
                        points[3] = { e.x, e.y };

                        path.AddBeziers( points, 4 );

                        m = e;
                    }
                        break;
                    case 'T':
                    {
                        it++;

                        auto e = ppoint( it );

                        Gdiplus::PointF points[4];
                        points[0] = { m.x, m.y };
                        points[1] = { ( m.x + c.x ) / 2, ( m.y + c.y ) / 2 };
                        points[2] = { ( e.x + c.x ) / 2, ( e.y + c.y ) / 2 };
                        points[3] = { e.x, e.y };

                        path.AddBeziers( points, 4 );

                        m = e;
                        c = {};
                    }
                        break;
                    case 'Z':
                    {
                        it++;
                        m = {};
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

                if ( element.filled.colors.index() != 0 )
                {
                    g.FillPath( create_brush( element.filled ).get(), &path );
                }

                g.DrawPath( create_pen( element.stroke ).get(), &path );
            },
            [&]( const xui::drawcmd::image_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                g.DrawImage( _p->_textures[element.id].image, Gdiplus::RectF( element.rect.x, element.rect.y, element.rect.w, element.rect.h ) );
            },
            [&]( const xui::drawcmd::circle_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                if ( element.filled.colors.index() != 0 )
                {
                    g.FillEllipse( create_brush( element.filled ).get(), element.center.x - element.radius, element.center.y - element.radius, element.radius * 2, element.radius * 2 );
                }

                g.DrawEllipse( create_pen( element.border ).get(), element.center.x - element.radius, element.center.y - element.radius, element.radius * 2, element.radius * 2 );
            },
            [&]( const xui::drawcmd::ellipse_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                if ( element.filled.colors.index() != 0 )
                {
                    g.FillEllipse( create_brush( element.filled ).get(), element.center.x - element.radius.x, element.center.y - element.radius.y, element.radius.x * 2, element.radius.y * 2 );
                }

                g.DrawEllipse( create_pen( element.border ).get(), element.center.x - element.radius.x, element.center.y - element.radius.y, element.radius.x * 2, element.radius.y * 2 );
            },
            [&]( const xui::drawcmd::polygon_element & element )
            {
                Gdiplus::Graphics g( _p->_hdc );
                g.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                std::vector<Gdiplus::PointF> points;
                for ( const auto & it : element.points )
                {
                    points.push_back( { it.x, it.y } );
                }

                if ( element.filled.colors.index() != 0 )
                {
                    g.FillPolygon( create_brush( element.filled ).get(), points.data(), points.size() );
                }

                g.DrawPolygon( create_pen( element.border ).get(), points.data(), points.size() );
            }
            ), cmd.element );
        }
    }
    
    if ( old_obj != nullptr )
        SelectObject( _p->_hdc, old_obj );
}

std::shared_ptr<Gdiplus::Pen> gdi_implement::create_pen( const xui::stroke & stroke ) const
{
    Gdiplus::DashStyle style = Gdiplus::DashStyleSolid;

    switch ( stroke.style )
    {
    case xui::stroke::SOLID: style = Gdiplus::DashStyleSolid; break;
    case xui::stroke::DASHED: style = Gdiplus::DashStyleDash; break;
    case xui::stroke::DOTTED: style = Gdiplus::DashStyleDot; break;
    case xui::stroke::DASH_DOT: style = Gdiplus::DashStyleDashDot; break;
    case xui::stroke::DASH_DOT_DOT: style = Gdiplus::DashStyleDashDotDot; break;
    default:
        break;
    }
    
    auto pen = std::make_shared<Gdiplus::Pen>( Gdiplus::Color( stroke.color.a, stroke.color.r, stroke.color.g, stroke.color.b ), stroke.width );
    pen->SetDashStyle( style );
    return pen;
}

std::shared_ptr<Gdiplus::Brush> gdi_implement::create_brush( const xui::filled & filled ) const
{
    switch ( filled.style )
    {
    case xui::filled::SOLID:
        return std::make_shared<Gdiplus::SolidBrush>( Gdiplus::Color{ std::get<xui::color>( filled.colors ).a, std::get<xui::color>( filled.colors ).r, std::get<xui::color>( filled.colors ).g, std::get<xui::color>( filled.colors ).b } );
    case xui::filled::DENSE1:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyle10Percent, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::DENSE2:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyle20Percent, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::DENSE3:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyle30Percent, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::DENSE4:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyle40Percent, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::DENSE5:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyle50Percent, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::DENSE6:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyle60Percent, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::DENSE7:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyle70Percent, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::HORIZONTAL:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyleHorizontal, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::VERTICAL:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyleVertical, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::CROSS:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyleCross, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::FORWARD:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyleForwardDiagonal, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::BACKWARD:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyleBackwardDiagonal, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::DIAGCROSS:
        return std::make_shared<Gdiplus::HatchBrush>( Gdiplus::HatchStyleDiagonalCross, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).fore.a, std::get<xui::hatch_color>( filled.colors ).fore.r, std::get<xui::hatch_color>( filled.colors ).fore.g, std::get<xui::hatch_color>( filled.colors ).fore.b }, Gdiplus::Color{ std::get<xui::hatch_color>( filled.colors ).back.a, std::get<xui::hatch_color>( filled.colors ).back.r, std::get<xui::hatch_color>( filled.colors ).back.g, std::get<xui::hatch_color>( filled.colors ).back.b } );
    case xui::filled::LINEAR_GRADIENT:
        return std::make_shared<Gdiplus::LinearGradientBrush>( Gdiplus::PointF{ std::get<xui::linear_gradient>( filled.colors ).p1.x,std::get<xui::linear_gradient>( filled.colors ).p1.y }, Gdiplus::PointF{ std::get<xui::linear_gradient>( filled.colors ).p2.x,std::get<xui::linear_gradient>( filled.colors ).p2.y }, Gdiplus::Color{ std::get<xui::linear_gradient>( filled.colors ).c1.a, std::get<xui::linear_gradient>( filled.colors ).c1.r, std::get<xui::linear_gradient>( filled.colors ).c1.g, std::get<xui::linear_gradient>( filled.colors ).c1.b }, Gdiplus::Color{ std::get<xui::linear_gradient>( filled.colors ).c2.a, std::get<xui::linear_gradient>( filled.colors ).c2.r, std::get<xui::linear_gradient>( filled.colors ).c2.g, std::get<xui::linear_gradient>( filled.colors ).c2.b } );
    case xui::filled::TEXTURE:
    {
        auto it = std::find_if( _p->_textures.begin(), _p->_textures.end(), [&]( const auto & val ) { return val.name == std::get<xui::texture_brush>( filled.colors ).image; } );
        if ( it != _p->_textures.end() )
        {
            return std::make_shared<Gdiplus::TextureBrush>( it->image, (Gdiplus::WrapMode)std::get<xui::texture_brush>( filled.colors ).mode );
        }
    }
    }

    return nullptr;
}

void gdi_implement::set_unicode( xui::window_id id, wchar_t unicode )
{
    _p->_windows[id].events._unicodes.push_back( unicode );
}

void gdi_implement::set_wheel( xui::window_id id, const xui::vec2 & dt )
{
    _p->_windows[id].events._cursorwheel = dt;
}

void gdi_implement::set_cursor( xui::window_id id, const xui::vec2 & pos )
{
    _p->_windows[id].events._cursorold = _p->_windows[id].events._cursorpos;
    _p->_windows[id].events._cursorpos = pos;
}

void gdi_implement::set_touchs( xui::window_id id, std::span<xui::vec2> touchs )
{
    _p->_windows[id].events._touchs.assign( touchs.begin(), touchs.end() );
}

void gdi_implement::set_event( xui::window_id id, xui::event key, int val, xui::action act )
{
    if ( id != xui::invalid_id )
    {
        if ( key >= xui::event::MOUSE_EVENT_BEG && key <= xui::event::MOUSE_EVENT_END )
        {
            if ( key == xui::event::MOUSE_ACTIVE && act == xui::action::RELEASE )
            {
                _p->_windows[id].events._cursorpos = {};
                _p->_windows[id].events._cursorold = {};
                _p->_windows[id].events._cursorwheel = {};

                std::fill( _p->_windows[id].events._events.begin() + (size_t)xui::event::MOUSE_EVENT_BEG, _p->_windows[id].events._events.begin() + (size_t)xui::event::MOUSE_EVENT_END + 1, 0 );
            }

            if ( act == xui::action::RELEASE )
                _p->_windows[id].events._events[(size_t)key] = 0;
            else
                _p->_windows[id].events._events[(size_t)key] = 1;
        }
        else if ( key >= xui::event::KEY_EVENT_BEG && key <= xui::event::KEY_EVENT_END )
        {
            _p->_windows[id].events._events[(size_t)key] = 1;
        }
    }
}
