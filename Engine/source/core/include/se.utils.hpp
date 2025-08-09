#pragma once
#include <utility>
#include <chrono>
#include <string>
#include <type_traits>
#include <spdlog/spdlog.h>
#include <glfw/include/GLFW/glfw3.h>
#include <tcb/span.hpp>
#include <string_view>
#include <magic_enum/magic_enum.hpp>
#include <fstream>

// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
// ┃ Compile-time constants                                                    ┃
// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
// Number of interleaving frame resources allocated
#define SE_FRAME_FLIGHTS_COUNT 2
// alias the external library namespace
namespace ext = TCB_SPAN_NAMESPACE_NAME;


namespace se {
	// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
	// ┃ log                                                                       ┃
	// ┠───────────────────────────────────────────────────────────────────────────┨
	// ┃ Just a wrapper of spdlog.                                                 ┃
	// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

  #define SE_WARN       SPDLOG_WARN
  #define SE_TRACE      SPDLOG_TRACE
  #define SE_DEBUG      SPDLOG_DEBUG
  #define SE_ERROR      SPDLOG_ERROR
  #define SE_CRITICAL   SPDLOG_CRITICAL
  template<typename... Args> void info(Args&&... args) { ::spdlog::info(std::forward<Args>(args)...); }
  template<typename... Args> void warn(Args&&... args) { ::spdlog::warn(std::forward<Args>(args)...); }
  template<typename... Args> void trace(Args&&... args) { ::spdlog::trace(std::forward<Args>(args)...); }
  template<typename... Args> void debug(Args&&... args) { ::spdlog::debug(std::forward<Args>(args)...); }
  template<typename... Args> void error(Args&&... args) { ::spdlog::error(std::forward<Args>(args)...); }
  template<typename... Args> void critical(Args&&... args) { ::spdlog::error(std::forward<Args>(args)...); }
  template<typename... Args> void set_pattern(Args&&... args) { ::spdlog::set_pattern(std::forward<Args>(args)...); }
  inline void set_level(::spdlog::level::level_enum log_level) { ::spdlog::set_level(log_level); }


  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ memory                                                                    ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┃ Memory management system.                              							     ┃
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

  /** A general memory allocator, probably faster than new char[].
    * An easier way to use the allocator is by creating se::MiniBuffer. */
  struct Memory {
    /** allocate size bytes of the memory */
    static auto allocate(size_t size) noexcept -> void*;
    /** free the allocated memory given pointer and size */
    static auto free(void* p, size_t size) noexcept -> void;
  };

  /** An interface of buffer. */
  struct MiniBuffer {
    /** the data pointer and size of the allocated buffer */
    void* m_data = nullptr; 
    size_t m_size = 0;
    /** identify whether the buffer is a reference */
    bool m_isReference = false;

    /** various constructors*/
    MiniBuffer(); MiniBuffer(size_t size);
    MiniBuffer(MiniBuffer const& b); MiniBuffer(MiniBuffer&& b);
    /** ctor from external memory, notice this only provides an inference,
      * without actual memory copy or release. */
    MiniBuffer(void* data, size_t size);
    /** release the buffer and dctor*/
    ~MiniBuffer(); auto release() noexcept -> void;
    /** the deep copy / moved copy of the buffer */
    auto operator=(MiniBuffer const& b)->MiniBuffer&;
    auto operator=(MiniBuffer&& b)->MiniBuffer&;
    /** cast the buffer as a span */
    template<class T> auto as_span() -> ext::span<T> {
      return ext::span<T>((T*)m_data, m_size / sizeof(T));
    }
  };


  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ file                                                                      ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┃ A lightweight interface of filesystem.                  							     ┃
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

  struct Filesys {
    // read and write files with Minibuffer interface
    //─────────────────────────────────────────────────────────────────
    static auto sync_read_file(std::string const& path, MiniBuffer& buffer) noexcept -> bool;
    static auto sync_write_file(std::string const& path, MiniBuffer& buffer) noexcept -> bool;

    // manipulate the file path string
    //─────────────────────────────────────────────────────────────────
    static auto preprocess(std::string const& path) noexcept -> std::string;
    static auto get_executable_path() noexcept -> std::string;
    static auto get_parent_path(std::string const& path) noexcept -> std::string;
    static auto get_stem(std::string const& path) noexcept -> std::string;
    static auto get_extension(std::string const& path) noexcept -> std::string;
    static auto get_filename(std::string const& path) noexcept -> std::string;
    static auto get_absolute_path(std::string const& path) noexcept -> std::string;
    static auto file_exists(std::string const& path) noexcept -> bool;
    static auto resolve_path(std::string const& path, std::vector<std::string> const& s) noexcept -> std::string;
  };


  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ resource                                                                  ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┃ An interface to provide uid for resource management.    							     ┃
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  using UID = uint64_t;
  struct Resources {
    static auto query_runtime_uid() noexcept -> UID;
    static auto query_string_uid(std::string const& str) noexcept -> UID;
    static auto query_string_uid(std::string_view str) noexcept -> UID;
  };
  
	// ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
	// ┃ useful templates                                                          ┃
	// ┠───────────────────────────────────────────────────────────────────────────┨
	// ┃ Bitflags, Singleton, Signal.                                              ┃
	// ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  // bitflags
  //─────────────────────────────────────────────────────────────────
  template <typename E>
  struct Flags {
  	using T = std::underlying_type_t<E>; T _mask;
  	// construction from a single enum value or raw mask
    constexpr Flags()           : _mask(0) {}
  	constexpr Flags(E e)        : _mask(static_cast<T>(e)) {}
  	constexpr Flags(T mask)     : _mask(mask)             {}
  	constexpr T mask() const     { return _mask; }
  	// bitwise operators
  	constexpr Flags operator|(Flags rhs) const { return Flags(_mask | rhs._mask); }
  	constexpr Flags operator&(Flags rhs) const { return Flags(_mask & rhs._mask); }
  	constexpr Flags& operator|=(Flags rhs)      { _mask |= rhs._mask; return *this; }
  	constexpr Flags& operator&=(Flags rhs)      { _mask &= rhs._mask; return *this; }
  	// allow checking “if (f) …”
  	explicit constexpr operator bool() const    { return _mask != 0; }
  };
  // free‐function overloads so you can do E|E → Flags<E> directly
  #define ENABLE_BITMASK_OPERATORS(EnumType)											\
  inline Flags<EnumType> operator|(EnumType lhs, EnumType rhs) {						\
  	using T = std::underlying_type_t<EnumType>;                                  	\
  	return Flags<EnumType>(static_cast<T>(lhs) | static_cast<T>(rhs));     			\
  }                                                                              		\
  inline Flags<EnumType> operator&(EnumType lhs, EnumType rhs) {						\
  	using T = std::underlying_type_t<EnumType>;										\
  	return Flags<EnumType>(static_cast<T>(lhs) & static_cast<T>(rhs));				\
  }                                                                              		\
  
  // singleton
  //─────────────────────────────────────────────────────────────────
  template <class T>
  struct Singleton {
    // get the singleton instance
    static T* instance();
    // explicitly release singleton resource
    static void release();
  private:
    Singleton() {}
    Singleton(Singleton<T>&) {}
    Singleton(Singleton<T>&&) {}
    ~Singleton() {}
    Singleton<T>& operator=(Singleton<T> const) {}
    Singleton<T>& operator=(Singleton<T>&&) {}
  private:
    static T* pinstance;
  };

  #define SINGLETON(T, CTOR)     \
  private:                       \
  friend struct Singleton<T>;		 \
  T() CTOR							         \
  public:

  template <class T>
  T* Singleton<T>::instance() {
    if (pinstance == nullptr) pinstance = new T();
    return pinstance;
  }
  template <class T>
  void Singleton<T>::release() {
    if (pinstance != nullptr) delete pinstance;
  }
  template <class T>
  T* Singleton<T>::pinstance = nullptr;

  // signal
  //─────────────────────────────────────────────────────────────────
  template <class... T>
  struct Signal {
    using slot = std::function<void(T...)>;
    std::vector<slot> m_connectedSlots;
    auto connect(slot const& new_slot) noexcept -> void {
      m_connectedSlots.push_back(new_slot); }
    template <class... U>
    auto emit(U&&... args) noexcept -> void {
      for (auto& slot_iter : m_connectedSlots) 
        slot_iter(std::forward<U>(args)...); }
  };

  // calling a functio before main starts
  //─────────────────────────────────────────────────────────────────
  auto init_extensions() noexcept -> void;

  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ platform                                                                  ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┃ Cross platform API for utils.                                             ┃
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  struct Platform {
  	static auto string_cast(const std::string& utf8str) noexcept -> std::wstring;
  	static auto string_cast(const std::wstring& utf16str) noexcept -> std::string;

    static auto open_file(std::string const& filter, std::string const& path) noexcept -> std::string;
    static auto save_file(std::string const& filter, std::string const& name) noexcept -> std::string;
  };

  struct timer {
    /** start time point record */
    std::chrono::steady_clock::time_point m_startTimePoint;
    /** previous time point record */
    std::chrono::steady_clock::time_point m_prevTimePoint;
    /** delta time between this and prev tick */
    double m_deltaTime = 0.f;

    timer();
    /** update the timer */
    auto update() noexcept -> void;
    /** get the delta time */
    auto delta_time() noexcept -> double;
    /** get the total time */
    auto total_time() noexcept -> double;
  };

  /** A world time point record. */
  struct Worldtime {
    /** get current world time point */
    static auto get() noexcept -> Worldtime;
    /** output the current time point to a string */
    auto to_string() noexcept -> std::string;

    // Approximate durations
    using days = std::chrono::duration<int, std::ratio<86400>>;       // 1 day = 86400 seconds
    using years = std::chrono::duration<int, std::ratio<31556952>>;   // 1 year ≈ 365.2425 days
    years y; days d;
    std::chrono::hours h;
    std::chrono::minutes m;
    std::chrono::seconds s;
  };

  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ configuration                                                             ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┃ An interface to load application configure.                               ┃
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  struct Configuration {
    SINGLETON(Configuration, {});
    std::string m_configFilePath;
    std::unordered_map<std::string, std::string> m_macroDefined;
    std::unordered_map<std::string, std::string> m_stringProperties;
    std::unordered_map<std::string, std::vector<std::string>> m_stringArrayProperties;

    static auto set_macro(std::string const& name, std::string const& path) noexcept -> void;
    static auto set_config_file(std::string const& path = "./runtime.config") noexcept -> void;
    static auto string_property(std::string const& name) noexcept -> std::string const&;
    static auto string_array_property(std::string const& name) noexcept -> std::vector<std::string> const&;
    static auto on_draw_gui() noexcept -> void;
  };

  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ window                                                                    ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┃ Window and input system, loaded from glfw.                                ┃
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  struct Window;
  struct Input {
    Window* m_attachedWindow;

  	enum struct CodeEnum {
  		key_unkown = -1,
  		key_space = 32,
  		key_apostrophe = 39,
  		key_comma = 44,
  		key_minus = 45,
  		key_period = 46,
  		key_slash = 47,
  		key_0 = 48,
  		key_1 = 49,
  		key_2 = 50,
  		key_3 = 51,
  		key_4 = 52,
  		key_5 = 53,
  		key_6 = 54,
  		key_7 = 55,
  		key_8 = 56,
  		key_9 = 57,
  		key_semicolon = 59,
  		key_equal = 61,
  		key_a = 65,
  		key_b = 66,
  		key_c = 67,
  		key_d = 68,
  		key_e = 69,
  		key_f = 70,
  		key_g = 71,
  		key_h = 72,
  		key_i = 73,
  		key_j = 74,
  		key_k = 75,
  		key_l = 76,
  		key_m = 77,
  		key_n = 78,
  		key_o = 79,
  		key_p = 80,
  		key_q = 81,
  		key_r = 82,
  		key_s = 83,
  		key_t = 84,
  		key_u = 85,
  		key_v = 86,
  		key_w = 87,
  		key_x = 88,
  		key_y = 89,
  		key_z = 90,
  		key_left_bracket = 91,
  		key_backslash = 92,
  		key_right_bracket = 93,
  		key_grave_accent = 96,
  		key_world_1 = 161,
  		key_world_2 = 162,
  		key_escape = 256,
  		key_enter = 257,
  		key_tab = 258,
  		key_backspace = 259,
  		key_insert = 260,
  		key_delete = 261,
  		key_right = 262,
  		key_left = 263,
  		key_down = 264,
  		key_up = 265,
  		key_page_up = 266,
  		key_page_down = 267,
  		key_home = 268,
  		key_end = 269,
  		key_caps_lock = 280,
  		key_scroll_lock = 281,
  		key_num_lock = 282,
  		key_print_screen = 283,
  		key_pause = 284,
  		key_f1 = 290,
  		key_f2 = 291,
  		key_f3 = 292,
  		key_f4 = 293,
  		key_f5 = 294,
  		key_f6 = 295,
  		key_f7 = 296,
  		key_f8 = 297,
  		key_f9 = 298,
  		key_f10 = 299,
  		key_f11 = 300,
  		key_f12 = 301,
  		key_f13 = 302,
  		key_f14 = 303,
  		key_f15 = 304,
  		key_f16 = 305,
  		key_f17 = 306,
  		key_f18 = 307,
  		key_f19 = 308,
  		key_f20 = 309,
  		key_f21 = 310,
  		key_f22 = 311,
  		key_f23 = 312,
  		key_f24 = 313,
  		key_f25 = 314,
  		key_kp_0 = 320,
  		key_kp_1 = 321,
  		key_kp_2 = 322,
  		key_kp_3 = 323,
  		key_kp_4 = 324,
  		key_kp_5 = 325,
  		key_kp_6 = 326,
  		key_kp_7 = 327,
  		key_kp_8 = 328,
  		key_kp_9 = 329,
  		key_kp_decimal = 330,
  		key_kp_divide = 331,
  		key_kp_multiply = 332,
  		key_kp_subtract = 333,
  		key_kp_add = 334,
  		key_kp_enter = 335,
  		key_kp_equal = 336,
  		key_left_shift = 340,
  		key_left_control = 341,
  		key_left_alt = 342,
  		key_left_super = 343,
  		key_right_shift = 344,
  		key_right_control = 345,
  		key_right_alt = 346,
  		key_right_super = 347,
  		key_menu = 348,
  		key_last = 348,
  		mouse_button_1 = 0,
  		mouse_button_2 = 1,
  		mouse_button_3 = 2,
  		mouse_button_4 = 3,
  		mouse_button_5 = 4,
  		mouse_button_6 = 5,
  		mouse_button_7 = 6,
  		mouse_button_8 = 7,
  		mouse_button_last = 7,
  		mouse_button_left = 0,
  		mouse_button_right = 1,
  		mouse_button_middle = 2,
  	};
  
  	auto is_key_pressed(CodeEnum keycode) noexcept -> bool;
  	auto get_mouse_x() noexcept -> float;
  	auto get_mouse_y() noexcept -> float;
  	auto enable_cursor() noexcept -> void;
  	auto disable_cursor() noexcept -> void;
  	auto get_mouse_position(int button) noexcept -> std::pair<float, float>;
  	auto get_mouse_scroll_x() noexcept -> float;
  	auto get_mouse_scroll_y() noexcept -> float;
  	auto is_mouse_button_pressed(CodeEnum button) noexcept -> bool;
  };

  struct Window {
    size_t m_width, m_height;
    std::wstring const m_name;
    bool m_shouldQuit = false;
    GLFWwindow* m_wndHandle = nullptr;
    Signal<size_t, size_t> m_onResizeSignal;
    bool m_iconified = false;
    Input m_input;

    Window(size_t width = 1280, size_t height = 720, std::wstring const& name = L"se2025");
    // Life cycle
    //─────────────────────────────────────────────────────────────────
    /** intialize created window */
    auto init() noexcept -> void;
    /** return whether the window is still runniong or has been closed */
    auto is_running() noexcept -> bool;
    /** fetch window events */
    auto fetch_events() noexcept -> int;
    /** destroy window */
    auto destroy() noexcept -> void;
    // Event-based behaviors
    //─────────────────────────────────────────────────────────────────
    /** resizie the window */
    auto resize(size_t x, size_t y) noexcept -> void;
    /** check whether the window is resized */
    auto is_resized() noexcept -> bool;
    /** check whether the window is iconified */
    auto is_iconified() noexcept -> bool { return m_iconified; }
    /** get width */
    auto get_width() const noexcept -> size_t { return m_width; }
    /** get height */
    auto get_height() const noexcept -> size_t { return m_height; }
    /** connect resize signal events */
    auto connect_resize_event(std::function<void(size_t, size_t)> const& func) noexcept -> void;
    // Fetch properties
    //─────────────────────────────────────────────────────────────────
    /** return the high DPI value */
    auto get_high_dpi() noexcept -> float;
    /** return window handle */
    auto get_handle() noexcept -> void* { return (void*)m_wndHandle; }
    /* return window framebuffer size */
    auto get_framebuffer_size(int* width, int* height) noexcept -> void;
    /** return window input */
    auto get_input() noexcept -> Input* { return &m_input; }
  };

  // ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  // ┃ profile                                                                   ┃
  // ┠───────────────────────────────────────────────────────────────────────────┨
  // ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

  struct ProfileSegment {
    std::string tag;
    uint32_t threadID;
    uint64_t start, end;
  };

  struct InstrumentationSession {
    std::string name;
    InstrumentationSession(std::string const& n) : name(n) {}
  };

  struct ProfileSession {
    SINGLETON(ProfileSession, {});

    std::ofstream outputStream;
    uint64_t profileCount = 0;
    InstrumentationSession* currentSession;

    auto write_header() noexcept -> void;
    auto write_footer() noexcept -> void;
    auto write_segment(ProfileSegment const& seg) noexcept -> void;
    void begin_session(const std::string& name, const std::string& filepath = "profile.json");
    void end_session();
  };

  struct InstrumentationTimer {
    const char* m_Name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    bool m_Stopped;

    InstrumentationTimer(const char* name);
    ~InstrumentationTimer();
    void stop();
  };
  
  #if defined(_MSC_VER)
      #define FUNC_SIG __FUNCSIG__
  #elif defined(__GNUC__) || defined(__clang__)
      #define FUNC_SIG __PRETTY_FUNCTION__
  #else
      #define FUNC_SIG __func__  // fallback, just the function name
  #endif

  #define PROFILE_SCOPE_NAME(name) se::InstrumentationTimer timer##name(#name)
  #define PROFILE_SCOPE_STOP(name) timer##name.stop();
  #define PROFILE_SCOPE(name) se::InstrumentationTimer timer##__LINE__(name)
  #define PROFILE_BEGIN_SESSION(name, filepath) \
   Singleton<se::ProfileSession>::instance()->begin_session(name, filepath);
  #define PROFILE_END_SESSION() \
   Singleton<se::ProfileSession>::instance()->end_session();
  #define PROFILE_SCOPE_FUNCTION() PROFILE_SCOPE(FUNC_SIG)
}