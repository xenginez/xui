#include "xui.h"

#include <map>
#include <span>
#include <array>
#include <deque>
#include <regex>
#include <memory>
#include <format>
#include <algorithm>

#define XUI_ERR( CODE )  if ( _p->_error ) _p->_error( this, xui_category::make_error_code( xui::err::CODE ) );
#define XUI_SCALE( VAL ) ( VAL * _p->_factor )

template<> struct std::formatter< std::span<std::string_view, std::dynamic_extent>, char > : public std::formatter<std::string_view>
{
    template<class FormatContext> auto format( const std::span<std::string_view> & p, FormatContext & context ) const
    {
        auto result = context.out();
        for ( auto it : p )
        {
            result = std::format_to( result, "{}", it );
        }
        return result;
    }
};

namespace
{
    static std::regex int_regex{ R"([-+]?([0-9]*[0-9]+))" };
    static std::regex flt_regex{ R"([-+]?([0-9]*.[0-9]+|[0-9]+))" };
    static std::regex func_regex{ R"(\S*\([\s\S]*\))" };

    static std::map<std::string_view, int> style_enums =
    {
        // stroke style
        { "none", xui::drawcmd::stroke::NONE },
        { "solid", xui::drawcmd::stroke::SOLID },
        { "dashed", xui::drawcmd::stroke::DASHED },
        { "dotted", xui::drawcmd::stroke::DOTTED },
        { "dashdot", xui::drawcmd::stroke::DASH_DOT },
        { "dashdotdot", xui::drawcmd::stroke::DASH_DOT_DOT },

        // alignment
        { "left"	, xui::alignment_flag::LEFT },
        { "right"	, xui::alignment_flag::RIGHT },
        { "top"		, xui::alignment_flag::TOP },
        { "bottom"	, xui::alignment_flag::BOTTOM },
        { "center"	, xui::alignment_flag::CENTER },
        { "vcenter"	, xui::alignment_flag::VCENTER },
        { "hcenter"	, xui::alignment_flag::HCENTER },
        { "left top"	, xui::alignment_flag::LEFT | xui::alignment_flag::TOP },
        { "top left"	, xui::alignment_flag::LEFT | xui::alignment_flag::TOP },
        { "left bottom"	, xui::alignment_flag::LEFT | xui::alignment_flag::BOTTOM },
        { "bottom left"	, xui::alignment_flag::LEFT | xui::alignment_flag::BOTTOM },
        { "right top"	, xui::alignment_flag::RIGHT | xui::alignment_flag::TOP },
        { "top right"	, xui::alignment_flag::RIGHT | xui::alignment_flag::TOP },
        { "right bottom", xui::alignment_flag::RIGHT | xui::alignment_flag::BOTTOM },
        { "bottom right", xui::alignment_flag::RIGHT | xui::alignment_flag::BOTTOM },
        { "left vcenter", xui::alignment_flag::LEFT | xui::alignment_flag::VCENTER },
        { "vcenter left", xui::alignment_flag::LEFT | xui::alignment_flag::VCENTER },
        { "right vcenter", xui::alignment_flag::LEFT | xui::alignment_flag::VCENTER },
        { "vcenter right", xui::alignment_flag::LEFT | xui::alignment_flag::VCENTER },
        { "top hcenter", xui::alignment_flag::TOP | xui::alignment_flag::HCENTER },
        { "hcenter top", xui::alignment_flag::TOP | xui::alignment_flag::HCENTER },
        { "bottom hcenter", xui::alignment_flag::BOTTOM | xui::alignment_flag::HCENTER },
        { "hcenter bottom", xui::alignment_flag::BOTTOM | xui::alignment_flag::HCENTER },
        { "vcenter hcenter"	, xui::alignment_flag::CENTER },
        { "hcenter vcenter"	, xui::alignment_flag::CENTER },

        // direction
        { "left right", xui::direction::LEFT_RIGHT },
        { "right left", xui::direction::RIGHT_LEFT },
        { "top bottom", xui::direction::TOP_BOTTOM },
        { "bottom top", xui::direction::BOTTOM_TOP },
    };
    static std::map<std::string_view, std::uint32_t> style_colors =
    {
        {"transparent", 0x00000000 },
        { "maroon", 0x800000FF },
        { "darkred", 0x8B0000FF },
        { "brown", 0xA52A2AFF },
        { "firebrick", 0xB22222FF },
        { "crimson", 0xDC143CFF },
        { "red", 0xFF0000FF },
        { "mediumvioletred", 0xC71585FF },
        { "palevioletred", 0xD87093FF },
        { "deeppink", 0xFF1493FF },
        { "fuchsia", 0xFF00FFFF },
        { "magenta", 0xFF00FFFF },
        { "hotpink", 0xFF69B4FF },
        { "pink", 0xFFC0CBFF },
        { "lightpink", 0xFFB6C1FF },
        { "mistyrose", 0xFFE4E1FF },
        { "lavenderblush", 0xFFF0F5FF },
        { "indigo", 0x4B0082FF },
        { "purple", 0x800080FF },
        { "darkmagenta", 0x8B008BFF },
        { "darkorchid", 0x9932CCFF },
        { "blue", 0x0000FFFF },
        { "blueviolet", 0x8A2BE2FF },
        { "darkviolet", 0x9400D3FF },
        { "slateblue", 0x6A5ACDFF },
        { "mediumpurple", 0x9370DBFF },
        { "mediumslateblue", 0x7B68EEFF },
        { "mediumorchid", 0xBA55D3FF },
        { "violet", 0xEE82EEFF },
        { "plum", 0xDDA0DDFF },
        { "thistle", 0xD8BFD8FF },
        { "lavender", 0xE6E6FAFF },
        { "saddlebrown", 0x8B4513FF },
        { "sienna", 0xA0522DFF },
        { "chocolate", 0xD2691EFF },
        { "indianred", 0xCD5C5CFF },
        { "rosybrown", 0xBC8F8FFF },
        { "lightcorol", 0xF08080FF },
        { "salmon", 0xFA8072FF },
        { "lightsalmon", 0xFFA07AFF },
        { "orangered", 0xFF4500FF },
        { "tomato", 0xFF6347FF },
        { "coral", 0xFF7F50FF },
        { "darkorange", 0xFF8C00FF },
        { "sandybrown", 0xF4A460FF },
        { "peru", 0xCD853FFF },
        { "tan", 0xD2B48CFF },
        { "burlywood", 0xDEB887FF },
        { "wheat", 0xF5DEB3FF },
        { "moccasin", 0xFFE4B5FF },
        { "navajowhite", 0xFFDEADFF },
        { "peachpuff", 0xFFDAB9FF },
        { "bisque", 0xFFE4C4FF },
        { "antuquewhite", 0xFAEBD7FF },
        { "papayawhip", 0xFFEFD5FF },
        { "cornsilk", 0xFFF8DCFF },
        { "oldlace", 0xFDF5E6FF },
        { "linen", 0xFAF0E6FF },
        { "seashell", 0xFFF5EEFF },
        { "snow", 0xFFFAFAFF },
        { "floralwhite", 0xFFFAF0FF },
        { "ivory", 0xFFFFF0FF },
        { "mintcream", 0xF5FFFAFF },
        { "darkgoldenrod", 0xB8860BFF },
        { "goldenrod", 0xDAA520FF },
        { "gold", 0xFFD700FF },
        { "yellow", 0xFFFF00FF },
        { "darkkhaki", 0xBDB76BFF },
        { "khaki", 0xF0E68CFF },
        { "palegoldenrod", 0xEEE8AAFF },
        { "beige", 0xF5F5DCFF },
        { "lemonchiffon", 0xFFFACDFF },
        { "lightgoldenrodyellow", 0xFAFAD2FF },
        { "lightyellow", 0xFFFFE0FF },
        { "darkslategray", 0x2F4F4FFF },
        { "darkolivegreen", 0x556B2FFF },
        { "olive", 0x808000FF },
        { "darkgreen", 0x006400FF },
        { "forestgreen", 0x228B22FF },
        { "seagreen", 0x2E8B57FF },
        { "green", 0x008080FF },
        { "teal", 0x008080FF },
        { "lightseagreen", 0x20B2AAFF },
        { "madiumaquamarine", 0x66CDAAFF },
        { "mediumseagreen", 0x3CB371FF },
        { "darkseagreen", 0x8FBC8FFF },
        { "yellowgreen", 0x9ACD32FF },
        { "limegreen", 0x32CD32FF },
        { "lime", 0x00FF00FF },
        { "chartreuse", 0x7FFF00FF },
        { "lawngreen", 0x7CFC00FF },
        { "greenyellow", 0xADFF2FFF },
        { "mediumspringgreen", 0x00FA9AFF },
        { "springgreen", 0x00FF7FFF },
        { "lightgreen", 0x90EE90FF },
        { "palegreen", 0x98F898FF },
        { "aquamarine", 0x7FFFD4FF },
        { "honeydew", 0xF0FFF0FF },
        { "midnightblue", 0x191970FF },
        { "navy", 0x000080FF },
        { "darkblue", 0x00008BFF },
        { "darkslateblue", 0x483D8BFF },
        { "mediumblue", 0x0000CDFF },
        { "royalblue", 0x4169E1FF },
        { "dodgerblue", 0x1E90FFFF },
        { "cornflowerblue", 0x6495EDFF },
        { "deepskyblue", 0x00BFFFFF },
        { "lightskyblue", 0x87CEFAFF },
        { "lightsteelblue", 0xB0C4DEFF },
        { "lightblue", 0xADD8E6FF },
        { "steelblue", 0x4682B4FF },
        { "darkcyan", 0x008B8BFF },
        { "cadetblue", 0x5F9EA0FF },
        { "darkturquoise", 0x00CED1FF },
        { "mediumturquoise", 0x48D1CCFF },
        { "turquoise", 0x40E0D0FF },
        { "skyblue", 0x87CECBFF },
        { "powderblue", 0xB0E0E6FF },
        { "paleturquoise", 0xAFEEEEFF },
        { "lightcyan", 0xE0FFFFFF },
        { "azure", 0xF0FFFFFF },
        { "aliceblue", 0xF0F8FFFF },
        { "aqua", 0x00FFFFFF },
        { "cyan", 0x00FFFFFF },
        { "black", 0x000000FF },
        { "dimgray", 0x696969FF },
        { "gray", 0x808080FF },
        { "slategray", 0x708090FF },
        { "lightslategray", 0x778899FF },
        { "darkgray", 0xA9A9A9FF },
        { "silver", 0xC0C0C0FF },
        { "lightgray", 0xD3D3D3FF },
        { "gainsboro", 0xDCDCDCFF },
        { "whitesmoke", 0xF5F5F5FF },
        { "ghostwhite", 0xF8F8FFFF },
        { "white", 0xFFFFFFFF }
    };
    static std::map<std::string_view, std::function<xui::style::variant( std::string_view )>> style_functions =
    {
        {"url", xui::style::parse_url },
        {"rgb", xui::style::parse_rgb },
        {"rgba", xui::style::parse_rgba },
        {"vec4", xui::style::parse_vec4 },
    };

    class xui_category : public std::error_category
    {
    public:
        const char * name() const noexcept override
        {
            return "xui::category";
        }
        std::string message( int value ) const override
        {


            return "";
        }

        template<typename T > static std::error_code make_error_code( T code )
        {
            static xui_category category;

            return std::error_code( (int)code, category );
        }
    };
}

bool xui::rect::contains( const xui::point & p ) const
{
    return ( p.x > x && p.x < ( x + w ) ) && ( p.y > y && p.y < ( y + h ) );
}
xui::rect xui::rect::margins_added( float left, float right, float top, float bottom ) const
{
    return { x - left, y - top, w + left + right, h + top + bottom };
}

xui::color::color()
    : r( 0 ), g( 0 ), b( 0 ), a( 255 )
{
}
xui::color::color( std::uint32_t hex )
{
    r = ( hex & 0xFF000000 ) >> 24;
    g = ( hex & 0x00FF0000 ) >> 16;
    b = ( hex & 0x0000FF00 ) >> 8;
    a = ( hex & 0x000000FF );
}
xui::color::color( std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a )
    : r( r ), g( g ), b( b ), a( a )
{
}
xui::color xui::color::lerp( const xui::color & target, float t ) const
{
    return {
        static_cast<std::uint8_t>( std::lerp( r, target.r, t ) ),
        static_cast<std::uint8_t>( std::lerp( g, target.g, t ) ),
        static_cast<std::uint8_t>( std::lerp( b, target.b, t ) ),
        static_cast<std::uint8_t>( std::lerp( a, target.a, t ) )
    };
}

xui::style::style( std::pmr::memory_resource * res )
    : _selectors( res )
{

}
void xui::style::load( std::string_view str )
{
    auto it = str.begin();

    skip( it, str.end() );

    while ( it != str.end() )
    {
        auto beg = it;
        auto end = adv<'{'>( it, str.end() );

        selector select;
        select.name = { beg, end };

        if ( check<'{'>( it, str.end() ) )
        {
            while ( !check<'}'>( it, str.end() ) )
            {
                beg = it;
                end = adv<':'>( it, str.end() );

                std::string name( beg, end );

                check<':'>( it, str.end() );

                beg = it;
                end = adv<';'>( it, str.end() );

                std::string val( beg, end );

                if ( val.find( "#" ) == 0 )
                {
                    xui::color col;

                    auto beg = val.begin() + 1;
                    auto end = beg + std::min<std::size_t>( 2, std::distance( beg, val.end() ) );

                    if ( end != val.end() )
                    {
                        col.r = std::stoi( std::string( beg, end ), nullptr, 16 );
                    }

                    if ( end != val.end() )
                    {
                        beg = end;
                        end = beg + std::min<std::size_t>( 2, std::distance( beg, val.end() ) );
                        col.g = std::stoi( std::string( beg, end ), nullptr, 16 );
                    }

                    if ( end != val.end() )
                    {
                        beg = end;
                        end = beg + std::min<std::size_t>( 2, std::distance( beg, val.end() ) );
                        col.b = std::stoi( std::string( beg, end ), nullptr, 16 );
                    }

                    if ( end != val.end() )
                    {
                        beg = end;
                        end = beg + std::min<std::size_t>( 2, std::distance( beg, val.end() ) );
                        col.a = std::stoi( std::string( beg, end ), nullptr, 16 );
                    }

                    select.attrs.insert( { name, col } );
                }
                else if ( std::regex_match( val, int_regex ) )
                {
                    select.attrs.insert( { name, std::stoi( val ) } );
                }
                else if ( std::regex_match( val, flt_regex ) )
                {
                    select.attrs.insert( { name, std::stof( val ) } );
                }
                else if ( std::regex_match( val, func_regex ) )
                {
                    auto it = style_functions.find( { val.begin(), val.begin() + val.find( '(' ) } );
                    if ( it != style_functions.end() )
                    {
                        auto beg = val.begin();
                        select.attrs.insert( { name, it->second( { adv<'('>( beg, val.end() ) + 1, val.end() } ) } );
                    }
                    else
                    {
                        select.attrs.insert( { name, val } );
                    }
                }
                else if ( style_enums.find( val ) != style_enums.end() )
                {
                    select.attrs.insert( { name, style_enums[val] } );
                }
                else if ( style_colors.find( val ) != style_colors.end() )
                {
                    select.attrs.insert( { name, xui::color( style_colors[val] ) } );
                }
                else
                {
                    select.attrs.insert( { name, val } );
                }

                check<';'>( it, str.end() );
            }
        }

        check<','>( it, str.end() );

        _selectors.insert( { select.name, select } );
    }

    int i = 0;
}
xui::style::variant xui::style::find( std::string_view name ) const
{
    // {type}-{element}-{element}-{element}:{action}#{id}@{attr}
    std::string_view type, action, id, attr;
    std::pmr::vector<std::string_view> elements( _selectors.get_allocator().resource() );

    // {attr}
    if ( name.find( '@' ) != std::string_view::npos )
    {
        attr = { name.begin() + name.find( '@' ) + 1, name.end() };
        name = { name.begin(), name.begin() + name.find( '@' ) };
    }

    // {id}
    if ( name.find( '#' ) != std::string_view::npos )
    {
        id = { name.begin() + name.find( '#' ), name.end() };
        name = { name.begin(), name.begin() + name.find( '#' ) };
    }

    // {action}
    if ( name.find( ':' ) != std::string_view::npos )
    {
        action = { name.begin() + name.find( ':' ), name.end() };
        name = { name.begin(), name.begin() + name.find( ':' ) };
    }

    // {element}
    while ( name.find( '-' ) != std::string_view::npos )
    {
        elements.insert( elements.begin(), { name.begin() + name.find_last_of( '-' ), name.end() } );
        name = { name.begin(), name.begin() + name.find_last_of( '-' ) };
    }

    // {type}
    if ( !name.empty() )
    {
        type = name;
    }

    // {type}-{element}-{element}-{element}:{action}#{id}@{attr}
    auto it = _selectors.find( std::format( "{}{}{}{}", type, std::span<std::string_view>{ elements }, action, id ) );
    if ( it != _selectors.end() )
    {
        auto it2 = it->second.attrs.find( { attr.begin(), attr.end() } );
        if ( it2 != it->second.attrs.end() )
            return it2->second;
    }

    // {type}-{element}-{element}-{element}:{action}@{attr}
    it = _selectors.find( std::format( "{}{}{}", type, std::span<std::string_view>{ elements }, action ) );
    if ( it != _selectors.end() )
    {
        auto it2 = it->second.attrs.find( { attr.begin(), attr.end() } );
        if ( it2 != it->second.attrs.end() )
            return it2->second;
    }

    // {type}-{element}-{element}-{element}@{attr}
    it = _selectors.find( std::format( "{}{}", type, std::span<std::string_view>{ elements } ) );
    if ( it != _selectors.end() )
    {
        auto it2 = it->second.attrs.find( { attr.begin(), attr.end() } );
        if ( it2 != it->second.attrs.end() )
            return it2->second;
    }

    // {type}-{element}-{element}-{element}@{attr}
    // {type}-{element}-{element}@{attr}
    // {type}-{element}@{attr}
    if ( !elements.empty() )
    {
        auto beg = elements.begin();
        auto end = elements.end();

        while ( beg != end )
        {
            it = _selectors.find( std::format( "{}{}", type, std::span<std::string_view>{ beg, end } ) );
            if ( it != _selectors.end() )
            {
                auto it2 = it->second.attrs.find( { attr.begin(), attr.end() } );
                if ( it2 != it->second.attrs.end() )
                    return it2->second;
            }

            --end;
        }
    }

    // {type}@{attr}
    it = _selectors.find( { type.begin(), type.end() } );
    if ( it != _selectors.end() )
    {
        auto it2 = it->second.attrs.find( { attr.begin(), attr.end() } );
        if ( it2 != it->second.attrs.end() )
            return it2->second;
    }

    // *@{attr}
    it = _selectors.find( "*" );
    if ( it != _selectors.end() )
    {
        auto it2 = it->second.attrs.find( { attr.begin(), attr.end() } );
        if ( it2 != it->second.attrs.end() )
            return it2->second;
    }

    return {};
}

xui::style::url xui::style::parse_url( std::string_view val )
{
    return url( val );
}

xui::vec4 xui::style::parse_vec4( std::string_view val )
{
    xui::vec4 result;

    auto beg = val.begin();
    auto r_beg = beg;
    auto r_end = adv<','>( beg, val.end() );
    result.x = std::stoi( std::string( r_beg, r_end ) );

    auto g_beg = r_end + 1; beg = g_beg;
    auto g_end = adv<','>( beg, val.end() );
    result.y = std::stoi( std::string( g_beg, g_end ) );

    auto b_beg = g_end + 1; beg = b_beg;
    auto b_end = adv<','>( beg, val.end() );
    result.z = std::stoi( std::string( b_beg, b_end ) );

    auto a_beg = b_end + 1; beg = a_beg;
    auto a_end = val.end();
    result.w = std::stoi( std::string( a_beg, a_end ) );

    return result;
}

xui::color xui::style::parse_rgb( std::string_view val )
{
    xui::color result;

    auto beg = val.begin();
    auto r_beg = beg;
    auto r_end = adv<','>( beg, val.end() );
    result.r = std::stoi( std::string( r_beg, r_end ) );

    auto g_beg = r_end + 1; beg = g_beg;
    auto g_end = adv<','>( beg, val.end() );
    result.g = std::stoi( std::string( g_beg, g_end ) );

    auto b_beg = g_end + 1; beg = b_beg;
    auto b_end = val.end();
    result.b = std::stoi( std::string( b_beg, b_end ) );

    return result;
}

xui::color xui::style::parse_rgba( std::string_view val )
{
    xui::color result;

    auto beg = val.begin();
    auto r_beg = beg;
    auto r_end = adv<','>( beg, val.end() );
    result.r = std::stoi( std::string( r_beg, r_end ) );

    auto g_beg = r_end + 1; beg = g_beg;
    auto g_end = adv<','>( beg, val.end() );
    result.g = std::stoi( std::string( g_beg, g_end ) );

    auto b_beg = g_end + 1; beg = b_beg;
    auto b_end = adv<','>( beg, val.end() );
    result.b = std::stoi( std::string( b_beg, b_end ) );

    auto a_beg = b_end + 1; beg = a_beg;
    auto a_end = val.end();
    result.a = std::stoi( std::string( a_beg, a_end ) );

    return result;
}

struct xui::context::private_p
{
public:
    private_p( std::pmr::memory_resource * res )
        : _res( res )
        , _commands( res )
        , _rects( res )
        , _types( res )
        , _fonts( res )
        , _ids( res )
        , _styles( res )
        , _windows( res )
        , _textures( res )
        , _windowflags( res )
    {
    }

public:
    float _factor = 1.0f;
    xui::implement * _impl = nullptr;
    error_callback_type _error = nullptr;
    std::pmr::memory_resource * _res = nullptr;

public:
    std::pmr::vector<xui::drawcmd> _commands;

public:
    std::pmr::deque<xui::rect> _rects;
    std::pmr::deque<std::string> _types;
    std::pmr::deque<xui::font_id> _fonts;
    std::pmr::deque<xui::string_id> _ids;
    std::pmr::deque<xui::style *> _styles;
    std::pmr::deque<xui::window_id> _windows;
    std::pmr::deque<xui::texture_id> _textures;
    std::pmr::deque<xui::window_flag> _windowflags;
};

xui::context::context( std::pmr::memory_resource * res )
    : _p( new ( res->allocate( sizeof( private_p ) ) ) private_p( res ) )
{
}

xui::context::~context()
{
    auto res = _p->_commands.get_allocator().resource();

    _p->~private_p();

    res->deallocate( _p, sizeof( private_p ) );
}

std::string_view xui::context::dark_style()
{
    return
R"(
    *{
        border-width: 1;
        border-style: solid;

        margin-left: 0;
        margin-right: 0;
        margin-top: 0;
        margin-bottom: 0;

        padding-left: 0;
        padding-right: 0;
        padding-top: 0;
        padding-bottom: 0;
    },
    window{
        background-color: black;
        border-radius: vec4(10, 10, 10, 10);
    },
    window-titlebar{
        background-color: rgb(76, 74, 72);
        border-radius: vec4(10, 10, 0, 0);
    },
    window-titlebar-title{
        border-color: white;
        text-align: left vcenter;
    },
    window-titlebar-closebox{
        stroke-color: white;
        border-color: transparent;
        background-color: transparent;
        border-radius: vec4(0, 10, 0, 0);
    },
    window-titlebar-closebox:hover{
        stroke-color: white;
        border-color: transparent;
        background-color: red;
    },
    window-titlebar-minimizebox{
        stroke-color: white;
        border-color: transparent;
        background-color: transparent;
        border-radius: vec4(0, 0, 0, 0);
    },
    window-titlebar-minimizebox:hover{
        stroke-color: white;
        border-color: transparent;
        background-color: rgb(94, 92, 91);
    },
    window-titlebar-maximizebox{
        stroke-color: white;
        border-color: transparent;
        background-color: transparent;
        border-radius: vec4(0, 0, 0, 0);
    },
    window-titlebar-maximizebox:hover{
        stroke-color: white;
        border-color: transparent;
        background-color: rgb(94, 92, 91);
    },
    menubar{
    },
    label{
        stroke-color: green;
    },
    image{
    },
    slider{
        border-color: white;
        background-color: rgb(94, 92, 91);
    },
    slider-cursor{
        border-color: transparent;
        background-color: red;
        direction: left right;
    },
    process{
        border-color: white;
        background-color: rgb(94, 92, 91);
    },
    process-text{
        stroke-color: white;
        text-align: left vcenter;
    },
    process-cursor{
        border-color: transparent;
        background-color: red;
        direction: bottom top;
    },
    textedit_base{
    },
    button{
        border-color: black;
        background-color: white;
        border-radius: vec4(10, 10, 0, 0);
    },
    button:hover{
        border-color: black;
        background-color: red;
    },
    button:active{
        border-color: black;
        background-color: green;
    },
    button-text{
        text-align: right top;
        stroke-color: black;
    },
    button-text:hover{
        stroke-color: white;
    },
    button-text:active{
        stroke-color: red;
    },
    combobox{
    },
    combobox-list{
    },
    tabview{
    },
    tabview-tab{
    },
    listview{
    },
    listview-item{
    },
    treeview{
    },
    treeview-item{
    },
    tableview{
    },
    tableview-header{
    },
    tableview-item{
    }
)";
}

std::string_view xui::context::light_style()
{
    return "";
}

void xui::context::init( implement * impl )
{
    _p->_impl = impl;
}

void xui::context::release()
{
    _p->_impl = nullptr;
}

void xui::context::set_scale( float factor )
{
    _p->_factor = factor;
}

void xui::context::set_error( const xui::error_callback_type & callback )
{
    _p->_error = callback;
}

void xui::context::push_style( xui::style * style )
{
    _p->_styles.emplace_back( style );
}

void xui::context::pop_style()
{
    _p->_styles.pop_back();
}

xui::style::variant xui::context::current_style( std::string_view attr )
{
    std::string name{ _p->_types.back() };

    if ( !_p->_ids.empty() )
    {
        name.append( "#" );
        name.append( _p->_ids.back() );
    }
    name.append( "@" );
    name.append( attr );

    for ( auto it = _p->_styles.rbegin(); it != _p->_styles.rend(); ++it )
    {
        auto val = ( *it )->find( name );
        if ( val.index() != 0 )
            return val;
    }

    return {};
}

void xui::context::push_font( xui::font_id font )
{
    _p->_fonts.push_back( font );
}

void xui::context::pop_font()
{
    _p->_fonts.pop_back();
}

xui::font_id xui::context::current_font() const
{
    if ( _p->_fonts.empty() )
    {
        XUI_ERR( ERR_NO );
        return xui::invalid_id;
    }

    return _p->_fonts.back();
}

void xui::context::push_window( xui::window_id id )
{
    _p->_windows.push_back( id );
}

void xui::context::pop_window()
{
    return _p->_windows.pop_back();
}

xui::window_id xui::context::current_window() const
{
    if ( _p->_windows.empty() )
    {
        XUI_ERR( ERR_NO );
        return xui::invalid_id;
    }

    return _p->_windows.back();
}

void xui::context::push_texture( xui::texture_id id )
{
    _p->_textures.push_back( id );
}

void xui::context::pop_texture()
{
    _p->_textures.pop_back();
}

xui::texture_id xui::context::current_texture() const
{
    if ( _p->_textures.empty() )
    {
        XUI_ERR( ERR_NO );
        return xui::invalid_id;
    }

    return _p->_textures.back();
}

void xui::context::push_string_id( xui::string_id id )
{
    _p->_ids.push_back( id );
}

void xui::context::pop_string_id()
{
    _p->_ids.pop_back();
}

xui::string_id xui::context::current_string_id() const
{
    if ( _p->_ids.empty() )
    {
        XUI_ERR( ERR_NO );
        return {};
    }

    return _p->_ids.back();
}

void xui::context::push_rect( const xui::rect & rect )
{
    _p->_rects.push_back( rect );
}

void xui::context::pop_rect()
{
    _p->_rects.pop_back();
}

xui::rect xui::context::currrent_rect() const
{
    if ( _p->_rects.empty() )
    {
        XUI_ERR( ERR_NO );
        return {};
    }

    return _p->_rects.back();
}

void xui::context::push_window_flag( xui::window_flag flag )
{
    _p->_windowflags.push_back( flag );
}

void xui::context::pop_window_flag()
{
    _p->_windowflags.pop_back();
}

xui::window_flag xui::context::current_flag() const
{
    return _p->_windowflags.back();
}

void xui::context::begin()
{
    _p->_commands.clear();
}

std::span<xui::drawcmd> xui::context::end()
{
    std::sort( _p->_commands.begin(), _p->_commands.end(), []( const auto & left, const auto & right ) { return left.id < right.id; } );

    return _p->_commands;
}

bool xui::context::begin_window( std::string_view title, xui::texture_id icon_id, xui::window_flag flags )
{
    auto id = current_window();
    auto wrect = currrent_rect();
    wrect.x = 0; wrect.y = 0;

    auto status = _p->_impl->get_window_status( current_window() );
    if ( (status & xui::windowstatus::SHOW ) != 0 )
    {
        draw_type( "window", [&]()
        {
            if ( ( flags & xui::window_flag::WINDOW_NO_BACKGROUND ) == 0 )
            {
                draw_rect( wrect );
            }
            if ( ( flags & xui::window_flag::WINDOW_NO_TITLEBAR ) == 0 )
            {
                draw_element( "titlebar", [&]()
                {
                    draw_rect( { 0, 0, wrect.w, 30 } );

                    draw_image( icon_id, { 8, 5, 20, 20 } );

                    draw_element( "title", [&]()
                    {
                        draw_text( title, current_font(), { 30, 5, wrect.w - 150, 20 } );
                    } );

                    if ( ( flags & xui::window_flag::WINDOW_NO_CLOSEBOX ) == 0 )
                    {
                        draw_element( "closebox", [&]()
                        {
                            xui::point center = { wrect.w - 25, 15 };
                            xui::rect rect = { center.x - 25, 0, 50, 30 };

                            if ( _p->_impl->get_key(id, xui::event::KEY_MOUSE_LEFT) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                                _p->_impl->remove_window( id );
                            else if ( _p->_impl->get_key( id, xui::event::MOUSE_ENTER) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                                draw_action( "hover", [&]() { draw_rect( rect ); } );
                            else
                                draw_rect( rect );

                            draw_path()
                                .moveto( { center.x - 5, center.y - 5 } )
                                .lineto( { center.x + 5, center.y + 5 } )
                                .closepath()
                                .moveto( { center.x + 5, center.y - 5 } )
                                .lineto( { center.x - 5, center.y + 5 } )
                                .closepath();
                        } );
                    }
                    if ( ( flags & xui::window_flag::WINDOW_NO_MAXIMIZEBOX ) == 0 )
                    {
                        draw_element( "maximizebox", [&]()
                        {
                            xui::point center = { wrect.w - 75, 15 };
                            xui::rect rect = { center.x - 25, 0, 50, 30 };

                            if ( _p->_impl->get_key( id, xui::event::KEY_MOUSE_LEFT) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                                _p->_impl->set_window_status( current_window(), ( ( status & xui::windowstatus::MAXIMIZE ) != 0 ) ? xui::windowstatus::RESTORE : xui::windowstatus::MAXIMIZE );
                            else if ( _p->_impl->get_key( id, xui::event::MOUSE_ENTER) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                                draw_action( "hover", [&]() { draw_rect( rect ); } );
                            else
                                draw_rect( rect );

                            if ( ( status & xui::windowstatus::MAXIMIZE ) != 0 )
                            {
                                draw_path()
                                    .moveto( { center.x - 3, center.y - 5 } )
                                    .lineto( { center.x - 3, center.y + 3 } )
                                    .lineto( { center.x + 5, center.y + 3 } )
                                    .lineto( { center.x + 5, center.y - 5 } )
                                    .lineto( { center.x - 3, center.y - 5 } )
                                    .closepath()
                                    .moveto( { center.x - 5, center.y - 3 } )
                                    .lineto( { center.x - 5, center.y + 5 } )
                                    .lineto( { center.x + 3, center.y + 5 } )
                                    .lineto( { center.x + 3, center.y - 3 } )
                                    .lineto( { center.x - 5, center.y - 3 } )
                                    .closepath();
                            }
                            else
                            {
                                draw_path()
                                    .moveto( { center.x - 5, center.y - 5 } )
                                    .lineto( { center.x - 5, center.y + 5 } )
                                    .lineto( { center.x + 5, center.y + 5 } )
                                    .lineto( { center.x + 5, center.y - 5 } )
                                    .lineto( { center.x - 5, center.y - 5 } )
                                    .closepath();
                            }
                        } );
                    }
                    if ( ( flags & xui::window_flag::WINDOW_NO_MINIMIZEBOX ) == 0 )
                    {
                        draw_element( "minimizebox", [&]()
                        {
                            xui::point center = { wrect.w - 125, 15 };
                            xui::rect rect = { center.x - 25, 0, 50, 30 };

                            if ( _p->_impl->get_key( id, xui::event::KEY_MOUSE_LEFT) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                                _p->_impl->set_window_status( current_window(), xui::windowstatus::MINIMIZE );
                            else if ( _p->_impl->get_key( id, xui::event::MOUSE_ENTER) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                                draw_action( "hover", [&]() { draw_rect( rect ); } );
                            else
                                draw_rect( rect );

                            draw_path()
                                .moveto( { center.x - 5, center.y } )
                                .lineto( { center.x + 5, center.y } )
                                .closepath();
                        } );
                    }
                } );
            }
        } );
    }

    if ( ( flags & xui::window_flag::WINDOW_NO_TITLEBAR ) == 0 )
        push_rect( { 0, 30, wrect.w, wrect.h - 30 } );
    else
        push_rect( wrect );

    status = _p->_impl->get_window_status( current_window() );
    return ( ( status & xui::windowstatus::SHOW ) != 0 );
}

void xui::context::end_window()
{
    pop_rect();
}

bool xui::context::image( xui::texture_id id )
{
    draw_type( "image", [&]() { draw_image( id, currrent_rect() ); } );

    return true;
}

bool xui::context::label( std::string_view text )
{
    draw_type( "label", [&]() { draw_text( text, current_font(), currrent_rect() ); } );

    return true;
}

bool xui::context::button( std::string_view text )
{
    auto id = current_window();
    auto rect = currrent_rect();

    draw_type( "button", [&]()
    {
        if ( _p->_impl->get_key( id, xui::event::KEY_MOUSE_LEFT) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
            draw_action( "active", [&]() { draw_rect( rect ); } );
        else if ( _p->_impl->get_key( id, xui::event::MOUSE_ENTER) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
            draw_action( "hover", [&]() { draw_rect( rect ); } );
        else
            draw_rect( rect );

        draw_element( "text", [&]()
        {
            if ( !text.empty() )
            {
                if ( _p->_impl->get_key( id, xui::event::KEY_MOUSE_LEFT) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                    draw_action( "active", [&]() { draw_text( text, current_font(), rect ); } );
                else if ( _p->_impl->get_key( id, xui::event::MOUSE_ENTER) && rect.contains( _p->_impl->get_cursor_pos( id ) ) )
                    draw_action( "hover", [&]() { draw_text( text, current_font(), rect ); } );
                else
                    draw_text( text, current_font(), rect );
            }
        } );
    } );

    return ( _p->_impl->get_key( id, xui::event::KEY_MOUSE_LEFT) && rect.contains( _p->_impl->get_cursor_pos( id ) ) );
}

bool xui::context::process( float value, std::string_view text )
{
    draw_type( "process", [&]()
    {
        auto background_rect = currrent_rect();

        draw_rect( background_rect );

        draw_element( "cursor", [&]()
        {
            xui::rect cursor_rect;

            switch ( current_style( "direction" ).value<xui::direction>( xui::direction::LEFT_RIGHT ) )
            {
            case xui::LEFT_RIGHT:
                cursor_rect = { background_rect.x, background_rect.y, background_rect.w * value, background_rect.h };
                break;
            case xui::RIGHT_LEFT:
                cursor_rect = { background_rect.x + ( background_rect.w - ( background_rect.w * value ) ), background_rect.y, background_rect.w * value, background_rect.h };
                break;
            case xui::TOP_BOTTOM:
                cursor_rect = { background_rect.x, background_rect.y, background_rect.w, background_rect.h * value };
                break;
            case xui::BOTTOM_TOP:
                cursor_rect = { background_rect.x, background_rect.y + ( background_rect.h - ( background_rect.h * value ) ), background_rect.w, background_rect.h * value };
                break;
            default:
                break;
            }

            draw_rect( cursor_rect );
        } );

        if ( !text.empty() )
        {
            draw_element( "text", [&]() { draw_text( text, current_font(), background_rect ); } );
        }
    } );

    return true;
}

bool xui::context::textedit( xui::textedit_base * state, std::string_view hint )
{
    draw_type( "textedit", [&]()
    {
        auto rect = currrent_rect();

        draw_rect( rect );

        if ( state->size() == 0 )
        {
            draw_text( hint, current_font(), rect );
        }
        else
        {

        }
    } );

    return false;
}

bool xui::context::slider_int( int * cur, int size, int min, int max )
{
    draw_type( "slider", [&]()
    {
        auto background_rect = currrent_rect();

        draw_rect( background_rect );

        draw_element( "cursor", [&]()
        {
            xui::rect cursor_rect;
            int cursor_w = std::max<int>( 10, background_rect.w / ( max - min ) );
            int cursor_h = background_rect.h;

            for ( int i = 0; i < size; i++ )
            {
                switch ( current_style( "direction" ).value<xui::direction>( xui::direction::LEFT_RIGHT ) )
                {
                case xui::LEFT_RIGHT:
                    cursor_rect = {
                        background_rect.x + ( ( background_rect.w / ( max - min ) ) * cur[0] ) - ( cursor_w / 2 ),
                        background_rect.y,
                        std::max( 10.0f, background_rect.w / ( max - min ) ),
                        background_rect.h
                    };
                    break;
                case xui::RIGHT_LEFT:
                    break;
                case xui::TOP_BOTTOM:
                    break;
                case xui::BOTTOM_TOP:
                    break;
                default:
                    break;
                }

                draw_rect( cursor_rect );
            }
        } );
    } );

    return true;
}

void xui::context::push_type( std::string_view type )
{
    _p->_types.push_back( { type.begin(), type.end() } );
}

void xui::context::push_element( std::string_view element )
{
    _p->_types.push_back( std::format( "{}-{}", _p->_types.back(), element ) );
}

void xui::context::push_action( std::string_view action )
{
    _p->_types.push_back( std::format( "{}:{}", _p->_types.back(), action ) );
}

void xui::context::pop_action()
{
    _p->_types.pop_back();
}

void xui::context::pop_element()
{
    _p->_types.pop_back();
}

void xui::context::pop_type()
{
    _p->_types.pop_back();
}

xui::drawcmd::text_element & xui::context::draw_text( std::string_view text, xui::font_id id, const xui::rect & rect )
{
    xui::drawcmd::text_element element;

    element.font = id;
    element.text = text;
    element.rect = rect;
    element.color = current_style( "stroke-color" ).value<xui::color>();
    element.align = current_style( "text-align" ).value<xui::alignment_flag>( xui::alignment_flag::CENTER );

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::text_element>( _p->_commands.back().element );
}

xui::drawcmd::line_element & xui::context::draw_line( const xui::point & p1, const xui::point & p2 )
{
    xui::drawcmd::line_element element;

    element.p1 = p1;
    element.p2 = p2;
    element.stroke.style = current_style( "stroke-style" ).value<int>( xui::drawcmd::stroke::SOLID );
    element.stroke.width = current_style( "stroke-width" ).value<float>( 1.0f );
    element.stroke.color = current_style( "stroke-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::line_element>( _p->_commands.back().element );
}

xui::drawcmd::rect_element & xui::context::draw_rect( const xui::rect & rect )
{
    xui::drawcmd::rect_element element;

    element.rect = rect;
    element.border.style = current_style( "border-style" ).value<int>( xui::drawcmd::stroke::SOLID );
    element.border.width = current_style( "border-width" ).value<float>( 1.0f );
    element.border.color = current_style( "border-color" ).value<xui::color>();
    element.border.radius = current_style( "border-radius" ).value<xui::vec4>();
    element.filled.color = current_style( "background-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::rect_element>( _p->_commands.back().element );
}

xui::drawcmd::path_element & xui::context::draw_path( std::string_view data )
{
    xui::drawcmd::path_element element;

    element.data = data;
    element.stroke.style = current_style( "stroke-style" ).value<int>( xui::drawcmd::stroke::SOLID );
    element.stroke.width = current_style( "stroke-width" ).value<float>( 1.0f );
    element.stroke.color = current_style( "stroke-color" ).value<xui::color>();
    element.filled.color = current_style( "background-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::path_element>( _p->_commands.back().element );
}

xui::drawcmd::image_element & xui::context::draw_image( xui::texture_id id, const xui::rect & rect )
{
    xui::drawcmd::image_element element;

    element.id = id;
    element.rect = rect;

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::image_element>( _p->_commands.back().element );
}

xui::drawcmd::circle_element & xui::context::draw_circle( const xui::point & center, float radius )
{
    xui::drawcmd::circle_element element;

    element.center = center;
    element.radius = radius;

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::circle_element>( _p->_commands.back().element );
}

xui::drawcmd::ellipse_element & xui::context::draw_ellipse( const xui::point & center, const xui::point & radius )
{
    xui::drawcmd::ellipse_element element;

    element.center = center;
    element.radius = radius;

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::ellipse_element>( _p->_commands.back().element );
}

xui::drawcmd::polygon_element & xui::context::draw_polygon( std::span<xui::point> points )
{
    xui::drawcmd::polygon_element element;

    element.points.assign( points.begin(), points.end() );

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::polygon_element>( _p->_commands.back().element );
}
