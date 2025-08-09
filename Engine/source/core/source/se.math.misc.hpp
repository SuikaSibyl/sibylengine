
template <typename Predicate>
auto find_interval(int size, const Predicate& pred) noexcept -> int {
	int first = 0, len = size;
	while (len > 0) {
		int half = len >> 1, middle = first + half;
		// Bisect range based on value of _pred_ at _middle_
		if (pred(middle)) {
			first = middle + 1;
			len -= half + 1;
		}
		else
			len = half;
	}
	return clamp(first - 1, 0, size - 2);
}

template <class T>
inline auto max(Point2<T> const& a, Point2<T> const& b) noexcept -> Point2<T> {
  return Point2<T>{std::max(a.x, b.x), std::max(a.y, b.y)};
}

template <class T>
inline auto min(Point2<T> const& a, Point2<T> const& b) noexcept -> Point2<T> {
  return Point2<T>{std::min(a.x, b.x), std::min(a.y, b.y)};
}

template <class T> inline auto faceforward(Normal3<T> const& n, Vector3<T> const& v) { return (dot(n, v) < 0.f) ? -n : n; }

template <class T>
auto Normal3<T>::operator*=(T s) -> Normal3<T>& {
  this->x *= s; this->y *= s; this->z *= s;
  return *this;
}

template <class T>
auto Normal3<T>::operator-() const->Normal3<T> {
  return Normal3<T>{-this->x, -this->y, -this->z};
}

// iterator for ibounds2
// -----------------------
struct ibounds2Iterator : public std::forward_iterator_tag {
  ipoint2 p;
  ibounds2 const* bounds;

  ibounds2Iterator(const ibounds2& b, const ipoint2& pt)
    : p(pt), bounds(&b) {}

  auto operator++()->ibounds2Iterator;
  auto operator++(int)->ibounds2Iterator;
  auto operator==(const ibounds2Iterator& bi) const -> bool;
  auto operator!=(const ibounds2Iterator& bi) const -> bool;
  auto operator*() const noexcept -> ipoint2 { return p; }

  auto advance() noexcept -> void;
};

inline auto begin(ibounds2 const& b) -> ibounds2Iterator;
inline auto end(ibounds2 const& b) -> ibounds2Iterator;

// template impl
// -----------------------
template <class T>
Bounds2<T>::Bounds2() {
  T minNum = std::numeric_limits<T>::lowest();
  T maxNum = std::numeric_limits<T>::max();

  pMin = Vector2<T>(maxNum);
  pMax = Vector2<T>(minNum);
}

template <class T>
Bounds2<T>::Bounds2(Point2<T> const& p) {
  pMin = p;
  pMax = p;
}

template <class T>
Bounds2<T>::Bounds2(Point2<T> const& p1, Point2<T> const& p2) {
  pMin = Vector2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
  pMax = Vector2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
}

template <class T>
auto Bounds2<T>::operator[](uint32_t i) const -> Point2<T> const& {
  return (i == 0) ? pMin : pMax;
}

template <class T>
auto Bounds2<T>::operator[](uint32_t i) -> Point2<T>& {
  return (i == 0) ? pMin : pMax;
}

template <class T>
auto Bounds2<T>::corner(uint32_t c) const -> Point2<T> {
  return Point2<T>((*this)[(c & 1)].x, (*this)[(c & 2) ? 1 : 0].y);
}

template <class T>
auto Bounds2<T>::diagonal() const -> Vector2<T> {
  return pMax - pMin;
}

template <class T>
auto Bounds2<T>::surfaceArea() const -> T {
  Vector2<T> d = diagonal();
  return d.x * d.y;
}

template <class T>
auto Bounds2<T>::maximumExtent() const -> uint32_t {
  Vector3<T> d = diagonal();
  if (d.x > d.y)
    return 0;
  else
    return 1;
}

template <class T>
auto Bounds2<T>::lerp(Point2<T> const& t) const -> Point2<T> {
  return Point2<T>(
    { se::lerp(pMin.x, pMax.x, t.x), se::lerp(pMin.y, pMax.y, t.y) });
}

template <class T>
auto Bounds2<T>::offset(Point2<T> const& p) const -> Vector2<T> {
  Vector2<T> o = p - pMin;
  if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
  if (pMax.y > pMin.y) o.x /= pMax.y - pMin.y;
  return o;
}

template <class T>
auto Bounds2<T>::boundingCircle(Point2<T>* center, float* radius) const
-> void {
  *center = (pMin + pMax) / 2;
  *radius = inside(*center, *this) ? distance(*center, pMax) : 0;
}

template <class T>
inline auto unionPoint(Bounds2<T> const& b, Point2<T> const& p) noexcept
-> Bounds2<T> {
  return Bounds2<T>(
    Point2<T>(std::min(b.pMin.x, p.x), std::min(b.pMin.y, p.y)),
    Point2<T>(std::max(b.pMax.x, p.x), std::max(b.pMax.y, p.y)));
}

template <class T>
inline auto unionBounds(Bounds2<T> const& b1, Bounds2<T> const& b2) noexcept
-> Bounds2<T> {
  return Bounds2<T>(Point2<T>(std::min(b1.pMin.x, b2.pMin.x),
    std::min(b1.pMin.y, b2.pMin.y)),
    Point2<T>(std::max(b1.pMax.x, b2.pMax.x),
      std::max(b1.pMax.y, b2.pMax.y)));
}

template <class T>
inline auto intersect(Bounds2<T> const& b1, Bounds2<T> const& b2) noexcept
-> Bounds2<T> {
  return Bounds2<T>(Point2<T>(std::max(b1.pMin.x, b2.pMin.x),
    std::max(b1.pMin.y, b2.pMin.y)),
    Point2<T>(std::min(b1.pMax.x, b2.pMax.x),
      std::min(b1.pMax.y, b2.pMax.y)));
}

template <class T>
inline auto overlaps(Bounds2<T> const& b1, Bounds2<T> const& b2) noexcept
-> Bounds2<T> {
  bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
  bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
  return (x && y);
}

template <class T>
inline auto inside(Point2<T> const& p, Bounds2<T> const& b) noexcept -> bool {
  return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
    p.y <= b.pMax.y);
}

template <class T>
inline auto insideExclusive(Point2<T> const& p, Bounds2<T> const& b) noexcept
-> bool {
  return (p.x >= b.pMin.x && p.x < b.pMax.x&& p.y >= b.pMin.y &&
    p.y < b.pMax.y);
}

template <class T>
inline auto expand(Bounds2<T> const& b, float delta) noexcept -> bool {
  return Bounds2<T>(b.pMin - Vector2<T>(delta), b.pMax + Vector2<T>(delta));
}

inline auto begin(ibounds2 const& b) -> ibounds2Iterator {
  return ibounds2Iterator(b, b.pMin);
}

inline auto end(ibounds2 const& b) -> ibounds2Iterator {
  // Normally, the ending point is at the minimum x value and one past
  // the last valid y value.
  ipoint2 pEnd = ivec2(b.pMin.x, b.pMax.y);
  // However, if the bounds are degenerate, override the end point to
  // equal the start point so that any attempt to iterate over the bounds
  // exits out immediately.
  if (b.pMin.x >= b.pMax.x || b.pMin.y >= b.pMax.y) pEnd = b.pMin;
  return ibounds2Iterator(b, pEnd);
}

template <class T>
Bounds3<T>::Bounds3() {
  T minNum = std::numeric_limits<T>::lowest();
  T maxNum = std::numeric_limits<T>::max();

  pMin = Vector3<T>(maxNum);
  pMax = Vector3<T>(minNum);
}

template <class T>
Bounds3<T>::Bounds3(Point3<T> const& p) {
  pMin = p;
  pMax = p;
}

template <class T>
Bounds3<T>::Bounds3(Point3<T> const& p1, Point3<T> const& p2) {
  pMin = Vector3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
    std::min(p1.z, p2.z));
  pMax = Vector3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
    std::max(p1.z, p2.z));
}

template <class T>
auto Bounds3<T>::operator[](uint32_t i) const -> Point3<T> const& {
  return (i == 0) ? pMin : pMax;
}

template <class T>
auto Bounds3<T>::operator[](uint32_t i) -> Point3<T>& {
  return (i == 0) ? pMin : pMax;
}

template <class T>
auto Bounds3<T>::corner(uint32_t c) const -> Point3<T> {
  return Point3<T>((*this)[(c & 1)].x, (*this)[(c & 2) ? 1 : 0].y,
    (*this)[(c & 4) ? 1 : 0].z);
}

template <class T>
auto Bounds3<T>::diagonal() const -> Vector3<T> {
  return pMax - pMin;
}

template <class T>
auto Bounds3<T>::surfaceArea() const -> T {
  Vector3<T> d = diagonal();
  return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

template <class T>
auto Bounds3<T>::volume() const -> T {
  Vector3<T> d = diagonal();
  return d.x * d.y * d.z;
}

template <class T>
auto Bounds3<T>::maximumExtent() const -> uint32_t {
  Vector3<T> d = diagonal();
  if (d.x > d.y && d.x > d.z)
    return 0;
  else if (d.y > d.z)
    return 1;
  else
    return 2;
}

template <class T>
auto Bounds3<T>::lerp(Point3<T> const& t) const -> Point3<T> {
  return Point3<T>({ se::lerp(pMin.x, pMax.x, t.x),
                    se::lerp(pMin.y, pMax.y, t.y),
                    se::lerp(pMin.z, pMax.z, t.z) });
}

template <class T>
auto Bounds3<T>::offset(Point3<T> const& p) const -> Vector3<T> {
  Vector3<T> o = p - pMin;
  if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
  if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
  if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
  return o;
}

template <class T>
auto Bounds3<T>::bounding_sphere(Point3<T>* center, float* radius) const
-> void {
  *center = (pMin + pMax) / 2;
  *radius = inside(*center, *this) ? distance(*center, pMax) : 0;
}

template <class T>
auto Bounds3<T>::intersectP(ray3 const& ray, float* hitt0, float* hitt1) const
-> bool {
  float t0 = 0, t1 = ray.tMax;
  for (int i = 0; i < 3; ++i) {
    // could handle infinite (1/0) cases
    float invRayDir = 1 / ray.d[i];
    float tNear = (pMin[i] - ray.o[i]) * invRayDir;
    float tFar = (pMax[i] - ray.o[i]) * invRayDir;
    if (tNear > tFar) std::swap(tNear, tFar);
    // could handle NaN (0/0) cases
    t0 = tNear > t0 ? tNear : t0;
    t1 = tFar < t1 ? tFar : t1;
    if (t0 > t1) return false;
  }
  if (hitt0) *hitt0 = t0;
  if (hitt1) *hitt1 = t1;
  return true;
}

template <class T>
auto Bounds3<T>::intersectP(ray3 const& ray, vec3 const& invDir,
  ivec3 const& dirIsNeg) const -> bool {
  Bounds3<float> const& bounds = *this;
  // Check for ray intersection against x and y slabs
  float tMin = (bounds[0 + dirIsNeg[0]].x - ray.o.x) * invDir.x;
  float tMax = (bounds[1 - dirIsNeg[0]].x - ray.o.x) * invDir.x;
  float tyMin = (bounds[0 + dirIsNeg[1]].y - ray.o.y) * invDir.y;
  float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.o.y) * invDir.y;
  if (tMin > tyMax || tyMin > tMax) return false;
  if (tyMin > tMin) tMin = tyMin;
  if (tyMax < tMax) tMax = tyMax;
  // Check for ray intersection against z slab
  float tzMin = (bounds[0 + dirIsNeg[2]].z - ray.o.z) * invDir.z;
  float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.o.z) * invDir.z;
  if (tMin > tzMax || tzMin > tMax) return false;
  if (tzMin > tMin) tMin = tzMin;
  if (tzMax < tMax) tMax = tzMax;
  return (tMin < ray.tMax) && (tMax > 0);
}

template <class T>
inline auto unionPoint(Bounds3<T> const& b, Point3<T> const& p) noexcept
-> Bounds3<T> {
  return Bounds3<T>(
    Point3<T>(std::min(b.pMin.x, p.x), std::min(b.pMin.y, p.y),
      std::min(b.pMin.z, p.z)),
    Point3<T>(std::max(b.pMax.x, p.x), std::max(b.pMax.y, p.y),
      std::max(b.pMax.z, p.z)));
}

template <class T>
inline auto unionBounds(Bounds3<T> const& b1, Bounds3<T> const& b2) noexcept
-> Bounds3<T> {
  return Bounds3<T>(Point3<T>(std::min(b1.pMin.x, b2.pMin.x),
    std::min(b1.pMin.y, b2.pMin.y),
    std::min(b1.pMin.z, b2.pMin.z)),
    Point3<T>(std::max(b1.pMax.x, b2.pMax.x),
      std::max(b1.pMax.y, b2.pMax.y),
      std::max(b1.pMax.z, b2.pMax.z)));
}

template <class T>
inline auto intersect(Bounds3<T> const& b1, Bounds3<T> const& b2) noexcept
-> Bounds3<T> {
  return Bounds3<T>(Point3<T>(std::max(b1.pMin.x, b2.pMin.x),
    std::max(b1.pMin.y, b2.pMin.y),
    std::max(b1.pMin.z, b2.pMin.z)),
    Point3<T>(std::min(b1.pMax.x, b2.pMax.x),
      std::min(b1.pMax.y, b2.pMax.y),
      std::min(b1.pMax.z, b2.pMax.z)));
}

template <class T>
inline auto overlaps(Bounds3<T> const& b1, Bounds3<T> const& b2) noexcept
-> Bounds3<T> {
  bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
  bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
  bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
  return (x && y && z);
}

template <class T>
inline auto inside(Point3<T> const& p, Bounds3<T> const& b) noexcept -> bool {
  return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
    p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
}

template <class T>
inline auto insideExclusive(Point3<T> const& p, Bounds3<T> const& b) noexcept
-> bool {
  return (p.x >= b.pMin.x && p.x < b.pMax.x&& p.y >= b.pMin.y &&
    p.y < b.pMax.y&& p.z >= b.pMin.z && p.z < b.pMax.z);
}

template <class T>
inline auto expand(Bounds3<T> const& b, float delta) noexcept -> bool {
  return Bounds3<T>(b.pMin - Vector3<T>(delta), b.pMax + Vector3<T>(delta));
}
