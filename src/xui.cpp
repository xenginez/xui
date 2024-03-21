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
    static constexpr int action_nil_type = 0;
    static constexpr int action_menu_type = 1;
    static constexpr int action_focus_type = 2;
    static std::regex int_regex{ R"([-+]?([0-9]*[0-9]+))" };
    static std::regex flt_regex{ R"([-+]?([0-9]*.[0-9]+|[0-9]+))" };

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

    struct style_type
    {
        style_type( auto beg, auto end, std::pmr::memory_resource * res )
            : actions( res ), elements( res ), type( beg, end, res )
        {
        }
        style_type( std::pmr::memory_resource * res )
            : actions( res ), elements( res ), type( res )
        {
        }

        std::pmr::string type;
        std::pmr::deque<std::string> elements;
        std::pmr::deque<std::string> actions;
    };
    struct focus_type
    {
        int type = 0;
        xui::rect rect;
        std::string name;
        xui::window_id id = xui::invalid_id;
        xui::event event = xui::event::EVENT_MAX_COUNT;
    };

    struct window_type
    {
        int flags;
        xui::texture_id icon;
        std::string_view title;
    };
    using info_type = std::variant<std::monostate, window_type>;
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
    return { x + left, y + top, w - left + right, h - top + bottom };
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

bool xui::style::parse( std::string_view str )
{
    _selectors.clear();

    bool result = true;
    std::string name;
    auto beg = str.begin();
    auto end = str.end();

    while ( beg != end )
    {
        if ( std::isspace( *beg ) )
        {
            ++beg;
        }
        else if ( *beg == '{' )
        {
            _selectors.insert( { name, parse_selector( beg, end ) } );
            name.clear();
        }
        else if ( *beg == ',' )
        {
            ++beg;
        }
        else
        {
            name.push_back( *beg++ );
        }
    }

    return result;
}

xui::style::variant xui::style::find( std::string_view name ) const
{
    // {id}#{type}-{element}-{element}-{element}:{action}@{attr}
    std::string_view type, action, id, attr;
    std::pmr::vector<std::string_view> elements( _selectors.get_allocator().resource() );


    // {id}
    if ( name.find( '#' ) != std::string_view::npos )
    {
        id = { name.begin(), name.begin() + name.find( '#' ) + 1 };
        name = { name.begin() + name.find( '#' ) + 1, name.end() };
    }
    // {attr}
    if ( name.find( '@' ) != std::string_view::npos )
    {
        attr = { name.begin() + name.find( '@' ) + 1, name.end() };
        name = { name.begin(), name.begin() + name.find( '@' ) };
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

    std::optional<xui::style::variant> opt;

    // {id}#{type}-{element}-{element}-{element}:{action}@{attr}
    opt = find( std::format( "{}{}{}{}", id, type, std::span<std::string_view>{ elements }, action ), attr );
    if ( opt.has_value() && opt.value().index() != variant::inherit_idx )
    {
        return opt.value();
    }

    // {type}-{element}-{element}-{element}:{action}@{attr}
    opt = find( std::format( "{}{}{}", type, std::span<std::string_view>{ elements }, action ), attr );
    if ( opt.has_value() && opt.value().index() != variant::inherit_idx )
    {
        return opt.value();
    }

    // {type}-{element}-{element}-{element}@{attr}
    opt = find( std::format( "{}{}", type, std::span<std::string_view>{ elements } ), attr );
    if ( opt.has_value() && opt.value().index() != variant::inherit_idx )
    {
        return opt.value();
    }

    // {type}-{element}-{element}@{attr}
    // {type}-{element}@{attr}
    if ( !elements.empty() )
    {
        auto beg = elements.begin();
        auto end = elements.end();

        while ( beg != end )
        {
            opt = find( std::format( "{}{}", type, std::span<std::string_view>{ beg, end } ), attr );
            if ( opt.has_value() && opt.value().index() != variant::inherit_idx )
            {
                return opt.value();
            }

            --end;
        }
    }

    // {type}@{attr}
    opt = find( { type.begin(), type.end() }, attr );
    if ( opt.has_value() && opt.value().index() != variant::inherit_idx )
    {
        return opt.value();
    }

    // *@{attr}
    opt = find( "*", attr );
    if ( opt.has_value() && opt.value().index() != variant::inherit_idx )
    {
        return opt.value();
    }


    return {};
}

std::optional<xui::style::variant> xui::style::find( std::string_view type, std::string_view attr ) const
{
    auto it = _selectors.find( { type.begin(), type.end() } );
    if ( it != _selectors.end() )
    {
        auto it2 = it->second.attrs.find( { attr.begin(), attr.end() } );
        if ( it2 != it->second.attrs.end() )
        {
            if ( it2->second.index() == xui::style::variant::inherit_idx )
            {
                int i = 0;
            }
            return it2->second;
        }
    }
    return {};
}

xui::style::selector xui::style::parse_selector( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '{' && "" );

    std::string name;
    xui::style::selector select;

    while ( beg != end )
    {
        if ( std::isspace( *beg ) )
        {
            ++beg;
        }
        else if ( *beg == ':' )
        {
            ++beg;
            select.attrs.insert( { name, parse_attribute( beg, end ) } );
            name.clear();
        }
        else if( *beg == ';' )
        {
            ++beg;
            name.clear();
        }
        else if ( *beg == '}' )
        {
            ++beg;
            return select;
        }
        else
        {
            name.push_back( *beg++ );
        }
    }

    return select;
}

xui::style::variant xui::style::parse_attribute( std::string_view::iterator & beg, std::string_view::iterator end )
{
    std::string value;

    while ( beg != end )
    {
        if ( value.empty() && std::isspace( *beg ) )
        {
            ++beg;
        }
        else if ( *beg == '#' )
        {
            return parse_hex( beg, end );
        }
        else if ( *beg == '(' )
        {
            if ( functions().find( value ) != functions().end() )
            {
                return functions()[value]( beg, end );
            }
            else
            {
                break;
            }
        }
        else if ( *beg == ',' || *beg == ';' || *beg == ')' )
        {
            ++beg;

            while ( std::isspace( value.back() ) ) value.pop_back();

            if ( value == "inherit" )
            {
                return xui::style::inherit();
            }
            else if ( std::regex_match( value, int_regex ) )
            {
                return std::stoi( value );
            }
            else if ( std::regex_match( value, flt_regex ) )
            {
                return std::stof( value );
            }
            else if ( flags().find( value ) != flags().end() )
            {
                return flags()[value];
            }
            else if ( colors().find( value ) != colors().end() )
            {
                return xui::color( colors()[value] );
            }
            else
            {
                break;
            }
        }
        else
        {
            value.push_back( *beg++ );
        }
    }

    return value;
}

xui::color xui::style::parse_light( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    return parse_attribute( beg, end ).value<xui::color>().light();
}

xui::color xui::style::parse_dark( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    return parse_attribute( beg, end ).value<xui::color>().dark();
}

xui::color xui::style::parse_rgba( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::color result;

    result.r = parse_attribute( beg, end ).value<int>(); if (check<','>( beg, end ) ) ++beg;
    result.g = parse_attribute( beg, end ).value<int>(); if (check<','>( beg, end ) ) ++beg;
    result.b = parse_attribute( beg, end ).value<int>(); if (check<','>( beg, end ) ) ++beg;
    result.a = parse_attribute( beg, end ).value<int>(); if (check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::color xui::style::parse_rgb( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::color result;

    result.r = parse_attribute( beg, end ).value<int>(); if ( check<','>( beg, end ) ) ++beg;
    result.g = parse_attribute( beg, end ).value<int>(); if ( check<','>( beg, end ) ) ++beg;
    result.b = parse_attribute( beg, end ).value<int>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::vec2 xui::style::parse_vec2( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::vec2 result;

    result.x = parse_attribute( beg, end ).value<float>(); if ( check<','>( beg, end ) ) ++beg;
    result.y = parse_attribute( beg, end ).value<float>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::vec4 xui::style::parse_vec4( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::vec4 result;

    result.x = parse_attribute( beg, end ).value<float>(); if ( check<','>( beg, end ) ) ++beg;
    result.y = parse_attribute( beg, end ).value<float>(); if ( check<','>( beg, end ) ) ++beg;
    result.z = parse_attribute( beg, end ).value<float>(); if ( check<','>( beg, end ) ) ++beg;
    result.w = parse_attribute( beg, end ).value<float>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::color xui::style::parse_hex( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '#' && "" );

    xui::color color;

    auto it = beg;
    while ( *it != ';' ) ++it;

    std::from_chars( beg.operator->(), it.operator->(), color.hex, 16 );
    beg = it;

    return color;
}

xui::url xui::style::parse_url( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    std::string result;

    while ( beg != end && *beg != ')' )
    {
        if ( !std::isspace( *beg ) )
            result.push_back( *beg++ );
        else
            ++beg;
    }

    check<')'>( beg, end );

    return result;
}

xui::hatch_color xui::style::parse_hatch( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::hatch_color result;

    result.fore = parse_attribute( beg, end ).value<xui::color>(); if ( check<','>( beg, end ) ) ++beg;
    result.back = parse_attribute( beg, end ).value<xui::color>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::texture_brush xui::style::parse_sample( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::texture_brush result;

    result.image = parse_attribute( beg, end ).value<xui::url>(); if ( check<','>( beg, end ) ) ++beg;
    result.mode = parse_attribute( beg, end ).value<xui::texture_brush::warp>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::linear_gradient xui::style::parse_linear( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::linear_gradient result;

    result.p1 = parse_attribute( beg, end ).value<xui::vec2>(); if ( check<','>( beg, end ) ) ++beg;
    result.p2 = parse_attribute( beg, end ).value<xui::vec2>(); if ( check<','>( beg, end ) ) ++beg;
    result.c1 = parse_attribute( beg, end ).value<xui::color>(); if ( check<','>( beg, end ) ) ++beg;
    result.c2 = parse_attribute( beg, end ).value<xui::color>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::stroke xui::style::parse_stroke( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::stroke result;

    result.style = parse_attribute( beg, end ).value<uint32_t>(); if ( check<','>( beg, end ) ) ++beg;
    result.width = parse_attribute( beg, end ).value<float>(); if ( check<','>( beg, end ) ) ++beg;
    result.color = parse_attribute( beg, end ).value<xui::color>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::border xui::style::parse_border( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::border result;

    result.style = parse_attribute( beg, end ).value<uint32_t>(); if ( check<','>( beg, end ) ) ++beg;
    result.width = parse_attribute( beg, end ).value<float>(); if ( check<','>( beg, end ) ) ++beg;
    result.color = parse_attribute( beg, end ).value<xui::color>(); if ( check<','>( beg, end ) ) ++beg;
    result.radius = parse_attribute( beg, end ).value<xui::vec4>(); if ( check<')'>( beg, end ) ) ++beg;

    return result;
}

xui::filled xui::style::parse_filled( std::string_view::iterator & beg, std::string_view::iterator end )
{
    _ASSERT( *beg++ == '(' && "" );

    xui::filled result;

    result.style = parse_attribute( beg, end ).value<uint32_t>(); if ( check<','>( beg, end ) ) ++beg;
    switch ( result.style )
    {
    case xui::filled::SOLID:
        result.colors = parse_attribute( beg, end ).value<xui::color>(); if ( check<')'>( beg, end ) ) ++beg;
        break;
    case xui::filled::DENSE1:
    case xui::filled::DENSE2:
    case xui::filled::DENSE3:
    case xui::filled::DENSE4:
    case xui::filled::DENSE5:
    case xui::filled::DENSE6:
    case xui::filled::DENSE7:
    case xui::filled::HORIZONTAL:
    case xui::filled::VERTICAL:
    case xui::filled::CROSS:
    case xui::filled::FORWARD:
    case xui::filled::BACKWARD:
    case xui::filled::DIAGCROSS:
        result.colors = parse_attribute( beg, end ).value<xui::hatch_color>(); if ( check<')'>( beg, end ) ) ++beg;
        break;
    case xui::filled::TEXTURE:
        result.colors = parse_attribute( beg, end ).value<xui::texture_brush>(); if ( check<')'>( beg, end ) ) ++beg;
        break;
    case xui::filled::LINEAR_GRADIENT:
        result.colors = parse_attribute( beg, end ).value<xui::linear_gradient>(); if ( check<')'>( beg, end ) ) ++beg;
        break;
    }

    return result;
}

std::map<std::string_view, std::uint32_t> & xui::style::flags()
{
    static std::map<std::string_view, std::uint32_t> style_flags =
    {
        // texture_brush mode
        { "tile", xui::texture_brush::warp::WRAP_TILE },
        { "flipx", xui::texture_brush::warp::WRAP_TILEFLIPX },
        { "flipy", xui::texture_brush::warp::WRAP_TILEFLIPY },
        { "flipxy", xui::texture_brush::warp::WRAP_TILEFLIPXY },
        { "clamp", xui::texture_brush::warp::WRAP_CLAMP },

        // stroke filled share
        { "solid", 0 }, // { "solid", xui::stroke::SOLID }, { "solid", xui::drawcmd::filled::SOLID },

        // stroke style
        { "dashed", xui::stroke::DASHED },
        { "dotted", xui::stroke::DOTTED },
        { "dashdot", xui::stroke::DASH_DOT },
        { "dashdotdot", xui::stroke::DASH_DOT_DOT },

        // filled style
        { "dense1", xui::filled::DENSE1 },
        { "dense2", xui::filled::DENSE2 },
        { "dense3", xui::filled::DENSE3 },
        { "dense4", xui::filled::DENSE4 },
        { "dense5", xui::filled::DENSE5 },
        { "dense6", xui::filled::DENSE6 },
        { "dense7", xui::filled::DENSE7 },
        { "horizontal", xui::filled::HORIZONTAL },
        { "vertical", xui::filled::VERTICAL },
        { "cross", xui::filled::CROSS },
        { "forward", xui::filled::FORWARD },
        { "backward", xui::filled::BACKWARD },
        { "diagcross", xui::filled::DIAGCROSS },
        { "linear", xui::filled::LINEAR_GRADIENT },
        { "texture", xui::filled::TEXTURE },

        // alignment
        { "left", xui::alignment_flag::ALIGN_LEFT },
        { "right", xui::alignment_flag::ALIGN_RIGHT },
        { "top", xui::alignment_flag::ALIGN_TOP },
        { "bottom", xui::alignment_flag::ALIGN_BOTTOM },
        { "center", xui::alignment_flag::ALIGN_CENTER },
        { "vcenter", xui::alignment_flag::ALIGN_VCENTER },
        { "hcenter", xui::alignment_flag::ALIGN_HCENTER },
        { "left top", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_TOP },
        { "top left", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_TOP },
        { "left bottom", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_BOTTOM },
        { "bottom left", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_BOTTOM },
        { "right top", xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_TOP },
        { "top right", xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_TOP },
        { "right bottom", xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_BOTTOM },
        { "bottom right", xui::alignment_flag::ALIGN_RIGHT | xui::alignment_flag::ALIGN_BOTTOM },
        { "left vcenter", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "vcenter left", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "right vcenter", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "vcenter right", xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER },
        { "top hcenter", xui::alignment_flag::ALIGN_TOP | xui::alignment_flag::ALIGN_HCENTER },
        { "hcenter top", xui::alignment_flag::ALIGN_TOP | xui::alignment_flag::ALIGN_HCENTER },
        { "bottom hcenter", xui::alignment_flag::ALIGN_BOTTOM | xui::alignment_flag::ALIGN_HCENTER },
        { "hcenter bottom", xui::alignment_flag::ALIGN_BOTTOM | xui::alignment_flag::ALIGN_HCENTER },
        { "vcenter hcenter", xui::alignment_flag::ALIGN_CENTER },
        { "hcenter vcenter", xui::alignment_flag::ALIGN_CENTER },

        // direction
        { "ltr", (std::uint32_t)xui::direction::LEFT_RIGHT },
        { "rtl", (std::uint32_t)xui::direction::RIGHT_LEFT },
        { "ttb", (std::uint32_t)xui::direction::TOP_BOTTOM },
        { "btt", (std::uint32_t)xui::direction::BOTTOM_TOP },
        { "left right", (std::uint32_t)xui::direction::LEFT_RIGHT },
        { "right left", (std::uint32_t)xui::direction::RIGHT_LEFT },
        { "top bottom", (std::uint32_t)xui::direction::TOP_BOTTOM },
        { "bottom top", (std::uint32_t)xui::direction::BOTTOM_TOP },
        { "left to right", (std::uint32_t)xui::direction::LEFT_RIGHT },
        { "right to left", (std::uint32_t)xui::direction::RIGHT_LEFT },
        { "top to bottom", (std::uint32_t)xui::direction::TOP_BOTTOM },
        { "bottom to top", (std::uint32_t)xui::direction::BOTTOM_TOP },
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

std::map<std::string_view, std::function<xui::style::variant( std::string_view::iterator &, std::string_view::iterator )>> & xui::style::functions()
{
    static std::map<std::string_view, std::function<xui::style::variant( std::string_view::iterator &, std::string_view::iterator )>> style_functions =
    {
        {"url", xui::style::parse_url },
        {"rgb", xui::style::parse_rgb },
        {"rgba", xui::style::parse_rgba },
        {"vec2", xui::style::parse_vec2 },
        {"vec4", xui::style::parse_vec4 },
        {"dark", xui::style::parse_dark },
        {"light", xui::style::parse_light },
        {"hatch", xui::style::parse_hatch },
        {"sample", xui::style::parse_sample },
        {"linear", xui::style::parse_linear },
        {"stroke", xui::style::parse_stroke },
        {"border", xui::style::parse_border },
        {"filled", xui::style::parse_filled },
    };
    return style_functions;
}





struct xui::context::private_p
{
public:
    private_p( std::pmr::memory_resource * res )
        : _res( res )
        , _commands( res )
        , _disables( res )
        , _infos( res )
        , _rects( res )
        , _focus( res )
        , _types( res )
        , _fonts( res )
        , _styles( res )
        , _strids( res )
        , _windows( res )
        , _textures( res )
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
    int _str_id = 0;
    std::pmr::deque<bool> _disables;
    std::pmr::deque<info_type> _infos;
    std::pmr::deque<xui::rect> _rects;
    std::pmr::deque<focus_type> _focus;
    std::pmr::deque<style_type> _types;
    std::pmr::deque<xui::font_id> _fonts;
    std::pmr::deque<xui::style *> _styles;
    std::pmr::deque<std::string> _strids;
    std::pmr::deque<xui::window_id> _windows;
    std::pmr::deque<xui::texture_id> _textures;
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
        font-color: white;
        text-align: center;

        margin: vec4( 0, 0, 0, 0 );
        padding: vec4( 0, 0, 0, 0 );

        filled: filled( solid, black );
        stroke: stroke( solid, 1, white );
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
    },
    window{
        border: border( solid, 1, transparent, vec4( 10, 10, 10, 10 ) );
        filled: filled( solid, black );
    },
    window-resize{
        stroke: stroke( solid, 1, white );
        filled: filled( dense2, hatch( white, transparent ) );
    },
    window-titlebar{
        filled: filled( solid, rgb( 76, 74, 72 ) );
        border: border( solid, 1, transparent, vec4( 10, 10, 0, 0 ) );
    },
    window-titlebar-title{
        font-color: white;
        text-align: left vcenter;
    },
    window-titlebar-closebox{
        stroke: stroke( solid, 1, white );
        border: border( solid, 1, transparent, vec4( 0, 10, 0, 0 ) );
        filled: filled( solid, rgb( 76, 74, 72 ) );
    },
    window-titlebar-closebox:hover{
        stroke: stroke( solid, 1, white );
        border: border( solid, 1, transparent, vec4( 0, 10, 0, 0 ) );
        filled: filled( solid, red );
    },
    window-titlebar-closebox:disable{
        stroke: stroke( solid, 1, darkgray );
        border: border( solid, 1, transparent, vec4( 0, 10, 0, 0 ) );
        filled: filled( solid, rgb( 76, 74, 72 ) );
    },
    window-titlebar-minimizebox{
        stroke: stroke( solid, 1, white );
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, rgb( 76, 74, 72 ) );
    },
    window-titlebar-minimizebox:hover{
        stroke: stroke( solid, 1, white );
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, rgb( 94, 92, 91 ) );
    },
    window-titlebar-minimizebox:disable{
        stroke: stroke( solid, 1, darkgray );
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, rgb( 76, 74, 72 ) );
    },
    window-titlebar-maximizebox{
        stroke: stroke( solid, 1, white );
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, rgb( 76, 74, 72 ) );
    },
    window-titlebar-maximizebox:hover{
        stroke: stroke( solid, 1, white );
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, rgb( 94, 92, 91 ) );
    },
    window-titlebar-maximizebox:disable{
        stroke: stroke( solid, 1, darkgray );
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, rgb( 76, 74, 72 ) );
    },
    menubar{
        filled: filled( solid, rgb( 76, 74, 72 ) );
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
    },
    menubar-menu{
        minimum-width: 50;
        maximum-width: 150;
    },
    menubar-menu:hover{
        filled: filled( solid, green );
    },
    menubar-menu:active{
        filled: filled( solid, darkgray );
    },
    menubar-item{
    },
    menubar-list{
    },
    label{
        font-color: green;
    },
    image{
    },
    slider{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, white );
        direction: bottom to top;
    },
    slider:hover{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, green );
    },
    slider:active{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, yellow );
    },
    slider-cursor{
        border: border( solid, 1, transparent, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, red );
    },
    process{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, rgb( 94, 92, 91 ) );
    },
    process-text{
        font-color: white;
    },
    process-cursor{
        border-color: transparent;
        filled: filled( solid, red );
        direction: left to right;
    },
    button{
        border: border( solid, 1, black, vec4( 10, 0, 10, 0 ) );
        filled: filled( solid, white );
    },
    button:hover{
        border: border( solid, 1, black, vec4( 10, 0, 10, 0 ) );
        filled: filled( solid, red );
    },
    button:active{
        border: border( solid, 1, black, vec4( 10, 0, 10, 0 ) );
        filled: filled( solid, green );
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
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, blue );
    },
    radio:hover{
        filled: filled( solid, light( blue ) );
    },
    radio:active{
        filled: filled( solid, green );
    },
    radio-indicator{
        filled: filled( solid, white );
    },
    check{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, blue );
    },
    check:hover{
        filled: filled( solid, light( blue ) );
    },
    check:active{
        filled: filled( solid, green );
    },
    check-indicator{
        stroke: stroke( solid, 3, white );
    },
    scrollbar{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, green );
    },
    scrollbar-cursor{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, red );
    },
    scrollbar-arrow{
        border: border( solid, 1, white, vec4( 0, 0, 0, 0 ) );
        filled: filled( solid, blue );
        stroke: stroke( solid, 3, white );
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

xui::style::variant xui::context::current_style( std::string_view attr ) const
{
    std::string name = style_name();

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

void xui::context::push_style_type( std::string_view type )
{
    _p->_types.push_back( { type.begin(), type.end(), _p->_res } );
}

void xui::context::pop_style_type()
{
    _p->_types.pop_back();
}

std::string_view xui::context::current_style_type() const
{
    return _p->_types.back().type;
}

void xui::context::push_style_element( std::string_view element )
{
    _p->_types.back().elements.push_back( { element.begin(), element.end() } );
}

void xui::context::pop_style_element()
{
    _p->_types.back().elements.pop_back();
}

std::string_view xui::context::current_style_element() const
{
    return _p->_types.back().elements.back();
}

void xui::context::push_style_action( std::string_view action )
{
    _p->_types.back().actions.push_back( { action.begin(), action.end() } );
}

void xui::context::pop_style_action()
{
    _p->_types.back().actions.pop_back();
}

std::string_view xui::context::current_style_action() const
{
    return _p->_types.back().actions.back();
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

void xui::context::push_rect( const xui::rect & rect )
{
    _p->_rects.push_back( rect );
}

void xui::context::margins_currrent_rect( float left, float right, float top, float bottom )
{
    _p->_rects.back() = _p->_rects.back().margins_added( left, right, top, bottom );
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

void xui::context::push_disable( bool val )
{
    _p->_disables.push_back( val );
}

void xui::context::pop_disable()
{
    _p->_disables.pop_back();
}

bool xui::context::current_disable() const
{
    if ( _p->_disables.empty() )
        return false;

    return _p->_disables.back();
}

void xui::context::push_string_id( xui::string_id id )
{
    _p->_strids.push_back( { id.begin(), id.end() } );
}

void xui::context::pop_string_id()
{
    _p->_strids.pop_back();
}

xui::string_id xui::context::current_string_id() const
{
    if ( _p->_strids.empty() )
    {
        XUI_ERR( ERR_NO );
        return {};
    }

    return _p->_strids.back();
}

void xui::context::push_window_id( xui::window_id id )
{
    _p->_windows.push_back( id );
}

void xui::context::pop_window_id()
{
    return _p->_windows.pop_back();
}

xui::window_id xui::context::current_window_id() const
{
    if ( _p->_windows.empty() )
    {
        XUI_ERR( ERR_NO );
        return xui::invalid_id;
    }

    return _p->_windows.back();
}

void xui::context::push_texture_id( xui::texture_id id )
{
    _p->_textures.push_back( id );
}

void xui::context::pop_texture_id()
{
    _p->_textures.pop_back();
}

xui::texture_id xui::context::current_texture_id() const
{
    if ( _p->_textures.empty() )
    {
        XUI_ERR( ERR_NO );
        return xui::invalid_id;
    }

    return _p->_textures.back();
}

void xui::context::begin()
{
    _p->_str_id = 0;
    _p->_commands.clear();

    while ( !_p->_focus.empty() )
    {
        if ( _p->_focus.back().type == action_focus_type )
        {
            if ( _p->_focus.back().event >= xui::event::KEY_EVENT_BEG && _p->_focus.back().event <= xui::event::KEY_EVENT_END )
            {

            }
            else if ( _p->_focus.back().event >= xui::event::MOUSE_EVENT_BEG && _p->_focus.back().event <= xui::event::MOUSE_EVENT_END )
            {
                if ( _p->_impl->get_event( _p->_focus.back().id, _p->_focus.back().event ) == 0 )
                {
                    _p->_focus.pop_back();
                    continue;
                }
            }
            else if ( _p->_focus.back().event >= xui::event::GAMEPAD_EVENT_BEG && _p->_focus.back().event <= xui::event::GAMEPAD_EVENT_END )
            {

            }
        }
        else if ( _p->_focus.back().type == action_menu_type )
        {
            if ( _p->_focus.back().event >= xui::event::MOUSE_EVENT_BEG && _p->_focus.back().event <= xui::event::MOUSE_EVENT_END )
            {
                if ( _p->_impl->get_event( _p->_focus.back().id, _p->_focus.back().event ) == 1 )
                {
                    if ( !_p->_focus.back().rect.contains( _p->_impl->get_cursor_pos( _p->_focus.back().id ) ) )
                    {
                        _p->_focus.pop_back();
                        continue;
                    }
                }
            }
        }

        break;
    }
}

std::span<xui::drawcmd> xui::context::end()
{
    _p->_disables.clear();
    _p->_infos.clear();
    _p->_rects.clear();
    _p->_types.clear();
    _p->_fonts.clear();
    _p->_styles.clear();
    _p->_strids.clear();
    _p->_windows.clear();
    _p->_textures.clear();

    std::sort( _p->_commands.begin(), _p->_commands.end(), []( const auto & left, const auto & right ) { return left.id < right.id; } );

    return _p->_commands;
}

bool xui::context::begin_window( std::string_view title, xui::texture_id icon_id, int flags )
{
    return begin_window( std::format( "_window_{}", _p->_str_id++ ), title, icon_id, flags );
}

bool xui::context::begin_window( xui::string_id str_id, std::string_view title, xui::texture_id icon_id, int flags )
{
    push_string_id( str_id );
    {
        window_type t;
        t.flags = flags;
        t.icon = icon_id;
        t.title = title;
        _p->_infos.push_back( t );
    }

    auto id = current_window_id();
    auto wrect = currrent_rect();
    auto cursorpos = _p->_impl->get_cursor_pos( id );

    wrect.x = 0; wrect.y = 0;

    auto status = _p->_impl->get_window_status( id );

    if ( ( status & xui::window_status::WINDOW_SHOW ) != 0 )
    {
        draw_style_type( "window", [&]()
        {
            xui::rect title_rect = { 0, 0, wrect.w, 30 };
            xui::rect move_rect = { 0, 0, wrect.w - 150, 30 };
            xui::rect resize_rect = { wrect.x + ( wrect.w - 15 ), wrect.y + ( wrect.h - 15 ), 15, 15 };

            if ( ( flags & xui::window_flag::WINDOW_NO_BACKGROUND ) == 0 )
            {
                draw_rect( wrect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
            }
            if ( ( flags & xui::window_flag::WINDOW_NO_TITLEBAR ) == 0 )
            {
                wrect.y += 30;
                wrect.h -= 30;
            }
        } );
    }

    push_rect( wrect );

    status = _p->_impl->get_window_status( id );
    return ( ( status & xui::window_status::WINDOW_SHOW ) != 0 );
}

void xui::context::end_window()
{
    auto t = std::get<window_type>( _p->_infos.back() );
    _p->_infos.pop_back();

    pop_string_id();
    pop_rect();

    auto id = current_window_id();
    auto wrect = currrent_rect();
    auto cursorpos = _p->_impl->get_cursor_pos( id );

    wrect.x = 0; wrect.y = 0;

    auto status = _p->_impl->get_window_status( id );

    if ( ( status & xui::window_status::WINDOW_SHOW ) != 0 )
    {
        draw_style_type( "window", [&]()
        {
            xui::rect title_rect = { 0, 0, wrect.w, 30 };
            xui::rect move_rect = { 0, 0, wrect.w - 150, 30 };
            xui::rect resize_rect = { wrect.x + ( wrect.w - 15 ), wrect.y + ( wrect.h - 15 ), 15, 15 };

            if ( ( t.flags & xui::window_flag::WINDOW_NO_TITLEBAR ) == 0 )
            {
                wrect.y += 30;
                wrect.h -= 30;

                draw_style_element( "titlebar", [&]()
                {
                    draw_rect( title_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                    draw_image( t.icon, { 8, 5, 20, 20 } );

                    draw_style_element( "title", [&]()
                    {
                        draw_text( t.title, current_font(), { 30, 5, wrect.w - 150, 20 }, current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
                    } );

                    xui::rect box_rect = { title_rect.w, title_rect.y, 50, title_rect.h };

                    draw_style_element( "closebox", [&]()
                    {
                        box_rect.x -= 50;

                        draw_disable( ( t.flags & xui::window_flag::WINDOW_NO_CLOSEBOX ) != 0, [&]()
                        {
                            draw_rect( box_rect, [&]()
                            {
                                std::string_view action = get_action_name();
                                draw_style_action( action, [&]()
                                {
                                    draw_rect( box_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                                    draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                                        .moveto( { box_rect.center().x - 5, box_rect.center().y - 5 } )
                                        .lineto( { box_rect.center().x + 5, box_rect.center().y + 5 } )
                                        .closepath()
                                        .moveto( { box_rect.center().x + 5, box_rect.center().y - 5 } )
                                        .lineto( { box_rect.center().x - 5, box_rect.center().y + 5 } )
                                        .closepath();
                                } );

                                if ( action == "active" )
                                    _p->_impl->remove_window( id );
                            } );
                        } );
                    } );

                    draw_style_element( "maximizebox", [&]()
                    {
                        box_rect.x -= 49;

                        draw_disable( ( t.flags & xui::window_flag::WINDOW_NO_MAXIMIZEBOX ) != 0, [&]()
                        {
                            draw_rect( box_rect, [&]()
                            {
                                std::string_view action = get_action_name();
                                draw_style_action( action, [&]()
                                {
                                    draw_rect( box_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                                    if ( ( status & xui::window_status::WINDOW_MAXIMIZE ) != 0 )
                                    {
                                        draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
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
                                        draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                                            .moveto( { box_rect.center().x - 5, box_rect.center().y - 5 } )
                                            .lineto( { box_rect.center().x - 5, box_rect.center().y + 5 } )
                                            .lineto( { box_rect.center().x + 5, box_rect.center().y + 5 } )
                                            .lineto( { box_rect.center().x + 5, box_rect.center().y - 5 } )
                                            .lineto( { box_rect.center().x - 5, box_rect.center().y - 5 } )
                                            .closepath();
                                    }
                                } );

                                if ( action == "active" )
                                    _p->_impl->set_window_status( id, ( ( status & xui::window_status::WINDOW_MAXIMIZE ) != 0 ) ? xui::window_status::WINDOW_RESTORE : xui::window_status::WINDOW_MAXIMIZE );
                            } );
                        } );
                    } );

                    draw_style_element( "minimizebox", [&]()
                    {
                        box_rect.x -= 48;

                        draw_disable( ( t.flags & xui::window_flag::WINDOW_NO_MINIMIZEBOX ) != 0, [&]()
                        {
                            draw_rect( box_rect, [&]()
                            {
                                std::string_view action = get_action_name();
                                draw_style_action( action, [&]()
                                {
                                    draw_rect( box_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                                    draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                                        .moveto( { box_rect.center().x - 5, box_rect.center().y } )
                                        .lineto( { box_rect.center().x + 5, box_rect.center().y } )
                                        .closepath();
                                } );

                                if ( action == "active" )
                                    _p->_impl->set_window_status( id, xui::window_status::WINDOW_MINIMIZE );
                            } );
                        } );
                    } );

                    if ( ( t.flags & xui::window_flag::WINDOW_NO_MOVE ) == 0 )
                    {
                        draw_rect( move_rect, [&]()
                        {
                            if ( get_action_name( action_focus_type ) == "active" )
                            {
                                auto wr = _p->_impl->get_window_rect( id );
                                auto dt = _p->_impl->get_cursor_dt( id );
                                _p->_impl->set_window_rect( id, { wr.x + dt.x, wr.y + dt.y, wr.w, wr.h } );
                            }
                        } );
                    }
                } );
            }
            if ( ( t.flags & xui::window_flag::WINDOW_NO_RESIZE ) == 0 )
            {
                draw_style_element( "resize", [&]()
                {
                    draw_rect( resize_rect, [&]()
                    {
                        if ( get_action_name( action_focus_type ) == "active" )
                        {
                            auto wr = _p->_impl->get_window_rect( id );
                            auto dt = _p->_impl->get_cursor_dt( id );
                            _p->_impl->set_window_rect( id, { wr.x, wr.y, std::max( wr.w + dt.x, 150.0f ), std::max( wr.h + dt.y, title_rect.h + resize_rect.h ) } );
                        }
                    } );

                    draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                        .moveto( { resize_rect.x, resize_rect.y + resize_rect.h } )
                        .lineto( { resize_rect.x + resize_rect.w, resize_rect.y + resize_rect.h } )
                        .lineto( { resize_rect.x + resize_rect.w, resize_rect.y } )
                        .lineto( { resize_rect.x, resize_rect.y + resize_rect.h } )
                        .closepath();
                } );
            }
        } );
    }
}

bool xui::context::image( xui::texture_id id )
{
    return image( std::format( "_image_{}", _p->_str_id++ ), id );
}

bool xui::context::image( xui::string_id str_id, xui::texture_id id )
{
    draw_string_id( str_id, [&]()
    {
        draw_style_type( "image", [&]()
        {
            draw_image( id, currrent_rect() );
        } );
    } );
    return true;
}

bool xui::context::label( std::string_view text )
{
    return label( std::format( "_label_{}", _p->_str_id++ ), text );
}

bool xui::context::label( xui::string_id str_id, std::string_view text )
{
    draw_string_id( str_id, [&]()
    {
        draw_style_type( "label", [&]()
        {
            draw_text( text, current_font(), currrent_rect(), current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
        } );
    } );
    return true;
}

bool xui::context::radio( bool & checked )
{
    return radio( std::format( "_radio_{}", _p->_str_id++ ), checked );
}

bool xui::context::radio( xui::string_id str_id, bool & checked )
{
    draw_string_id( str_id, [&]()
    {
        auto id = current_window_id();
        auto rect = currrent_rect();
        float raduis = std::min( rect.w, rect.h ) / 2;

        draw_style_type( "radio", [&]()
        {
            std::string_view action = get_action_name();
            if ( action == "active" )
                checked = !checked;

            draw_style_action( action, [&]()
            {
                draw_circle( { rect.x + raduis, rect.y + raduis }, raduis, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
            } );

            if ( checked )
            {
                draw_style_element( "indicator", [&]()
                {
                    draw_style_action( action, [&]()
                    {
                        draw_circle( { rect.x + raduis, rect.y + raduis }, ( raduis * 0.7f ), current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
                    } );
                } );
            }
        } );
    } );
    return checked;
}

bool xui::context::check( bool & checked )
{
    return check( std::format( "_check_{}", _p->_str_id++ ), checked );
}

bool xui::context::check( xui::string_id str_id, bool & checked )
{
    draw_string_id( str_id, [&]()
    {
        auto id = current_window_id();
        auto rect = currrent_rect();
        rect.w = std::min( rect.w, rect.h );
        rect.h = rect.w;

        draw_style_type( "check", [&]()
        {
            std::string_view action = get_action_name();
            if ( action == "active" ) checked = !checked;

            draw_style_action( action, [&]()
            {
                draw_rect( rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
            } );

            if ( checked )
            {
                draw_style_element( "indicator", [&]()
                {
                    draw_style_action( action, [&]()
                    {
                        draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                            .moveto( { rect.x + ( rect.w * 0.2f ), rect.y + ( rect.h * 0.5f ) } )
                            .lineto( { rect.x + ( rect.w * 0.4f ), rect.y + ( rect.h * 0.7f ) } )
                            .lineto( { rect.x + ( rect.w * 0.8f ), rect.y + ( rect.h * 0.2f ) } );
                    } );
                } );
            }
        } );
    } );
    return checked;
}

bool xui::context::button( std::string_view text )
{
    return button( std::format( "_button_{}", _p->_str_id++ ), text );
}

bool xui::context::button( xui::string_id str_id, std::string_view text )
{
    std::string_view action;

    draw_string_id( str_id, [&]()
    {
        auto id = current_window_id();
        auto rect = currrent_rect();

        draw_style_type( "button", [&]()
        {
            action = get_action_name();

            draw_style_action( action, [&]()
            {
                draw_rect( rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
            } );

            if ( !text.empty() )
            {
                draw_style_element( "text", [&]()
                {
                    draw_style_action( action, [&]()
                    {
                        draw_text( text, current_font(), rect, current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
                    } );
                } );
            }
        } );
    } );

    return action == "active";
}

float xui::context::slider( float & value, float min, float max )
{
    return slider( std::format( "_slider_{}", _p->_str_id++ ), value, min, max );
}

float xui::context::slider( xui::string_id str_id, float & value, float min, float max )
{
    draw_string_id( str_id, [&]()
    {
        auto id = current_window_id();

        draw_style_type( "slider", [&]()
        {
            auto back_rect = currrent_rect();
            xui::vec2 pos = _p->_impl->get_cursor_pos( id );
            std::string_view action = get_action_name( action_focus_type );

            draw_style_action( action, [&]()
            {
                draw_rect( back_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
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
                    draw_rect( cursor_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
                } );
            } );
        } );
    });
    return value;
}

bool xui::context::process( float value, float min, float max, std::string_view text )
{
    return process( std::format( "_process_{}", _p->_str_id++ ), value, min, max, text );
}

bool xui::context::process( xui::string_id str_id, float value, float min, float max, std::string_view text )
{
    draw_string_id( str_id, [&]()
    {
        draw_style_type( "process", [&]()
        {
            auto back_rect = currrent_rect();

            draw_rect( back_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

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

                draw_rect( cursor_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
            } );

            if ( !text.empty() )
            {
                draw_style_element( "text", [&]()
                {
                    draw_text( text, current_font(), back_rect, current_style( "font-color", xui::color() ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
                } );
            }
        } );
    } );
    return true;
}

float xui::context::scrollbar( float & value, float step, float min, float max, xui::direction dir )
{
    return scrollbar( std::format( "_scrollbar_{}", _p->_str_id++ ), value, step, min, max, dir );
}

float xui::context::scrollbar( xui::string_id str_id, float & value, float step, float min, float max, xui::direction dir )
{
    draw_string_id( str_id, [&]()
    {
        auto id = current_window_id();
        std::string_view action;

        draw_style_type( "scrollbar", [&]()
        {
            auto back_rect = currrent_rect();
            xui::vec2 pos = _p->_impl->get_cursor_pos( id );
            float arrow_radius = std::min( back_rect.w, back_rect.h );

            action = get_action_name();

            draw_style_action( action, [&]()
            {
                draw_rect( back_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
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
                    push_rect( detect_rect );
                    action = get_action_name( action_focus_type );
                    pop_rect();
                    break;
                case xui::direction::TOP_BOTTOM:
                case xui::direction::BOTTOM_TOP:
                    detect_rect = { back_rect.x,  back_rect.y + arrow_radius, back_rect.w, back_rect.h - arrow_radius * 2 };
                    push_rect( detect_rect );
                    action = get_action_name( action_focus_type );
                    pop_rect();
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
                        draw_rect( cursor_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
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
                    draw_rect( arrow_rect, [&]()
                    {
                        action = get_action_name();

                        if ( action == "active" )
                            value = std::clamp( value + ( dir == xui::direction::LEFT_RIGHT ? -step : step ), min, max );

                        draw_style_action( action, [&]()
                        {
                            draw_rect( arrow_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                            draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                                .moveto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.5f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.3f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.7f } )
                                .closepath();
                        } );
                    } );

                    // right
                    arrow_rect = { back_rect.x + back_rect.w - arrow_radius, back_rect.y, arrow_radius, arrow_radius };
                    draw_rect( arrow_rect, [&]()
                    {
                        action = get_action_name();

                        if ( action == "active" )
                            value = std::clamp( value + ( dir == xui::direction::RIGHT_LEFT ? -step : step ), min, max );

                        draw_style_action( action, [&]()
                        {
                            draw_rect( arrow_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                            draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                                .moveto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.5f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.3f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.7f } )
                                .closepath();
                        } );
                    } );
                    break;
                case xui::direction::TOP_BOTTOM:
                case xui::direction::BOTTOM_TOP:
                    // up
                    arrow_rect = { back_rect.x, back_rect.y, arrow_radius, arrow_radius };
                    draw_rect( arrow_rect, [&]()
                    {
                        action = get_action_name();

                        if ( action == "active" )
                            value = std::clamp( value + ( dir == xui::direction::TOP_BOTTOM ? -step : step ), min, max );

                        draw_style_action( action, [&]()
                        {
                            draw_rect( arrow_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                            draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                                .moveto( { arrow_rect.x + arrow_rect.w * 0.5f, arrow_rect.y + arrow_rect.h * 0.3f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.7f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.7f } )
                                .closepath();
                        } );
                    } );

                    // down
                    arrow_rect = { back_rect.x, back_rect.y + back_rect.h - arrow_radius, arrow_radius, arrow_radius };
                    draw_rect( arrow_rect, [&]()
                    {
                        action = get_action_name();

                        if ( action == "active" )
                            value = std::clamp( value + ( dir == xui::direction::BOTTOM_TOP ? -step : step ), min, max );

                        draw_style_action( action, [&]()
                        {
                            draw_rect( arrow_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

                            draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                                .moveto( { arrow_rect.x + arrow_rect.w * 0.5f, arrow_rect.y + arrow_rect.h * 0.7f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.3f, arrow_rect.y + arrow_rect.h * 0.3f } )
                                .lineto( { arrow_rect.x + arrow_rect.w * 0.7f, arrow_rect.y + arrow_rect.h * 0.3f } )
                                .closepath();
                        } );
                    } );
                    break;
                }
            } );
        } );
    });
    return value;
}

bool xui::context::begin_menubar()
{
    return begin_menubar( std::format( "_menubar_{}", _p->_str_id++ ) );
}

bool xui::context::begin_menubar( xui::string_id str_id )
{
    push_string_id( str_id );

    auto rect = currrent_rect();

    xui::rect back_rect = { rect.x, rect.y, rect.w, 30 };

    push_style_type( "menubar" );
    push_rect( back_rect );

    draw_rect( back_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

    return true;
}

void xui::context::end_menubar()
{
    pop_rect();
    pop_style_type();
    pop_string_id();

    auto rect = currrent_rect();
    push_rect( { rect.x, rect.y + 30, rect.w, rect.h - 30 } );
}

bool xui::context::begin_menu( std::string_view name )
{
    return begin_menu( std::format( "_menu_{}", _p->_str_id++ ), name );
}

bool xui::context::begin_menu( xui::string_id str_id, std::string_view name )
{
    push_string_id( str_id );

    std::string_view action;

    draw_style_element( "menu", [&]()
    {
        auto rect = currrent_rect();
        auto minw = current_style( "minimum-width", 10.0f );
        auto maxw = std::min( rect.w, current_style( "maximum-width", 50.0f ) );
        auto size = _p->_impl->font_size( current_font(), name );
        xui::rect menu_rect = { rect.x, rect.y, std::clamp( size.w, minw, maxw ), rect.h };

        draw_rect( menu_rect, [&]()
        {
            action = get_action_name( action_menu_type );

            draw_style_action( action, [&]()
            {
                draw_rect( menu_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
                draw_text( name, current_font(), menu_rect, current_style( "font-color", xui::color( 255, 255, 255 ) ), current_style( "text-align", xui::alignment_flag::ALIGN_VCENTER ) );
            } );
        } );

        margins_currrent_rect( menu_rect.w, 0, 0, 0 );

        if ( action == "active" )
            push_rect( { menu_rect.x, menu_rect.y + menu_rect.h, 0, 0 } );
        else
            push_rect( rect );
    } );

    return ( action == "active" );
}

void xui::context::end_menu()
{
    pop_rect();
    pop_string_id();
}

bool xui::context::menu_item( std::string_view name, bool menu )
{
    return menu_item( std::format( "_menu_item_{}", _p->_str_id++ ), name, menu );
}

bool xui::context::menu_item( xui::string_id str_id, std::string_view name, bool menu )
{
    std::string_view action;

    draw_string_id( str_id, [&]()
    {
        draw_style_element( "item", [&]()
        {
            auto rect = currrent_rect();
            auto size = _p->_impl->font_size( current_font(), name );
            xui::rect item_rect = { rect.x, rect.y, std::max( size.w, rect.w ), 30 };

            draw_rect( item_rect, [&]()
            {
                action = get_action_name( menu ? action_menu_type : action_nil_type );

                draw_style_action( action, [&]()
                {
                    draw_text( name, current_font(), item_rect, current_style( "font-color", xui::color( 255, 255, 255 ) ), current_style( "text-align", (xui::alignment_flag)( xui::alignment_flag::ALIGN_LEFT | xui::alignment_flag::ALIGN_VCENTER ) ) );

                    if ( menu )
                    {
                        draw_path( current_style( "stroke", xui::stroke() ), current_style( "filled", xui::filled() ) )
                            .moveto( { item_rect.x + item_rect.w - ( 30.0f * 0.7f ), item_rect.y + item_rect.h * 0.5f } )
                            .lineto( { item_rect.x + item_rect.w - ( 30.0f * 0.3f ), item_rect.y + item_rect.h * 0.3f } )
                            .lineto( { item_rect.x + item_rect.w - ( 30.0f * 0.3f ), item_rect.y + item_rect.h * 0.7f } )
                            .closepath();
                    }
                } );
            } );

            margins_currrent_rect( 0, item_rect.h, item_rect.w - rect.w, item_rect.h );
        } );
    } );

    return ( action == "active" );
}

xui::drawcmd::text_element & xui::context::draw_text( std::string_view text, xui::font_id id, const xui::rect & rect, const xui::color & font_color, xui::alignment_flag text_align )
{
    xui::drawcmd::text_element element;

    element.font = id;
    element.text = text;
    element.rect = rect;
    element.color = font_color;
    element.align = text_align;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::text_element>( _p->_commands.back().element );
}

xui::drawcmd::line_element & xui::context::draw_line( const xui::vec2 & p1, const xui::vec2 & p2, const xui::stroke & stroke )
{
    xui::drawcmd::line_element element;

    element.p1 = p1;
    element.p2 = p2;
    element.stroke = stroke;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::line_element>( _p->_commands.back().element );
}

xui::drawcmd::rect_element & xui::context::draw_rect( const xui::rect & rect, const xui::border & border, const xui::filled filled )
{
    xui::drawcmd::rect_element element;

    element.rect = rect;
    element.border = border;
    element.filled = filled;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::rect_element>( _p->_commands.back().element );
}

xui::drawcmd::path_element & xui::context::draw_path( const xui::stroke & stroke, const xui::filled filled )
{
    xui::drawcmd::path_element element;

    element.stroke = stroke;
    element.filled = filled;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::path_element>( _p->_commands.back().element );
}

xui::drawcmd::image_element & xui::context::draw_image( xui::texture_id id, const xui::rect & rect )
{
    xui::drawcmd::image_element element;

    element.id = id;
    element.rect = rect;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::image_element>( _p->_commands.back().element );
}

xui::drawcmd::circle_element & xui::context::draw_circle( const xui::vec2 & center, float radius, const xui::border & border, const xui::filled filled )
{
    xui::drawcmd::circle_element element;

    element.center = center;
    element.radius = radius;
    element.border = border;
    element.filled = filled;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::circle_element>( _p->_commands.back().element );
}

xui::drawcmd::ellipse_element & xui::context::draw_ellipse( const xui::vec2 & center, const xui::vec2 & radius, const xui::border & border, const xui::filled filled )
{
    xui::drawcmd::ellipse_element element;

    element.center = center;
    element.radius = radius;
    element.border = border;
    element.filled = filled;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::ellipse_element>( _p->_commands.back().element );
}

xui::drawcmd::polygon_element & xui::context::draw_polygon( std::span<xui::vec2> points, const xui::border & border, const xui::filled filled )
{
    xui::drawcmd::polygon_element element;

    element.points.assign( points.begin(), points.end() );
    element.border = border;
    element.filled = filled;

    _p->_commands.push_back( { current_window_id(), element } );

    return std::get<xui::drawcmd::polygon_element>( _p->_commands.back().element );
}


std::string xui::context::style_name() const
{
    std::string result;

    if ( !_p->_strids.empty() )
    {
        result.append( _p->_strids.back() );
        result.append( "#" );
    }

    result.append( _p->_types.back().type.begin(), _p->_types.back().type.end() );

    for ( const auto & it : _p->_types.back().elements )
    {
        result.append( "-" );
        result.append( it );
    }

    if ( !_p->_types.back().actions.empty() )
    {
        result.append( ":" );
        result.append( _p->_types.back().actions.back() );
    }

    return result;
}

std::string xui::context::focus_name() const
{
    std::string result;

    if ( !_p->_strids.empty() )
    {
        result.append( _p->_strids.back() );
        result.append( "#" );
    }

    result.append( _p->_types.back().type.begin(), _p->_types.back().type.end() );

    for ( const auto & it : _p->_types.back().elements )
    {
        result.append( "-" );
        result.append( it );
    }

    return result;
}

void xui::context::push_focus( xui::event event, int type )
{
    focus_type t;
    t.id = current_window_id();
    t.type = type;
    t.name = focus_name();
    t.rect = currrent_rect();
    t.event = event;

    if ( !_p->_focus.empty() && _p->_focus.back().type != type )
    {
        _p->_focus.clear();
    }
    else if ( !_p->_focus.empty() && _p->_focus.back().type == type && type == action_menu_type )
    {
        while ( !_p->_focus.empty() && _p->_focus.back().name != t.name )
            _p->_focus.pop_back();
    }

    _p->_focus.push_back( t );
}

void xui::context::pop_focus()
{
    if ( !_p->_focus.empty() )
        _p->_focus.pop_back();
}

bool xui::context::exist_menu() const
{
    if ( _p->_focus.empty() )
        return false;

    auto name = focus_name();
    return std::find_if( _p->_focus.begin(), _p->_focus.end(), [&]( const auto & val ) { return val.name.find( name ) != std::string::npos; } ) != _p->_focus.end();
}

bool xui::context::inherit_focus() const
{
    if ( _p->_focus.empty() )
        return true;

    if ( _p->_focus.back().type != action_focus_type )
        return true;

    return focus_name().find( _p->_focus.back().name ) != std::string::npos;
}

bool xui::context::current_focus() const
{
    if ( _p->_focus.empty() )
        return false;

    return _p->_focus.back().name == focus_name();
}

std::string_view xui::context::get_action_name( int type )
{
    /*
    * hover
    * active
    * disable
    * selected
    */
    auto id = current_window_id();
    auto rect = currrent_rect();
    auto pos = _p->_impl->get_cursor_pos( id );

    if ( current_disable() )
    {
        return "disable";
    }
    else if ( ( type == action_menu_type ) && exist_menu() )
    {
        return "active";
    }
    else if ( ( type == action_focus_type ) && current_focus() )
    {
        return "active";
    }
    else if( inherit_focus() )
    {
        if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && rect.contains( pos ) )
        {
            if( type != action_nil_type )
                push_focus( xui::event::KEY_MOUSE_LEFT, type );
            
            return "active";
        }
        else if ( _p->_impl->get_event( id, xui::event::MOUSE_ACTIVE ) && rect.contains( pos ) )
        {
            return "hover";
        }
    }

    return "";
}
