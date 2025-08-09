#include "se.math.hpp"
#include <cstdint>

namespace se {
	auto next_float_up(float v) noexcept -> float {
		// Hanlde infinity and negative zero
		if (std::isinf(v) && v > 0.f) return v; if (v == -0.f) return 0.f;
		// Advance v to next higher float
		uint32_t ui = float_to_bits(v);
		if (v >= 0) ++ui; else --ui;
		return bits_to_float(ui);
	}
	
	auto next_float_down(float v) noexcept -> float {
		// Hanlde infinity and positive zero
		if (std::isinf(v) && v < 0.f) return v;
		if (v == +0.f) return -0.f;
		// Advance v to next higher float
		uint32_t ui = float_to_bits(v);
		if (v >= 0) --ui;
		else ++ui;
		return bits_to_float(ui);
	}

	auto ctz(uint32_t value) noexcept -> uint32_t {
    #ifdef _WIN32
      unsigned long trailing_zero = 0;
      if (_BitScanForward(&trailing_zero, value)) return trailing_zero;
      // This is undefined, I better choose 32 than 0
      else return 32;
    #elif defined(__linux__)
      return __builtin_ctz(value);
    #endif
	}

	auto clz(uint32_t value) noexcept -> uint32_t {
    #ifdef _WIN32
      unsigned long leading_zero = 0;
      if (_BitScanReverse(&leading_zero, value)) return 31 - leading_zero;
      // Same remarks as above
      else return 32;
    #elif defined(__linux__)
      return __builtin_clz(value);
    #endif
	}
	
	auto log2int(uint32_t v) noexcept -> int { return 31 - clz(v); }
	auto count_trailing_zeros(uint32_t v) noexcept -> int { return ctz(v); }

	auto round_up_pow2(int32_t v) noexcept -> int32_t {
		--v; v |= v >> 1; v |= v >> 2; v |= v >> 4;
		v |= v >> 8; v |= v >> 16; return v + 1;
	}

	auto alignUp(uint32_t a, uint32_t b) -> uint32_t {
		const uint32_t res = a + b - 1;
		return res - res % b;
	}

  union uif32 {
    uif32() : i(0) {}
    uif32(float f) : f(f) {}
    uif32(unsigned int i) : i(i) {}

    float f;
    unsigned int i;
  };

  typedef uif32 uif;

  float overflow() {
    volatile float f = 1e10;
    for (int i = 0; i < 10; ++i)
      f *= f; // this will overflow before the forloop terminates
    return f;
  }

  half::half(float f) {
    uif Entry;
    Entry.f = f;
    int i = (int)Entry.i;

    int s = (i >> 16) & 0x00008000;
    int e = ((i >> 23) & 0x000000ff) - (127 - 15);
    int m = i & 0x007fffff;

    if (e <= 0) {
      if (e < -10) {
        hdata = s;
        return;
      }

      m = (m | 0x00800000) >> (1 - e);

      if (m & 0x00001000)
        m += 0x00002000;

      hdata = (s | (m >> 13));
      return;
    }
    else if (e == 0xff - (127 - 15)) {
      if (m == 0) {
        hdata = (s | 0x7c00);
        return;
      }
      else {
        m >>= 13;
        hdata = (s | 0x7c00 | m | (m == 0));
        return;
      }
    }
    else {
      if (m & 0x00001000) {
        m += 0x00002000;
        if (m & 0x00800000) {
          m = 0;     // overflow in significand,
          e += 1;     // adjust exponent
        }
      }

      if (e > 30) {
        overflow();        // Cause a hardware floating point overflow;
        hdata = (s | 0x7c00);
        return;
        // if this returns, the half becomes an
      }   // infinity with the same sign as f.
      hdata = (s | (e << 10) | (m >> 13));
      return;
    }
  }

  float half::to_float() const {
    int s = (hdata >> 15) & 0x00000001;
    int e = (hdata >> 10) & 0x0000001f;
    int m = hdata & 0x000003ff;

    if (e == 0) {
      if (m == 0) {
        uif result;
        result.i = (unsigned int)(s << 31);
        return result.f;
      }
      else {
        while (!(m & 0x00000400)) {
          m <<= 1;
          e -= 1;
        }

        e += 1;
        m &= ~0x00000400;
      }
    }
    else if (e == 31) {
      if (m == 0) {
        uif result;
        result.i = (unsigned int)((s << 31) | 0x7f800000);
        return result.f;
      }
      else {
        uif result;
        result.i = (unsigned int)((s << 31) | 0x7f800000 | (m << 13));
        return result.f;
      }
    }

    e = e + (127 - 15);
    m = m << 13;

    uif Result;
    Result.i = (unsigned int)((s << 31) | (e << 23) | m);
    return Result.f;
  }

  Quaternion::Quaternion(mat3 const& m) {
    // Notice:
    // T = 4 - 4*qx2 - 4*qy2 - 4*qz2
    //   = 4(1 - qx2 - qy2 - qz2)
    //   = m00 + m11 + m22 + 1
    float trace = m.data[0][0] + m.data[1][1] + m.data[2][2];
    if (trace > 0) {
      // case that the function inside the square root is positive
      float s = 0.5f / std::sqrt(trace + 1);
      w = 0.25f / s;
      x = (m.data[2][1] - m.data[1][2]) * s;
      y = (m.data[0][2] - m.data[2][0]) * s;
      z = (m.data[1][0] - m.data[0][1]) * s;
    }
    else if (m.data[0][0] > m.data[1][1] && m.data[0][0] > m.data[2][2]) {
      // case [0][0] is the largest
      float s =
        2.0f * std::sqrt(1.f + m.data[0][0] - m.data[1][1] - m.data[2][2]);
      w = (m.data[2][1] - m.data[1][2]) / s;
      x = 0.25f * s;
      y = (m.data[0][1] + m.data[1][0]) / s;
      z = (m.data[0][2] + m.data[2][0]) / s;

    }
    else if (m.data[1][1] > m.data[2][2]) {
      // case [1][1] is the largest
      float s =
        2.0f * std::sqrt(1.f + m.data[1][1] - m.data[0][0] - m.data[2][2]);
      w = (m.data[0][2] - m.data[2][0]) / s;
      x = (m.data[0][1] + m.data[1][0]) / s;
      y = 0.25f * s;
      z = (m.data[1][2] + m.data[2][1]) / s;
    }
    else {
      // case [2][2] is the largest
      float s =
        2.0f * std::sqrt(1.f + m.data[2][2] - m.data[0][0] - m.data[1][1]);
      w = (m.data[1][0] - m.data[0][1]) / s;
      x = (m.data[0][2] + m.data[2][0]) / s;
      y = (m.data[1][2] + m.data[2][1]) / s;
      z = 0.25f * s;
    }
  }

  Quaternion::Quaternion(mat4 const& m) : Quaternion(mat3(m)) {}

  auto Quaternion::length_squared() const -> float {
    return x * x + y * y + z * z + w * w;
  }

  auto Quaternion::length() const -> float { return std::sqrt(length_squared()); }

  auto Quaternion::toMat3() const noexcept -> mat3 {
    // Math::mat4 quat_transform = quat.toMat4();
    se::vec3 x = (*this) * se::vec3(1, 0, 0);
    se::vec3 y = (*this) * se::vec3(0, 1, 0);
    se::vec3 z = (*this) * se::vec3(0, 0, 1);
    // Extract the position of the transform
    return se::mat3(x.x, y.x, z.x,  // X basis (& Scale)
      x.y, y.y, z.y,  // Y basis (& scale)
      x.z, y.z, z.z); // Z basis (& scale)
  }

  auto Quaternion::toMat4() const noexcept -> mat4 {
    // Math::mat4 quat_transform = quat.toMat4();
    se::vec3 x = (*this) * se::vec3(1, 0, 0);
    se::vec3 y = (*this) * se::vec3(0, 1, 0);
    se::vec3 z = (*this) * se::vec3(0, 0, 1);
    // Extract the position of the transform
    return se::mat4(x.x, y.x, z.x, 0,  // X basis (& Scale)
      x.y, y.y, z.y, 0,  // Y basis (& scale)
      x.z, y.z, z.z, 0,  // Z basis (& scale)
      0, 0, 0, 1);
  }

  auto Quaternion::conjugate() noexcept -> Quaternion {
    return Quaternion(-vs.v, vs.s);
  }

  auto Quaternion::reciprocal() noexcept -> Quaternion {
    return conjugate() / length_squared();
  }

  auto Quaternion::operator/(float s) const -> Quaternion {
    Quaternion ret;
    ret.vs.v = this->vs.v / vs.s;
    ret.vs.s = this->vs.s / vs.s;
    return ret;
  }

  auto Quaternion::operator+(Quaternion const& q2) const -> Quaternion {
    Quaternion ret;
    ret.vs.v = vs.v + q2.vs.v;
    ret.vs.s = vs.s + q2.vs.s;
    return ret;
  }

  auto Quaternion::operator*(Quaternion const& q2) const -> Quaternion {
    Quaternion ret;
    ret.vs.v = cross(vs.v, q2.vs.v) + q2.vs.v * vs.s + vs.v * q2.vs.s;
    ret.vs.s = vs.s * q2.vs.s - dot(vs.v, q2.vs.v);
    return ret;
  }

  auto Quaternion::operator*(se::vec3 const& v) const -> se::vec3 {
    return this->vs.v * 2.0f * se::dot(this->vs.v, v) +
      v * (this->vs.s * this->vs.s - se::dot(this->vs.v, this->vs.v)) +
      se::cross(this->vs.v, v) * 2.0f * this->vs.s;
  }

  auto Quaternion::operator+=(Quaternion const& q) -> Quaternion& {
    vs.v += q.vs.v;
    vs.s += q.vs.s;
    return *this;
  }

  auto Quaternion::operator-() const -> Quaternion {
    return Quaternion{ -x, -y, -z, -w };
  }

	auto slerp(float t, Quaternion const& q1,
		Quaternion const& q2) noexcept -> Quaternion {
		Quaternion previousQuat = q1, nextQuat = q2;
		float dotProduct = dot(previousQuat, nextQuat);
		// make sure we take the shortest path in case dot Product is negative
		if (dotProduct < 0.0) {
			nextQuat = -nextQuat;
			dotProduct = -dotProduct;
		}
		// if the two quaternions are too close to each other, just linear
		// interpolate between the 4D vector
		if (dotProduct > 0.9995) {
			return normalize(previousQuat + t * (nextQuat - previousQuat));
		}
		// perform the spherical linear interpolation
		float theta_0 = std::acos(dotProduct);
		float theta = t * theta_0;
		float sin_theta = std::sin(theta);
		float sin_theta_0 = std::sin(theta_0);
		float scalePreviousQuat = std::cos(theta) - dotProduct * sin_theta / sin_theta_0;
		float scaleNextQuat = sin_theta / sin_theta_0;
		return scalePreviousQuat * previousQuat + scaleNextQuat * nextQuat;
	}

	auto offset_ray_origin(point3 const& p,
		vec3 const& pError,
		normal3 const& n,
		vec3 const& w) noexcept
		-> point3 {
		float d = dot((vec3)abs(n), pError);
		vec3 offset = d * vec3(n);
		if (dot(w, n) < 0) offset = -offset;
		point3 po = p + offset;
		// Round offset point po away from p
		for (int i = 0; i < 3; ++i) {
			if (offset.at(i) > 0)
				po.at(i) = next_float_up(po.at(i));
			else if (offset.at(i) < 0)
				po.at(i) = next_float_down(po.at(i));
		}
		return po;
	}

  auto spherical_direction(float sinTheta, float cosTheta,
    float phi) noexcept -> vec3 {
    return vec3(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
      cosTheta);
  }

  auto spherical_direction(float sinTheta, float cosTheta, float phi,
    vec3 const& x, vec3 const& y,
    vec3 const& z) noexcept
    -> vec3 {
    return sinTheta * std::cos(phi) * x + sinTheta * std::sin(phi) * y +
      cosTheta * z;
  }

  auto spherical_theta(vec3 const& v) noexcept -> float {
    return std::acos(clamp(v.z, -1.f, 1.f));
  }

  auto spherical_phi(vec3 const& v) noexcept -> float {
    float p = std::atan2(v.y, v.x);
    return (p < 0) ? (p + 2 * M_FLOAT_PI) : p;
  }

  float MachineEpsilon = std::numeric_limits<float>::epsilon() * 0.5f;
  inline auto gamma(int n) noexcept -> float {
    return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
  }

  Transform::Transform(float const mat[4][4]) {
    m = mat4(mat);
    mInv = inverse(m);
  }

  Transform::Transform(mat4 const& m) : m(m), mInv(inverse(m)) {}

  Transform::Transform(mat4 const& m, mat4 const& mInverse)
    : m(m), mInv(mInverse) {}

  Transform::Transform(Quaternion const& q) {
    mat3 mat3x3 = q.toMat3();
    m = mat4{ mat3x3.data[0][0],
             mat3x3.data[0][1],
             mat3x3.data[0][2],
             0,
             mat3x3.data[1][0],
             mat3x3.data[1][1],
             mat3x3.data[2][2],
             0,
             mat3x3.data[2][0],
             mat3x3.data[2][1],
             mat3x3.data[2][2],
             0,
             0,
             0,
             0,
             1 };
    mInv = inverse(m);
  }

  auto Transform::is_identity() const noexcept -> bool {
    static Transform identity;
    return *this == identity;
  }

  auto Transform::has_scale() const noexcept -> bool {
    float la2 = ((*this) * vec3(1, 0, 0)).length_squared();
    float lb2 = ((*this) * vec3(0, 1, 0)).length_squared();
    float lc2 = ((*this) * vec3(0, 0, 1)).length_squared();
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
    return (NOT_ONE(la2) || NOT_ONE(lb2) || NOT_ONE(lc2));
#undef NOT_ONE
  }

  auto Transform::swaps_handness() const noexcept -> bool {
    float det = m.data[0][0] *
      (m.data[1][1] * m.data[2][2] - m.data[1][2] * m.data[2][1]) -
      m.data[0][1] *
      (m.data[1][0] * m.data[2][2] - m.data[1][2] * m.data[2][0]) +
      m.data[0][2] *
      (m.data[1][0] * m.data[2][1] - m.data[1][1] * m.data[2][0]);
    return det < 0;
  }

  auto Transform::operator==(Transform const& t) const -> bool {
    return m == t.m;
  }

  auto Transform::operator!=(Transform const& t) const -> bool {
    return !(*this == t);
  }

  auto Transform::operator*(point3 const& p) const -> point3 {
    vec3 s(m.data[0][0] * p.x + m.data[0][1] * p.y + m.data[0][2] * p.z +
      m.data[0][3],
      m.data[1][0] * p.x + m.data[1][1] * p.y + m.data[1][2] * p.z +
      m.data[1][3],
      m.data[2][0] * p.x + m.data[2][1] * p.y + m.data[2][2] * p.z +
      m.data[2][3]);
    s = s / (m.data[3][0] * p.x + m.data[3][1] * p.y + m.data[3][2] * p.z +
      m.data[3][3]);
    return point3(s);
  }

  auto Transform::operator*(vec3 const& v) const -> vec3 {
    return vec3{ m.data[0][0] * v.x + m.data[0][1] * v.y + m.data[0][2] * v.z,
                m.data[1][0] * v.x + m.data[1][1] * v.y + m.data[1][2] * v.z,
                m.data[2][0] * v.x + m.data[2][1] * v.y + m.data[2][2] * v.z };
  }

  auto Transform::operator*(normal3 const& n) const -> normal3 {
    return normal3{
        mInv.data[0][0] * n.x + mInv.data[1][0] * n.y + mInv.data[2][0] * n.z,
        mInv.data[0][1] * n.x + mInv.data[1][1] * n.y + mInv.data[2][1] * n.z,
        mInv.data[0][2] * n.x + mInv.data[1][2] * n.y + mInv.data[2][2] * n.z };
  }

  auto Transform::operator*(ray3 const& s) const -> ray3 {
    vec3 oError;
    point3 o = (*this) * s.o;
    vec3 d = (*this) * s.d;
    // offset ray origin to edge of error bounds and compute tMax
    float lengthSquared = d.length_squared();
    float tMax = s.tMax;
    if (lengthSquared > 0) {
      float dt = dot((vec3)abs(d), oError) / lengthSquared;
      o += d * dt;
      tMax -= dt;
    }
    return ray3{ o, d, tMax };
  }

  auto Transform::operator*(bounds3 const& b) const -> bounds3 {
    Transform const& m = *this;
    bounds3 ret(m * point3(b.pMin.x, b.pMin.y, b.pMin.z));
    ret = unionPoint(ret, m * point3(b.pMax.x, b.pMin.y, b.pMin.z));
    ret = unionPoint(ret, m * point3(b.pMin.x, b.pMax.y, b.pMin.z));
    ret = unionPoint(ret, m * point3(b.pMin.x, b.pMin.y, b.pMax.z));
    ret = unionPoint(ret, m * point3(b.pMax.x, b.pMax.y, b.pMin.z));
    ret = unionPoint(ret, m * point3(b.pMax.x, b.pMin.y, b.pMax.z));
    ret = unionPoint(ret, m * point3(b.pMin.x, b.pMax.y, b.pMax.z));
    ret = unionPoint(ret, m * point3(b.pMax.x, b.pMax.y, b.pMax.z));
    return ret;
  }

  auto Transform::operator*(Transform const& t2) const -> Transform {
    return Transform(mul(m, t2.m), mul(t2.mInv, mInv));
  }

  auto Transform::operator()(point3 const& p, vec3& absError) const -> point3 {
    // Compute transformed coordinates from point p
    vec3 s((m.data[0][0] * p.x + m.data[0][1] * p.y) +
      (m.data[0][2] * p.z + m.data[0][3]),
      (m.data[1][0] * p.x + m.data[1][1] * p.y) +
      (m.data[1][2] * p.z + m.data[1][3]),
      (m.data[2][0] * p.x + m.data[2][1] * p.y) +
      (m.data[2][2] * p.z + m.data[2][3]));
    float wp = m.data[3][0] * p.x + m.data[3][1] * p.y + m.data[3][2] * p.z +
      m.data[3][3];
    // Compute absolute error for transformed point
    float xAbsSum = (std::abs(m.data[0][0] * p.x) + std::abs(m.data[0][1] * p.y) +
      std::abs(m.data[0][2] * p.z) + std::abs(m.data[0][3]));
    float yAbsSum = (std::abs(m.data[1][0] * p.x) + std::abs(m.data[1][1] * p.y) +
      std::abs(m.data[1][2] * p.z) + std::abs(m.data[1][3]));
    float zAbsSum = (std::abs(m.data[2][0] * p.x) + std::abs(m.data[2][1] * p.y) +
      std::abs(m.data[2][2] * p.z) + std::abs(m.data[2][3]));
    // TODO :: FIX absError bug when (wp!=1)
    absError = gamma(3) * vec3(xAbsSum, yAbsSum, zAbsSum);
    // return transformed point
    if (wp == 1)
      return s;
    else
      return s / wp;
  }

  auto Transform::operator()(point3 const& p, vec3 const& pError, vec3& tError) const -> point3 {
    // Compute transformed coordinates from point p
    vec3 s((m.data[0][0] * p.x + m.data[0][1] * p.y) +
      (m.data[0][2] * p.z + m.data[0][3]),
      (m.data[1][0] * p.x + m.data[1][1] * p.y) +
      (m.data[1][2] * p.z + m.data[1][3]),
      (m.data[2][0] * p.x + m.data[2][1] * p.y) +
      (m.data[2][2] * p.z + m.data[2][3]));
    float wp = m.data[3][0] * p.x + m.data[3][1] * p.y + m.data[3][2] * p.z +
      m.data[3][3];
    // Compute absolute error for transformed point
    float xAbsSum = (std::abs(m.data[0][0] * p.x) + std::abs(m.data[0][1] * p.y) +
      std::abs(m.data[0][2] * p.z) + std::abs(m.data[0][3]));
    float yAbsSum = (std::abs(m.data[1][0] * p.x) + std::abs(m.data[1][1] * p.y) +
      std::abs(m.data[1][2] * p.z) + std::abs(m.data[1][3]));
    float zAbsSum = (std::abs(m.data[2][0] * p.x) + std::abs(m.data[2][1] * p.y) +
      std::abs(m.data[2][2] * p.z) + std::abs(m.data[2][3]));
    float xPError = std::abs(m.data[0][0]) * pError.x +
      std::abs(m.data[0][1]) * pError.y +
      std::abs(m.data[0][2]) * pError.z;
    float yPError = std::abs(m.data[1][0]) * pError.x +
      std::abs(m.data[1][1]) * pError.y +
      std::abs(m.data[1][2]) * pError.z;
    float zPError = std::abs(m.data[2][0]) * pError.x +
      std::abs(m.data[2][1]) * pError.y +
      std::abs(m.data[2][2]) * pError.z;
    // TODO :: FIX absError bug when (wp!=1)
    tError = gamma(3) * vec3(xAbsSum, yAbsSum, zAbsSum) +
      (gamma(3) + 1) * vec3(xPError, yPError, zPError);
    // return transformed point
    if (wp == 1)
      return s;
    else
      return s / wp;
  }

  auto Transform::operator()(vec3 const& v) const->vec3 {
    float x = float(v.x), y = float(v.y), z = float(v.z);
    float xp = m.data[0][0] * x + m.data[0][1] * y + m.data[0][2] * z;
    float yp = m.data[1][0] * x + m.data[1][1] * y + m.data[1][2] * z;
    float zp = m.data[2][0] * x + m.data[2][1] * y + m.data[2][2] * z;
    return vec3(xp, yp, zp);
  }

  auto Transform::operator()(vec3 const& v, vec3& absError) const -> vec3 {
    absError.x =
      gamma(3) * (std::abs(m.data[0][0] * v.x) + std::abs(m.data[0][1] * v.y) +
        std::abs(m.data[0][2] * v.z));
    absError.y =
      gamma(3) * (std::abs(m.data[1][0] * v.x) + std::abs(m.data[1][1] * v.y) +
        std::abs(m.data[1][2] * v.z));
    absError.z =
      gamma(3) * (std::abs(m.data[2][0] * v.x) + std::abs(m.data[2][1] * v.y) +
        std::abs(m.data[2][2] * v.z));

    return vec3{ m.data[0][0] * v.x + m.data[0][1] * v.y + m.data[0][2] * v.z,
                m.data[1][0] * v.x + m.data[1][1] * v.y + m.data[1][2] * v.z,
                m.data[2][0] * v.x + m.data[2][1] * v.y + m.data[2][2] * v.z };
  }

  auto Transform::operator()(vec3 const& v, vec3 const& pError, vec3& tError) const -> vec3 {
    // Compute absolute error for transformed point
    float xAbsSum = (std::abs(m.data[0][0] * v.x) + std::abs(m.data[0][1] * v.y) +
      std::abs(m.data[0][2] * v.z));
    float yAbsSum = (std::abs(m.data[1][0] * v.x) + std::abs(m.data[1][1] * v.y) +
      std::abs(m.data[1][2] * v.z));
    float zAbsSum = (std::abs(m.data[2][0] * v.x) + std::abs(m.data[2][1] * v.y) +
      std::abs(m.data[2][2] * v.z));
    float xPError = std::abs(m.data[0][0]) * pError.x +
      std::abs(m.data[0][1]) * pError.y +
      std::abs(m.data[0][2]) * pError.z;
    float yPError = std::abs(m.data[1][0]) * pError.x +
      std::abs(m.data[1][1]) * pError.y +
      std::abs(m.data[1][2]) * pError.z;
    float zPError = std::abs(m.data[2][0]) * pError.x +
      std::abs(m.data[2][1]) * pError.y +
      std::abs(m.data[2][2]) * pError.z;

    // TODO :: FIX absError bug when (wp!=1)
    tError = gamma(3) * vec3(xAbsSum, yAbsSum, zAbsSum) +
      (gamma(3) + 1) * vec3(xPError, yPError, zPError);

    return vec3{ m.data[0][0] * v.x + m.data[0][1] * v.y + m.data[0][2] * v.z,
                m.data[1][0] * v.x + m.data[1][1] * v.y + m.data[1][2] * v.z,
                m.data[2][0] * v.x + m.data[2][1] * v.y + m.data[2][2] * v.z };
  }

  auto Transform::operator()(ray3 const& r, vec3& oError, vec3& dError) const -> ray3 {
    point3 o = (*this)(r.o, oError);
    vec3 d = (*this)(r.d, dError);
    // offset ray origin to edge of error bounds and compute tMax
    float lengthSquared = d.length_squared();
    float tMax = r.tMax;
    if (lengthSquared > 0) {
      float dt = dot((vec3)abs(d), oError) / lengthSquared;
      o += d * dt;
      tMax -= dt;
    }
    return ray3{ o, d, tMax };
  }

  auto inverse(Transform const& t) noexcept -> Transform {
    return Transform(t.mInv, t.m);
  }

  auto transpose(Transform const& t) noexcept -> Transform {
    return Transform(transpose(t.m), transpose(t.mInv));
  }

  auto translate(vec3 const& delta) noexcept -> Transform {
    mat4 m(1, 0, 0, delta.x, 0, 1, 0, delta.y, 0, 0, 1, delta.z, 0, 0, 0, 1);
    mat4 minv(1, 0, 0, -delta.x, 0, 1, 0, -delta.y, 0, 0, 1, -delta.z, 0, 0, 0,
      1);
    return Transform(m, minv);
  }

  auto scale(float x, float y, float z) noexcept -> Transform {
    mat4 m(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
    mat4 minv(1.f / x, 0, 0, 0, 0, 1.f / y, 0, 0, 0, 0, 1.f / z, 0, 0, 0, 0, 1);
    return Transform(m, minv);
  }

  auto rotate_x(float theta) noexcept -> Transform {
    float sinTheta = std::sin(radians(theta));
    float cosTheta = std::cos(radians(theta));
    mat4 m(1, 0, 0, 0, 0, cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0, 0,
      0, 0, 1);
    return Transform(m, transpose(m));
  }

  auto rotate_y(float theta) noexcept -> Transform {
    float sinTheta = std::sin(radians(theta));
    float cosTheta = std::cos(radians(theta));
    mat4 m(cosTheta, 0, sinTheta, 0, 0, 1, 0, 0, -sinTheta, 0, cosTheta, 0, 0,
      0, 0, 1);
    return Transform(m, transpose(m));
  }

  auto rotate_z(float theta) noexcept -> Transform {
    float sinTheta = std::sin(radians(theta));
    float cosTheta = std::cos(radians(theta));
    mat4 m(cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 1);
    return Transform(m, transpose(m));
  }

  auto rotate(float theta, vec3 const& axis) noexcept -> Transform {
    vec3 a = normalize(axis);
    float sinTheta = std::sin(radians(theta));
    float cosTheta = std::cos(radians(theta));
    mat4 m;
    // Compute rotation of first basis vector
    m.data[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
    m.data[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
    m.data[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
    m.data[0][3] = 0;
    // Compute rotations of second and third basis vectors
    m.data[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
    m.data[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
    m.data[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
    m.data[1][3] = 0;

    m.data[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
    m.data[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
    m.data[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
    m.data[2][3] = 0;

    return Transform(m, transpose(m));
  }

  auto look_at(point3 const& pos, point3 const& look,
    vec3 const& up) noexcept -> Transform {
    mat4 cameraToWorld;
    // Initialize fourth column of viewing matrix
    cameraToWorld.data[0][3] = pos.x;
    cameraToWorld.data[1][3] = pos.y;
    cameraToWorld.data[2][3] = pos.z;
    cameraToWorld.data[3][3] = 1;
    // Initialize first three columns of viewing matrix
    vec3 dir = normalize(look - pos);
    vec3 left = normalize(cross(dir, normalize(up)));
    vec3 newUp = cross(dir, left);
    cameraToWorld.data[0][0] = left.x;
    cameraToWorld.data[0][1] = newUp.x;
    cameraToWorld.data[0][2] = dir.x;
    cameraToWorld.data[1][0] = left.y;
    cameraToWorld.data[1][1] = newUp.y;
    cameraToWorld.data[1][2] = dir.y;
    cameraToWorld.data[2][0] = left.z;
    cameraToWorld.data[2][1] = newUp.z;
    cameraToWorld.data[2][2] = dir.z;
    cameraToWorld.data[3][0] = 0;
    cameraToWorld.data[3][1] = 0;
    cameraToWorld.data[3][2] = 0;
    return Transform(inverse(cameraToWorld), cameraToWorld);
  }

  auto orthographic(float zNear, float zFar) noexcept -> Transform {
    return scale(1, 1, 1.f / (zFar - zNear)) *
      translate({ 0, 0, -zNear });
  }

  auto ortho(float left, float right, float bottom, float top, float zNear,
    float zFar) noexcept -> Transform {
    mat4 trans = { float(2) / (right - left),
                        0,
                        0,
                        -(right + left) / (right - left),
                        0,
                        float(2) / (top - bottom),
                        0,
                        -(top + bottom) / (top - bottom),
                        0,
                        0,
                        float(1) / (zFar - zNear),
                        -1 * zNear / (zFar - zNear),
                        0,
                        0,
                        0,
                        1 };
    return Transform(trans);
  }

  auto perspective(float fov, float n, float f) noexcept -> Transform {
    // perform projective divide for perspective projection
    mat4 persp{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,        1.0f,
               0.0f, 0.0f, 0.0f, 0.0f, f / (f - n), -f * n / (f - n),
               0.0f, 0.0f, 1.0f, 0.0f };
    // scale canonical perspective view to specified field of view
    float invTanAng = 1.f / std::tan(radians(fov) / 2);
    return scale(invTanAng, invTanAng, 1) * Transform(persp);
  }

  auto perspective(float fov, float aspect, float n, float f) noexcept -> Transform {
    // perform projective divide for perspective projection
    mat4 persp{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,        1.0f,
               0.0f, 0.0f, 0.0f, 0.0f, f / (f - n), -f * n / (f - n),
               0.0f, 0.0f, 1.0f, 0.0f };
    // scale canonical perspective view to specified field of view
    float invTanAng = 1.f / std::tan(radians(fov) / 2);
    return scale(invTanAng / aspect, invTanAng, 1) * Transform(persp);
  }

  auto angle_between(vec3 v1, vec3 v2) noexcept -> float {
		if (dot(v1, v2) < 0) return M_FLOAT_PI - 2 * safe_asin(length(v1 + v2) / 2);
		else return 2 * safe_asin(length(v2 - v1) / 2);
	}

  auto euler_angle_to_rotation_matrix(se::vec3 eulerAngles) noexcept -> se::mat3 {
    float x = eulerAngles.x; // Roll
    float y = eulerAngles.y; // Pitch
    float z = eulerAngles.z; // Yaw
    float cx = std::cos(x);
    float sx = std::sin(x);
    float cy = std::cos(y);
    float sy = std::sin(y);
    float cz = std::cos(z);
    float sz = std::sin(z);
    se::mat3 R;
    R.data[0][0] = cz * cy;
    R.data[0][1] = cz * sy * sx - sz * cx;
    R.data[0][2] = cz * sy * cx + sz * sx;
    R.data[1][0] = sz * cy;
    R.data[1][1] = sz * sy * sx + cz * cx;
    R.data[1][2] = sz * sy * cx - cz * sx;
    R.data[2][0] = -sy;
    R.data[2][1] = cy * sx;
    R.data[2][2] = cy * cx;
    return R;
  }

  auto euler_angle_to_quaternion(se::vec3 eulerAngles) noexcept -> se::Quaternion {
    se::mat3 const mat = euler_angle_to_rotation_matrix(eulerAngles);
    return se::Quaternion(mat);
  }

  auto rotation_matrix_to_euler_angles(se::mat3 R) noexcept -> se::vec3 {
    float sy = std::sqrt(R.data[0][0] * R.data[0][0] + R.data[1][0] * R.data[1][0]);
    bool singular = sy < 1e-6;
    float x, y, z;
    if (!singular) {
      x = atan2(R.data[2][1], R.data[2][2]);
      y = atan2(-R.data[2][0], sy);
      z = atan2(R.data[1][0], R.data[0][0]);
    }
    else {
      x = atan2(-R.data[1][2], R.data[1][1]);
      y = atan2(-R.data[2][0], sy);
      z = 0;
    }
    return { x, y, z };
  }

  auto euler_angle_degree_to_rotation_matrix(se::vec3 eulerAngles) noexcept -> se::mat3 {
    return se::mat4::rotateZ(eulerAngles.z) *
      se::mat4::rotateY(eulerAngles.y) *
      se::mat4::rotateX(eulerAngles.x);
  }

  auto euler_angle_degree_to_quaternion(se::vec3 eulerAngles) noexcept -> se::Quaternion {
    return se::Quaternion(euler_angle_degree_to_rotation_matrix(eulerAngles));
  }

  auto decompose(se::mat4 const& m, se::vec3* t, se::Quaternion* rquat, se::vec3* s) noexcept -> void {
    // Extract translation T from transformation matrix
    // which could be found directly from matrix
    t->x = m.data[0][3];
    t->y = m.data[1][3];
    t->z = m.data[2][3];

    // Compute new transformation matrix M without translation
    se::mat4 M = m;
    for (int i = 0; i < 3; i++) M.data[i][3] = M.data[3][i] = 0.f;
    M.data[3][3] = 1.f;

    // Extract rotation R from transformation matrix
    // use polar decomposition, decompose into R&S by averaging M with its
    // inverse transpose until convergence to get R (because pure rotation
    // matrix has similar inverse and transpose)
    float norm;
    int count = 0;
    se::mat4 R = M;
    do {
      // Compute next matrix Rnext in series
      se::mat4 rNext;
      se::mat4 rInvTrans = inverse(transpose(R));
      for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
          rNext.data[i][j] = 0.5f * (R.data[i][j] + rInvTrans.data[i][j]);
      // Compute norm of difference between R and Rnext
      norm = 0.f;
      for (int i = 0; i < 3; ++i) {
        float n = std::abs(R.data[i][0] = rNext.data[i][0]) +
          std::abs(R.data[i][1] = rNext.data[i][1]) +
          std::abs(R.data[i][2] = rNext.data[i][2]);
        norm = std::max(norm, n);
      }
      R = rNext;
    } while (++count < 100 && norm > .0001);

    se::vec3 r, euler = rotation_matrix_to_euler_angles(se::mat3{ R });
    r.x = euler.x * 180. / se::M_DOUBLE_PI;
    r.y = euler.y * 180. / se::M_DOUBLE_PI;
    r.z = euler.z * 180. / se::M_DOUBLE_PI;
    *rquat = euler_angle_degree_to_rotation_matrix(r);

    // Compute scale S using rotationand original matrix
    se::mat4 invR = inverse(R);
    se::mat4 Mtmp = M;
    se::mat4 smat = mul(invR, Mtmp);
    s->x = se::vec3(smat.data[0][0], smat.data[1][0], smat.data[2][0]).length();
    s->y = se::vec3(smat.data[0][1], smat.data[1][1], smat.data[2][1]).length();
    s->z = se::vec3(smat.data[0][2], smat.data[1][2], smat.data[2][2]).length();
  }

  auto decompose(se::mat4 const& m, se::vec3* t, se::vec3* r, se::vec3* s) noexcept -> void {
    // Extract translation T from transformation matrix
    // which could be found directly from matrix
    t->x = m.data[0][3];
    t->y = m.data[1][3];
    t->z = m.data[2][3];

    // Compute new transformation matrix M without translation
    se::mat4 M = m;
    for (int i = 0; i < 3; i++) M.data[i][3] = M.data[3][i] = 0.f;
    M.data[3][3] = 1.f;

    // Extract rotation R from transformation matrix
    // use polar decomposition, decompose into R&S by averaging M with its
    // inverse transpose until convergence to get R (because pure rotation
    // matrix has similar inverse and transpose)
    float norm;
    int count = 0;
    se::mat4 R = M;
    do {
      // Compute next matrix Rnext in series
      se::mat4 rNext;
      se::mat4 rInvTrans = inverse(transpose(R));
      for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
          rNext.data[i][j] = 0.5f * (R.data[i][j] + rInvTrans.data[i][j]);
      // Compute norm of difference between R and Rnext
      norm = 0.f;
      for (int i = 0; i < 3; ++i) {
        float n = std::abs(R.data[i][0] = rNext.data[i][0]) +
          std::abs(R.data[i][1] = rNext.data[i][1]) +
          std::abs(R.data[i][2] = rNext.data[i][2]);
        norm = std::max(norm, n);
      }
      R = rNext;
    } while (++count < 100 && norm > .0001);

    se::vec3 euler = rotation_matrix_to_euler_angles(se::mat3{ R });
    r->x = euler.x * 180. / se::M_DOUBLE_PI;
    r->y = euler.y * 180. / se::M_DOUBLE_PI;
    r->z = euler.z * 180. / se::M_DOUBLE_PI;

    // Compute scale S using rotationand original matrix
    se::mat4 invR = inverse(R);
    se::mat4 Mtmp = M;
    se::mat4 smat = mul(invR, Mtmp);
    s->x = se::vec3(smat.data[0][0], smat.data[1][0], smat.data[2][0]).length();
    s->y = se::vec3(smat.data[0][1], smat.data[1][1], smat.data[2][1]).length();
    s->z = se::vec3(smat.data[0][2], smat.data[1][2], smat.data[2][2]).length();
  }
  auto AnimationCurve::evaluate(float time) noexcept -> float {
    if (time > m_keyFrames.back().time) {  // right warp
      switch (m_preWrapMode) {
      case WrapMode::CLAMP:
        return m_keyFrames.back().value;
        break;
      case WrapMode::REPEAT: {
        int passCount = int((time - m_keyFrames.back().time) /
          (m_keyFrames.back().time - m_keyFrames.front().time));
        time = time - (passCount + 1) *
          (m_keyFrames.back().time - m_keyFrames.front().time);
      } break;
      case WrapMode::PINGPOMG: {
        int passCount = int((time - m_keyFrames.back().time) /
          (m_keyFrames.back().time - m_keyFrames.front().time));
        bool needReverse = (passCount % 2 == 0);
        time = time - (passCount + 1) *
          (m_keyFrames.back().time - m_keyFrames.front().time);
        if (needReverse)
          time = m_keyFrames.front().time + m_keyFrames.back().time - time;
      } break;
      default:
        break;
      }
    }
    else if (time < m_keyFrames.front().time) {  // left warp
      switch (m_preWrapMode) {
      case WrapMode::CLAMP:
        return m_keyFrames.front().value;
        break;
      case WrapMode::REPEAT: {
        int passCount = int((m_keyFrames.front().time - time) /
          (m_keyFrames.back().time - m_keyFrames.front().time));
        time = time + (passCount + 1) *
          (m_keyFrames.back().time - m_keyFrames.front().time);
      } break;
      case WrapMode::PINGPOMG: {
        int passCount = int((m_keyFrames.front().time - time) /
          (m_keyFrames.back().time - m_keyFrames.front().time));
        bool needReverse = (passCount % 2 == 0);
        time = time + (passCount + 1) *
          (m_keyFrames.back().time - m_keyFrames.front().time);
        if (needReverse)
          time = m_keyFrames.front().time + m_keyFrames.back().time - time;
      } break;
      default:
        break;
      }
    }

    int left = 0;
    while (left + 1 < m_keyFrames.size()) {
      if (m_keyFrames[left].time <= time && 
        m_keyFrames[left + 1].time > time) break;
      left++;
    }

    float t_l = 0;
    float t_r = 1;
    while (true) {
      float t = 0.5f * (t_l + t_r);
      Point point = evaluate(m_keyFrames[left], m_keyFrames[left + 1], t);
      float error = std::abs(point.time - time);
      if (error < m_errorTolerence)
        return point.value;
      else if (point.time < time)
        t_l = t;
      else
        t_r = t;
    }
  }

  auto compareKeyFrameByTime(KeyFrame const& lhv, KeyFrame const& rhv) noexcept
    -> bool {
    return lhv.time < rhv.time;
  }

  auto AnimationCurve::sort_all_key_frames() noexcept -> void {
    std::sort(m_keyFrames.begin(), m_keyFrames.end(), compareKeyFrameByTime);
  }

  auto AnimationCurve::evaluate(KeyFrame const& keyframe0, KeyFrame const& keyframe1, float t) noexcept -> Point {
    // regular Cubic Hermite spline with tangents defined by hand
    float dt = keyframe1.time - keyframe0.time;
    float m0 = keyframe0.outTangent * dt;
    float m1 = keyframe1.inTangent * dt;
    float t2 = t * t;
    float t3 = t2 * t;
    float a = 2 * t3 - 3 * t2 + 1;
    float b = t3 - 2 * t2 + t;
    float c = t3 - t2;
    float d = -2 * t3 + 3 * t2;
    float time = a * keyframe0.time + b * m0 + c * m1 + d * keyframe1.time;
    float value = a * keyframe0.value + b * m0 + c * m1 + d * keyframe1.value;
    return Point{ time, value };
  }
}