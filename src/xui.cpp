#include "xui.h"

#include <array>
#include <deque>
#include <regex>
#include <memory>
#include <algorithm>
#include <iostream>

#define XUI_ERR( CODE )  if ( _p->_error ) _p->_error( this, xui_category::make_error_code( xui::err::CODE ) );
#define XUI_SCALE( VAL ) ( VAL * _p->_factor )

std::string_view data;

namespace xui
{
    using string = std::string;
    
    BEG_STRUCT( bool )
        CONSTRUCT( *object = data == "true"; );
    END_STRUCT( bool )

    BEG_STRUCT( int )
        CONSTRUCT( std::from_chars( data.data(), data.data() + data.size(), *object ); );
    END_STRUCT( int )

    BEG_STRUCT( float )
        CONSTRUCT( std::from_chars( data.data(), data.data() + data.size(), *object ); );
    END_STRUCT( float )

    BEG_STRUCT( string )
        CONSTRUCT( object->assign( data.begin(), data.end() ); );
    END_STRUCT( string )

    BEG_STRUCT( url )
        CONSTRUCT( *object = data; );
    END_STRUCT( url )

    BEG_STRUCT( size )
        CONSTRUCT(
        {
            auto beg = data.begin();
            auto end = data.end();
            if ( data.find( ' ' ) != std::string_view::npos )
                end = data.begin() + data.find( ' ' );
            std::from_chars( beg.operator->(), end.operator->(), object->w );

            if ( end == data.end() )
                return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->h );
        } );
        PROP( w, std::from_chars( data.data(), data.data() + data.size(), object->w ); )
        PROP( h, std::from_chars( data.data(), data.data() + data.size(), object->h ); )
    END_STRUCT( size )

    BEG_STRUCT( rect )
        CONSTRUCT(
        {
            auto beg = data.begin();
            auto end = data.end();
            if ( data.find( ' ' ) != std::string_view::npos )
                end = data.begin() + data.find( ' ' );

            std::from_chars( beg.operator->(), end.operator->(), object->x );

            if ( end == data.end() )
                return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->y );

            if ( end == data.end() )
                return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->w );

            if ( end == data.end() )
                return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->h );
        } );
        PROP( x, std::from_chars( data.data(), data.data() + data.size(), object->x ); )
        PROP( y, std::from_chars( data.data(), data.data() + data.size(), object->y ); )
        PROP( w, std::from_chars( data.data(), data.data() + data.size(), object->w ); )
        PROP( h, std::from_chars( data.data(), data.data() + data.size(), object->h ); )
    END_STRUCT( rect )
            
    BEG_STRUCT( vec2 )
        CONSTRUCT(
        {
            auto beg = data.begin();
            auto end = data.end();
            if ( data.find( ' ' ) != std::string_view::npos )
                end = data.begin() + data.find( ' ' );
            std::from_chars( beg.operator->(), end.operator->(), object->x );

            if ( end == data.end() ) return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->y );
        } );
        PROP( x, std::from_chars( data.data(), data.data() + data.size(), object->x ); )
        PROP( y, std::from_chars( data.data(), data.data() + data.size(), object->y ); )
    END_STRUCT( vec2 )
            
    BEG_STRUCT( vec4 )
        CONSTRUCT(
        {
            auto beg = data.begin();
            auto end = data.end();
            if ( data.find( ' ' ) != std::string_view::npos )
                end = data.begin() + data.find( ' ' );

            std::from_chars( beg.operator->(), end.operator->(), object->x );

            if ( end == data.end() ) return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->y );

            if ( end == data.end() ) return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->z );

            if ( end == data.end() ) return;
            beg = end + 1;
            end = data.end();
            std::from_chars( beg.operator->(), end.operator->(), object->w );
        } );
        PROP( x, std::from_chars( data.data(), data.data() + data.size(), object->x ); )
        PROP( y, std::from_chars( data.data(), data.data() + data.size(), object->y ); )
        PROP( z, std::from_chars( data.data(), data.data() + data.size(), object->z ); )
        PROP( w, std::from_chars( data.data(), data.data() + data.size(), object->w ); )
    END_STRUCT( vec4 )
            
    BEG_STRUCT( color )
        CONSTRUCT(
        {
            if ( data.front() == '#' )
            {
                std::from_chars( data.data() + 1, data.data() + data.size(), object->hex, 16 );
            }
            else if ( data.find( "rgba" ) != std::string_view::npos )
            {
                auto beg = data.begin() + data.find( '(' ) + 1;
                auto end = data.begin() + data.find( ',' );
                std::from_chars( beg.operator->(), end.operator->(), object->r );

                beg = end + 1;
                end = data.begin() + data.find( ',', std::distance( data.begin(), beg ) );
                std::from_chars( beg.operator->(), end.operator->(), object->g );

                beg = end + 1;
                end = data.begin() + data.find( ',', std::distance( data.begin(), beg ) );
                std::from_chars( beg.operator->(), end.operator->(), object->b );

                beg = end + 1;
                end = data.begin() + data.find( ')', std::distance( data.begin(), beg ) );
                std::from_chars( beg.operator->(), end.operator->(), object->a );
            }
            else if ( data.find( "rgb" ) != std::string_view::npos )
            {
                auto beg = data.begin() + data.find( '(' ) + 1;
                auto end = data.begin() + data.find( ',' );
                std::from_chars( beg.operator->(), end.operator->(), object->r );

                beg = end + 1;
                end = data.begin() + data.find( ',', std::distance( data.begin(), beg ) );
                std::from_chars( beg.operator->(), end.operator->(), object->g );

                beg = end + 1;
                end = data.begin() + data.find( ')', std::distance( data.begin(), beg ) );
                std::from_chars( beg.operator->(), end.operator->(), object->b );
            }
        } );
        PROP( r, std::from_chars( data.data(), data.data() + data.size(), object->r ); )
        PROP( g, std::from_chars( data.data(), data.data() + data.size(), object->g ); )
        PROP( b, std::from_chars( data.data(), data.data() + data.size(), object->b ); )
        PROP( a, std::from_chars( data.data(), data.data() + data.size(), object->a ); )
    END_STRUCT( color )
            
    BEG_STRUCT( hatch )
        CONSTRUCT( {} );
        PROP_TYPE( fore_color, color )
        PROP_TYPE( back_color, color )
    END_STRUCT( hatch )

    BEG_STRUCT( linear )
        CONSTRUCT( {} );
        PROP_TYPE( p1, vec2 )
        PROP_TYPE( c1, color )
        PROP_TYPE( p1, vec2 )
        PROP_TYPE( c2, color )
    END_STRUCT( linear )
            
    BEG_STRUCT( stroke )
        CONSTRUCT( {} );
        PROP( style,
        {
            switch ( xui::hash( data.data() ) )
            {
            case xui::hash( "solid" ): object->style = xui::stroke::SOLID; break;
            case xui::hash( "dashed" ): object->style = xui::stroke::DASHED; break;
            case xui::hash( "dotted" ): object->style = xui::stroke::DOTTED; break;
            case xui::hash( "dash_dot" ): object->style = xui::stroke::DASH_DOT; break;
            case xui::hash( "dash_dot_dot" ): object->style = xui::stroke::DASH_DOT_DOT; break;
            }
        } )
        PROP_TYPE( width, float )
        PROP_TYPE( color, color )
    END_STRUCT( stroke )

    BEG_STRUCT( border, stroke )
        CONSTRUCT( {} );
        PROP_TYPE( radius, vec4 )
    END_STRUCT( border )

    BEG_ENUM( direction )
        VALUE( "ltr", direction::LEFT_RIGHT )
        VALUE( "rtl", direction::RIGHT_LEFT )
        VALUE( "left to right", direction::LEFT_RIGHT )
        VALUE( "right to left", direction::RIGHT_LEFT )
        VALUE( "top to bottom", direction::TOP_BOTTOM )
        VALUE( "bottom to top", direction::BOTTOM_TOP )
    END_ENUM( direction )

    BEG_ENUM( orientation )
        VALUE( "top", orientation::ORIENT_TOP )
        VALUE( "left", orientation::ORIENT_LEFT )
        VALUE( "right", orientation::ORIENT_RIGHT )
        VALUE( "bottom", orientation::ORIENT_BOTTOM )
    END_ENUM( orientation )

    BEG_FLAGS( alignment_flag )
        FLAG( "left", alignment_flag::ALIGN_LEFT )
        FLAG( "right", alignment_flag::ALIGN_RIGHT )
        FLAG( "top", alignment_flag::ALIGN_TOP )
        FLAG( "bottom", alignment_flag::ALIGN_BOTTOM )
        FLAG( "vcenter", alignment_flag::ALIGN_VCENTER )
        FLAG( "hcenter", alignment_flag::ALIGN_HCENTER )
        FLAG( "center", alignment_flag::ALIGN_CENTER )
    END_FLAGS( alignment_flag )

    BEG_CLASS( window );
        ATTR( filled, color )
        BEG_ELEMENT( title )
            ATTR( filled, color )
        END_ELEMENT( title )
    END_CLASS( window );
}

namespace
{
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

void xui::style::meta_struct::_calc_construct( const meta_struct & _m, void * _o, std::string_view _d )
{
    std::array<std::string_view, 10> list;

    uint32_t * object = (uint32_t *)( _o );

    size_t i = 0;
    auto beg = _d.begin();
    auto it = _d.begin();
    for ( size_t i = 0; i < list.size() && it != _d.end(); i++ )
    {
        while ( it != _d.end() && ( *it ) != '|' ) ++it;
        auto it2 = it;
        while ( *beg == ' ' ) ++beg;
        while ( *it == ' ' ) --it;

        list[i] = { beg, it };

        if ( it2 != _d.end() ) it = it2 + 1;
    }

    for ( size_t i = 0; i < list.size() && !list[i].empty(); i++ )
    {
        auto it = std::find_if( _m.propertys.begin(), _m.propertys.end(), [&]( const auto & prop ) { return ( prop.name == list[i] ); } );
        if ( it != _m.propertys.end() )
        {
            uint32_t p = 0; it->setter( &p, list[i] );
            *object |= p;
        }
    }
}

bool xui::style::parse( std::string_view str )
{
    return false;
}

xui::style::variant xui::style::find( std::string_view name ) const
{
    return {};
}

void xui::style::register_class( meta_class * cls )
{
    meta_class_map().insert( { cls->name, cls } );
}

void xui::style::register_struct( meta_struct * str )
{
    meta_struct_map().insert( { str->name, str } );
}

const xui::style::meta_class * xui::style::find_class( std::string_view name )
{
    auto it = meta_class_map().find( name );
    return it != meta_class_map().end() ? it->second : nullptr;
}

const xui::style::meta_struct * xui::style::find_struct( std::string_view name )
{
    auto it = meta_struct_map().find( name );
    return it != meta_struct_map().end() ? it->second : nullptr;
}

std::map<std::string_view, xui::style::meta_class *> & xui::style::meta_class_map()
{
    static std::map<std::string_view, xui::style::meta_class *> map;
    return map;
}

std::map<std::string_view, xui::style::meta_struct *> & xui::style::meta_struct_map()
{
    static std::map<std::string_view, xui::style::meta_struct *> map;
    return map;
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
        , _types( res )
        , _focus( res )
        , _menus( res )
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
    std::pmr::deque<style_type> _types;
    std::pmr::deque<focus_type> _focus;
    std::pmr::deque<std::string> _menus;
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
    },
    menubar-menu{
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
        text-align: center;
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
        if ( _p->_focus.back().event >= xui::event::KEY_EVENT_BEG && _p->_focus.back().event <= xui::event::KEY_EVENT_END )
        {

        }
        else if ( _p->_focus.back().event >= xui::event::GAMEPAD_EVENT_BEG && _p->_focus.back().event <= xui::event::GAMEPAD_EVENT_END )
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
    auto str_id = std::format( "_window_{}", _p->_str_id++ );
    return begin_window( str_id, title, icon_id, flags );
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
                            if ( get_action_name( true ) == "active" )
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
                        if ( get_action_name( true ) == "active" )
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
    auto str_id = std::format( "_image_{}", _p->_str_id++ );
    return image( str_id, id );
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
    auto str_id = std::format( "_label_{}", _p->_str_id++ );
    return label( str_id, text );
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
    auto str_id = std::format( "_radio_{}", _p->_str_id++ );
    return radio( str_id, checked );
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
    auto str_id = std::format( "_check_{}", _p->_str_id++ );
    return check( str_id, checked );
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
    auto str_id = std::format( "_button_{}", _p->_str_id++ );
    return button( str_id, text );
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
    auto str_id = std::format( "_slider_{}", _p->_str_id++ );
    return slider( str_id, value, min, max );
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
            std::string_view action = get_action_name( true );

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
    auto str_id = std::format( "_process_{}", _p->_str_id++ );
    return process( str_id, value, min, max, text );
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
    auto str_id = std::format( "_scrollbar_{}", _p->_str_id++ );
    return scrollbar( str_id, value, step, min, max, dir );
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
                    action = get_action_name( true );
                    pop_rect();
                    break;
                case xui::direction::TOP_BOTTOM:
                case xui::direction::BOTTOM_TOP:
                    detect_rect = { back_rect.x,  back_rect.y + arrow_radius, back_rect.w, back_rect.h - arrow_radius * 2 };
                    push_rect( detect_rect );
                    action = get_action_name( true );
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

bool xui::context::begin_menubar( xui::string_id str_id )
{
    if ( str_id.empty() )
        push_string_id( std::format( "_menubar_{}", _p->_str_id++ ) );
    else
        push_string_id( str_id );

    auto rect = currrent_rect();

    xui::rect back_rect = { rect.x, rect.y + 30, rect.w, 30 };

    push_style_type( "menubar" );
    push_rect( back_rect );

    draw_rect( back_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );

    return false;
}

bool xui::context::begin_menu( std::string_view name )
{
    auto rect = currrent_rect();

    draw_style_element( "menu", [&]()
    {
        std::string_view action = get_action_name();
        auto size = _p->_impl->font_size( current_font(), name );
        xui::rect menu_rect = { rect.x, rect.y, size.w, rect.h };

        draw_style_action( action, [&]()
        {
            draw_rect( menu_rect, current_style( "border", xui::border() ), current_style( "filled", xui::filled() ) );
            draw_text( name, current_font(), menu_rect, current_style( "font-color", xui::color( 255, 255, 255 ) ), current_style( "text-align", xui::alignment_flag::ALIGN_CENTER ) );
        } );
    });

    return false;
}

void xui::context::end_menubar()
{
    pop_rect();
    pop_style_type();

    auto rect = currrent_rect();
    push_rect( { rect.x, rect.y + 30, rect.w, rect.h - 30 } );
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

    if ( !_p->_strids.empty() )
    {
        result.append( "#" );
        result.append( _p->_strids.back() );
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

void xui::context::push_focus( xui::event event )
{
    focus_type type;
    type.id = current_window_id();
    type.name = focus_name();
    type.event = event;
    _p->_focus.push_back( type );
}

void xui::context::pop_focus()
{
    if ( !_p->_focus.empty() )
        _p->_focus.pop_back();
}

bool xui::context::current_focus() const
{
    if ( _p->_focus.empty() )
        return false;

    return _p->_focus.back().name == focus_name();
}

bool xui::context::inherit_focus() const
{
    if ( _p->_focus.empty() )
        return true;

    return focus_name().find( _p->_focus.back().name ) != std::string::npos;
}

std::string_view xui::context::get_action_name( bool focus )
{
    auto id = current_window_id();
    auto rect = currrent_rect();
    auto pos = _p->_impl->get_cursor_pos( id );

    if ( current_disable() )
    {
        return "disable";
    }
    else if ( focus && current_focus() )
    {
        return "active";
    }
    else if( inherit_focus() )
    {
        if ( _p->_impl->get_event( id, xui::event::KEY_MOUSE_LEFT ) && rect.contains( pos ) )
        {
            push_focus( xui::event::KEY_MOUSE_LEFT );

            return "active";
        }
        else if ( _p->_impl->get_event( id, xui::event::MOUSE_ACTIVE ) && rect.contains( pos ) )
        {
            return "hover";
        }
    }

    return "";
}
