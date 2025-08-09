#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <se.utils.hpp>
#if defined(_WIN32)
  #include <pmmintrin.h>
  #include <xmmintrin.h>
#elif defined(__linux__)
#include <xmmintrin.h>  // for __m128 and _mm_shuffle_ps
#include <emmintrin.h>  // optional: for other intrinsics like _mm_set1_ps
#include <pmmintrin.h>  // SSE3 intrinsics
#endif

namespace se {
	inline float M_FLOAT_PI = 3.14159265358979323846f;
	inline double M_DOUBLE_PI = 3.14159265358979323846f;

	inline float M_FLOAT_INV_PI   = 1.f / M_FLOAT_PI;
	inline float M_FLOAT_INV_2Pi  = 1.f / (2 * M_FLOAT_PI);
	inline float M_FLOAT_INV_4Pi  = 1.f / (4 * M_FLOAT_PI);
	inline float M_FLOAT_PI_OVER2 = M_FLOAT_PI / 2;
	inline float M_FLOAT_PI_OVER4 = M_FLOAT_PI / 4;

	inline double M_DOUBLE_INV_PI   = 1.f / M_DOUBLE_PI;
	inline double M_DOUBLE_INV_2Pi  = 1.f / (2 * M_DOUBLE_PI);
	inline double M_DOUBLE_INV_4Pi  = 1.f / (4 * M_DOUBLE_PI);
	inline double M_DOUBLE_PI_OVER2 = M_DOUBLE_PI / 2;
	inline double M_DOUBLE_PI_OVER4 = M_DOUBLE_PI / 4;

	inline auto radians(float deg) noexcept -> float { return (M_FLOAT_PI / 180) * deg; }
	inline auto radians(double deg) noexcept -> double { return (M_DOUBLE_PI / 180) * deg; }
	inline auto degrees(float rad) -> float { return (180.f / M_FLOAT_PI) * rad; }
	inline auto degrees(double rad) -> double { return (180. / M_DOUBLE_PI) * rad; }
	inline auto safe_sqrt(float x) noexcept -> float { return std::sqrt(std::max(0.f, x)); }
  inline auto safe_asin(float x) noexcept -> float { return std::asin(std::clamp(x, -1.f, 1.f)); }
  inline auto safe_acos(float x) noexcept -> float { return std::acos(std::clamp(x, -1.f, 1.f)); }
	inline auto float_to_bits(float f) noexcept -> uint32_t { uint32_t bits = *reinterpret_cast<uint32_t*>(&f); return bits; }
	inline auto bits_to_float(uint32_t b) noexcept -> float { float f = *reinterpret_cast<float*>(&b); return f; }
	inline auto next_float_up(float v) noexcept -> float;
	inline auto next_float_down(float v) noexcept -> float;
	inline auto ctz(uint32_t value) noexcept -> uint32_t;
	inline auto clz(uint32_t value) noexcept -> uint32_t;
	inline auto log2int(uint32_t v) noexcept -> int;
	inline auto round_up_pow2(int32_t v) noexcept -> int32_t;
	inline auto count_trailing_zeros(uint32_t v) noexcept -> int;
	inline auto align_up(uint32_t a, uint32_t b) -> uint32_t;

	template <class T> constexpr inline auto sqr(T v) noexcept -> T { return v * v; }
	template <class T> inline auto clamp(T const& val, T const& min, T const& max) noexcept -> T { if (val < min) return min; else if (val > max) return max; else return val; }
	template <class T> inline auto mod(T a, T b) noexcept -> T { T result = a - (a / b) * b; return (T)((result < 0) ? result + b : result); }
	template <class T> inline auto is_power_of_2(T v) noexcept -> T { return v && !(v & (v - 1)); }
	template <class T> inline auto lerp(float t, T const& a, T const& b) noexcept -> T { return a * (1 - t) + b * t; }
	template <> inline auto mod(float a, float b) noexcept -> float { return std::fmod(a, b); }
	template <typename Predicate> auto find_interval(int size, const Predicate& pred) noexcept -> int;


	template <class T>
	struct Vector2 {
		union {
			T data[2];
			struct { T x, y; };
			struct { T r, g; };
			struct { T u, v; };
			struct { T s, t; };
		};

		Vector2() : x(0), y(0) {}
		Vector2(T const& _v) : x(_v), y(_v) {}
		Vector2(T const& _x, T const& _y) : x(_x), y(_y) {}

		template <class U> explicit operator Vector2<U>() const;
		operator T* () { return data; }
		operator const T* const() { return static_cast<const T*>(data); }
		auto operator[](size_t idx) -> T& { return data[idx]; }
		auto operator[](size_t idx) const -> T const& { return data[idx]; }
		auto operator-() const->Vector2<T>;
		auto operator*(T s) const->Vector2<T>;
		auto operator/(T s) const->Vector2<T>;
		auto operator+(Vector2<T> const& v) const->Vector2<T>;
		auto operator-(Vector2<T> const& v) const->Vector2<T>;
		auto operator*(Vector2<T> const& v) const->Vector2<T>;
		auto operator/(Vector2<T> const& v) const->Vector2<T>;
		auto operator==(Vector2<T> const& v) const -> bool;
		auto operator!=(Vector2<T> const& v) const -> bool;
		auto operator*=(T s)->Vector2<T>&;
		auto operator/=(T s)->Vector2<T>&;
		auto operator+=(Vector2<T> const& v)->Vector2<T>&;
		auto operator-=(Vector2<T> const& v)->Vector2<T>&;
		auto operator*=(Vector2<T> const& v)->Vector2<T>&;
		auto operator/=(Vector2<T> const& v)->Vector2<T>&;

    auto length() const -> float;
    auto length_squared() const -> float;
    auto to_string() const -> std::string;
	};
  using bvec2 = Vector2<bool>;
  using vec2 = Vector2<float>;
  using ivec2 = Vector2<int32_t>;
  using uvec2 = Vector2<uint32_t>;
  using dvec2 = Vector2<double>;
  using svec2 = Vector2<size_t>;


  template <class T>
  struct Vector3 {
    union {
      T data[3];
      struct { T x, y, z; };
      struct { Vector2<T> xy; T _z; };
      struct { T r, g, b; };
      struct { T s, t, p; };
    };

    Vector3() : x(0), y(0), z(0) {}
    Vector3(T const& _v) : x(_v), y(_v), z(_v) {}
    Vector3(Vector2<T> const& _v) : x(_v.x), y(_v.y), z(0) {}
    Vector3(Vector2<T> const& _v, T const& _z) : x(_v.x), y(_v.y), z(_z) {}
    Vector3(T const& _x, T const& _y, T const& _z = 0) : x(_x), y(_y), z(_z) {}

    template <class U> explicit operator Vector3<U>() const;
    explicit operator Vector2<T>() const { return Vector2<T>{x, y}; }
    operator T* () { return data; }
    operator const T* const() { return static_cast<const T*>(data); }
    auto operator[](size_t idx) -> T& { return data[idx]; }
    auto operator[](size_t idx) const -> T const& { return data[idx]; }
    auto operator-() const->Vector3<T>;
    auto operator*(T s) const->Vector3<T>;
    auto operator/(T s) const->Vector3<T>;
    auto operator+(Vector3<T> const& v) const->Vector3<T>;
    auto operator-(Vector3<T> const& v) const->Vector3<T>;
    auto operator*(Vector3<T> const& v) const->Vector3<T>;
    auto operator/(Vector3<T> const& v) const->Vector3<T>;
    auto operator==(Vector3<T> const& v) const -> bool;
    auto operator!=(Vector3<T> const& v) const -> bool;
    auto operator*=(T s)->Vector3<T>&;
    auto operator/=(T s)->Vector3<T>&;
    auto operator+=(Vector3<T> const& v)->Vector3<T>&;
    auto operator-=(Vector3<T> const& v)->Vector3<T>&;
    auto operator*=(Vector3<T> const& v)->Vector3<T>&;

    auto to_string() const->std::string;
    auto length() const -> float;
    auto length_squared() const -> float;
    auto at(size_t idx) -> T& { return data[idx]; }
    auto at(size_t idx) const -> T { return data[idx]; }
  };
  using bvec3 = Vector3<bool>;
  using vec3 = Vector3<float>;
  using dvec3 = Vector3<double>;
  using ivec3 = Vector3<int32_t>;
  using uvec3 = Vector3<uint32_t>;


  template <class T>
  struct Vector4 {
    #ifdef _WIN32
      __declspec(align(16)) union {
    #elif defined(__linux__)
      union __attribute__((aligned(16))) {
    #endif
      T data[4];
      struct { T x, y, z, w; };
      struct { T r, g, b, a; };
      struct { T s, t, p, q; };
    };

    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(T const& _v) : x(_v), y(_v), z(_v), w(_v) {}
    Vector4(Vector2<T> const& _v) : x(_v.x), y(_v.y), z(0), w(0) {}
    Vector4(Vector2<T> const& _v, T _z, T _w) : x(_v.x), y(_v.y), z(_z), w(_w) {}
    Vector4(Vector2<T> const& _v1, Vector2<T> const& _v2)
      : x(_v1.x), y(_v1.y), z(_v2.x), w(_v2.y) {}
    Vector4(Vector3<T> const& _v) : x(_v.x), y(_v.y), z(_v.z), w(0) {}
    Vector4(Vector3<T> const& _v, T _w) : x(_v.x), y(_v.y), z(_v.z), w(_w) {}
    Vector4(T const& _x, T const& _y, T const& _z = 0, T const& _w = 0)
      : x(_x), y(_y), z(_z), w(_w) {}

    template <class U> explicit operator Vector4<U>() const;
    explicit operator Vector3<T>() { return Vector3<T>{x, y, z}; }
    operator T* () { return data; }
    operator const T* const() { return static_cast<const T*>(data); }
    auto operator[](size_t idx) -> T& { return data[idx]; }
    auto operator[](size_t idx) const -> T const& { return data[idx]; }
    auto operator-() const->Vector4<T>;
    auto operator*(T s) const->Vector4<T>;
    auto operator/(T s) const->Vector4<T>;
    auto operator*=(T s)->Vector4<T>&;
    auto operator/=(T s)->Vector4<T>&;
    auto operator+(Vector4<T> const& v) const->Vector4<T>;
    auto operator-(Vector4<T> const& v) const->Vector4<T>;
    auto operator*(Vector4<T> const& v) const->Vector4<T>;
    auto operator+=(Vector4<T> const& v)->Vector4<T>&;
    auto operator-=(Vector4<T> const& v)->Vector4<T>&;
    auto operator*=(Vector4<T> const& v)->Vector4<T>&;
    auto operator==(Vector4<T> const& v) const -> bool;
    auto operator!=(Vector4<T> const& v) const -> bool;

    auto to_string() const->std::string;
    auto length() const -> float;
    auto length_squared() const -> float;
    inline auto xyz() noexcept -> Vector3<T> { return Vector3<T>(x, y, z); }
    inline auto xzy() noexcept -> Vector3<T> { return Vector3<T>{x, z, y}; }
    inline auto xyw() noexcept -> Vector3<T> { return Vector3<T>{x, y, w}; }
    inline auto xwy() noexcept -> Vector3<T> { return Vector3<T>{x, w, y}; }
    inline auto xzw() noexcept -> Vector3<T> { return Vector3<T>{x, z, w}; }
    inline auto xwz() noexcept -> Vector3<T> { return Vector3<T>{x, w, z}; }
  };
  using bvec4 = Vector4<bool>;
  using vec4 = Vector4<float>;
  using ivec4 = Vector4<int32_t>;
  using uvec4 = Vector4<uint32_t>;
  using dvec4 = Vector4<double>;


  template <class T>
  struct Matrix2x2 {
    union {
      T data[2][2];
    };
  };
  using mat2 = Matrix2x2<float>;
  using imat2 = Matrix2x2<int32_t>;
  using umat2 = Matrix2x2<uint32_t>;


  template <class T>
  struct Matrix3x3 {
    union {
      T data[3][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
    };

    Matrix3x3() = default;
    Matrix3x3(T const mat[3][3]);
    Matrix3x3(Vector3<T> x, Vector3<T> y, Vector3<T> z);
    Matrix3x3(T t00, T t01, T t02, T t10, T t11, T t12, T t20, T t21, T t22);

    auto row(int i) const noexcept -> Vector3<T>;
    auto col(int i) const noexcept -> Vector3<T>;

  };
  using mat3 = Matrix3x3<float>;
  using dmat3 = Matrix3x3<double>;
  using imat3 = Matrix3x3<int32_t>;
  using umat3 = Matrix3x3<uint32_t>;


  template <class T>
  struct Matrix4x4 {
    alignas(16) T data[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    };

    Matrix4x4() = default;
    Matrix4x4(T s);
    Matrix4x4(T const mat[4][4]);
    Matrix4x4(Vector4<T> const& a, Vector4<T> const& b, Vector4<T> const& c,
      Vector4<T> const& d);
    Matrix4x4(T t00, T t01, T t02, T t03, T t10, T t11, T t12, T t13, T t20,
      T t21, T t22, T t23, T t30, T t31, T t32, T t33);

    operator Matrix3x3<T>() const;
    auto operator==(Matrix4x4<T> const& t) const -> bool;
    auto operator!=(Matrix4x4<T> const& t) const -> bool;
    auto operator-() const->Matrix4x4<T>;
    auto operator+(Matrix4x4<T> const& t) const->Matrix4x4<T>;
    auto operator-(Matrix4x4<T> const& t) const->Matrix4x4<T>;

    auto to_string() const noexcept -> std::string;
    auto row(int i) const noexcept -> Vector4<T>;
    auto col(int i) const noexcept -> Vector4<T>;
    auto set_row(int i, Vector4<T> const& x) noexcept -> void;

    static inline auto translate(Vector3<T> const& delta) noexcept -> Matrix4x4<T>;
    static inline auto scale(float x, float y, float z) noexcept -> Matrix4x4<T>;
    static inline auto scale(Vector3<T> const& scale) noexcept -> Matrix4x4<T>;
    static inline auto rotateX(float theta) noexcept -> Matrix4x4<T>;
    static inline auto rotateY(float theta) noexcept -> Matrix4x4<T>;
    static inline auto rotateZ(float theta) noexcept -> Matrix4x4<T>;
    static inline auto rotate(float theta, vec3 const& axis) noexcept -> Matrix4x4<T>;
  };
  using mat4 = Matrix4x4<float>;
  using dmat4 = Matrix4x4<double>;
  using imat4 = Matrix4x4<int32_t>;
  using umat4 = Matrix4x4<uint32_t>;


  template <class T>
  struct Point2 :public Vector2<T>
  {
    Point2(Vector2<T> const& v = { 0,0 })
      :Vector2<T>(v) {}

    Point2(T const& x, T const& y)
      :Vector2<T>(x, y) {}

    template<class U>
    Point2(Vector2<U> const& v = { 0,0,0 })
      : Vector2<T>((T)v.x, (T)v.y) {}

    template <typename U>
    explicit Point2(Point2<U> const& p)
      :Vector2<T>((T)p.x, (T)p.y) {}

    auto operator+(Vector2<T> const& a) const -> Point2 { return Point2{ this->x + a.x,this->y + a.y }; }
    auto operator-(Vector2<T> const& a) const -> Point2 { return Point2{ this->x - a.x,this->y - a.y }; }
    template <typename U>
    explicit operator Point2<U>() const { return Point2<U>((U)this->x, (U)this->y); }
  };
  using point2 = Point2<float>;
  using ipoint2 = Point2<int32_t>;
  using upoint2 = Point2<uint32_t>;


  template <class T>
  struct Point3 :public Vector3<T> {
    Point3(T const& _x, T const& _y, T const& _z = 0)
      :Vector3<T>(_x, _y, _z) {}

    Point3(Vector3<T> const& v = { 0,0,0 })
      :Vector3<T>(v) {}

    template<class U>
    Point3(Vector3<U> const& v = { 0,0,0 })
      : Vector3<T>((T)v.x, (T)v.y, (T)v.z) {}

    template <typename U>
    explicit Point3(Point3<U> const& p)
      :Vector3<T>((T)p.x, (T)p.y, (T)p.z) {}

    template <typename U>
    explicit operator Vector3<U>() const { return Vector3<U>(this->x, this->y, this->z); }
  };
  using point3 = Point3<float>;
  using ipoint3 = Point3<int32_t>;
  using upoint3 = Point3<uint32_t>;


  template <class T>
  struct Normal3 :public Vector3<T> {
    Normal3(T const& _x, T const& _y, T const& _z = 0)
      :Vector3<T>(_x, _y, _z) {}

    Normal3(Vector3<T> const& v = { 0,0,0 })
      :Vector3<T>(v) {}

    template <typename U>
    explicit Normal3(Normal3<U> const& p)
      :Vector3<T>((T)p.x, (T)p.y, (T)p.z) {}

    auto operator*=(T s)->Normal3<T>&;
    auto operator-() const->Normal3<T>;

    template <typename U>
    explicit operator Vector3<U>() const { return Vector3<U>(this->x, this->y, this->z); }
  };
  using normal3 = Normal3<float>;
  using inormal3 = Normal3<int32_t>;
  using unormal3 = Normal3<uint32_t>;


  struct ray3 {
    point3 o;
    vec3 d;
    mutable float tMax;

    ray3() : tMax(std::numeric_limits<float>::infinity()) {}
    ray3(point3 const& o, vec3 const& d,
      float tMax = std::numeric_limits<float>::infinity())
      : o(o), d(d), tMax(tMax) {}
    auto operator()(float t) const->point3 { return o + d * t; }
  };

  template <class T>
  struct Bounds2 {
    Point2<T> pMin;
    Point2<T> pMax;

    Bounds2();
    Bounds2(Point2<T> const& p);
    Bounds2(Point2<T> const& p1, Point2<T> const& p2);

    auto operator[](uint32_t i) const->Point2<T> const&;
    auto operator[](uint32_t i)->Point2<T>&;

    template <typename U>
    explicit operator Bounds2<U>() const { return Bounds2<U>((Point2<U>)this->pMin, (Point2<U>)this->pMax); }
    auto corner(uint32_t c) const->Point2<T>;
    auto diagonal() const->Vector2<T>;
    auto surfaceArea() const->T;
    auto maximumExtent() const->uint32_t;
    auto lerp(Point2<T> const& t) const->Point2<T>;
    auto offset(Point2<T> const& p) const->Vector2<T>;
    auto boundingCircle(Point2<T>* center, float* radius) const -> void;
  };
  using bounds2 = Bounds2<float>;
  using ibounds2 = Bounds2<int32_t>;


  template <class T>
  struct Bounds3 {
    Point3<T> pMin;
    Point3<T> pMax;

    Bounds3();
    Bounds3(Point3<T> const& p);
    Bounds3(Point3<T> const& p1, Point3<T> const& p2);

    auto operator[](uint32_t i) const->Point3<T> const&;
    auto operator[](uint32_t i)->Point3<T>&;

    auto corner(uint32_t c) const->Point3<T>;
    auto diagonal() const->Vector3<T>;
    auto surfaceArea() const->T;
    auto volume() const->T;
    auto maximumExtent() const->uint32_t;
    auto lerp(Point3<T> const& t) const->Point3<T>;
    auto offset(Point3<T> const& p) const->Vector3<T>;
    auto bounding_sphere(Point3<T>* center, float* radius) const -> void;
    auto intersectP(ray3 const& ray, float* hitt0, float* hitt1) const -> bool;
    auto intersectP(ray3 const& ray, vec3 const& invDir, ivec3 const& dirIsNeg) const -> bool;
  };
  using bounds3 = Bounds3<float>;
  using ibounds3 = Bounds3<int32_t>;

  template <class T>
  auto union_bounds(Bounds3<T> const& a, Bounds3<T> const& b) noexcept -> Bounds3<T> {
    Bounds3<T> ret;
    ret.pMin = min(b.pMin, a.pMin);
    ret.pMax = max(b.pMax, a.pMax);
    return ret;
  }

  struct half {
    short hdata;
    half() = default;
    half(float f);
    float to_float() const;
  };

  struct Quaternion {
    union {
      struct { vec3 v; float s; } vs;
      float data[4];
      struct { float x, y, z, w; };
    };

    Quaternion() : vs({vec3(0.f), 1.f}) {}
    Quaternion(float x, float y, float z, float w) : vs({vec3(x, y, z), w}) {}
    Quaternion(vec3 const& v, float s) : vs({v, s}) {}
    Quaternion(vec3 const& eulerAngle);
    Quaternion(mat3 const& m);
    Quaternion(mat4 const& m);

    auto toMat3() const noexcept -> mat3;
    auto toMat4() const noexcept -> mat4;
    auto length() const -> float;
    auto length_squared() const -> float;
    auto conjugate() noexcept -> Quaternion;
    auto reciprocal() noexcept -> Quaternion;

    auto operator/(float s) const->Quaternion;
    auto operator+(Quaternion const& q2) const->Quaternion;
    auto operator*(Quaternion const& q2) const->Quaternion;
    auto operator*(vec3 const& v) const->vec3;
    auto operator+=(Quaternion const& q)->Quaternion&;
    auto operator-() const->Quaternion;
  };

  struct Transform {
    mat4 m;
    mat4 mInv;

    Transform() = default;
    Transform(float const mat[4][4]);
    Transform(mat4 const& m);
    Transform(mat4 const& m, mat4 const& mInverse);
    Transform(Quaternion const& q);

    auto is_identity() const noexcept -> bool;
    auto has_scale() const noexcept -> bool;
    auto swaps_handness() const noexcept -> bool;

    auto operator==(Transform const& t) const -> bool;
    auto operator!=(Transform const& t) const -> bool;
    auto operator*(point3 const& p) const->point3;
    auto operator*(vec3 const& v) const->vec3;
    auto operator*(normal3 const& n) const->normal3;
    auto operator*(ray3 const& s) const->ray3;
    auto operator*(bounds3 const& b) const->bounds3;
    auto operator*(Transform const& t2) const->Transform;
    auto operator()(point3 const& p, vec3& absError) const->point3;
    auto operator()(point3 const& p, vec3 const& pError, vec3& tError) const ->point3;
    auto operator()(vec3 const& v) const->vec3;
    auto operator()(vec3 const& v, vec3& absError) const->vec3;
    auto operator()(vec3 const& v, vec3 const& pError, vec3& tError) const ->vec3;
    auto operator()(ray3 const& r, vec3& oError, vec3& dError) const->ray3;
  };


  // Spherical theta-phi
  auto spherical_direction(float sinTheta, float cosTheta, float phi) noexcept -> vec3;
  auto spherical_direction(float sinTheta, float cosTheta, float phi, vec3 const& x, vec3 const& y, vec3 const& z) noexcept -> vec3;
  auto spherical_theta(vec3 const& v) noexcept -> float;
  auto spherical_phi(vec3 const& v) noexcept -> float;

  // Quaternion arithmetic
  inline auto dot(Quaternion const& q1, Quaternion const& q2) noexcept -> float { return dot(q1.vs.v, q2.vs.v) + q1.vs.s * q2.vs.s; }
  inline auto normalize(Quaternion const& q) noexcept -> Quaternion { return q / std::sqrt(dot(q, q)); }
  inline auto slerp(float t, Quaternion const& q1, Quaternion const& q2) noexcept -> Quaternion;
  inline auto offset_ray_origin(point3 const& p, vec3 const& pError, normal3 const& n, vec3 const& w) noexcept -> point3;

  // Quaternion operators
  inline auto operator*(float s, Quaternion const& q)->Quaternion { return Quaternion{ s * q.x, s * q.y, s * q.z, s * q.w }; }
  inline auto operator*(Quaternion const& q, float s)->Quaternion { return Quaternion{ s * q.x, s * q.y, s * q.z, s * q.w }; }
  inline auto operator-(Quaternion const& q1, Quaternion const& q2) -> Quaternion { return Quaternion{ q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w }; }

  // Transform arithmetic
  auto inverse(Transform const& t) noexcept -> Transform;
  auto transpose(Transform const& t) noexcept -> Transform;
  auto translate(vec3 const& delta) noexcept -> Transform;
  auto scale(float x, float y, float z) noexcept -> Transform;
  auto rotate_x(float theta) noexcept -> Transform;
  auto rotate_y(float theta) noexcept -> Transform;
  auto rotate_z(float theta) noexcept -> Transform;
  auto rotate(float theta, vec3 const& axis) noexcept -> Transform;
  auto look_at(point3 const& pos, point3 const& look, vec3 const& up) noexcept -> Transform;
  auto orthographic(float zNear, float zFar) noexcept -> Transform;
  auto ortho(float left, float right, float bottom, float top, float zNear, float zFar) noexcept -> Transform;
  auto perspective(float fov, float n, float f) noexcept -> Transform;
  auto perspective(float fov, float aspect, float n, float f) noexcept -> Transform;
  auto angle_between(vec3 v1, vec3 v2) noexcept -> float;
  auto euler_angle_to_rotation_matrix(se::vec3 e) noexcept -> se::mat3;
  auto euler_angle_to_quaternion(se::vec3 e) noexcept -> se::Quaternion;
  auto euler_angle_degree_to_rotation_matrix(se::vec3 e) noexcept -> se::mat3;
  auto euler_angle_degree_to_quaternion(se::vec3 e) noexcept -> se::Quaternion;
  auto rotation_matrix_to_euler_angles(se::mat3 R) noexcept -> se::vec3;
  auto decompose(se::mat4 const& m, se::vec3* t, se::Quaternion* quat, se::vec3* s) noexcept -> void;
  auto decompose(se::mat4 const& m, se::vec3* t, se::vec3* r, se::vec3* s) noexcept -> void;


  enum struct WrapMode {
    CLAMP,
    REPEAT,
    PINGPOMG,
  };

  struct KeyFrame {
    float time;
    float value;
    float inTangent;
    float outTangent;
  };

  struct AnimationCurve {
    WrapMode m_preWrapMode = WrapMode::CLAMP;
    WrapMode m_postWrapMode = WrapMode::CLAMP;
    float m_errorTolerence = 0.00001f;
    std::vector<KeyFrame> m_keyFrames;
    struct Point { float time; float value; };

    AnimationCurve() = default;
    AnimationCurve(std::initializer_list<KeyFrame> const& initializer_list)
      : m_keyFrames(initializer_list) { sort_all_key_frames(); }
    auto evaluate(float time) noexcept -> float;
    auto sort_all_key_frames() noexcept -> void;
    auto evaluate(KeyFrame const& keyframe0, KeyFrame const& keyframe1, float t) noexcept -> Point;
  };

  #include "../source/se.math.vec.hpp"
  #include "../source/se.math.mat.hpp"
  #include "../source/se.math.misc.hpp"

  struct Random {
    // Global generator and distribution
    static std::mt19937& get_rng() {
      static std::random_device rd;
      static std::mt19937 gen(rd());
      return gen;
    }

    static int uniform_int(int min = 0, int max = 100) {
      static std::uniform_int_distribution<> distrib;
      distrib.param(std::uniform_int_distribution<>::param_type(min, max));
      return distrib(get_rng());
    }
  };
}