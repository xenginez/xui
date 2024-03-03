#pragma once

#include <map>
#include <span>
#include <format>
#include <string>
#include <variant>
#include <functional>
#include <system_error>
#include <memory_resource>

namespace xui
{
	class size;
	class rect;
	class vec4;
	class point;
	class color;
	class style;
	class drawcmd;
	class context;
	class implement;
	class textedit_state;

	enum err
	{
		ERR_NO = 0,
	};
	enum event
	{
		KEY_EVENT_BEG = 0,
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

		KEY_GAMEPAD_START,			// Menu (Xbox)      + (Switch)   Start/Options (PS)
		KEY_GAMEPAD_BACK,			// View (Xbox)      - (Switch)   Share (PS)
		KEY_GAMEPAD_FACE_LEFT,		// X (Xbox)         Y (Switch)   Square (PS)        // Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
		KEY_GAMEPAD_FACE_RIGHT,		// B (Xbox)         A (Switch)   Circle (PS)        // Cancel / Close / Exit
		KEY_GAMEPAD_FACE_UP,		// Y (Xbox)         X (Switch)   Triangle (PS)      // Text Input / On-screen Keyboard
		KEY_GAMEPAD_FACE_DOWN,		// A (Xbox)         B (Switch)   Cross (PS)         // Activate / Open / Toggle / Tweak
		KEY_GAMEPAD_DPAD_LEFT,		// D-pad Left                                       // Move / Tweak / Resize Window (in Windowing mode)
		KEY_GAMEPAD_DPAD_RIGHT,		// D-pad Right                                      // Move / Tweak / Resize Window (in Windowing mode)
		KEY_GAMEPAD_DPAD_UP,		// D-pad Up                                         // Move / Tweak / Resize Window (in Windowing mode)
		KEY_GAMEPAD_DPAD_DOWN,		// D-pad Down                                       // Move / Tweak / Resize Window (in Windowing mode)
		KEY_GAMEPAD_L1,				// L Bumper (Xbox)  L (Switch)   L1 (PS)            // Tweak Slower / Focus Previous (in Windowing mode)
		KEY_GAMEPAD_R1,				// R Bumper (Xbox)  R (Switch)   R1 (PS)            // Tweak Faster / Focus Next (in Windowing mode)
		KEY_GAMEPAD_L2,				// L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
		KEY_GAMEPAD_R2,				// R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
		KEY_GAMEPAD_L3,				// L Stick (Xbox)   L3 (Switch)  L3 (PS)
		KEY_GAMEPAD_R3,				// R Stick (Xbox)   R3 (Switch)  R3 (PS)
		KEY_GAMEPAD_L_STICK_LEFT,	// [Analog]                                         // Move Window (in Windowing mode)
		KEY_GAMEPAD_L_STICK_RIGHT,	// [Analog]                                         // Move Window (in Windowing mode)
		KEY_GAMEPAD_L_STICK_UP,		// [Analog]                                         // Move Window (in Windowing mode)
		KEY_GAMEPAD_L_STICK_DOWN,	// [Analog]                                         // Move Window (in Windowing mode)
		KEY_GAMEPAD_R_STICK_LEFT,	// [Analog]
		KEY_GAMEPAD_R_STICK_RIGHT,	// [Analog]
		KEY_GAMEPAD_R_STICK_UP,		// [Analog]
		KEY_GAMEPAD_R_STICK_DOWN,	// [Analog]

		KEY_MOUSE_LEFT, KEY_MOUSE_RIGHT, KEY_MOUSE_MIDDLE, KEY_MOUSE_X1, KEY_MOUSE_X2,
		KEY_EVENT_END = KEY_MOUSE_X2,

		MOUSE_EVENT_BEG = KEY_EVENT_END,
		MOUSE_ENTER, MOUSE_LEAVE, MOUSE_MOVE, MOUSE_WHEEL,
		MOUSE_EVENT_END = MOUSE_WHEEL,

		WINDOW_EVENT_BEG = MOUSE_EVENT_END,
		WINDOW_CLOSE,
		WINDOW_EVENT_END = WINDOW_CLOSE,

		EVENT_MAX_COUNT,
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
	enum windowstatus
	{
		SHOW		= 1 << 0,
		HIDE		= 1 << 1,
		RESTORE		= 1 << 2,
		MINIMIZE	= 1 << 3,
		MAXIMIZE	= 1 << 4,
	};

	enum font_flag
	{
		FONT_NONE						= 0,
		FONT_BOLD						= 1 << 0,
		FONT_ITALIC						= 1 << 1,
		FONT_UNDERLINE					= 1 << 2,
		FONT_STRIKEOUT					= 1 << 3,
	};
	enum window_flag
	{
		WINDOW_NONE							= 0,
		WINDOW_NO_MOVE						= 1 << 0,
		WINDOW_NO_RESIZE					= 1 << 1,
		WINDOW_NO_TITLEBAR					= 1 << 2,
		WINDOW_NO_COLLAPSE					= 1 << 3,
		WINDOW_NO_BACKGROUND				= 1 << 4,
		WINDOW_NO_MOUSE_INPUTS				= 1 << 5,
		WINDOW_NO_FOCUS_ON_APPEARING		= 1 << 6,
		WINDOW_NO_BRING_TO_FRONT_ON_FOCUS	= 1 << 7,
		WINDOW_NO_CLOSEBOX					= 1 << 8,
		WINDOW_NO_MINIMIZEBOX				= 1 << 9,
		WINDOW_NO_MAXIMIZEBOX				= 1 << 10,
	};
	enum textedit_flag
	{

	};
	enum modifier_flag
	{
		MODIFIER_NONE			= 0,
		MODIFIER_ALT			= 1 << 0,
		MODIFIER_CTRL			= 1 << 1,
		MODIFIER_SHIFT			= 1 << 2,
		MODIFIER_SUPER			= 1 << 3,
		MODIFIER_KEYPAD			= 1 << 4,
	};
	enum alignment_flag
	{
		LEFT		= 1 << 0,
		RIGHT		= 1 << 1,
		TOP			= 1 << 2,
		BOTTOM		= 1 << 3,
		VCENTER		= 1 << 4,
		HCENTER		= 1 << 5,
		CENTER		= VCENTER | HCENTER,
	};

	using font_id = std::size_t;
	using window_id = std::size_t;
	using texture_id = std::size_t;
	using string_id = std::string_view;
	static constexpr const std::size_t invalid_id = std::numeric_limits<std::size_t>::max();

	using error_callback_type = std::function<void( const context *, std::error_code )>;
	using item_data_callback_type = std::function<std::string_view( const context *, int row, int col )>;

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
		bool contains( const xui::point & p ) const;
		xui::rect margins_added( float left, float right, float top, float bottom ) const;
	};

	class vec4
	{
	public:
		float x = 0, y = 0, z = 0, w = 0;
	};

	class point
	{
	public:
		float x = 0, y = 0;
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
		xui::color lerp( const xui::color & target, float t ) const;
	};

	class style
	{
	public:
		struct url : public std::string
		{
		public:
			std::string::basic_string;
		};
		struct variant : public std::variant<std::monostate, int, float, std::string, xui::color, xui::vec4, url>
		{
		public:
			using std::variant<std::monostate, int, float, std::string, xui::color, xui::vec4, url>::variant;

		public:
			template<typename T> T value( const T & def = {} ) const
			{
				if ( index() == 0 )
					return def;

				if constexpr ( std::is_enum_v<T> )
				{
					return (T)std::get<int>( *this );
				}
				else if constexpr ( std::is_same_v<T, int> )
				{
					if ( index() == 2 )
						return (T)std::get<float>( *this );

					return std::get<T>( *this );
				}
				else if constexpr ( std::is_same_v<T, float> )
				{
					if ( index() == 3 )
						return (T)std::get<float>( *this );

					return std::get<T>( *this );
				}
				else if constexpr ( std::is_same_v<T, std::string> )
				{
					if ( index() == 6 )
						return std::get<url>( *this );

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
			std::string name;
			std::pmr::map<std::string, variant> attrs;
		};

	public:
		style( std::pmr::memory_resource * res = std::pmr::get_default_resource() );

	public:
		void load( std::string_view str );
		variant find( std::string_view name ) const;

	private:
		template<typename It> void skip( It & it, const It & end ) const
		{
			if ( it != end )
			{
				while ( it != end && ( *it == '\n' || *it == '\r' || *it == '\t' || *it == ' ' ) )
					++it;
			}
		}
		template<char c, typename It> It adv( It & it, const It & end ) const
		{
			if ( it != end )
			{
				while ( it != end && *it != c ) ++it;

				auto result = it;

				skip( it, end );

				return result;
			}

			return it;
		}
		template<char c, typename It> bool check( It & it, const It & end ) const
		{
			if ( it != end )
			{
				skip( it, end );

				if ( it != end && *it == c )
				{
					++it;

					skip( it, end );

					return true;
				}
			}

			return false;
		}

	private:
		std::pmr::map<std::string, selector> _selectors;
	};

	class drawcmd
	{
	public:
		struct stroke
		{
			enum
			{
				NONE,				//
				SOLID,				// _____________
				DASHED,				// _ _ _ _ _ _ _
				DOTTED,				// . . . . . . .
				DASH_DOT,			// _ . _ . _ . _
				DASH_DOT_DOT,		// _ . . _ . . _
			};

			int style = 1;
			float width = 1;
			xui::color color;
			xui::vec4 radius;
		};
		struct filled
		{
			xui::color color;
		};

	public:
		struct text_element
		{
			xui::rect rect;
			xui::color color;
			std::string text;
			xui::font_id font;
			xui::alignment_flag align = xui::alignment_flag::CENTER;
		};
		struct line_element
		{
			xui::point p1, p2;
			xui::drawcmd::stroke stroke;
		};
		struct rect_element
		{
			xui::rect rect;
			xui::drawcmd::stroke border;
			xui::drawcmd::filled filled;
		};
		struct path_element
		{
			inline path_element & moveto( const xui::point & p )
			{
				data.append( std::format( "M{} {} ", p.x, p.y ) );
				return *this;
			}
			inline path_element & lineto( const xui::point & p )
			{
				data.append( std::format( "L{} {} ", p.x, p.y ) );
				return *this;
			}
			inline path_element & curveto( const xui::point & c1, const xui::point & c2, const xui::point & e )
			{
				data.append( std::format( "C{} {} {} {} ", c1.x, c1.y, c2.x, c2.y, e.x, e.y ) );
				return *this;
			}
			inline path_element & smooth_curveto( const xui::point & c, const xui::point & e )
			{
				data.append( std::format( "S{} {} {} {} ", c.x, c.y, e.x, e.y ) );
				return *this;
			}
			inline path_element & quadratic_bezier_curve( const xui::point & c, const xui::point & e )
			{
				data.append( std::format( "Q{} {} {} {} ", c.x, c.y, e.x, e.y ) );
				return *this;
			}
			inline path_element & smooth_quadratic_bezier_curveto( const xui::point & e )
			{
				data.append( std::format( "T{} {} ", e.x, e.y ) );
				return *this;
			}
			inline path_element & elliptical_arc( float x_raduis, float y_raduis, float x_angle, bool arc_len, bool arc_dir, const xui::point & e )
			{
				data.append( std::format( "A{} {} {} {} {} {} {} ", x_raduis, y_raduis, x_angle, arc_len ? 1 : 0, arc_dir ? 1 : 0, e.x, e.y ) );
				return *this;
			}
			inline path_element & closepath()
			{
				data.append( "Z " );
				return *this;
			}

			std::string data;
			xui::drawcmd::stroke stroke;
			xui::drawcmd::filled filled;
		};
		struct image_element
		{
			xui::rect rect;
			xui::texture_id id;
		};
		struct circle_element
		{
			float radius = 1;
			xui::point center;
			xui::drawcmd::stroke border;
			xui::drawcmd::filled filled;
		};
		struct ellipse_element
		{
			xui::point center;
			xui::point radius;
			xui::drawcmd::stroke border;
			xui::drawcmd::filled filled;
		};
		struct polygon_element
		{
			xui::drawcmd::stroke border;
			xui::drawcmd::filled filled;
			std::pmr::vector<xui::point> points;
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

		void push_font( xui::font_id font );
		void pop_font();
		xui::font_id current_font() const;

		void push_window( xui::window_id id );
		void pop_window();
		xui::window_id current_window() const;

		void push_texture( xui::texture_id id );
		void pop_texture();
		xui::texture_id current_texture() const;

		void push_string_id( xui::string_id id );
		void pop_string_id();
		xui::string_id current_string_id() const;

		void push_rect( const xui::rect & rect );
		void pop_rect();
		xui::rect currrent_rect() const;

		void push_window_flag( xui::window_flag flag );
		void pop_window_flag();
		xui::window_flag current_flag() const;

	public:
		void begin();
		std::span<xui::drawcmd> end();

	public:
		bool begin_window( std::string_view title, xui::texture_id icon, xui::window_flag flags = xui::window_flag::WINDOW_NONE );
		void end_window();

	public:
		void image( xui::texture_id id );
		void label( std::string_view text );
		bool button( std::string_view text );
		void process(float value, std::string_view text = "" );
		bool textedit( textedit_state * state );

	public:
		bool slider_int();
		bool slider_float();
		bool slider_angle();

	public:
		bool radio_button();
		bool check_button();
		bool image_button();

	public:
		bool begin_menubar();
		void end_menubar();
		bool begin_menu();
		void end_menu();
		bool menu_item();
		bool menu_separator();

	public:
		bool begin_combobox();
		void end_combobox();
		bool combobox_item();

	public:
		bool begin_tabview();
		void end_tabview();
		bool begin_tab();
		void end_tab();

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
		void end_tableview();
		void tableview_header();
		void tableview_item();

	public:
		template<typename F> std::span<xui::drawcmd> draw( F && f )
		{
			begin();
			f();
			return end();
		}

	private:
		void push_type( std::string_view type );
		void push_element( std::string_view type );
		void push_action( std::string_view type );
		void pop_action();
		void pop_element();
		void pop_type();
		template<typename F> void draw_type( std::string_view type, F && func )
		{
			push_type( type );
			func();
			pop_type();
		}
		template<typename F> void draw_element( std::string_view element, F && func )
		{
			push_element( element );
			func();
			pop_element();
		}
		template<typename F> void draw_action( std::string_view action, F && func )
		{
			push_action( action );
			func();
			pop_action();
		}

	private:
		xui::drawcmd::text_element & draw_text( std::string_view text, xui::font_id id, const xui::rect & rect );
		xui::drawcmd::line_element & draw_line( const xui::point & p1, const xui::point & p2 );
		xui::drawcmd::rect_element & draw_rect( const xui::rect & rect );
		xui::drawcmd::path_element & draw_path( std::string_view data = "" );
		xui::drawcmd::image_element & draw_image( xui::texture_id id, const xui::rect & rect );
		xui::drawcmd::circle_element & draw_circle( const xui::point & center, float radius );
		xui::drawcmd::ellipse_element & draw_ellipse( const xui::point & center, const xui::point & radius );
		xui::drawcmd::polygon_element & draw_polygon( std::span<xui::point> points );

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
		virtual xui::windowstatus get_window_status( xui::window_id id ) const = 0;
		virtual void set_window_status( xui::window_id id, xui::windowstatus show ) = 0;
		virtual xui::rect get_window_rect( xui::window_id id ) const = 0;
		virtual void set_window_rect( xui::window_id id, const xui::rect & rect ) = 0;
		virtual std::string get_window_title( xui::window_id id ) const = 0;
		virtual void set_window_title( xui::window_id id, std::string_view title ) = 0;
		virtual void remove_window( xui::window_id id ) = 0;

	public:
		virtual bool load_font_file( std::string_view filename ) = 0;
		virtual xui::font_id create_font( std::string_view family, int size, xui::font_flag flag ) = 0;
		virtual int font_hight( xui::font_id id ) const = 0;
		virtual void remove_font( xui::font_id id ) = 0;

	public:
		virtual xui::texture_id create_texture( std::string_view filename ) = 0;
		virtual xui::size texture_size( xui::texture_id id ) const = 0;
		virtual void remove_texture( xui::texture_id id ) = 0;

	public:
		virtual int get_key( xui::window_id id, xui::event key ) const = 0;
		virtual xui::point get_cursor_pos( xui::window_id id ) const = 0;
	};

	class textedit_state
	{

	};

	namespace system_resource
	{
		static constexpr xui::string_id FONT_DEFAULT		= "font://default";

		static constexpr xui::string_id ICON_APPLICATION	= "icon://application";
		static constexpr xui::string_id ICON_ERROR			= "icon://error";
		static constexpr xui::string_id ICON_WARNING		= "icon://warning";
		static constexpr xui::string_id ICON_INFORMATION	= "icon://information";
	}

	inline xui::point	operator-( const xui::point & lhs )
	{
		return xui::point( -lhs.x, -lhs.y );
	}
	inline xui::point	operator+( const xui::point & lhs, const float rhs )
	{
		return xui::point( lhs.x + rhs, lhs.y + rhs );
	}
	inline xui::point	operator-( const xui::point & lhs, const float rhs )
	{
		return xui::point( lhs.x - rhs, lhs.y - rhs );
	}
	inline xui::point	operator*( const xui::point & lhs, const float rhs )
	{
		return xui::point( lhs.x * rhs, lhs.y * rhs );
	}
	inline xui::point	operator/( const xui::point & lhs, const float rhs )
	{
		return xui::point( lhs.x / rhs, lhs.y / rhs );
	}
	inline xui::point	operator+( const xui::point & lhs, const xui::point & rhs )
	{
		return xui::point( lhs.x + rhs.x, lhs.y + rhs.y );
	}
	inline xui::point	operator-( const xui::point & lhs, const xui::point & rhs )
	{
		return xui::point( lhs.x - rhs.x, lhs.y - rhs.y );
	}
	inline xui::point	operator*( const xui::point & lhs, const xui::point & rhs )
	{
		return xui::point( lhs.x * rhs.x, lhs.y * rhs.y );
	}
	inline xui::point	operator/( const xui::point & lhs, const xui::point & rhs )
	{
		return xui::point( lhs.x / rhs.x, lhs.y / rhs.y );
	}
	inline xui::point & operator+=( xui::point & lhs, const float rhs )
	{
		lhs.x += rhs; lhs.y += rhs; return lhs;
	}
	inline xui::point & operator-=( xui::point & lhs, const float rhs )
	{
		lhs.x -= rhs; lhs.y -= rhs; return lhs;
	}
	inline xui::point &	operator*=( xui::point & lhs, const float rhs )
	{
		lhs.x *= rhs; lhs.y *= rhs; return lhs;
	}
	inline xui::point &	operator/=( xui::point & lhs, const float rhs )
	{
		lhs.x /= rhs; lhs.y /= rhs; return lhs;
	}
	inline xui::point &	operator+=( xui::point & lhs, const xui::point & rhs )
	{
		lhs.x += rhs.x; lhs.y += rhs.y; return lhs;
	}
	inline xui::point &	operator-=( xui::point & lhs, const xui::point & rhs )
	{
		lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs;
	}
	inline xui::point &	operator*=( xui::point & lhs, const xui::point & rhs )
	{
		lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs;
	}
	inline xui::point &	operator/=( xui::point & lhs, const xui::point & rhs )
	{
		lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs;
	}

	inline bool			operator==( const xui::point & lhs, const xui::point & rhs )
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	inline bool			operator!=( const xui::point & lhs, const xui::point & rhs )
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
