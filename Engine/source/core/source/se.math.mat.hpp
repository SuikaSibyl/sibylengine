
template <class T>
auto Matrix3x3<T>::row(int i) const noexcept -> Vector3<T> {
  if (i < 0 || i >= 3)
    return Vector3<T>{0.};
  else
    return Vector3<T>{data[i][0], data[i][1], data[i][2]};
}

template <class T>
auto Matrix3x3<T>::col(int i) const noexcept->Vector3<T> {
  if (i < 0 || i >= 3)
    return Vector3<T>{0};
  else
    return Vector3<T>{data[0][i], data[1][i], data[2][i]};
}

template <class T>
Matrix3x3<T>::Matrix3x3(T const mat[3][3]) {
  memcpy(&(data[0][0]), &(mat[0][0]), sizeof(T) * 9);
}

template <class T>
Matrix3x3<T>::Matrix3x3(Vector3<T> x, Vector3<T> y, Vector3<T> z) {
  data[0][0] = x.x;
  data[0][1] = x.y;
  data[0][2] = x.z;
  data[1][0] = y.x;
  data[1][1] = y.y;
  data[1][2] = y.z;
  data[2][0] = z.x;
  data[2][1] = z.y;
  data[2][2] = z.z;
}

template <class T>
Matrix3x3<T>::Matrix3x3(T t00, T t01, T t02, T t10, T t11, T t12, T t20, T t21,
  T t22) {
  data[0][0] = t00;
  data[0][1] = t01;
  data[0][2] = t02;
  data[1][0] = t10;
  data[1][1] = t11;
  data[1][2] = t12;
  data[2][0] = t20;
  data[2][1] = t21;
  data[2][2] = t22;
}

template <class T>
inline auto mul(Matrix3x3<T> const& m, Vector3<T> const& v) noexcept
-> Vector3<T> {
  Vector3<T> s;
  for (size_t i = 0; i < 3; ++i) {
    s.data[i] = m.data[i][0] * v.data[0] + m.data[i][1] * v.data[1] +
      m.data[i][2] * v.data[2];
  }
  return s;
}

template <class T>
auto operator*(Matrix3x3<T> const& m, Vector3<T> const& v) -> Vector3<T> {
  return mul<T>(m, v);
}

template <class T>
auto operator/(Matrix3x3<T> const& m, T s) -> Matrix3x3<T> {
  return Matrix3x3<T>(m.row(0) / s, m.row(1) / s, m.row(2) / s);
}

template <class T>
Matrix3x3<T> adjoint(Matrix3x3<T> const& m) {
  return Matrix3x3<T>(cross(m.row(1), m.row(2)), cross(m.row(2), m.row(0)),
    cross(m.row(0), m.row(1)));
}

template <class T>
Matrix3x3<T> transpose(Matrix3x3<T> const& m) {
  return Matrix3x3<T>(m.col(0), m.col(1), m.col(2));
}

template <class T>
double invert(Matrix3x3<T>& inv, Matrix3x3<T> const& m) {
  Matrix3x3<T> A = adjoint(m);
  double d = dot(A.row(0), m.row(0));
  if (d == 0.0) return 0.0;
  inv = transpose(A) / d;
  return d;
}


template <class T>
auto Matrix4x4<T>::row(int i) const noexcept -> Vector4<T> {
  if (i < 0 || i >= 4)
    return Vector4<T>{0};
  else
    return Vector4<T>{data[i][0], data[i][1], data[i][2], data[i][3]};
}

template <class T>
auto Matrix4x4<T>::col(int i) const noexcept->Vector4<T> {
  if (i < 0 || i >= 4)
    return Vector4<T>{nanf};
  else
    return Vector4<T>{data[0][i], data[1][i], data[2][i], data[3][i]};
}

template <class T>
auto Matrix4x4<T>::set_row(int i, Vector4<T> const& x) noexcept -> void {
  if (i < 0 || i >= 4)
    return;
  else {
    data[i][0] = x.x;
    data[i][1] = x.y;
    data[i][2] = x.z;
    data[i][3] = x.w;
  }
}

template <class T>
Matrix4x4<T>::Matrix4x4(T s) {
  data[0][0] = s;
  data[0][1] = 0;
  data[0][2] = 0;
  data[0][3] = 0;
  data[1][0] = 0;
  data[1][1] = s;
  data[1][2] = 0;
  data[1][3] = 0;
  data[2][0] = 0;
  data[2][1] = 0;
  data[2][2] = s;
  data[2][3] = 0;
  data[3][0] = 0;
  data[3][1] = 0;
  data[3][2] = 0;
  data[3][3] = 1;
}

template <class T>
Matrix4x4<T>::Matrix4x4(T const mat[4][4]) {
  memcpy(&(data[0][0]), &(mat[0][0]), sizeof(T) * 16);
}

template <class T>
Matrix4x4<T>::Matrix4x4(Vector4<T> const& a, Vector4<T> const& b,
  Vector4<T> const& c, Vector4<T> const& d) {
  data[0][0] = a.x;
  data[0][1] = a.y;
  data[0][2] = a.z;
  data[0][3] = a.w;
  data[1][0] = b.x;
  data[1][1] = b.y;
  data[1][2] = b.z;
  data[1][3] = b.w;
  data[2][0] = c.x;
  data[2][1] = c.y;
  data[2][2] = c.z;
  data[2][3] = c.w;
  data[3][0] = d.x;
  data[3][1] = d.y;
  data[3][2] = d.z;
  data[3][3] = d.w;
}

template <class T>
Matrix4x4<T>::Matrix4x4(T t00, T t01, T t02, T t03, T t10, T t11, T t12, T t13,
  T t20, T t21, T t22, T t23, T t30, T t31, T t32,
  T t33) {
  data[0][0] = t00;
  data[0][1] = t01;
  data[0][2] = t02;
  data[0][3] = t03;
  data[1][0] = t10;
  data[1][1] = t11;
  data[1][2] = t12;
  data[1][3] = t13;
  data[2][0] = t20;
  data[2][1] = t21;
  data[2][2] = t22;
  data[2][3] = t23;
  data[3][0] = t30;
  data[3][1] = t31;
  data[3][2] = t32;
  data[3][3] = t33;
}

template <class T>
auto Matrix4x4<T>::operator==(Matrix4x4<T> const& t) const -> bool {
  return (memcmp(&(data[0][0]), &(t.data[0][0]), sizeof(T) * 16) == 0) ? true
    : false;
}

template <class T>
auto Matrix4x4<T>::operator!=(Matrix4x4<T> const& t) const -> bool {
  return !(*this == t);
}

template <class T>
auto Matrix4x4<T>::operator-() const -> Matrix4x4<T> {
  return Matrix4x4<T>{-data[0][0], -data[0][1], -data[0][2], -data[0][3],
    -data[1][0], -data[1][1], -data[1][2], -data[1][3],
    -data[2][0], -data[2][1], -data[2][2], -data[2][3],
    -data[3][0], -data[3][1], -data[3][2], -data[3][3]};
}

template <class T>
auto Matrix4x4<T>::operator+(Matrix4x4<T> const& t) const -> Matrix4x4<T> {
  return Matrix4x4<T>{data[0][0] + t.data[0][0], data[0][1] + t.data[0][1], data[0][2] + t.data[0][2], data[0][3] + t.data[0][3],
    data[1][0] + t.data[1][0], data[1][1] + t.data[1][1], data[1][2] + t.data[1][2], data[1][3] + t.data[1][3],
    data[2][0] + t.data[2][0], data[2][1] + t.data[2][1], data[2][2] + t.data[2][2], data[2][3] + t.data[2][3],
    data[3][0] + t.data[3][0], data[3][1] + t.data[3][1], data[3][2] + t.data[3][2], data[3][3] + t.data[3][3]};
}

template <class T>
auto Matrix4x4<T>::operator-(Matrix4x4<T> const& t) const -> Matrix4x4<T> {
  return Matrix4x4<T>{data[0][0] - t.data[0][0], data[0][1] - t.data[0][1], data[0][2] - t.data[0][2], data[0][3] - t.data[0][3],
    data[1][0] - t.data[1][0], data[1][1] - t.data[1][1], data[1][2] - t.data[1][2], data[1][3] - t.data[1][3],
    data[2][0] - t.data[2][0], data[2][1] - t.data[2][1], data[2][2] - t.data[2][2], data[2][3] - t.data[2][3],
    data[3][0] - t.data[3][0], data[3][1] - t.data[3][1], data[3][2] - t.data[3][2], data[3][3] - t.data[3][3]};
}

template <class T>
Matrix4x4<T>::operator Matrix3x3<T>() const {
  return Matrix3x3<T>{
    data[0][0], data[0][1], data[0][2], data[1][0], data[1][1],
      data[1][2], data[2][0], data[2][1], data[2][2],
  };
}

template <class T>
inline auto transpose(Matrix4x4<T> const& m) noexcept -> Matrix4x4<T> {
  return Matrix4x4<T>(m.data[0][0], m.data[1][0], m.data[2][0], m.data[3][0],
    m.data[0][1], m.data[1][1], m.data[2][1], m.data[3][1],
    m.data[0][2], m.data[1][2], m.data[2][2], m.data[3][2],
    m.data[0][3], m.data[1][3], m.data[2][3], m.data[3][3]);
}

template <class T>
inline auto trace(Matrix4x4<T> const& m) noexcept -> T {
  return m.data[0][0] + m.data[1][1] + m.data[2][2] + m.data[3][3];
}

template <class T>
inline auto mul(Matrix4x4<T> const& m1, Matrix4x4<T> const& m2) noexcept
-> Matrix4x4<T> {
  Matrix4x4<T> s;
  for (size_t i = 0; i < 4; ++i)
    for (size_t j = 0; j < 4; ++j)
      s.data[i][j] =
      m1.data[i][0] * m2.data[0][j] + m1.data[i][1] * m2.data[1][j] +
      m1.data[i][2] * m2.data[2][j] + m1.data[i][3] * m2.data[3][j];
  return s;
}

template <class T>
inline auto operator*(Matrix4x4<T> const& m1, Matrix4x4<T> const& m2) -> Matrix4x4<T> {
  return mul<T>(m1, m2);
}

#define SHUFFLE_PARAM(x, y, z, w) ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))

inline auto _mm_replicate_x_ps(__m128 const& v) noexcept -> __m128 {
  return _mm_shuffle_ps(v, v, SHUFFLE_PARAM(0, 0, 0, 0));
}

inline auto _mm_replicate_y_ps(__m128 const& v) noexcept -> __m128 {
  return _mm_shuffle_ps(v, v, SHUFFLE_PARAM(1, 1, 1, 1));
}

inline auto _mm_replicate_z_ps(__m128 const& v) noexcept -> __m128 {
  return _mm_shuffle_ps(v, v, SHUFFLE_PARAM(2, 2, 2, 2));
}
inline auto _mm_replicate_w_ps(__m128 const& v) noexcept -> __m128 {
  return _mm_shuffle_ps(v, v, SHUFFLE_PARAM(3, 3, 3, 3));
}

template <>
inline auto mul(Matrix4x4<float> const& m1, Matrix4x4<float> const& m2) noexcept
-> Matrix4x4<float> {
  Matrix4x4<float> s;

  __m128 v, result;
  __m128 const mrow0 = _mm_load_ps(&m2.data[0][0]);
  __m128 const mrow1 = _mm_load_ps(&m2.data[1][0]);
  __m128 const mrow2 = _mm_load_ps(&m2.data[2][0]);
  __m128 const mrow3 = _mm_load_ps(&m2.data[3][0]);

  for (int i = 0; i < 4; ++i) {
    v = _mm_load_ps(&m1.data[i][0]);
    result = _mm_mul_ps(_mm_replicate_x_ps(v), mrow0);
    result = _mm_add_ps(_mm_mul_ps(_mm_replicate_y_ps(v), mrow1), result);
    result = _mm_add_ps(_mm_mul_ps(_mm_replicate_z_ps(v), mrow2), result);
    result = _mm_add_ps(_mm_mul_ps(_mm_replicate_w_ps(v), mrow3), result);
    _mm_store_ps(&s.data[i][0], result);
  }
  return s;
}

template <>
inline auto operator*(Matrix4x4<float> const& m1, Matrix4x4<float> const& m2)
-> Matrix4x4<float> {
  return mul<float>(m1, m2);
}

template <class T>
inline auto mul(Matrix4x4<T> const& m, Vector4<T> const& v) noexcept
-> Vector4<T> {
  Vector4<T> s;
  for (size_t i = 0; i < 4; ++i) {
    s.data[i] = m.data[i][0] * v.data[0] + m.data[i][1] * v.data[1] +
      m.data[i][2] * v.data[2] + m.data[i][3] * v.data[3];
  }
  return s;
}

template <class T>
inline auto operator*(Matrix4x4<T> const& m, Vector4<T> const& v)->Vector4<T> {
  return mul<T>(m, v);
}

template <>
inline auto mul(Matrix4x4<float> const& m, Vector4<float> const& v) noexcept
-> Vector4<float> {
  Vector4<float> s;
  __m128 mrow0, mrow1, mrow2, mrow3;
  __m128 acc_0, acc_1, acc_2, acc_3;
  __m128 const vcol = _mm_load_ps(&v.data[0]);

  mrow0 = _mm_load_ps(&(m.data[0][0]));
  mrow1 = _mm_load_ps(&(m.data[1][0]));
  mrow2 = _mm_load_ps(&(m.data[2][0]));
  mrow3 = _mm_load_ps(&(m.data[3][0]));

  acc_0 = _mm_mul_ps(mrow0, vcol);
  acc_1 = _mm_mul_ps(mrow1, vcol);
  acc_2 = _mm_mul_ps(mrow2, vcol);
  acc_3 = _mm_mul_ps(mrow3, vcol);

  acc_0 = _mm_hadd_ps(acc_0, acc_1);
  acc_2 = _mm_hadd_ps(acc_2, acc_3);
  acc_0 = _mm_hadd_ps(acc_0, acc_2);
  _mm_store_ps(&s.data[0], acc_0);
  return s;
}

template <>
inline auto operator*(Matrix4x4<float> const& m, Vector4<float> const& v)
-> Vector4<float> {
  return mul<float>(m, v);
}

template <class T>
inline auto determinant(Matrix4x4<T> const& m) noexcept -> double {
  double Result[4][4];
  double tmp[12]; /* temp array for pairs */
  double src[16]; /* array of transpose source matrix */
  double det;     /* determinant */
  /* transpose matrix */
  for (int i = 0; i < 4; i++) {
    src[i + 0] = m.data[i][0];
    src[i + 4] = m.data[i][1];
    src[i + 8] = m.data[i][2];
    src[i + 12] = m.data[i][3];
  }
  /* calculate pairs for first 8 elements (cofactors) */
  tmp[0] = src[10] * src[15];
  tmp[1] = src[11] * src[14];
  tmp[2] = src[9] * src[15];
  tmp[3] = src[11] * src[13];
  tmp[4] = src[9] * src[14];
  tmp[5] = src[10] * src[13];
  tmp[6] = src[8] * src[15];
  tmp[7] = src[11] * src[12];
  tmp[8] = src[8] * src[14];
  tmp[9] = src[10] * src[12];
  tmp[10] = src[8] * src[13];
  tmp[11] = src[9] * src[12];
  /* calculate first 8 elements (cofactors) */
  Result[0][0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
  Result[0][0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
  Result[0][1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
  Result[0][1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
  Result[0][2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
  Result[0][2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
  Result[0][3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
  Result[0][3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
  Result[1][0] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
  Result[1][0] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
  Result[1][1] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
  Result[1][1] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
  Result[1][2] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
  Result[1][2] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
  Result[1][3] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
  Result[1][3] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];
  /* calculate pairs for second 8 elements (cofactors) */
  tmp[0] = src[2] * src[7];
  tmp[1] = src[3] * src[6];
  tmp[2] = src[1] * src[7];
  tmp[3] = src[3] * src[5];
  tmp[4] = src[1] * src[6];
  tmp[5] = src[2] * src[5];

  tmp[6] = src[0] * src[7];
  tmp[7] = src[3] * src[4];
  tmp[8] = src[0] * src[6];
  tmp[9] = src[2] * src[4];
  tmp[10] = src[0] * src[5];
  tmp[11] = src[1] * src[4];
  /* calculate second 8 elements (cofactors) */
  Result[2][0] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
  Result[2][0] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
  Result[2][1] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
  Result[2][1] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
  Result[2][2] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
  Result[2][2] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
  Result[2][3] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
  Result[2][3] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
  Result[3][0] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
  Result[3][0] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
  Result[3][1] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
  Result[3][1] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
  Result[3][2] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
  Result[3][2] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
  Result[3][3] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
  Result[3][3] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];
  /* calculate determinant */
  det = src[0] * Result[0][0] + src[1] * Result[0][1] + src[2] * Result[0][2] +
    src[3] * Result[0][3];
  return det;
}

template <class T>
inline auto adjoint(Matrix4x4<T> const& m) noexcept -> Matrix4x4<T> {
  Matrix4x4<T> A;
  A.set_row(0, cross(m.row(1), m.row(2), m.row(3)));
  A.set_row(1, cross(-m.row(0), m.row(2), m.row(3)));
  A.set_row(2, cross(m.row(0), m.row(1), m.row(3)));
  A.set_row(3, cross(-m.row(0), m.row(1), m.row(2)));
  return A;
}

template <class T>
inline auto inverse(Matrix4x4<T> const& m) noexcept -> Matrix4x4<T> {
  //  Inversion by Cramer's rule.  Code taken from an Intel publication
  double Result[4][4];
  double tmp[12]; /* temp array for pairs */
  double src[16]; /* array of transpose source matrix */
  double det;     /* determinant */
  /* transpose matrix */
  for (int i = 0; i < 4; i++) {
    src[i + 0] = m.data[i][0];
    src[i + 4] = m.data[i][1];
    src[i + 8] = m.data[i][2];
    src[i + 12] = m.data[i][3];
  }
  /* calculate pairs for first 8 elements (cofactors) */
  tmp[0] = src[10] * src[15];
  tmp[1] = src[11] * src[14];
  tmp[2] = src[9] * src[15];
  tmp[3] = src[11] * src[13];
  tmp[4] = src[9] * src[14];
  tmp[5] = src[10] * src[13];
  tmp[6] = src[8] * src[15];
  tmp[7] = src[11] * src[12];
  tmp[8] = src[8] * src[14];
  tmp[9] = src[10] * src[12];
  tmp[10] = src[8] * src[13];
  tmp[11] = src[9] * src[12];
  /* calculate first 8 elements (cofactors) */
  Result[0][0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
  Result[0][0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
  Result[0][1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
  Result[0][1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
  Result[0][2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
  Result[0][2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
  Result[0][3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
  Result[0][3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
  Result[1][0] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
  Result[1][0] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
  Result[1][1] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
  Result[1][1] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
  Result[1][2] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
  Result[1][2] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
  Result[1][3] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
  Result[1][3] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];
  /* calculate pairs for second 8 elements (cofactors) */
  tmp[0] = src[2] * src[7];
  tmp[1] = src[3] * src[6];
  tmp[2] = src[1] * src[7];
  tmp[3] = src[3] * src[5];
  tmp[4] = src[1] * src[6];
  tmp[5] = src[2] * src[5];

  tmp[6] = src[0] * src[7];
  tmp[7] = src[3] * src[4];
  tmp[8] = src[0] * src[6];
  tmp[9] = src[2] * src[4];
  tmp[10] = src[0] * src[5];
  tmp[11] = src[1] * src[4];
  /* calculate second 8 elements (cofactors) */
  Result[2][0] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
  Result[2][0] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
  Result[2][1] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
  Result[2][1] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
  Result[2][2] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
  Result[2][2] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
  Result[2][3] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
  Result[2][3] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
  Result[3][0] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
  Result[3][0] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
  Result[3][1] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
  Result[3][1] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
  Result[3][2] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
  Result[3][2] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
  Result[3][3] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
  Result[3][3] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];
  /* calculate determinant */
  det = src[0] * Result[0][0] + src[1] * Result[0][1] + src[2] * Result[0][2] +
    src[3] * Result[0][3];
  /* calculate matrix inverse */
  det = 1.0f / det;

  Matrix4x4<T> result;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      result.data[i][j] = float(Result[i][j] * det);
    }
  }
  return result;
}

template <class T>
inline auto Matrix4x4<T>::translate(Vector3<T> const& delta) noexcept
-> Matrix4x4<T> {
  return Matrix4x4<T>(1, 0, 0, delta.x, 0, 1, 0, delta.y, 0, 0, 1, delta.z, 0,
    0, 0, 1);
}

template <class T>
inline auto Matrix4x4<T>::scale(float x, float y, float z) noexcept
-> Matrix4x4<T> {
  return Matrix4x4<T>(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
}

template <class T>
inline auto Matrix4x4<T>::scale(Vector3<T> const& scale) noexcept
-> Matrix4x4<T> {
  return Matrix4x4<T>(scale.x, 0, 0, 0, 0, scale.y, 0, 0, 0, 0, scale.z, 0, 0,
    0, 0, 1);
}

template <class T>
inline auto Matrix4x4<T>::rotateX(float theta) noexcept -> Matrix4x4<T> {
  float sinTheta = std::sin(radians(theta));
  float cosTheta = std::cos(radians(theta));
  return Matrix4x4<T>(1, 0, 0, 0, 0, cosTheta, -sinTheta, 0, 0, sinTheta,
    cosTheta, 0, 0, 0, 0, 1);
}

template <class T>
inline auto Matrix4x4<T>::rotateY(float theta) noexcept -> Matrix4x4<T> {
  float sinTheta = std::sin(radians(theta));
  float cosTheta = std::cos(radians(theta));
  Matrix4x4<T> m(T(cosTheta), 0, T(sinTheta), 0, 0, 1, 0, 0, T(-sinTheta), 0,
    T(cosTheta), 0, 0, 0, 0, 1);
  return m;
}

template <class T>
inline auto Matrix4x4<T>::rotateZ(float theta) noexcept -> Matrix4x4<T> {
  float sinTheta = std::sin(radians(theta));
  float cosTheta = std::cos(radians(theta));
  Matrix4x4<T> m(cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 1);
  return m;
}

template <class T>
inline auto Matrix4x4<T>::rotate(float theta, vec3 const& axis) noexcept
-> Matrix4x4<T> {
  vec3 a = normalize(axis);
  float sinTheta = std::sin(radians(theta));
  float cosTheta = std::cos(radians(theta));
  Matrix4x4<T> m;
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
  return m;
}

template <class T>
auto Matrix4x4<T>::to_string() const noexcept ->std::string {
  return fmt::format("Mat4[[{}, {}, {}, {}],\n[{}, {}, {}, {}],\n[{}, {}, {}, {}],\n[{}, {}, {}, {}]]",
    std::to_string(data[0][0]), std::to_string(data[0][1]), std::to_string(data[0][2]), std::to_string(data[0][3]),
    std::to_string(data[1][0]), std::to_string(data[1][1]), std::to_string(data[1][2]), std::to_string(data[1][3]),
    std::to_string(data[2][0]), std::to_string(data[2][1]), std::to_string(data[2][2]), std::to_string(data[2][3]),
    std::to_string(data[3][0]), std::to_string(data[3][1]), std::to_string(data[3][2]), std::to_string(data[3][3]));
}
