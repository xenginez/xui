﻿#pragma once

#include <map>
#include <span>
#include <format>
#include <string>
#include <variant>
#include <optional>
#include <functional>
#include <system_error>
#include <memory_resource>

namespace xui
{
	class url;
	class size;
	class rect;
	class vec4;
	class vec2;
	class color;
	class stroke;
	class border;
	class filled;
	class hatch_color;
	class texture_brush;
	class linear_gradient;

	class style;
	class drawcmd;
	class context;
	class implement;

	template<typename ... Ts>
	struct overload : Ts ... { using Ts::operator() ...; };
	template<class... Ts> overload( Ts... ) -> overload<Ts...>;

	enum err
	{
		ERR_NO = 0,
	};
	enum event
	{
		// KeyBoard Events
		KEY_EVENT_BEG,
		KEY_TAB = KEY_EVENT_BEG,
		KEY_LEFT_ARROW,
		KEY_RIGHT_ARROW,
		KEY_UP_ARROW,
		KEY_DOWN_ARROW,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_HOME,
		KEY_END,
		KEY_INSERT,
		KEY_DELETE,
		KEY_BACKSPACE,
		KEY_SPACE,
		KEY_ENTER,
		KEY_ESCAPE,
		KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_ALT, KEY_LEFT_SUPER,
		KEY_RIGHT_CTRL, KEY_RIGHT_SHIFT, KEY_RIGHT_ALT, KEY_RIGHT_SUPER,
		KEY_MENU,
		KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
		KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
		KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
		KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
		KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
		KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
		KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18,
		KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24,
		KEY_APOSTROPHE,				// '
		KEY_COMMA,					// ,
		KEY_MINUS,					// -
		KEY_PERIOD,					// .
		KEY_SLASH,					// /
		KEY_SEMICOLON,				// ;
		KEY_EQUAL,					// =
		KEY_LEFT_BRACKET,			// [
		KEY_RIGHT_BRACKET,			// ]
		KEY_BACKSLASH,				// \ (this text inhibit multiline comment caused by backslash)
		KEY_GRAVE_ACCENT,			// `
		KEY_CAPS_LOCK,
		KEY_SCROLL_LOCK,
		KEY_NUM_LOCK,
		KEY_PRINT_SCREEN,
		KEY_PAUSE,
		KEY_KEYPAD_0, KEY_KEYPAD_1, KEY_KEYPAD_2, KEY_KEYPAD_3, KEY_KEYPAD_4,
		KEY_KEYPAD_5, KEY_KEYPAD_6, KEY_KEYPAD_7, KEY_KEYPAD_8, KEY_KEYPAD_9,
		KEY_KEYPAD_DECIMAL,
		KEY_KEYPAD_DIVIDE,
		KEY_KEYPAD_MULTIPLY,
		KEY_KEYPAD_SUBTRACT,
		KEY_KEYPAD_ADD,
		KEY_KEYPAD_ENTER,
		KEY_KEYPAD_EQUAL,
		KEY_APP_BACK,				// Available on some keyboard/mouses. Often referred as "Browser Back"
		KEY_APP_FORWARD,
		KEY_EVENT_END = KEY_APP_FORWARD,

		// Gamepad Events
		GAMEPAD_EVENT_BEG,
		GAMEPAD_START = GAMEPAD_EVENT_BEG,				// Menu (Xbox)      + (Switch)   Start/Options (PS)
		GAMEPAD_BACK,				// View (Xbox)      - (Switch)   Share (PS)
		GAMEPAD_FACE_LEFT,			// X (Xbox)         Y (Switch)   Square (PS)        // Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
		GAMEPAD_FACE_RIGHT,			// B (Xbox)         A (Switch)   Circle (PS)        // Cancel / Close / Exit
		GAMEPAD_FACE_UP,			// Y (Xbox)         X (Switch)   Triangle (PS)      // Text Input / On-screen Keyboard
		GAMEPAD_FACE_DOWN,			// A (Xbox)         B (Switch)   Cross (PS)         // Activate / Open / Toggle / Tweak
		GAMEPAD_DPAD_LEFT,			// D-pad Left                                       // Move / Tweak / Resize Window (in Windowing mode)
		GAMEPAD_DPAD_RIGHT,			// D-pad Right                                      // Move / Tweak / Resize Window (in Windowing mode)
		GAMEPAD_DPAD_UP,			// D-pad Up                                         // Move / Tweak / Resize Window (in Windowing mode)
		GAMEPAD_DPAD_DOWN,			// D-pad Down                                       // Move / Tweak / Resize Window (in Windowing mode)
		GAMEPAD_L1,					// L Bumper (Xbox)  L (Switch)   L1 (PS)            // Tweak Slower / Focus Previous (in Windowing mode)
		GAMEPAD_R1,					// R Bumper (Xbox)  R (Switch)   R1 (PS)            // Tweak Faster / Focus Next (in Windowing mode)
		GAMEPAD_L2,					// L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
		GAMEPAD_R2,					// R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
		GAMEPAD_L3,					// L Stick (Xbox)   L3 (Switch)  L3 (PS)
		GAMEPAD_R3,					// R Stick (Xbox)   R3 (Switch)  R3 (PS)
		GAMEPAD_L_STICK_LEFT,		// [Analog]                                         // Move Window (in Windowing mode)
		GAMEPAD_L_STICK_RIGHT,		// [Analog]                                         // Move Window (in Windowing mode)
		GAMEPAD_L_STICK_UP,			// [Analog]                                         // Move Window (in Windowing mode)
		GAMEPAD_L_STICK_DOWN,		// [Analog]                                         // Move Window (in Windowing mode)
		GAMEPAD_R_STICK_LEFT,		// [Analog]
		GAMEPAD_R_STICK_RIGHT,		// [Analog]
		GAMEPAD_R_STICK_UP,			// [Analog]
		GAMEPAD_R_STICK_DOWN,		// [Analog]
		GAMEPAD_EVENT_END = GAMEPAD_R_STICK_DOWN,

		// Mouse Events
		MOUSE_EVENT_BEG,
		MOUSE_ACTIVE = MOUSE_EVENT_BEG,
		KEY_MOUSE_LEFT, KEY_MOUSE_RIGHT, KEY_MOUSE_MIDDLE, KEY_MOUSE_X1, KEY_MOUSE_X2,
		MOUSE_EVENT_END = KEY_MOUSE_X2,

		EVENT_MAX_COUNT,
	};
	enum action
	{
		PRESS = 1,
		RELEASE = -1,
	};
	enum direction
	{
		LEFT_RIGHT,
		RIGHT_LEFT,
		TOP_BOTTOM,
		BOTTOM_TOP,
	};
	enum orientation
	{
		ORIENT_TOP,
		ORIENT_LEFT,
		ORIENT_RIGHT,
		ORIENT_BOTTOM,
	};

	enum font_flag
	{
		FONT_NONE = 0,
		FONT_BOLD = 1 << 0,
		FONT_ITALIC = 1 << 1,
		FONT_UNDERLINE = 1 << 2,
		FONT_STRIKEOUT = 1 << 3,
	};
	enum window_flag
	{
		WINDOW_NONE = 0,
		WINDOW_NO_MOVE = 1 << 0,
		WINDOW_NO_RESIZE = 1 << 1,
		WINDOW_NO_TITLEBAR = 1 << 2,
		WINDOW_NO_COLLAPSE = 1 << 3,
		WINDOW_NO_BACKGROUND = 1 << 4,
		WINDOW_NO_MOUSE_INPUTS = 1 << 5,
		WINDOW_NO_FOCUS_ON_APPEARING = 1 << 6,
		WINDOW_NO_BRING_TO_FRONT_ON_FOCUS = 1 << 7,
		WINDOW_NO_CLOSEBOX = 1 << 8,
		WINDOW_NO_MINIMIZEBOX = 1 << 9,
		WINDOW_NO_MAXIMIZEBOX = 1 << 10,
	};
	enum window_status
	{
		WINDOW_SHOW = 1 << 0,
		WINDOW_HIDE = 1 << 1,
		WINDOW_RESTORE = 1 << 2,
		WINDOW_MINIMIZE = 1 << 3,
		WINDOW_MAXIMIZE = 1 << 4,
	};
	enum modifier_flag
	{
		MOD_NONE = 0,
		MOD_SHIFT = 1 << 0,
		MOD_CONTROL = 1 << 1,
		MOD_ALT = 1 << 2,
		MOD_META = 1 << 3,
		MOD_KEYPAD = 1 << 4,
		MOD_GROUP = 1 << 5,
	};
	enum alignment_flag
	{
		ALIGN_LEFT = 1 << 0,
		ALIGN_RIGHT = 1 << 1,
		ALIGN_TOP = 1 << 2,
		ALIGN_BOTTOM = 1 << 3,
		ALIGN_VCENTER = 1 << 4,
		ALIGN_HCENTER = 1 << 5,
		ALIGN_CENTER = ALIGN_VCENTER | ALIGN_HCENTER,
	};

	using font_id = std::size_t;
	using window_id = std::size_t;
	using texture_id = std::size_t;
	using string_id = std::string_view;
	static constexpr const std::size_t invalid_id = std::numeric_limits<std::size_t>::max();

	using error_callback_type = std::function<void( const context *, std::error_code )>;
	using item_data_callback_type = std::function<std::string_view( const context *, int row, int col )>;

	class url : public std::string
	{
	public:
		using char_type = char;
		using string_type = std::string;
		using string_view_type = std::string_view;
		using iterator = string_type::iterator;
		using const_iterator = string_type::const_iterator;

	private:
		template<typename T> struct constexpr_flags;
		template<> struct constexpr_flags<char>
		{
			static constexpr const char * scheme_flag = "://";
			static constexpr const char * username_flag = "@";
			static constexpr const char * password_flag = ":";
			static constexpr const char * host_flag = "/";
			static constexpr const char * port_flag = ":";
			static constexpr const char * path_flag = "?";
			static constexpr const char * fragment_flag = "#";
			static constexpr const char query_flag = '=';
			static constexpr const char query_pair_flag = '&';
		};

	public:
		url() = default;
		url( xui::url && val );
		url( const xui::url & val );
		url( string_type && val );
		url( const string_type & val );
		url( string_view_type val );

	public:
		url & operator=( xui::url && val );
		url & operator=( const xui::url & val );
		url & operator=( string_type && val );
		url & operator=( const string_type & val );
		url & operator=( string_view_type val );

	public:
		string_view_type string_view() const;

	public:
		string_view_type scheme() const;
		string_view_type username() const;
		string_view_type password() const;
		string_view_type host() const;
		string_view_type port() const;
		string_view_type path() const;
		string_view_type querys() const;
		string_view_type fragment() const;

	public:
		std::size_t query_count() const;
		string_view_type query_at( std::size_t idx ) const;

	private:
		const_iterator find_it( string_view_type substr, const_iterator offset ) const;
		const_iterator find_it( string_view_type substr, const_iterator beg, const_iterator end ) const;

	private:
		void parse();

	private:
		string_view_type _scheme;
		string_view_type _username;
		string_view_type _password;
		string_view_type _host;
		string_view_type _port;
		string_view_type _path;
		string_view_type _query;
		string_view_type _fragment;
	};

	class vec2
	{
	public:
		float x = 0, y = 0;
	};

	class vec4
	{
	public:
		float x = 0, y = 0, z = 0, w = 0;
	};

	class size
	{
	public:
		float w = 0, h = 0;
	};

	class rect
	{
	public:
		float x = 0, y = 0, w = 0, h = 0;

	public:
		xui::vec2 center() const;
		bool contains( const xui::vec2 & p ) const;
		xui::rect margins_added( float left, float right, float top, float bottom ) const;
	};

	class color
	{
	public:
		union
		{
			std::uint32_t hex;
			struct
			{
				std::uint8_t r;
				std::uint8_t g;
				std::uint8_t b;
				std::uint8_t a;
			};
		};

	public:
		color();
		color( std::uint32_t hex );
		color( std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255 );

	public:
		xui::color dark() const;
		xui::color light() const;
		xui::color lerp( const xui::color & target, float t ) const;
	};

	class hatch_color
	{
	public:
		xui::color fore;
		xui::color back;
	};

	class texture_brush
	{
	public:
		enum warp
		{
			WRAP_TILE,
			WRAP_TILEFLIPX,
			WRAP_TILEFLIPY,
			WRAP_TILEFLIPXY,
			WRAP_CLAMP
		};

		xui::url image;
		warp mode = WRAP_TILE;
	};

	class linear_gradient
	{
	public:
		xui::vec2 p1, p2;
		xui::color c1, c2;
	};

	class stroke
	{
	public:
		enum
		{
			SOLID,				// _____________
			DASHED,				// _ _ _ _ _ _ _
			DOTTED,				// . . . . . . .
			DASH_DOT,			// _ . _ . _ . _
			DASH_DOT_DOT,		// _ . . _ . . _
		};

		uint32_t style = 1;
		float width = 1;
		xui::color color;
	};

	class border : public stroke
	{
	public:
		xui::vec4 radius;
	};

	class filled
	{
	public:
		enum
		{
			SOLID, // xui::color 
			DENSE1, // xui::style::hatch_color
			DENSE2,
			DENSE3,
			DENSE4,
			DENSE5,
			DENSE6,
			DENSE7,
			HORIZONTAL,
			VERTICAL,
			CROSS,
			FORWARD,
			BACKWARD,
			DIAGCROSS,
			TEXTURE, // xui::style::texture_brush
			LINEAR_GRADIENT, // xui::style::linear_gradient 
		};

		uint32_t style = SOLID;
		std::variant<std::monostate, xui::color, xui::hatch_color, xui::texture_brush, xui::linear_gradient> colors;
	};


	class style
	{
	private:
		struct inherit { };
		using variant_type = std::variant<std::monostate, int, float, uint32_t, std::string, xui::color, xui::vec2, xui::vec4, xui::url, xui::hatch_color, xui::texture_brush, xui::linear_gradient, xui::stroke, xui::border, xui::filled, inherit>;

	public:
		struct variant : public variant_type
		{
		public:
			using variant_type::variant;

		public:
			static constexpr const std::size_t nil_idx = 0;
			static constexpr const std::size_t int_idx = 1;
			static constexpr const std::size_t float_idx = 2;
			static constexpr const std::size_t flag_idx = 3;
			static constexpr const std::size_t string_idx = 4;
			static constexpr const std::size_t color_idx = 5;
			static constexpr const std::size_t vec2_idx = 6;
			static constexpr const std::size_t vec4_idx = 7;
			static constexpr const std::size_t url_idx = 8;
			static constexpr const std::size_t hatch_color_idx = 9;
			static constexpr const std::size_t texture_brush_idx = 10;
			static constexpr const std::size_t linear_gradient_idx = 11;
			static constexpr const std::size_t stroke_idx = 12;
			static constexpr const std::size_t border_idx = 13;
			static constexpr const std::size_t filled_idx = 14;
			static constexpr const std::size_t inherit_idx = 15;

		public:
			template<typename T> T value( const T & def = {} ) const
			{
				if ( index() == nil_idx )
					return def;

				if constexpr ( std::is_enum_v<T> )
				{
					return (T)std::get<uint32_t>( *this );
				}
				else if constexpr ( std::is_same_v<T, int> )
				{
					if ( index() == float_idx )
						return (T)std::get<float>( *this );

					return std::get<T>( *this );
				}
				else if constexpr ( std::is_same_v<T, float> )
				{
					if ( index() == int_idx )
						return (T)std::get<int>( *this );

					return std::get<T>( *this );
				}
				else if constexpr ( std::is_same_v<T, xui::size> )
				{
					if ( index() == vec2_idx )
					{
						auto v2 = std::get<xui::vec2>( *this );
						return xui::size( v2.x, v2.y );
					}

					return std::get<T>( *this );
				}
				else if constexpr ( std::is_same_v<T, xui::rect> )
				{
					if ( index() == vec4_idx )
					{
						auto v4 = std::get<xui::vec4>( *this );
						return xui::rect( v4.x, v4.y, v4.z, v4.w );
					}

					return std::get<T>( *this );
				}
				else
				{
					return std::get<T>( *this );
				}

				return def;
			}
		};
		struct selector
		{
			std::pmr::map<std::string, variant> attrs;
		};

	public:
		style( std::pmr::memory_resource * res = std::pmr::get_default_resource() );

	public:
		bool parse( std::string_view str );
		xui::style::variant find( std::string_view name ) const;
		template<typename T, typename Container> void get_values( Container & _c ) const
		{
			for ( const auto & it : _selectors )
			{
				for ( const auto & addr : it.second.attrs )
				{
					std::visit( overload(
						[&]( const T & val )
					{
						_c.push_back( val );
					},
						[]( const auto & )
					{}
					), addr.second );
				}
			}
		}

	private:
		std::optional<xui::style::variant> find( std::string_view type, std::string_view attr ) const;

	private:
		static xui::style::selector parse_selector( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::style::variant parse_attribute( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::color parse_light( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::color parse_dark( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::color parse_rgba( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::color parse_rgb( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::vec2 parse_vec2( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::vec4 parse_vec4( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::color parse_hex( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::url parse_url( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::hatch_color parse_hatch( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::texture_brush parse_sample( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::linear_gradient parse_linear( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::stroke parse_stroke( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::border parse_border( std::string_view::iterator & beg, std::string_view::iterator end );
		static xui::filled parse_filled( std::string_view::iterator & beg, std::string_view::iterator end );

	private:
		static std::map<std::string_view, std::uint32_t> & flags();
		static std::map<std::string_view, std::uint32_t> & colors();
		static std::map<std::string_view, std::function<xui::style::variant( std::string_view::iterator &, std::string_view::iterator )>> & functions();

	private:
		template<char c, typename It> static bool check( It & beg, const It & end )
		{
			while ( std::isspace( *beg ) ) ++beg;

			return ( beg != end && *beg == c );
		}

	private:
		std::pmr::map<std::string, selector> _selectors;
	};

	class drawcmd
	{
	public:
		struct text_element
		{
			xui::rect rect;
			xui::color color;
			std::string text;
			xui::font_id font;
			xui::alignment_flag align = xui::alignment_flag::ALIGN_CENTER;
		};
		struct line_element
		{
			xui::vec2 p1, p2;
			xui::stroke stroke;
		};
		struct rect_element
		{
			xui::rect rect;
			xui::border border;
			xui::filled filled;
		};
		struct path_element
		{
			inline path_element & moveto( const xui::vec2 & p )
			{
				data.append( std::format( "M{} {} ", p.x, p.y ) );
				return *this;
			}
			inline path_element & lineto( const xui::vec2 & p )
			{
				data.append( std::format( "L{} {} ", p.x, p.y ) );
				return *this;
			}
			inline path_element & curveto( const xui::vec2 & c1, const xui::vec2 & c2, const xui::vec2 & e )
			{
				data.append( std::format( "C{} {} {} {} {} {} ", c1.x, c1.y, c2.x, c2.y, e.x, e.y ) );
				return *this;
			}
			inline path_element & smooth_curveto( const xui::vec2 & c, const xui::vec2 & e )
			{
				data.append( std::format( "S{} {} {} {} ", c.x, c.y, e.x, e.y ) );
				return *this;
			}
			inline path_element & quadratic_belzier_curve( const xui::vec2 & c, const xui::vec2 & e )
			{
				data.append( std::format( "Q{} {} {} {} ", c.x, c.y, e.x, e.y ) );
				return *this;
			}
			inline path_element & smooth_quadratic_belzier_curveto( const xui::vec2 & e )
			{
				data.append( std::format( "T{} {} ", e.x, e.y ) );
				return *this;
			}
			inline path_element & closepath()
			{
				data.append( "Z " );
				return *this;
			}

			std::string data;
			xui::stroke stroke;
			xui::filled filled;
		};
		struct image_element
		{
			xui::rect rect;
			xui::texture_id id;
		};
		struct circle_element
		{
			float radius = 1;
			xui::vec2 center;
			xui::border border;
			xui::filled filled;
		};
		struct ellipse_element
		{
			xui::vec2 center;
			xui::vec2 radius;
			xui::border border;
			xui::filled filled;
		};
		struct polygon_element
		{
			xui::border border;
			xui::filled filled;
			std::pmr::vector<xui::vec2> points;
		};

	public:
		window_id id;
		std::variant<std::monostate, text_element, line_element, rect_element, path_element, image_element, circle_element, ellipse_element, polygon_element> element;
	};

	class context
	{
	private:
		struct private_p;

	public:
		context( std::pmr::memory_resource * res = std::pmr::get_default_resource() );
		~context();

	private:
		context( context && ) = delete;
		context( const context & ) = delete;
		context & operator=( context && ) = delete;
		context & operator=( const context & ) = delete;

	public:
		static std::string_view dark_style();
		static std::string_view light_style();

	public:
		void init( implement * impl );
		void release();

	public:
		void set_scale( float factor );
		void set_error( const xui::error_callback_type & callback );

	public:
		void push_style( xui::style * style );
		void pop_style();
		xui::style::variant current_style( std::string_view attr ) const;
		template<typename T> T current_style( std::string_view attr, const T & def ) const
		{
			return current_style( attr ).value<T>( def );
		}

		void push_style_type( std::string_view type );
		void pop_style_type();
		std::string_view current_style_type() const;

		void push_style_element( std::string_view type );
		void pop_style_element();
		std::string_view current_style_element() const;

		void push_style_action( std::string_view type );
		void pop_style_action();
		std::string_view current_style_action() const;

	public:
		void push_font( xui::font_id font );
		void pop_font();
		xui::font_id current_font() const;

		void push_rect( const xui::rect & rect );
		void pop_rect();
		xui::rect currrent_rect() const;

		void push_disable( bool val );
		void pop_disable();
		bool current_disable() const;

		void push_string_id( xui::string_id id );
		void pop_string_id();
		xui::string_id current_string_id() const;

		void push_window_id( xui::window_id id );
		void pop_window_id();
		xui::window_id current_window_id() const;

		void push_texture_id( xui::texture_id id );
		void pop_texture_id();
		xui::texture_id current_texture_id() const;

	public:
		template<typename F> void draw_style( xui::style * style, F && f )
		{
			push_style( style );
			f();
			pop_style();
		}
		template<typename F> void draw_style_type( std::string_view type, F && func )
		{
			if ( !type.empty() ) push_style_type( type );
			func();
			if ( !type.empty() ) pop_style_type();
		}
		template<typename F> void draw_style_element( std::string_view element, F && func )
		{
			if ( !element.empty() ) push_style_element( element );
			func();
			if ( !element.empty() ) pop_style_element();
		}
		template<typename F> void draw_style_action( std::string_view action, F && func )
		{
			if ( !action.empty() ) push_style_action( action );
			func();
			if ( !action.empty() ) pop_style_action();
		}
		template<typename F> void draw_font( xui::font_id id, F && func )
		{
			push_font( id );
			func();
			pop_font();
		}
		template<typename F> void draw_rect( const xui::rect & rect, F && f )
		{
			push_rect( rect );
			f();
			pop_rect();
		}
		template<typename F> void draw_disable( bool val, F && f )
		{
			push_disable( val );
			f();
			pop_disable();
		}
		template<typename F> void draw_string_id( xui::string_id str_id, F && f )
		{
			push_string_id( str_id );
			f();
			pop_string_id();
		}
		template<typename F> void draw_window_id( xui::window_id id, F && func )
		{
			push_window_id( id );
			func();
			pop_window_id();
		}
		template<typename F> void draw_texture_id( xui::texture_id id, F && func )
		{
			push_texture_id( id );
			func();
			pop_texture_id();
		}

	public:
		void begin();
		std::span<xui::drawcmd> end();

	public:
		bool begin_window( std::string_view title, xui::texture_id icon_id, int flags = xui::window_flag::WINDOW_NONE );
		bool begin_window( xui::string_id str_id, std::string_view title, xui::texture_id icon_id, int flags = xui::window_flag::WINDOW_NONE );
		void end_window();

	public:
		bool image( xui::texture_id id );
		bool image( xui::string_id str_id, xui::texture_id id );
		bool label( std::string_view text );
		bool label( xui::string_id str_id, std::string_view text );
		bool radio( bool & checked );
		bool radio( xui::string_id str_id, bool & checked );
		bool check( bool & checked );
		bool check( xui::string_id str_id, bool & checked );
		bool button( std::string_view text );
		bool button( xui::string_id str_id, std::string_view text );
		float slider( float & value, float min, float max );
		float slider( xui::string_id str_id, float & value, float min, float max );
		bool process( float value, float min, float max, std::string_view text = "" );
		bool process( xui::string_id str_id, float value, float min, float max, std::string_view text = "" );
		float scrollbar( float & value, float step, float min, float max, xui::direction dir = xui::direction::TOP_BOTTOM );
		float scrollbar( xui::string_id str_id, float & value, float step, float min, float max, xui::direction dir = xui::direction::TOP_BOTTOM );

	public:
		bool begin_menubar( xui::string_id str_id = "" );
		bool begin_menu( std::string_view name );
		bool menu_item( std::string_view name );
		bool menu_separator();
		void end_menu();
		void end_menubar();

	public:
		bool begin_combobox();
		void end_combobox();
		bool combobox_item();

	public:
		bool begin_tabview();
		bool begin_tab();
		void end_tab();
		void end_tabview();

	public:
		bool begin_listview();
		void end_listview();
		bool listview_item();

	public:
		bool begin_treeview();
		void end_treeview();
		bool treeview_item();

	public:
		bool begin_tableview();
		void tableview_header();
		void tableview_item();
		void end_tableview();

	public:
		template<typename F> std::span<xui::drawcmd> draw( F && f )
		{
			begin();
			f();
			return end();
		}
		template<typename F> bool draw_window( xui::string_id str_id, std::string_view title, xui::texture_id icon_id, int flags, F && f )
		{
			begin_window( title, icon_id, flags );
			f();
			end_window();
		}

	public:
		xui::drawcmd::text_element & draw_text( std::string_view text, xui::font_id id, const xui::rect & rect, const xui::color & font_color, xui::alignment_flag text_align );
		xui::drawcmd::line_element & draw_line( const xui::vec2 & p1, const xui::vec2 & p2, const xui::stroke & stroke );
		xui::drawcmd::rect_element & draw_rect( const xui::rect & rect, const xui::border & border, const xui::filled filled );
		xui::drawcmd::path_element & draw_path( const xui::stroke & stroke, const xui::filled filled );
		xui::drawcmd::image_element & draw_image( xui::texture_id id, const xui::rect & rect );
		xui::drawcmd::circle_element & draw_circle( const xui::vec2 & center, float radius, const xui::border & border, const xui::filled filled );
		xui::drawcmd::ellipse_element & draw_ellipse( const xui::vec2 & center, const xui::vec2 & radius, const xui::border & border, const xui::filled filled );
		xui::drawcmd::polygon_element & draw_polygon( std::span<xui::vec2> points, const xui::border & border, const xui::filled filled );

	private:
		std::string style_name() const;
		std::string focus_name() const;

	private:
		void push_focus( xui::event event );
		void pop_focus();
		bool current_focus() const;
		bool inherit_focus() const;
		std::string_view get_action_name( bool focus = false );

	private:
		private_p * _p;
	};

	class implement
	{
	private:
		implement( implement && ) = delete;
		implement( const implement & ) = delete;
		implement & operator=( implement && ) = delete;
		implement & operator=( const implement & ) = delete;

	public:
		implement() = default;
		virtual ~implement() = default;

	public:
		virtual xui::window_id create_window( std::string_view title, xui::texture_id icon, const xui::rect & rect, xui::window_id parent = xui::invalid_id ) = 0;
		virtual xui::window_id get_window_parent( xui::window_id id ) const = 0;
		virtual void set_window_parent( xui::window_id id, xui::window_id parent ) = 0;
		virtual xui::window_status get_window_status( xui::window_id id ) const = 0;
		virtual void set_window_status( xui::window_id id, xui::window_status show ) = 0;
		virtual xui::rect get_window_rect( xui::window_id id ) const = 0;
		virtual void set_window_rect( xui::window_id id, const xui::rect & rect ) = 0;
		virtual std::string get_window_title( xui::window_id id ) const = 0;
		virtual void set_window_title( xui::window_id id, std::string_view title ) = 0;
		virtual void remove_window( xui::window_id id ) = 0;

	public:
		virtual bool load_font_file( std::string_view filename ) = 0;
		virtual xui::font_id create_font( std::string_view family, int size, xui::font_flag flag ) = 0;
		virtual xui::size font_size( xui::font_id id, std::string_view text ) const = 0;
		virtual void remove_font( xui::font_id id ) = 0;

	public:
		virtual xui::texture_id create_texture( std::string_view filename ) = 0;
		virtual xui::size texture_size( xui::texture_id id ) const = 0;
		virtual void remove_texture( xui::texture_id id ) = 0;

	public:
		virtual xui::vec2 get_cursor_dt( xui::window_id id ) const = 0;
		virtual xui::vec2 get_cursor_pos( xui::window_id id ) const = 0;
		virtual xui::vec2 get_cusor_wheel( xui::window_id id ) const = 0;
		virtual std::string get_unicodes( xui::window_id id ) const = 0;
		virtual int get_event( xui::window_id id, xui::event key ) const = 0;
		virtual std::span<xui::vec2> get_touchs( xui::window_id id ) const = 0;
		virtual std::string get_clipboard_data( xui::window_id id, std::string_view mime ) const = 0;
		virtual bool set_clipboard_data( xui::window_id id, std::string_view mime, std::string_view data ) = 0;
	};


	inline xui::vec2	operator-( const xui::vec2 & lhs )
	{
		return xui::vec2( -lhs.x, -lhs.y );
	}
	inline xui::vec2	operator+( const xui::vec2 & lhs, const float rhs )
	{
		return xui::vec2( lhs.x + rhs, lhs.y + rhs );
	}
	inline xui::vec2	operator-( const xui::vec2 & lhs, const float rhs )
	{
		return xui::vec2( lhs.x - rhs, lhs.y - rhs );
	}
	inline xui::vec2	operator*( const xui::vec2 & lhs, const float rhs )
	{
		return xui::vec2( lhs.x * rhs, lhs.y * rhs );
	}
	inline xui::vec2	operator/( const xui::vec2 & lhs, const float rhs )
	{
		return xui::vec2( lhs.x / rhs, lhs.y / rhs );
	}
	inline xui::vec2	operator+( const xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		return xui::vec2( lhs.x + rhs.x, lhs.y + rhs.y );
	}
	inline xui::vec2	operator-( const xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		return xui::vec2( lhs.x - rhs.x, lhs.y - rhs.y );
	}
	inline xui::vec2	operator*( const xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		return xui::vec2( lhs.x * rhs.x, lhs.y * rhs.y );
	}
	inline xui::vec2	operator/( const xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		return xui::vec2( lhs.x / rhs.x, lhs.y / rhs.y );
	}
	inline xui::vec2 &  operator+=( xui::vec2 & lhs, const float rhs )
	{
		lhs.x += rhs; lhs.y += rhs; return lhs;
	}
	inline xui::vec2 &  operator-=( xui::vec2 & lhs, const float rhs )
	{
		lhs.x -= rhs; lhs.y -= rhs; return lhs;
	}
	inline xui::vec2 &  operator*=( xui::vec2 & lhs, const float rhs )
	{
		lhs.x *= rhs; lhs.y *= rhs; return lhs;
	}
	inline xui::vec2 &  operator/=( xui::vec2 & lhs, const float rhs )
	{
		lhs.x /= rhs; lhs.y /= rhs; return lhs;
	}
	inline xui::vec2 &  operator+=( xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		lhs.x += rhs.x; lhs.y += rhs.y; return lhs;
	}
	inline xui::vec2 &  operator-=( xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs;
	}
	inline xui::vec2 &  operator*=( xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs;
	}
	inline xui::vec2 &  operator/=( xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs;
	}

	inline bool			operator==( const xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	inline bool			operator!=( const xui::vec2 & lhs, const xui::vec2 & rhs )
	{
		return lhs.x != rhs.x || lhs.y != rhs.y;
	}
	inline bool			operator==( const xui::size & lhs, const xui::size & rhs )
	{
		return lhs.w == rhs.w && lhs.h == rhs.h;
	}
	inline bool			operator!=( const xui::size & lhs, const xui::size & rhs )
	{
		return lhs.w != rhs.w || lhs.h != rhs.h;
	}
	inline bool			operator==( const xui::rect & lhs, const xui::rect & rhs )
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.w == rhs.w && lhs.h == rhs.h;
	}
	inline bool			operator!=( const xui::rect & lhs, const xui::rect & rhs )
	{
		return lhs.x != rhs.x || lhs.y != rhs.y || lhs.w != rhs.w || lhs.h != rhs.h;
	}
}
