#include "xui.h"

#include <array>
#include <deque>
#include <regex>
#include <memory>
#include <algorithm>
#include <iostream>

#define XUI_ERR( CODE )  if ( _p->_error ) _p->_error( this, xui_category::make_error_code( xui::err::CODE ) );
#define XUI_SCALE( VAL ) ( VAL * _p->_factor )

namespace
{
    static std::regex int_regex{ R"([-+]?([0-9]*[0-9]+))" };
    static std::regex flt_regex{ R"([-+]?([0-9]*.[0-9]+|[0-9]+))" };
    static std::regex func_regex{ R"(\S*\([\s\S]*\))" };

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

xui::vec2 xui::rect::center() const
{
    return { ( x + w * 0.5f ), ( y + h * 0.5f ) };
}

bool xui::rect::contains( const xui::vec2 & p ) const
{
    return ( p.x > x && p.x < ( x + w ) ) && ( p.y > y && p.y < ( y + h ) );
}

xui::url::url( url && val )
    : string_type( val )
{
    parse();
}

xui::url::url( const url & val )
    : string_type( val )
{
    parse();
}

xui::url::url( string_type && val )
    : string_type( val )
{
    parse();
}

xui::url::url( const string_type & val )
    : string_type( val )
{
    parse();
}

xui::url::url( string_view_type val )
    : string_type( val.begin(), val.end() )
{
    parse();
}

xui::url & xui::url::operator=( xui::url && val )
{
    string_type::operator=( val );

    parse();

    return *this;
}

xui::url & xui::url::operator=( const xui::url & val )
{
    string_type::operator=( val );

    parse();

    return *this;
}

xui::url & xui::url::operator=( string_type && val )
{
    string_type::operator=( val );

    parse();

    return *this;
}

xui::url & xui::url::operator=( const string_type & val )
{
    string_type::operator=( val );

    parse();

    return *this;
}

xui::url & xui::url::operator=( string_view_type val )
{
    string_type::operator=( { val.begin(), val.end() } );

    parse();

    return *this;
}

xui::url::string_view_type xui::url::string_view() const
{
    return { string_type::begin(), string_type::end() };
}

xui::url::string_view_type xui::url::scheme() const
{
    return _scheme;
}

xui::url::string_view_type xui::url::username() const
{
    return _username;
}

xui::url::string_view_type xui::url::password() const
{
    return _password;
}

xui::url::string_view_type xui::url::host() const
{
    return _host;
}

xui::url::string_view_type xui::url::port() const
{
    return _port;
}

xui::url::string_view_type xui::url::path() const
{
    return _path;
}

xui::url::string_view_type xui::url::querys() const
{
    return _query;
}

xui::url::string_view_type xui::url::fragment() const
{
    return _fragment;
}

std::size_t xui::url::query_count() const
{
    return std::count( _query.begin(), _query.end(), constexpr_flags<char_type>::query_flag );
}

xui::url::string_view_type xui::url::query_at( std::size_t idx ) const
{
    auto beg = _query.begin();
    auto end = std::find( beg, _query.end(), constexpr_flags<char_type>::query_pair_flag );

    auto count = query_count();
    for ( std::size_t i = 0; i < idx && i < count; i++ )
    {
        beg = end + 1;
        end = std::find( beg, _query.end(), constexpr_flags<char_type>::query_pair_flag );
    }

    return { beg, end };
}

xui::url::const_iterator xui::url::find_it( string_view_type substr, const_iterator offset ) const
{
    auto idx = string_type::find( substr, std::distance( string_type::begin(), offset ) );
    if ( idx == string_type::npos )
        return string_type::end();
    return string_type::begin() + idx;
}

xui::url::const_iterator xui::url::find_it( string_view_type substr, const_iterator beg, const_iterator end ) const
{
    string_view_type tmp{ beg, end };

    auto idx = tmp.find( substr );
    if ( idx == string_type::npos )
        return end;

    return beg + idx;
}

void xui::url::parse()
{
    // {scheme}://{username}:{password}@{host}:{port}/{path}?{query}#{fragment}

    const_iterator ibeg = string_type::begin();
    const_iterator iend = string_type::end();

    iend = find_it( constexpr_flags<char_type>::scheme_flag, ibeg );
    if ( iend != string_type::end() )
    {
        _scheme = { ibeg, iend };
        iend += 3;
    }
    else
    {
        _scheme = { ibeg, iend };
        return;
    }

    ibeg = iend;
    iend = find_it( constexpr_flags<char_type>::host_flag, ibeg );
    if ( iend != string_type::end() )
    {
        auto im_end = find_it( constexpr_flags<char_type>::username_flag, ibeg, iend );
        if ( im_end != iend )
        {
            auto imm_end = im_end;

            imm_end = find_it( constexpr_flags<char_type>::port_flag, ibeg, im_end );
            if ( imm_end != im_end )
            {
                _username = { ibeg, imm_end };
                _password = { imm_end + 1, im_end };
            }
            else
            {
                _username = { ibeg, imm_end };
            }

            ibeg = im_end + 1;
        }

        im_end = find_it( constexpr_flags<char_type>::port_flag, ibeg, iend );
        if ( im_end != iend )
        {
            _host = { ibeg, im_end };
            _port = { im_end + 1, iend };
        }
        else
        {
            _host = { ibeg, iend };
        }

        iend += 1;
    }
    else
    {
        _host = { ibeg, iend };
        return;
    }

    ibeg = iend;
    iend = find_it( constexpr_flags<char_type>::path_flag, ibeg );
    if ( iend != string_type::end() )
    {
        _path = { ibeg, iend };
        iend += 1;
    }
    else
    {
        _path = { ibeg, iend };
        return;
    }

    ibeg = iend;
    iend = find_it( constexpr_flags<char_type>::fragment_flag, ibeg );
    if ( iend != string_type::end() )
    {
        _query = { ibeg, iend };
        iend += 1;
    }
    else
    {
        _query = { ibeg, iend };
        return;
    }

    ibeg = iend;
    _fragment = { ibeg, string_type::end() };
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

xui::color xui::color::dark() const
{
    return xui::color( std::clamp( r - 50, 0, 255 ),
                       std::clamp( g - 50, 0, 255 ),
                       std::clamp( b - 50, 0, 255 ),
                       a );
}

xui::color xui::color::light() const
{
    return xui::color( std::clamp( r + 50, 0, 255 ),
                       std::clamp( g + 50, 0, 255 ),
                       std::clamp( b + 50, 0, 255 ),
                       a );
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

                select.attrs.insert( { name, parse( { beg, end } ) } );

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

xui::style::variant xui::style::parse( std::string_view text )
{
    std::string val( text );
    while ( val.find( ' ' ) != std::string::npos )
        val.replace( val.find( ' ' ), 1, "" );

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

        return col;
    }
    else if ( std::regex_match( val.begin(), val.end(), int_regex) )
    {
        int value = 0;
        std::from_chars( val.data(), val.data() + val.size(), value );
        return value;
    }
    else if ( std::regex_match( val.begin(), val.end(), flt_regex ) )
    {
        float value = 0;
        std::from_chars( val.data(), val.data() + val.size(), value );
        return value;
    }
    else if ( std::regex_match( val.begin(), val.end(), func_regex ) )
    {
        auto it = functions().find( { val.begin(), val.begin() + val.find( '(' ) } );
        if ( it != functions().end() )
        {
            return it->second( { adv2<'('>( val.begin(), val.end() ) + 1, adv2<')'>( val.begin(), val.end() ) } );
        }
    }
    else if ( flags().find( val ) != flags().end() )
    {
        return flags()[val];
    }
    else if ( colors().find( val ) != colors().end() )
    {
        return xui::color( colors()[val] );
    }

    return val;
}

int xui::style::parse_int( std::string_view val )
{
    int result = 0;
    std::from_chars( val.data(), val.data() + val.size(), result );
    return result;
}

float xui::style::parse_flt( std::string_view val )
{
    float result = 0;
    std::from_chars( val.data(), val.data() + val.size(), result );
    return result;
}

xui::url xui::style::parse_url( std::string_view val )
{
    return xui::url( val );
}

xui::vec2 xui::style::parse_vec2( std::string_view val )
{
    xui::vec2 result;

    auto beg = val.begin();
    auto r_beg = beg;
    auto r_end = adv<','>( beg, val.end() );
    result.x = parse_flt( { r_beg, r_end } );

    check<','>( r_end, val.end() );

    auto g_beg = r_end; beg = g_beg;
    auto g_end = val.end();
    result.y = parse_flt( { g_beg, g_end } );

    return result;
}

xui::vec4 xui::style::parse_vec4( std::string_view val )
{
    xui::vec4 result;

    auto beg = val.begin();
    auto r_beg = beg;
    auto r_end = adv<','>( beg, val.end() );
    result.x = parse_flt( { r_beg, r_end } );

    check<','>( r_end, val.end() );

    auto g_beg = r_end; beg = g_beg;
    auto g_end = adv<','>( beg, val.end() );
    result.y = parse_flt( { g_beg, g_end } );

    check<','>( g_end, val.end() );

    auto b_beg = g_end; beg = b_beg;
    auto b_end = adv<','>( beg, val.end() );
    result.z = parse_flt( { b_beg, b_end } );

    check<','>( b_end, val.end() );

    auto a_beg = b_end; beg = a_beg;
    auto a_end = val.end();
    result.w = parse_flt( { a_beg, a_end } );

    return result;
}

xui::color xui::style::parse_rgb( std::string_view val )
{
    xui::color result;

    auto beg = val.begin();
    auto r_beg = beg;
    auto r_end = adv<','>( beg, val.end() );
    result.r = parse_int( { r_beg, r_end } );

    check<','>( r_end, val.end() );

    auto g_beg = r_end; beg = g_beg;
    auto g_end = adv<','>( beg, val.end() );
    result.g = parse_int( { g_beg, g_end } );

    check<','>( g_end, val.end() );

    auto b_beg = g_end; beg = b_beg;
    auto b_end = val.end();
    result.b = parse_int( { b_beg, b_end } );

    return result;
}

xui::color xui::style::parse_rgba( std::string_view val )
{
    xui::color result;

    auto beg = val.begin();
    auto r_beg = beg;
    auto r_end = adv<','>( beg, val.end() );
    result.r = parse_int( { r_beg, r_end } );

    check<','>( r_end, val.end() );

    auto g_beg = r_end; beg = g_beg;
    auto g_end = adv<','>( beg, val.end() );
    result.g = parse_int( { g_beg, g_end } );

    check<','>( g_end, val.end() );

    auto b_beg = g_end; beg = b_beg;
    auto b_end = adv<','>( beg, val.end() );
    result.b = parse_int( { b_beg, b_end } );

    check<','>( b_end, val.end() );

    auto a_beg = b_end; beg = a_beg;
    auto a_end = val.end();
    result.a = parse_int( { a_beg, a_end } );

    return result;
}

xui::color xui::style::parse_dark( std::string_view val )
{
    return parse( val ).value<xui::color>().dark();
}

xui::color xui::style::parse_light( std::string_view val )
{
    return parse( val ).value<xui::color>().light();
}

bool xui::style::register_flag( std::string_view name, std::uint32_t flag )
{
    auto it = flags().find( name );
    if ( it == flags().end() )
    {
        flags().insert( { name, flag } );
        return true;
    }
    return false;
}

bool xui::style::register_color( std::string_view name, const xui::color & color )
{
    auto it = colors().find( name );
    if ( it == colors().end() )
    {
        colors().insert( { name, color.hex } );
        return true;
    }
    return false;
}

bool xui::style::register_function( std::string_view name, const std::function<xui::style::variant( std::string_view )> & func )
{
    auto it = functions().find( name );
    if ( it == functions().end() )
    {
        functions().insert( { name, func } );
        return true;
    }
    return false;
}

std::map<std::string_view, std::uint32_t> & xui::style::flags()
{
    static std::map<std::string_view, std::uint32_t> style_flags =
    {
        // stroke style
        { "none", xui::drawcmd::stroke::NONE },
        { "solid", xui::drawcmd::stroke::SOLID },
        { "dashed", xui::drawcmd::stroke::DASHED },
        { "dotted", xui::drawcmd::stroke::DOTTED },
        { "dashdot", xui::drawcmd::stroke::DASH_DOT },
        { "dashdotdot", xui::drawcmd::stroke::DASH_DOT_DOT },

        // alignment
        { "left"	, xui::alignment_flag::ALIGN_LEFT },
        { "right"	, xui::alignment_flag::ALIGN_RIGHT },
        { "top"		, xui::alignment_flag::ALIGN_TOP },
        { "bottom"	, xui::alignment_flag::ALIGN_BOTTOM },
        { "center"	, xui::alignment_flag::ALIGN_CENTER },
        { "vcenter"	, xui::alignment_flag::ALIGN_VCENTER },
        { "hcenter"	, xui::alignment_flag::ALIGN_HCENTER },
        { "lefttop"	, xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_TOP },
        { "topleft"	, xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_TOP },
        { "leftbottom"	, xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_BOTTOM },
        { "bottomleft"	, xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_BOTTOM },
        { "righttop"	, xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_TOP },
        { "topright"	, xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_TOP },
        { "rightbottom", xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_BOTTOM },
        { "bottomright", xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_BOTTOM },
        { "leftvcenter", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "vcenterleft", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "rightvcenter", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "vcenterright", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "tophcenter", xui::alignment_flag::ALIGN_TOP | xui::alignment_flag::ALIGN_HCENTER },
        { "hcentertop", xui::alignment_flag::ALIGN_TOP | xui::alignment_flag::ALIGN_HCENTER },
        { "bottomhcenter", xui::alignment_flag::ALIGN_BOTTOM | xui::alignment_flag::ALIGN_HCENTER },
        { "hcenterbottom", xui::alignment_flag::ALIGN_BOTTOM | xui::alignment_flag::ALIGN_HCENTER },
        { "vcenterhcenter"	, xui::alignment_flag::ALIGN_CENTER },
        { "hcentervcenter"	, xui::alignment_flag::ALIGN_CENTER },

        // direction
        { "leftright", (std::uint32_t)xui::direction::LEFT_RIGHT },
        { "rightleft", (std::uint32_t)xui::direction::RIGHT_LEFT },
        { "topbottom", (std::uint32_t)xui::direction::TOP_BOTTOM },
        { "bottomtop", (std::uint32_t)xui::direction::BOTTOM_TOP },
    };
    return style_flags;
}

std::map<std::string_view, std::uint32_t> & xui::style::colors()
{
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
    return style_colors;
}

std::map<std::string_view, std::function<xui::style::variant( std::string_view )>> & xui::style::functions()
{
    static std::map<std::string_view, std::function<xui::style::variant( std::string_view )>> style_functions =
    {
        {"url", xui::style::parse_url },
        {"rgb", xui::style::parse_rgb },
        {"rgba", xui::style::parse_rgba },
        {"vec2", xui::style::parse_vec2 },
        {"vec4", xui::style::parse_vec4 },
        {"dark", xui::style::parse_dark },
        {"light", xui::style::parse_light },
    };
    return style_functions;
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

        font-color: white;
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
        font-color: white;
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
        font-color: green;
    },
    image{
    },
    slider{
        border-color: white;
        background-color: white;
        direction: bottom top;
    },
    slider:hover{
        border-color: white;
        background-color: green;
    },
    slider:active{
        border-color: white;
        background-color: yellow;
    },
    slider-cursor{
        border-color: transparent;
        background-color: red;
    },
    process{
        border-color: white;
        background-color: rgb(94, 92, 91);
    },
    process-text{
        font-color: white;
        text-align: center;
    },
    process-cursor{
        border-color: transparent;
        background-color: red;
        direction: bottom top;
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
        font-color: black;
        text-align: right top;
    },
    button-text:hover{
        font-color: white;
    },
    button-text:active{
        font-color: red;
    },
    radio{
        border-color: white;
        background-color: blue;
    },
    radio:hover{
        background-color: light( blue );
    },
    radio:active{
        background-color: green;
    },
    radio-indicator{
        background-color: white;
    },
    check{
        border-color: white;
        background-color: blue;
    },
    check:hover{
        background-color: light( blue );
    },
    check:active{
        background-color: green;
    },
    check-indicator{
        stroke-color: white;
        stroke-width: 3;
    },
    scrollbar{
        border-color: white;
        background-color: green;
    },
    scrollbar-cursor{
        border-color: white;
        background-color: red;
    },
    scrollbar-arrow{
        border-color: white;
        background-color: blue;
        stroke-color: white;
        stroke-width: 3;
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

bool xui::context::begin_window( std::string_view title, xui::texture_id icon_id, int flags )
{
    auto id = current_window();
    auto wrect = currrent_rect();
    auto cursorpos = _p->_impl->get_cursor( id );

    wrect.x = 0; wrect.y = 0;

    auto status = _p->_impl->get_window_status( id );

    if ( (status & xui::window_status::WINDOW_SHOW ) != 0 )
    {
        draw_style_type( "window", [&]()
        {
            xui::rect title_rect = { 0, 0, wrect.w, 30 };
            xui::rect resize_rect = { wrect.x + ( wrect.w - 30 ), wrect.y + ( wrect.h - 30 ), 30, 30 };

            if ( ( flags & xui::window_flag::WINDOW_NO_MOVE ) != 0 && _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && title_rect.contains( cursorpos ) )
            {

            }
            if ( ( flags & xui::window_flag::WINDOW_NO_RESIZE ) != 0 && _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && resize_rect.contains( cursorpos ) )
            {

            }

            if ( ( flags & xui::window_flag::WINDOW_NO_BACKGROUND ) == 0 )
            {
                draw_rect( wrect,
                           current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                           current_style( "border-width", 1.0f ),
                           current_style( "border-color", xui::color() ),
                           current_style( "border-radius", xui::vec4() ),
                           current_style( "background-color", xui::color() ) );
            }
            if ( ( flags & xui::window_flag::WINDOW_NO_TITLEBAR ) == 0 )
            {
                wrect.y += 30;
                wrect.h -= 30;

                draw_style_element( "titlebar", [&]()
                {
                    draw_rect( title_rect,
                               current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                               current_style( "border-width", 1.0f ),
                               current_style( "border-color", xui::color() ),
                               current_style( "border-radius", xui::vec4() ),
                               current_style( "background-color", xui::color() ) );

                    draw_image( icon_id, { 8, 5, 20, 20 } );

                    draw_style_element( "title", [&]()
                    {
                        draw_text( title, current_font(), { 30, 5, wrect.w - 150, 20 }, current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
                    } );

                    xui::rect box_rect = { title_rect.w, title_rect.y, 50, title_rect.h };

                    draw_style_element( "closebox", [&]()
                    {
                        box_rect.x -= 50;

                        if ( ( ( flags & xui::window_flag::WINDOW_NO_CLOSEBOX ) == 0 ) && _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && box_rect.contains( cursorpos ) )
                            _p->_impl->remove_window( id );
                        else if ( ( ( flags & xui::window_flag::WINDOW_NO_CLOSEBOX ) == 0 ) && _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && box_rect.contains( cursorpos ) )
                            draw_style_action( "hover", [&]()
                            {
                                draw_rect( box_rect,
                                current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                                current_style( "border-width", 1.0f ),
                                current_style( "border-color", xui::color() ),
                                current_style( "border-radius", xui::vec4() ),
                                current_style( "background-color", xui::color() ) );
                            } );
                        else
                            draw_rect( box_rect,
                                       current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                                       current_style( "border-width", 1.0f ),
                                       current_style( "border-color", xui::color() ),
                                       current_style( "border-radius", xui::vec4() ),
                                       current_style( "background-color", xui::color() ) );

                        draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                   current_style( "stroke-width", 1.0f ),
                                   ( ( flags & xui::window_flag::WINDOW_NO_CLOSEBOX ) == 0 ) ? current_style( "stroke-color", xui::color() ) : current_style( "stroke-color", xui::color() ).dark(),
                                   current_style( "background-color", xui::color() ) )
                            .moveto( { box_rect.center().x - 5, box_rect.center().y - 5 } )
                            .lineto( { box_rect.center().x + 5, box_rect.center().y + 5 } )
                            .closepath()
                            .moveto( { box_rect.center().x + 5, box_rect.center().y - 5 } )
                            .lineto( { box_rect.center().x - 5, box_rect.center().y + 5 } )
                            .closepath();
                    } );

                    draw_style_element( "maximizebox", [&]()
                    {
                        box_rect.x -= 50;

                        if ( ( ( flags & xui::window_flag::WINDOW_NO_MAXIMIZEBOX ) == 0 ) && _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && box_rect.contains( cursorpos ) )
                            _p->_impl->set_window_status( id, ( ( status & xui::window_status::WINDOW_MAXIMIZE ) != 0 ) ? xui::window_status::WINDOW_RESTORE : xui::window_status::WINDOW_MAXIMIZE );
                        else if ( ( ( flags & xui::window_flag::WINDOW_NO_MAXIMIZEBOX ) == 0 ) && _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && box_rect.contains( cursorpos ) )
                            draw_style_action( "hover", [&]()
                            {
                                draw_rect( box_rect,
                                current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                                current_style( "border-width", 1.0f ),
                                current_style( "border-color", xui::color() ),
                                current_style( "border-radius", xui::vec4() ),
                                current_style( "background-color", xui::color() ) );
                            } );
                        else
                            draw_rect( box_rect,
                                       current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                                       current_style( "border-width", 1.0f ),
                                       current_style( "border-color", xui::color() ),
                                       current_style( "border-radius", xui::vec4() ),
                                       current_style( "background-color", xui::color() ) );

                        if ( ( status & xui::window_status::WINDOW_MAXIMIZE ) != 0 )
                        {
                            draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                       current_style( "stroke-width", 1.0f ),
                                       ( ( flags & xui::window_flag::WINDOW_NO_MAXIMIZEBOX ) == 0 ) ? current_style( "stroke-color", xui::color() ) : current_style( "stroke-color", xui::color() ).dark(),
                                       current_style( "background-color", xui::color() ) )
                                .moveto( { box_rect.center().x - 3, box_rect.center().y - 5 } )
                                .lineto( { box_rect.center().x - 3, box_rect.center().y + 3 } )
                                .lineto( { box_rect.center().x + 5, box_rect.center().y + 3 } )
                                .lineto( { box_rect.center().x + 5, box_rect.center().y - 5 } )
                                .lineto( { box_rect.center().x - 3, box_rect.center().y - 5 } )
                                .closepath()
                                .moveto( { box_rect.center().x - 5, box_rect.center().y - 3 } )
                                .lineto( { box_rect.center().x - 5, box_rect.center().y + 5 } )
                                .lineto( { box_rect.center().x + 3, box_rect.center().y + 5 } )
                                .lineto( { box_rect.center().x + 3, box_rect.center().y - 3 } )
                                .lineto( { box_rect.center().x - 5, box_rect.center().y - 3 } )
                                .closepath();
                        }
                        else
                        {
                            draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                       current_style( "stroke-width", 1.0f ),
                                       ( ( flags & xui::window_flag::WINDOW_NO_MAXIMIZEBOX ) == 0 ) ? current_style( "stroke-color", xui::color() ) : current_style( "stroke-color", xui::color() ).dark(),
                                       current_style( "background-color", xui::color() ) )
                                .moveto( { box_rect.center().x - 5, box_rect.center().y - 5 } )
                                .lineto( { box_rect.center().x - 5, box_rect.center().y + 5 } )
                                .lineto( { box_rect.center().x + 5, box_rect.center().y + 5 } )
                                .lineto( { box_rect.center().x + 5, box_rect.center().y - 5 } )
                                .lineto( { box_rect.center().x - 5, box_rect.center().y - 5 } )
                                .closepath();
                        }
                    } );

                    draw_style_element( "minimizebox", [&]()
                    {
                        box_rect.x -= 50;

                        if ( ( ( flags & xui::window_flag::WINDOW_NO_MINIMIZEBOX ) == 0 ) && _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && box_rect.contains( cursorpos ) )
                            _p->_impl->set_window_status( id, xui::window_status::WINDOW_MINIMIZE );
                        else if ( ( ( flags & xui::window_flag::WINDOW_NO_MINIMIZEBOX ) == 0 ) && _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && box_rect.contains( cursorpos ) )
                            draw_style_action( "hover", [&]()
                            {
                                draw_rect( box_rect,
                                current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                                current_style( "border-width", 1.0f ),
                                current_style( "border-color", xui::color() ),
                                current_style( "border-radius", xui::vec4() ),
                                current_style( "background-color", xui::color() ) );
                            } );
                        else
                            draw_rect( box_rect,
                                       current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                                       current_style( "border-width", 1.0f ),
                                       current_style( "border-color", xui::color() ),
                                       current_style( "border-radius", xui::vec4() ),
                                       current_style( "background-color", xui::color() ) );

                        draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                   current_style( "stroke-width", 1.0f ),
                                   ( ( flags & xui::window_flag::WINDOW_NO_MINIMIZEBOX ) == 0 ) ? current_style( "stroke-color", xui::color() ) : current_style( "stroke-color", xui::color() ).dark(),
                                   current_style( "background-color", xui::color() ) )
                            .moveto( { box_rect.center().x - 5, box_rect.center().y } )
                            .lineto( { box_rect.center().x + 5, box_rect.center().y } )
                            .closepath();
                    } );
                } );
            }
            if ( ( flags & xui::window_flag::WINDOW_NO_RESIZE ) != 0 )
            {
                draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                           current_style( "stroke-width", 1.0f ),
                           current_style( "stroke-color", xui::color() ),
                           current_style( "background-color", xui::color() ) )
                    .moveto( { resize_rect.x, resize_rect.y + resize_rect.h } )
                    .lineto( { resize_rect.x + resize_rect.w, resize_rect.y + resize_rect.h } )
                    .lineto( { resize_rect.x + resize_rect.w, resize_rect.y } )
                    .closepath();
            }
        } );
    }

    push_rect( wrect );

    status = _p->_impl->get_window_status( id );
    return ( ( status & xui::window_status::WINDOW_SHOW ) != 0 );
}

void xui::context::end_window()
{
    pop_rect();
}

bool xui::context::image( xui::texture_id id )
{
    draw_style_type( "image", [&]() { draw_image( id, currrent_rect() ); } );

    return true;
}

bool xui::context::label( std::string_view text )
{
    draw_style_type( "label", [&]() { draw_text( text, current_font(), currrent_rect(), current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) ); } );

    return true;
}

bool xui::context::radio( bool & checked )
{
    auto id = current_window();
    auto rect = currrent_rect();
    float raduis = std::min( rect.w, rect.h ) / 2;
    std::string_view action;

    draw_style_type( "radio", [&]()
    {
        if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && rect.contains( _p->_impl->get_cursor( id ) ) )
        {
            checked = !checked;

            action = "active";
        }
        else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && rect.contains( _p->_impl->get_cursor( id ) ) )
        {
            action = "hover";
        }

        draw_style_action( action, [&]()
        {
            draw_circle( { rect.x + raduis, rect.y + raduis }, raduis,
            current_style( "border-style", xui::drawcmd::stroke::SOLID ),
            current_style( "border-width", 1.0f ),
            current_style( "border-color", xui::color() ),
            current_style( "border-radius", xui::vec4() ),
            current_style( "background-color", xui::color() ) );
        } );

        if ( checked )
        {
            draw_style_element( "indicator", [&]()
            {
                draw_style_action( action, [&]()
                {
                    draw_circle( { rect.x + raduis, rect.y + raduis }, ( raduis * 0.7f ),
                    current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                    current_style( "border-width", 1.0f ),
                    current_style( "border-color", xui::color() ),
                    current_style( "border-radius", xui::vec4() ),
                    current_style( "background-color", xui::color() ) );
                } );
            } );
        }
    } );

    return checked;
}

bool xui::context::check( bool & checked )
{
    auto id = current_window();
    auto rect = currrent_rect();
    rect.w = std::min( rect.w, rect.h );
    rect.h = rect.w;

    std::string_view action;

    draw_style_type( "check", [&]()
    {
        if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && rect.contains( _p->_impl->get_cursor( id ) ) )
        {
            checked = !checked;

            action = "active";
        }
        else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && rect.contains( _p->_impl->get_cursor( id ) ) )
        {
            action = "hover";
        }

        draw_style_action( action, [&]()
        {
            draw_rect( rect,
            current_style( "border-style", xui::drawcmd::stroke::SOLID ),
            current_style( "border-width", 1.0f ),
            current_style( "border-color", xui::color() ),
            current_style( "border-radius", xui::vec4() ),
            current_style( "background-color", xui::color() ) );
        } );

        if ( checked )
        {
            draw_style_element( "indicator", [&]()
            {
                draw_style_action( action, [&]()
                {
                    draw_path(
                        current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                        current_style( "stroke-width", 1.0f ),
                        current_style( "stroke-color", xui::color() ),
                        current_style( "background-color", xui::color() ) )
                    .moveto( { rect.x + ( rect.w * 0.2f ), rect.y + ( rect.h * 0.5f ) } )
                    .lineto( { rect.x + ( rect.w * 0.4f ), rect.y + ( rect.h * 0.7f ) } )
                    .lineto( { rect.x + ( rect.w * 0.8f ), rect.y + ( rect.h * 0.2f ) } );
                } );
            } );
        }
    } );

    return checked;
}

bool xui::context::button( std::string_view text )
{
    auto id = current_window();
    auto rect = currrent_rect();

    std::string_view action;
    draw_style_type( "button", [&]()
    {
        if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && rect.contains( _p->_impl->get_cursor( id ) ) )
            action = "active";
        else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && rect.contains( _p->_impl->get_cursor( id ) ) )
            action = "hover";
        else
            action = "";

        draw_style_action( action, [&]()
        {
            draw_rect( rect,
            current_style( "border-style", xui::drawcmd::stroke::SOLID ),
            current_style( "border-width", 1.0f ),
            current_style( "border-color", xui::color() ),
            current_style( "border-radius", xui::vec4() ),
            current_style( "background-color", xui::color() ) );
        } );

        draw_style_element( "text", [&]()
        {
            if ( !text.empty() )
            {
                std::string_view action;
                if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && rect.contains( _p->_impl->get_cursor( id ) ) )
                    action = "active";
                else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && rect.contains( _p->_impl->get_cursor( id ) ) )
                    action = "hover";
                else
                    action = "";

                draw_style_action( action, [&]()
                {
                    draw_text( text, current_font(), rect, current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
                } );
            }
        } );
    } );

    return ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT) && rect.contains( _p->_impl->get_cursor( id ) ) );
}

float xui::context::slider( float & value, float min, float max )
{
    auto id = current_window();
    std::string_view action;

    draw_style_type( "slider", [&]()
    {
        auto back_rect = currrent_rect();
        xui::vec2 pos = _p->_impl->get_cursor( id );
        if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && back_rect.contains( pos ) )
            action = "active";
        else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && back_rect.contains( pos ) )
            action = "hover";

        draw_style_action( action, [&]()
        {
            draw_rect( back_rect,
            current_style( "border-style", xui::drawcmd::stroke::SOLID ),
            current_style( "border-width", 1.0f ),
            current_style( "border-color", xui::color() ),
            current_style( "border-radius", xui::vec4() ),
            current_style( "background-color", xui::color() ) );
        } );

        draw_style_element( "cursor", [&]()
        {
            float cursor_w = 0;
            float cursor_h = 0;
            float cursor_step = 0;
            xui::rect cursor_rect = {};
            float backleft = back_rect.x;
            float backright = back_rect.x + back_rect.w;
            float backtop = back_rect.y;
            float backbottom = back_rect.y + back_rect.h;

            switch ( current_style( "direction" ).value<xui::direction>( xui::direction::LEFT_RIGHT ) )
            {
            case xui::direction::LEFT_RIGHT:
            {
                backleft += 5;
                backright -= 5;
                cursor_w = 10.0f;
                cursor_h = backbottom - backtop;
                cursor_step = ( backright - backleft ) / ( max - min );
                if ( action == "active" )
                    value = std::clamp( ( ( pos.x - backleft ) / ( backright - backleft ) ) * ( max - min ), min, max );

                cursor_rect = {
                    back_rect.x + ( cursor_step * value ),
                    back_rect.y,
                    cursor_w,
                    cursor_h
                };
                break;
            }
            case xui::direction::RIGHT_LEFT:
            {
                backleft += 5;
                backright -= 5;
                cursor_w = 10.0f;
                cursor_h = backbottom - backtop;
                cursor_step = ( backright - backleft ) / ( max - min );
                if ( action == "active" )
                    value = std::clamp( ( ( backright - pos.x ) / ( backright - backleft ) ) * ( max - min ), min, max );

                cursor_rect = {
                    ( back_rect.x + back_rect.w - cursor_w ) - ( cursor_step * value ),
                    back_rect.y,
                    cursor_w,
                    cursor_h
                };
                break;
            }
            case xui::direction::TOP_BOTTOM:
            {
                backtop += 5;
                backbottom -= 5;
                cursor_w = backright - backleft;
                cursor_h = 10.0f;
                cursor_step = ( backbottom - backtop ) / ( max - min );
                if ( action == "active" )
                    value = std::clamp( ( ( pos.y - backtop ) / ( backbottom - backtop ) ) * ( max - min ), min, max );

                cursor_rect = {
                    back_rect.x,
                    back_rect.y + ( cursor_step * value ),
                    cursor_w,
                    cursor_h
                };
                break;
            }
            case xui::direction::BOTTOM_TOP:
            {
                backtop += 5;
                backbottom -= 5;
                cursor_w = backright - backleft;
                cursor_h = 10.0f;
                cursor_step = ( backbottom - backtop ) / ( max - min );
                if ( action == "active" )
                    value = std::clamp( ( ( backbottom - pos.y ) / ( backbottom - backtop ) ) * ( max - min ), min, max );

                cursor_rect = {
                    back_rect.x,
                    ( back_rect.y + back_rect.h - cursor_h ) - ( cursor_step * value ),
                    cursor_w,
                    cursor_h
                };
                break;
            }
            }

            draw_style_action( action, [&]()
            {
                draw_rect( cursor_rect,
                current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                current_style( "border-width", 1.0f ),
                current_style( "border-color", xui::color() ),
                current_style( "border-radius", xui::vec4() ),
                current_style( "background-color", xui::color() ) );
            } );
        } );
    } );

    return value;
}

bool xui::context::process( float value, float min, float max, std::string_view text )
{
    draw_style_type( "process", [&]()
    {
        auto back_rect = currrent_rect();

        draw_rect( back_rect,
                   current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                   current_style( "border-width", 1.0f ),
                   current_style( "border-color", xui::color() ),
                   current_style( "border-radius", xui::vec4() ),
                   current_style( "background-color", xui::color() ) );

        draw_style_element( "cursor", [&]()
        {
            xui::rect cursor_rect;
            value = ( value - min ) / ( max - min );

            switch ( current_style( "direction" ).value<xui::direction>( xui::direction::LEFT_RIGHT ) )
            {
            case xui::direction::LEFT_RIGHT:
                cursor_rect = { back_rect.x, back_rect.y, back_rect.w * value, back_rect.h };
                break;
            case xui::direction::RIGHT_LEFT:
                cursor_rect = { back_rect.x + ( back_rect.w - ( back_rect.w * value ) ), back_rect.y, back_rect.w * value, back_rect.h };
                break;
            case xui::direction::TOP_BOTTOM:
                cursor_rect = { back_rect.x, back_rect.y, back_rect.w, back_rect.h * value };
                break;
            case xui::direction::BOTTOM_TOP:
                cursor_rect = { back_rect.x, back_rect.y + ( back_rect.h - ( back_rect.h * value ) ), back_rect.w, back_rect.h * value };
                break;
            default:
                break;
            }

            draw_rect( cursor_rect,
                       current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                       current_style( "border-width", 1.0f ),
                       current_style( "border-color", xui::color() ),
                       current_style( "border-radius", xui::vec4() ),
                       current_style( "background-color", xui::color() ) );
        } );

        if ( !text.empty() )
        {
            draw_style_element( "text", [&]()
            {
                draw_text( text, current_font(), back_rect, current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
            } );
        }
    } );

    return true;
}

float xui::context::scrollbar( float & value, float step, float min, float max, xui::direction dir )
{
    auto id = current_window();
    std::string_view action;

    draw_style_type( "scrollbar", [&]()
    {
        auto back_rect = currrent_rect();
        xui::vec2 pos = _p->_impl->get_cursor( id );
        float arrow_radius = std::min( back_rect.w, back_rect.h );

        if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && back_rect.contains( pos ) )
            action = "active";
        else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && back_rect.contains( pos ) )
            action = "hover";
        else
            action = "";

        draw_style_action( action, [&]()
        {
            draw_rect( back_rect,
            current_style( "border-style", xui::drawcmd::stroke::SOLID ),
            current_style( "border-width", 1.0f ),
            current_style( "border-color", xui::color() ),
            current_style( "border-radius", xui::vec4() ),
            current_style( "background-color", xui::color() ) );
        } );

        draw_style_element( "cursor", [&]()
        {
            std::string_view element;

            float cursor_w = 0;
            float cursor_h = 0;
            float cursor_step = 0;
            xui::rect cursor_rect = {};
            xui::rect detect_rect = {};

            switch ( dir )
            {
            case xui::direction::LEFT_RIGHT:
            case xui::direction::RIGHT_LEFT:
                detect_rect = { back_rect.x + arrow_radius,  back_rect.y, back_rect.w - arrow_radius * 2, back_rect.h };
                if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && detect_rect.contains( pos ) )
                    action = "active";
                else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && detect_rect.contains( pos ) )
                    action = "hover";
                else
                    action = "";
                break;
            case xui::direction::TOP_BOTTOM:
            case xui::direction::BOTTOM_TOP:
                detect_rect = { back_rect.x,  back_rect.y + arrow_radius, back_rect.w, back_rect.h - arrow_radius * 2 };
                if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && detect_rect.contains( pos ) )
                    action = "active";
                else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && detect_rect.contains( pos ) )
                    action = "hover";
                else
                    action = "";
                break;
                break;
            default:
                break;
            }

            float backleft = detect_rect.x;
            float backright = detect_rect.x + detect_rect.w;
            float backtop = detect_rect.y;
            float backbottom = detect_rect.y + detect_rect.h;

            switch ( dir )
            {
            case xui::direction::LEFT_RIGHT:
            {
                backleft += arrow_radius;
                backright -= arrow_radius;
                cursor_w = arrow_radius * 2;
                cursor_h = backbottom - backtop;
                cursor_step = ( backright - backleft ) / ( max - min );

                if ( action == "active" )
                    value = std::clamp( ( ( pos.x - backleft ) / ( backright - backleft ) ) * ( max - min ), min, max );

                cursor_rect = {
                    backleft + ( cursor_step * value ) - ( cursor_w * 0.5f ),
                    backtop,
                    cursor_w,
                    cursor_h
                };
                element = "horizontal";
                break;
            }
            case xui::direction::RIGHT_LEFT:
            {
                backleft += arrow_radius;
                backright -= arrow_radius;
                cursor_w = arrow_radius * 2;
                cursor_h = backbottom - backtop;
                cursor_step = ( backright - backleft ) / ( max - min );

                if ( action == "active" )
                    value = std::clamp( ( ( backright - pos.x ) / ( backright - backleft ) ) * ( max - min ), min, max );

                cursor_rect = {
                    ( backleft + ( backright - backleft ) ) - ( cursor_step * value ) - ( cursor_w * 0.5f ),
                    backtop,
                    cursor_w,
                    cursor_h
                };

                element = "horizontal";
                break;
            }
            case xui::direction::TOP_BOTTOM:
            {
                backtop += arrow_radius;
                backbottom -= arrow_radius;
                cursor_w = backright - backleft;
                cursor_h = arrow_radius * 2;
                cursor_step = ( backbottom - backtop ) / ( max - min );

                if ( action == "active" )
                    value = std::clamp( ( ( pos.y - backtop ) / ( backbottom - backtop ) ) * ( max - min ), min, max );

                cursor_rect = {
                    backleft,
                    backtop + ( cursor_step * value ) - ( cursor_h * 0.5f ),
                    cursor_w,
                    cursor_h
                };

                element = "vertical";
                break;
            }
            case xui::direction::BOTTOM_TOP:
            {
                backtop += arrow_radius;
                backbottom -= arrow_radius;
                cursor_w = backright - backleft;
                cursor_h = arrow_radius * 2;
                cursor_step = ( backbottom - backtop ) / ( max - min );

                if ( action == "active" )
                    value = std::clamp( ( ( backbottom - pos.y ) / ( backbottom - backtop ) ) * ( max - min ), min, max );

                cursor_rect = {
                    backleft,
                    ( backtop + ( backbottom - backtop ) ) - ( cursor_step * value ) - ( cursor_h * 0.5f ),
                    cursor_w,
                    cursor_h
                };

                element = "vertical";
                break;
            }
            }

            draw_style_element( element, [&]()
            {
                draw_style_action( action, [&]()
                {
                    draw_rect( cursor_rect,
                    current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                    current_style( "border-width", 1.0f ),
                    current_style( "border-color", xui::color() ),
                    current_style( "border-radius", xui::vec4() ),
                    current_style( "background-color", xui::color() ) );
                } );
            } );
        } );

        draw_style_element( "arrow", [&]()
        {
            xui::rect arrow_rect = {};

            switch ( dir )
            {
            case xui::direction::LEFT_RIGHT:
            case xui::direction::RIGHT_LEFT:
                // left
                arrow_rect = { back_rect.x, back_rect.y, arrow_radius, arrow_radius };
                {
                    if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && arrow_rect.contains( pos ) )
                        action = "active";
                    else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && arrow_rect.contains( pos ) )
                        action = "hover";
                    else
                        action = "";

                    if ( action == "active" )
                        value = std::clamp( value + ( dir == xui::direction::LEFT_RIGHT ? -step : step ), min, max );

                    draw_style_action( action, [&]()
                    {
                        draw_rect( arrow_rect,
                        current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                        current_style( "border-width", 1.0f ),
                        current_style( "border-color", xui::color() ),
                        current_style( "border-radius", xui::vec4() ),
                        current_style( "background-color", xui::color() ) );

                        draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                   current_style( "stroke-width", 1.0f ),
                                   current_style( "stroke-color", xui::color() ),
                                   current_style( "background-color", xui::color() ) )
                            .moveto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.5f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.3f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.7f } )
                            .closepath();
                    } );
                }

                // right
                arrow_rect = { back_rect.x + back_rect.w - arrow_radius, back_rect.y, arrow_radius, arrow_radius };
                {
                    if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && arrow_rect.contains( pos ) )
                        action = "active";
                    else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && arrow_rect.contains( pos ) )
                        action = "hover";
                    else
                        action = "";

                    if ( action == "active" )
                        value = std::clamp( value + ( dir == xui::direction::RIGHT_LEFT ? -step : step ), min, max );

                    draw_style_action( action, [&]()
                    {
                        draw_rect( arrow_rect,
                        current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                        current_style( "border-width", 1.0f ),
                        current_style( "border-color", xui::color() ),
                        current_style( "border-radius", xui::vec4() ),
                        current_style( "background-color", xui::color() ) );

                        draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                   current_style( "stroke-width", 1.0f ),
                                   current_style( "stroke-color", xui::color() ),
                                   current_style( "background-color", xui::color() ) )
                            .moveto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.5f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.3f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.7f } )
                            .closepath();
                    } );
                }
                break;
            case xui::direction::TOP_BOTTOM:
            case xui::direction::BOTTOM_TOP:
                // up
                arrow_rect = { back_rect.x, back_rect.y, arrow_radius, arrow_radius };
                {
                    if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && arrow_rect.contains( pos ) )
                        action = "active";
                    else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && arrow_rect.contains( pos ) )
                        action = "hover";
                    else
                        action = "";

                    if ( action == "active" )
                        value = std::clamp( value + ( dir == xui::direction::TOP_BOTTOM ? -step : step ), min, max );

                    draw_style_action( action, [&]()
                    {
                        draw_rect( arrow_rect,
                        current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                        current_style( "border-width", 1.0f ),
                        current_style( "border-color", xui::color() ),
                        current_style( "border-radius", xui::vec4() ),
                        current_style( "background-color", xui::color() ) );

                        draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                   current_style( "stroke-width", 1.0f ),
                                   current_style( "stroke-color", xui::color() ),
                                   current_style( "background-color", xui::color() ) )
                            .moveto( { arrow_rect.x + arrow_rect.w * 0.5f, arrow_rect.y + arrow_rect.h * 0.3f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.7f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.7f } )
                            .closepath();
                    } );
                }

                // down
                arrow_rect = { back_rect.x, back_rect.y + back_rect.h - arrow_radius, arrow_radius, arrow_radius };
                {
                    if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && arrow_rect.contains( pos ) )
                        action = "active";
                    else if ( _p->_impl->get_event( id, xui::event::MOUSE_ENTER ) && arrow_rect.contains( pos ) )
                        action = "hover";
                    else
                        action = "";

                    if ( action == "active" )
                        value = std::clamp( value + ( dir == xui::direction::BOTTOM_TOP ? -step : step ), min, max );

                    draw_style_action( action, [&]()
                    {
                        draw_rect( arrow_rect,
                        current_style( "border-style", xui::drawcmd::stroke::SOLID ),
                        current_style( "border-width", 1.0f ),
                        current_style( "border-color", xui::color() ),
                        current_style( "border-radius", xui::vec4() ),
                        current_style( "background-color", xui::color() ) );

                        draw_path( current_style( "stroke-style", xui::drawcmd::stroke::SOLID ),
                                   current_style( "stroke-width", 1.0f ),
                                   current_style( "stroke-color", xui::color() ),
                                   current_style( "background-color", xui::color() ) )
                            .moveto( { arrow_rect.x + arrow_rect.w * 0.5f, arrow_rect.y + arrow_rect.h * 0.7f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.3f } )
                            .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.3f } )
                            .closepath();
                    } );
                }
                break;
            }
        } );
    } );

    return value;
}


void xui::context::push_style_type( std::string_view type )
{
    _p->_types.push_back( { type.begin(), type.end() } );
}

void xui::context::pop_style_type()
{
    _p->_types.pop_back();
}

void xui::context::push_style_element( std::string_view element )
{
    _p->_types.push_back( std::format( "{}-{}", _p->_types.back(), element ) );
}

void xui::context::pop_style_element()
{
    _p->_types.pop_back();
}

void xui::context::push_style_action( std::string_view action )
{
    _p->_types.push_back( std::format( "{}:{}", _p->_types.back(), action ) );
}

void xui::context::pop_style_action()
{
    _p->_types.pop_back();
}

xui::drawcmd::text_element & xui::context::draw_text( std::string_view text, xui::font_id id, const xui::rect & rect, const xui::color & font_color, xui::alignment_flag text_align )
{
    xui::drawcmd::text_element element;

    element.font = id;
    element.text = text;
    element.rect = rect;
    element.color = font_color;// current_style( "font-color" ).value<xui::color>();
    element.align = text_align;// current_style( "text-align" ).value<xui::alignment_flag>( xui::alignment_flag::ALIGN_CENTER );

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::text_element>( _p->_commands.back().element );
}

xui::drawcmd::line_element & xui::context::draw_line( const xui::vec2 & p1, const xui::vec2 & p2, uint32_t stroke_style, float stroke_width, const xui::color & stroke_color )
{
    xui::drawcmd::line_element element;

    element.p1 = p1;
    element.p2 = p2;
    element.stroke.style = stroke_style;// current_style( "stroke-style" ).value<uint32_t>( xui::drawcmd::stroke::SOLID );
    element.stroke.width = stroke_width;// current_style( "stroke-width" ).value<float>( 1.0f )
    element.stroke.color = stroke_color;// current_style( "stroke-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::line_element>( _p->_commands.back().element );
}

xui::drawcmd::rect_element & xui::context::draw_rect( const xui::rect & rect, uint32_t border_style, float border_width, const xui::color & border_color, const xui::vec4 & border_radius, const xui::color & background_color )
{
    xui::drawcmd::rect_element element;

    element.rect = rect;
    element.border.style = border_style;// current_style( "border-style" ).value<uint32_t>( xui::drawcmd::stroke::SOLID );
    element.border.width = border_width;// current_style( "border-width" ).value<float>( 1.0f );
    element.border.color = border_color;// current_style( "border-color" ).value<xui::color>();
    element.border.radius = border_radius;// current_style( "border-radius" ).value<xui::vec4>();
    element.filled.color = background_color;// current_style( "background-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::rect_element>( _p->_commands.back().element );
}

xui::drawcmd::path_element & xui::context::draw_path( uint32_t stroke_style, float stroke_width, const xui::color & stroke_color, const xui::color & background_color )
{
    xui::drawcmd::path_element element;

    element.stroke.style = stroke_style;// current_style( "stroke-style" ).value<uint32_t>( xui::drawcmd::stroke::SOLID );
    element.stroke.width = stroke_width;// current_style( "stroke-width" ).value<float>( 1.0f );
    element.stroke.color = stroke_color;// current_style( "stroke-color" ).value<xui::color>();
    element.filled.color = background_color;// current_style( "background-color" ).value<xui::color>();

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

xui::drawcmd::circle_element & xui::context::draw_circle( const xui::vec2 & center, float radius, uint32_t border_style, float border_width, const xui::color & border_color, const xui::vec4 & border_radius, const xui::color & background_color )
{
    xui::drawcmd::circle_element element;

    element.center = center;
    element.radius = radius;
    element.border.style = border_style;// current_style( "border-style" ).value<uint32_t>( xui::drawcmd::stroke::SOLID );
    element.border.width = border_width;// current_style( "border-width" ).value<float>( 1.0f );
    element.border.color = border_color;// current_style( "border-color" ).value<xui::color>();
    element.border.radius = border_radius;// current_style( "border-radius" ).value<xui::vec4>();
    element.filled.color = background_color;// current_style( "background-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::circle_element>( _p->_commands.back().element );
}

xui::drawcmd::ellipse_element & xui::context::draw_ellipse( const xui::vec2 & center, const xui::vec2 & radius, uint32_t border_style, float border_width, const xui::color & border_color, const xui::vec4 & border_radius, const xui::color & background_color )
{
    xui::drawcmd::ellipse_element element;

    element.center = center;
    element.radius = radius;
    element.border.style = border_style;// current_style( "border-style" ).value<uint32_t>( xui::drawcmd::stroke::SOLID );
    element.border.width = border_width;// current_style( "border-width" ).value<float>( 1.0f );
    element.border.color = border_color;// current_style( "border-color" ).value<xui::color>();
    element.border.radius = border_radius;// current_style( "border-radius" ).value<xui::vec4>();
    element.filled.color = background_color;// current_style( "background-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::ellipse_element>( _p->_commands.back().element );
}

xui::drawcmd::polygon_element & xui::context::draw_polygon( std::span<xui::vec2> points, uint32_t border_style, float border_width, const xui::color & border_color, const xui::vec4 & border_radius, const xui::color & background_color )
{
    xui::drawcmd::polygon_element element;

    element.points.assign( points.begin(), points.end() );
    element.border.style = border_style;// current_style( "border-style" ).value<uint32_t>( xui::drawcmd::stroke::SOLID );
    element.border.width = border_width;// current_style( "border-width" ).value<float>( 1.0f );
    element.border.color = border_color;// current_style( "border-color" ).value<xui::color>();
    element.border.radius = border_radius;// current_style( "border-radius" ).value<xui::vec4>();
    element.filled.color = background_color;// current_style( "background-color" ).value<xui::color>();

    _p->_commands.push_back( { current_window(), element } );

    return std::get<xui::drawcmd::polygon_element>( _p->_commands.back().element );
}
