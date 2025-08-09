
template <class T> template <class U> Vector2<T>::operator Vector2<U>() const { return Vector2<U>(x, y); }

template <class S, class T> auto operator*(S s, Vector2<T> const& v) noexcept -> Vector2<T> { return Vector2<T>{v.x* s, v.y* s}; }

template <class T> inline auto abs(Vector2<T> const& v) noexcept -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; ++i) {
    result.data[i] = std::abs(v.data[i]);
  }
  return result;
}

template <class T>
inline auto floor(Vector2<T> const& v) noexcept -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; ++i) {
    result.data[i] = std::floor(v.data[i]);
  }
  return result;
}

template <class T>
inline auto ceil(Vector2<T> const& v) noexcept -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; ++i) {
    result.data[i] = std::ceil(v.data[i]);
  }
  return result;
}

template <class T>
inline auto dot(Vector2<T> const& x, Vector2<T> const& y) noexcept -> T {
  T result = 0;
  for (size_t i = 0; i < 2; ++i) {
    result += x[i] * y[i];
  }
  return result;
}

template <class T>
inline auto absDot(Vector2<T> const& x, Vector2<T> const& y) noexcept -> T {
  return std::abs(dot(x, y));
}

template <class T>
inline auto cross(Vector2<T> const& x, Vector2<T> const& y) noexcept -> T {
  return x[0] * y[1] - x[1] * y[0];
}

template <class T>
inline auto normalize(Vector2<T> const& v) noexcept -> Vector2<T> {
  return v / v.length();
}

template <class T>
inline auto length(Vector2<T> const& v) noexcept -> T {
  return v.length();
}

inline auto sign(float x) noexcept -> float {
  if (x > 0) return 1.f;
  else if (x < 0) return -1.f;
  else return 0.f;
}

template <class T>
inline auto sign(Vector2<T> const& v) noexcept -> Vector2<float> {
  return Vector2<float> { sign(v.x), sign(v.y) };
}

template <class T>
inline auto equal(Vector2<T> const& v1, Vector2<T> const& v2) noexcept -> bool {
  return v1.x == v2.x && v1.y == v2.y;
}

template <class T>
inline auto clamp_vec2(Vector2<T> const& x, Vector2<T> const& min, Vector2<T> const& max) noexcept -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; ++i) {
    result[i] = std::clamp(x[i], min[i], max[i]);
  }
  return result;
}

template <class T>
inline auto minComponent(Vector2<T> const& v) noexcept -> T {
  return std::min(v.x, v.y);
}

template <class T>
inline auto maxComponent(Vector2<T> const& v) noexcept -> T {
  return std::max(v.x, v.y);
}

template <class T>
inline auto maxDimension(Vector2<T> const& v) noexcept -> size_t {
  return (v.x > v.y) ? 0 : 1;
}

template <class T>
inline auto minDimension(Vector2<T> const& v) noexcept -> size_t {
  return (v.x < v.y) ? 0 : 1;
}

template <class T>
inline auto max(Vector2<T> const& x, Vector2<T> const& y) noexcept
-> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; ++i) {
    result[i] = std::max(x[i], y[i]);
  }
  return result;
}

template <class T>
inline auto min(Vector2<T> const& x, Vector2<T> const& y) noexcept
-> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; ++i) {
    result[i] = std::min(x[i], y[i]);
  }
  return result;
}

template <class T>
inline auto permute(Vector2<T> const& v, size_t x, size_t y) noexcept
-> Vector2<T> {
  return Vector2<T>(v[x], v[y]);
}

template <class T>
inline auto distance(Vector2<T> const& p1, Vector2<T> const& p2) noexcept
-> float {
  return (p1 - p2).length();
}

template <class T>
inline auto distanceSquared(Vector2<T> const& p1, Vector2<T> const& p2) noexcept
-> float {
  return (p1 - p2).lengthSquared();
}

template <class T>
inline auto operator*(T s, Vector2<T>& v) -> Vector2<T> {
  return v * s;
}

template <class T>
inline auto lerp(float t, Vector2<T> const& x, Vector2<T>& y) noexcept
-> Vector2<T> {
  return (1 - t) * x + t * y;
}


template <class T>
auto Vector2<T>::length_squared() const -> float {
  return x * x + y * y;
}

template <class T>
auto Vector2<T>::length() const -> float {
  return std::sqrt(length_squared());
}

template <class T>
auto Vector2<T>::operator-() const -> Vector2<T> {
  return Vector2<T>{-x, -y};
}

template <class T>
auto Vector2<T>::to_string() const->std::string {
  return fmt::format("Vec2[{}, {}]", std::to_string(data[0]), std::to_string(data[1]));
}

template <class T>
inline auto select(Vector2<bool> p, Vector2<T> const& a, Vector2<T> const& b) noexcept -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; ++i) {
    result.data[i] = p.data[i] ? a.data[i] : b.data[i];
  }
  return result;
}

template <class T>
auto Vector2<T>::operator*(T s) const -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; i++) {
    result.data[i] = data[i] * s;
  }
  return result;
}

template <class T>
auto Vector2<T>::operator/(T s) const -> Vector2<T> {
  float inv = 1.f / s;
  Vector2<T> result;
  for (size_t i = 0; i < 2; i++) {
    result.data[i] = data[i] * inv;
  }
  return result;
}

template <class T>
auto Vector2<T>::operator*=(T s) -> Vector2<T>& {
  for (size_t i = 0; i < 2; i++) {
    data[i] *= s;
  }
  return *this;
}

template <class T>
auto Vector2<T>::operator/=(T s) -> Vector2<T>& {
  float inv = 1.f / s;
  for (size_t i = 0; i < 2; i++) {
    data[i] *= inv;
  }
  return *this;
}

template <class T>
auto Vector2<T>::operator+(Vector2<T> const& v) const -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; i++) {
    result.data[i] = data[i] + v.data[i];
  }
  return result;
}

template <class T>
auto Vector2<T>::operator-(Vector2<T> const& v) const -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; i++) {
    result.data[i] = data[i] - v.data[i];
  }
  return result;
}

template <class T>
auto Vector2<T>::operator*(Vector2<T> const& v) const -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; i++) {
    result.data[i] = data[i] * v.data[i];
  }
  return result;
}

template <class T>
auto Vector2<T>::operator/(Vector2<T> const& v) const -> Vector2<T> {
  Vector2<T> result;
  for (size_t i = 0; i < 2; i++) {
    result.data[i] = data[i] / v[i];
  }
  return result;
}

template <class T>
auto Vector2<T>::operator+=(Vector2<T> const& v) -> Vector2<T>& {
  for (size_t i = 0; i < 2; i++) {
    data[i] += v.data[i];
  }
  return *this;
}

template <class T>
auto Vector2<T>::operator-=(Vector2<T> const& v) -> Vector2<T>& {
  for (size_t i = 0; i < 2; i++) {
    data[i] -= v.data[i];
  }
  return *this;
}

template <class T>
auto Vector2<T>::operator*=(Vector2<T> const& v) -> Vector2<T>& {
  for (size_t i = 0; i < 2; i++) {
    data[i] *= v.data[i];
  }
  return *this;
}

template <class T>
auto Vector2<T>::operator/=(Vector2<T> const& v) -> Vector2<T>& {
  for (size_t i = 0; i < 2; i++) {
    data[i] /= v.data[i];
  }
  return *this;
}

template <class T>
auto Vector2<T>::operator==(Vector2<T> const& v) const -> bool {
  return (x == v.x) && (y == v.y);
}

template <class T>
auto Vector2<T>::operator!=(Vector2<T> const& v) const -> bool {
  return !(*this == v);
}

template <class T>
auto operator>=(Vector2<T> const& v1, Vector2<T> const& v2) -> Vector2<bool> {
  Vector2<bool> res;
  for (size_t i = 0; i < 2; i++) {
    res.data[i] = v1.data[i] >= v2.data[i];
  }
  return res;
}
template <class T>
auto operator>(Vector2<T> const& v1, Vector2<T> const& v2) -> Vector2<bool> {
  Vector2<bool> res;
  for (size_t i = 0; i < 2; i++) {
    res.data[i] = v1.data[i] > v2.data[i];
  }
  return res;
}
template <class T>
auto operator<=(Vector2<T> const& v1, Vector2<T> const& v2) -> Vector2<bool> {
  Vector2<bool> res;
  for (size_t i = 0; i < 2; i++) {
    res.data[i] = v1.data[i] <= v2.data[i];
  }
  return res;
}
template <class T>
auto operator<(Vector2<T> const& v1, Vector2<T> const& v2) -> Vector2<bool> {
  Vector2<bool> res;
  for (size_t i = 0; i < 2; i++) {
    res.data[i] = v1.data[i] < v2.data[i];
  }
  return res;
}
template <class T>
auto operator==(Vector2<T> const& v1, Vector2<T> const& v2) -> Vector2<bool> {
  Vector2<bool> res;
  for (size_t i = 0; i < 2; i++) {
    res.data[i] = v1.data[i] == v2.data[i];
  }
  return res;
}
template <class T>
auto operator!=(Vector2<T> const& v1, Vector2<T> const& v2) -> Vector2<bool> {
  Vector2<bool> res;
  for (size_t i = 0; i < 2; i++) {
    res.data[i] = v1.data[i] != v2.data[i];
  }
  return res;
}

template <class T>
template <class U>
Vector3<T>::operator Vector3<U>() const {
  return Vector3<U>(x, y, z);
}

template <class S, class T>
auto operator*(S s, Vector3<T> const& v) noexcept -> Vector3<T> {
  return Vector3<T>{T(v.x* s), T(v.y* s), T(v.z* s)};
}

template <class T>
inline auto abs(Vector3<T> const& v) noexcept -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; ++i) {
    result.data[i] = std::abs(v.data[i]);
  }
  return result;
}

template <class T>
inline auto floor(Vector3<T> const& v) noexcept -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; ++i) {
    result.data[i] = std::floor(v.data[i]);
  }
  return result;
}

template <class T>
inline auto ceil(Vector3<T> const& v) noexcept -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; ++i) {
    result.data[i] = std::ceil(v.data[i]);
  }
  return result;
}

template <class T>
inline auto dot(Vector3<T> const& x, Vector3<T> const& y) noexcept -> T {
  T result = 0;
  for (size_t i = 0; i < 3; ++i) {
    result += x[i] * y[i];
  }
  return result;
}

template <class T>
inline auto absDot(Vector3<T> const& x, Vector3<T> const& y) noexcept -> T {
  return std::abs(dot(x, y));
}

template <class T>
inline auto cross(Vector3<T> const& x, Vector3<T> const& y) noexcept
-> Vector3<T> {
  return Vector3<T>((x[1] * y[2]) - (x[2] * y[1]),
    (x[2] * y[0]) - (x[0] * y[2]),
    (x[0] * y[1]) - (x[1] * y[0]));
}

template <class T>
inline auto normalize(Vector3<T> const& v) noexcept -> Vector3<T> {
  return v / v.length();
}

template <class T>
inline auto sign(Vector3<T> const& v) noexcept -> Vector3<float> {
  return Vector3<float>{sign(v.x), sign(v.y), sign(v.z)};
}

template <class T>
inline auto equal(Vector3<T> const& v1, Vector3<T> const& v2) noexcept -> bool {
  return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

template <class T>
inline auto minComponent(Vector3<T> const& v) noexcept -> T {
  return std::min(std::min(v.x, v.y), v.z);
}

template <class T>
inline auto maxComponent(Vector3<T> const& v) noexcept -> T {
  return std::max(std::max(v.x, v.y), v.z);
}

template <class T>
inline auto maxDimension(Vector3<T> const& v) noexcept -> size_t {
  return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
}

template <typename T>
inline auto permute(const Vector3<T>& p, int x, int y, int z) -> Vector3<T> {
  return Vector3<T>(p.data[x], p.data[y], p.data[z]);
}

template <class T>
inline auto minDimension(Vector3<T> const& v) noexcept -> size_t {
  return (v.x < v.y) ? ((v.x < v.z) ? 0 : 2) : ((v.y < v.z) ? 1 : 2);
}

template <class T>
inline auto max(Vector3<T> const& x, Vector3<T> const& y) noexcept
-> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; ++i) {
    result[i] = std::max(x[i], y[i]);
  }
  return result;
}

template <class T>
inline auto min(Vector3<T> const& x, Vector3<T> const& y) noexcept
-> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; ++i) {
    result[i] = std::min(x[i], y[i]);
  }
  return result;
}

template <class T>
inline auto permute(Vector3<T> const& v, size_t x, size_t y, size_t z) noexcept
-> Vector3<T> {
  return Vector3<T>(v[x], v[y], v[z]);
}

template <class T>
inline auto distance(Vector3<T> const& p1, Vector3<T> const& p2) noexcept
-> float {
  return (p1 - p2).length();
}

template <class T>
inline auto distance_squared(Vector3<T> const& p1, Vector3<T> const& p2) noexcept
-> float {
  return (p1 - p2).length_squared();
}

template <class T>
inline auto operator*(T s, Vector3<T>& v) -> Vector3<T> {
  return v * s;
}

template <class T>
inline auto lerp(float t, Vector3<T> const& x, Vector3<T>& y) noexcept
-> Vector3<T> {
  return (1 - t) * x + t * y;
}

template <class T>
inline auto faceForward(Vector3<T> const& n, Vector3<T> const& v) noexcept
-> Vector3<T> {
  return (dot(n, v) < 0.0f) ? -n : n;
}

template <class T>
inline auto length(Vector3<T> const& x) noexcept -> T {
  return std::sqrt(x.length_squared());
}

template <class T>
inline auto cos(Vector3<T> const& v) noexcept -> Vector3<T> {
  return Vector3<T>(std::cos(v.x), std::cos(v.y), std::cos(v.z));
}

template <class T>
inline auto sin(Vector3<T> const& v) noexcept -> Vector3<T> {
  return Vector3<T>(std::sin(v.x), std::sin(v.y), std::sin(v.z));
}

template <class T>
auto Vector3<T>::length_squared() const -> float {
  return x * x + y * y + z * z;
}

template <class T>
auto Vector3<T>::to_string() const->std::string {
  return fmt::format("Vec3[{}, {}, {}]", std::to_string(data[0]), std::to_string(data[1]), std::to_string(data[2]));
}

template <class T>
auto Vector3<T>::length() const -> float {
  return std::sqrt(length_squared());
}

template <class T>
auto Vector3<T>::operator-() const -> Vector3<T> {
  return Vector3<T>{-x, -y, -z};
}

template <class T>
auto Vector3<T>::operator*(T s) const -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; i++) {
    result.data[i] = data[i] * s;
  }
  return result;
}

template <class T>
auto Vector3<T>::operator/(T s) const -> Vector3<T> {
  float inv = 1.f / s;
  Vector3<T> result;
  for (size_t i = 0; i < 3; i++) {
    result.data[i] = data[i] * inv;
  }
  return result;
}

template <class T>
auto Vector3<T>::operator*=(T s) -> Vector3<T>& {
  for (size_t i = 0; i < 3; i++) {
    data[i] *= s;
  }
  return *this;
}

template <class T>
auto Vector3<T>::operator/=(T s) -> Vector3<T>& {
  float inv = 1.f / s;
  for (size_t i = 0; i < 3; i++) {
    data[i] *= inv;
  }
  return *this;
}

template <class T>
auto Vector3<T>::operator+(Vector3<T> const& v) const -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; i++) {
    result.data[i] = data[i] + v.data[i];
  }
  return result;
}

template <class T>
auto Vector3<T>::operator-(Vector3<T> const& v) const -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; i++) {
    result.data[i] = data[i] - v.data[i];
  }
  return result;
}

template <class T>
auto Vector3<T>::operator*(Vector3<T> const& v) const -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; i++) {
    result.data[i] = data[i] * v.data[i];
  }
  return result;
}

template <class T>
auto Vector3<T>::operator/(Vector3<T> const& v) const -> Vector3<T> {
  Vector3<T> result;
  for (size_t i = 0; i < 3; i++) {
    result.data[i] = data[i] / v.data[i];
  }
  return result;
}

template <class T>
auto Vector3<T>::operator+=(Vector3<T> const& v) -> Vector3<T>& {
  for (size_t i = 0; i < 3; i++) {
    data[i] += v.data[i];
  }
  return *this;
}

template <class T>
auto Vector3<T>::operator-=(Vector3<T> const& v) -> Vector3<T>& {
  for (size_t i = 0; i < 3; i++) {
    data[i] -= v.data[i];
  }
  return *this;
}

template <class T>
auto Vector3<T>::operator*=(Vector3<T> const& v) -> Vector3<T>& {
  for (size_t i = 0; i < 3; i++) {
    data[i] *= v.data[i];
  }
  return *this;
}

template <class T>
auto Vector3<T>::operator==(Vector3<T> const& v) const -> bool {
  return (x == v.x) && (y == v.y) && (z == v.z);
}

template <class T>
auto Vector3<T>::operator!=(Vector3<T> const& v) const -> bool {
  return !(*this == v);
}


template <class T>
template <class U>
Vector4<T>::operator Vector4<U>() const {
  return Vector4<U>(x, y, z, w);
}

template <class S, class T>
auto operator*(S s, Vector4<T> const& v) noexcept -> Vector4<T> {
  return Vector4<T>{v.x* s, v.y* s, v.z* s, v.w* s};
}

template <class T>
inline auto abs(Vector4<T> const& v) noexcept -> T {
  T result;
  for (size_t i = 0; i < 4; ++i) {
    result = std::abs(v.data[i]);
  }
  return result;
}

template <class T>
inline auto floor(Vector4<T> const& v) noexcept -> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; ++i) {
    result.data[i] = std::floor(v.data[i]);
  }
  return result;
}

template <class T>
inline auto ceil(Vector4<T> const& v) noexcept -> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; ++i) {
    result.data[i] = std::ceil(v.data[i]);
  }
  return result;
}

template <class T>
inline auto dot(Vector4<T> const& x, Vector4<T> const& y) noexcept -> T {
  T result;
  for (size_t i = 0; i < 4; ++i) {
    result += x[i] * y[i];
  }
  return result;
}

template <class T>
inline auto absDot(Vector4<T> const& x, Vector4<T> const& y) noexcept -> T {
  return std::abs(dot(x, y));
}

template <class T>
inline auto normalize(Vector4<T> const& v) noexcept -> Vector4<T> {
  return v / v.length();
}

template <class T>
inline auto sign(Vector4<T> const& v) noexcept -> Vector4<float> {
  return Vector4<float>{sign(v.x), sign(v.y), sign(v.z), sign(v.w)};
}

template <class T>
inline auto equal(Vector4<T> const& v1, Vector4<T> const& v2) noexcept -> bool {
  return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

template <class T>
inline auto minComponent(Vector4<T> const& v) noexcept -> T {
  return std::min(std::min(v.x, v.y), std::min(v.z, v.w));
}

template <class T>
inline auto maxComponent(Vector4<T> const& v) noexcept -> T {
  return std::max(std::max(v.x, v.y), std::max(v.z, v.w));
}

template <class T>
inline auto maxDimension(Vector4<T> const& v) noexcept -> size_t {
  return (v.x > v.y)
    ? ((v.x > v.z) ? ((v.x > v.w) ? 0 : 3) : ((v.z > v.w) ? 2 : 3))
    : ((v.y > v.z) ? ((v.y > v.w) ? 1 : 3) : ((v.z > v.w) ? 2 : 3));
}

template <class T>
inline auto minDimension(Vector4<T> const& v) noexcept -> size_t {
  return (v.x < v.y)
    ? ((v.x < v.z) ? ((v.x < v.w) ? 0 : 3) : ((v.z < v.w) ? 2 : 3))
    : ((v.y < v.z) ? ((v.y < v.w) ? 1 : 3) : ((v.z < v.w) ? 2 : 3));
}

template <class T>
inline auto max(Vector4<T> const& x, Vector4<T> const& y) noexcept
-> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; ++i) {
    result[i] = std::max(x[i], y[i]);
  }
  return result;
}

template <class T>
inline auto min(Vector4<T> const& x, Vector4<T> const& y) noexcept
-> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; ++i) {
    result[i] = std::min(x[i], y[i]);
  }
  return result;
}

template <class T>
inline auto permute(Vector4<T> const& v, size_t x, size_t y, size_t z,
  size_t w) noexcept -> Vector4<T> {
  return Vector4<T>(v[x], v[y], v[z], v[w]);
}

template <class T>
inline auto distance(Vector4<T> const& p1, Vector4<T> const& p2) noexcept
-> float {
  return (p1 - p2).length();
}

template <class T>
inline auto distanceSquared(Vector4<T> const& p1, Vector4<T> const& p2) noexcept
-> float {
  return (p1 - p2).lengthSquared();
}

template <class T>
inline auto operator*(T s, Vector4<T>& v) -> Vector4<T> {
  return v * s;
}

template <class T>
inline auto lerp(float t, Vector4<T> const& x, Vector4<T>& y) noexcept
-> Vector4<T> {
  return (1 - t) * x + t * y;
}

template <class T>
inline auto length(Vector4<T> const& x) noexcept -> T {
  return std::sqrt(x.length_squared());
}

template <class T>
auto Vector4<T>::length_squared() const -> float {
  return x * x + y * y + z * z + w * w;
}

template <class T>
auto Vector4<T>::to_string() const->std::string {
  return fmt::format("Vec4[{}, {}, {}, {}]", 
    std::to_string(data[0]), std::to_string(data[1]),
    std::to_string(data[2]), std::to_string(data[3]));
}

template <class T>
auto Vector4<T>::length() const -> float {
  return std::sqrt(length_squared());
}

template <class T>
auto Vector4<T>::operator-() const -> Vector4<T> {
  return Vector4<T>{-x, -y, -z, -w};
}

template <class T>
auto Vector4<T>::operator*(T s) const -> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; i++) {
    result.data[i] = data[i] * s;
  }
  return result;
}

template <class T>
auto Vector4<T>::operator/(T s) const -> Vector4<T> {
  float inv = 1.f / s;
  Vector4<T> result;
  for (size_t i = 0; i < 4; i++) {
    result.data[i] = data[i] * inv;
  }
  return result;
}

template <class T>
auto Vector4<T>::operator*=(T s) -> Vector4<T>& {
  for (size_t i = 0; i < 4; i++) {
    data[i] *= s;
  }
  return *this;
}

template <class T>
auto Vector4<T>::operator/=(T s) -> Vector4<T>& {
  float inv = 1.f / s;
  for (size_t i = 0; i < 4; i++) {
    data[i] *= inv;
  }
  return *this;
}

template <class T>
auto Vector4<T>::operator+(Vector4<T> const& v) const -> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; i++) {
    result.data[i] = data[i] + v.data[i];
  }
  return result;
}

template <class T>
auto Vector4<T>::operator-(Vector4<T> const& v) const -> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; i++) {
    result.data[i] = data[i] - v.data[i];
  }
  return result;
}

template <class T>
auto Vector4<T>::operator*(Vector4<T> const& v) const -> Vector4<T> {
  Vector4<T> result;
  for (size_t i = 0; i < 4; i++) {
    result.data[i] = data[i] * v.data[i];
  }
  return result;
}

template <class T>
auto Vector4<T>::operator+=(Vector4<T> const& v) -> Vector4<T>& {
  for (size_t i = 0; i < 4; i++) {
    data[i] += v.data[i];
  }
  return *this;
}

template <class T>
auto Vector4<T>::operator-=(Vector4<T> const& v) -> Vector4<T>& {
  for (size_t i = 0; i < 4; i++) {
    data[i] -= v.data[i];
  }
  return *this;
}

template <class T>
auto Vector4<T>::operator*=(Vector4<T> const& v) -> Vector4<T>& {
  for (size_t i = 0; i < 4; i++) {
    data[i] *= v.data[i];
  }
  return *this;
}

template <class T>
auto Vector4<T>::operator==(Vector4<T> const& v) const -> bool {
  return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w);
}

template <class T>
auto Vector4<T>::operator!=(Vector4<T> const& v) const -> bool {
  return !(*this == v);
}

template<class T>
auto cross(const Vector4<T>& a, const Vector4<T>& b, const Vector4<T>& c)
-> Vector4<T> {
  // Code adapted from VecLib4d.c in Graphics Gems V
  T d1 = (b[2] * c[3]) - (b[3] * c[2]);
  T d2 = (b[1] * c[3]) - (b[3] * c[1]);
  T d3 = (b[1] * c[2]) - (b[2] * c[1]);
  T d4 = (b[0] * c[3]) - (b[3] * c[0]);
  T d5 = (b[0] * c[2]) - (b[2] * c[0]);
  T d6 = (b[0] * c[1]) - (b[1] * c[0]);
  return Vector4<T>(
    -a[1] * d1 + a[2] * d2 - a[3] * d3, a[0] * d1 - a[2] * d4 + a[3] * d5,
    -a[0] * d2 + a[1] * d4 - a[3] * d6, a[0] * d3 - a[1] * d5 + a[2] * d6);
}
