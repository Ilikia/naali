/** @file float3x3.h
    @author Jukka Jyl�nki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 

    @brief A 3-by-3 matrix for linear operations in 3D space.
*/
#pragma once

#ifdef MATH_ENABLE_STL_SUPPORT
#include <ostream>
#include <cassert>
#endif

#include "MathFwd.h"
#include "MatrixProxy.h"
#include "CoordinateAxisConvention.h"

#ifdef OGRE_INTEROP
#include <OgreMatrix3.h>
#endif

#ifdef BULLET_INTEROP
#include "LinearMath/btMatrix3x3.h"
#endif

class float3x3;
class Plane;

/// A 3-by-3 matrix for linear transformations of 3D geometry.
/** This matrix can represent any kind of linear transformations of 3D geometry, which include rotation, 
    scale, shear, mirroring and orthographic projection. A 3x3 matrix cannot represent translation (which requires
    a 3x4 matrix), or perspective projection (which requires a 4x4 matrix).

    The elements of this matrix are

        m_00, m_01, m_02
        m_10, m_11, m_12
        m_20, m_21, m_22

    The element m_yx is the value on the row y and column x.
    You can access m_yx using the double-bracket notation m[y][x], or using the member function m.At(y, x);

    @note The member functions in this class use the convention that transforms are applied to vectors in the form 
    M * v. This means that "float3x3 M, M1, M2; M = M1 * M2;" gives a transformation M that applies M2 first, followed
    by M1 second, i.e. M * v = M1 * M2 * v = M1 * (M2 * v). This is the convention commonly used with OpenGL. The
    opposing convention (v * M) is commonly used with Direct3D.

    @note This class uses row-major storage, which means that the elements are packed in memory in order 
     m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], ...
    The elements for a single row of the matrix hold successive memory addresses. This is the same memory layout as 
     with C++ multidimensional arrays.

    Contrast this with column-major storage, in which the elements are packed in the memory in 
    order m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], ...
    There the elements for a single column of the matrix hold successive memory addresses. 
    This is exactly opposite from the standard C++ multidimensional arrays, since if you have e.g.
    int v[10][10], then v[0][9] comes in memory right before v[1][0]. ( [0][0], [0][1], [0][2], ... [1][0], [1][1], ...) */
class float3x3
{
public:
    /// Specifies the height of this matrix.
    /** [Category: Data] */
    enum { Rows = 3 };

    /// Specifies the width of this matrix.
    enum { Cols = 3 };

    /// Stores the data in this matrix in row-major format. [noscript]
    /** [Category: Data] */
    float v[Rows][Cols];

    /// A constant matrix that has zeroes in all its entries.
    /** [Category: Data] */
    static const float3x3 zero;

    /// A constant matrix that is the identity.
    /** The identity matrix looks like the following:
           1 0 0
           0 1 0
           0 0 1
        Transforming a vector by the identity matrix is like multiplying a number by one, i.e. the vector is not changed. */
    static const float3x3 identity;

    /// A compile-time constant float3x3 which has NaN in each element.
    /// For this constant, each element has the value of quiet NaN, or Not-A-Number.
    /// @note Never compare a float3x3 to this value! Due to how IEEE floats work, for each float x, both the expression "x == nan" and "x == nan" returns false!
    ///       That is, nothing is equal to NaN, not even NaN itself!
    static const float3x3 nan;

    /// Creates a new float3x3 with uninitialized member values.
    /** [Category: Create] [opaque-qtscript] */
    float3x3() {}

    /// The copy-ctor for float3x3 is the trivial copy-ctor, but it is explicitly written to be able to automatically pick up this function for QtScript bindings.
    float3x3(const float3x3 &rhs) { Set(rhs); }

    /// Constructs a new float3x3 by explicitly specifying all the matrix elements.
    /// The elements are specified in row-major format, i.e. the first row first followed by the second and third row.
    /// E.g. The element _10 denotes the scalar at second (index 1) row, first (index 0) column.
    float3x3(float _00, float _01, float _02,
             float _10, float _11, float _12,
             float _20, float _21, float _22);

    /// Constructs the matrix by explicitly specifying the four column vectors.
    /** @param col0 The first column. If this matrix represents a change-of-basis transformation, this parameter is the world-space
        direction of the local X axis.
        @param col1 The second column. If this matrix represents a change-of-basis transformation, this parameter is the world-space
        direction of the local Y axis.
        @param col2 The third column. If this matrix represents a change-of-basis transformation, this parameter is the world-space
        direction of the local Z axis. */
    float3x3(const float3 &col0, const float3 &col1, const float3 &col2);

    /// Constructs this float3x3 from the given quaternion.
    explicit float3x3(const Quat &orientation);

    /// Creates a new float3x3 that rotates about one of the principal axes by the given angle. [indexTitle: RotateX/Y/Z]
    /** Calling RotateX, RotateY or RotateZ is slightly faster than calling the more generic RotateAxisAngle function. */
    static float3x3 RotateX(float angleRadians);
    /** [similarOverload: RotateX] [hideIndex] */
    static float3x3 RotateY(float angleRadians);
    /** [similarOverload: RotateX] [hideIndex] */
    static float3x3 RotateZ(float angleRadians);

    /// Creates a new float3x3 that rotates about the given axis by the given angle.
    static float3x3 RotateAxisAngle(const float3 &axisDirection, float angleRadians);

    /// Creates a new float3x3 that rotates sourceDirection vector to coincide with the targetDirection vector.
    /** @note There are infinite such rotations - this function returns the rotation that has the shortest angle
        (when decomposed to axis-angle notation). */
    static float3x3 RotateFromTo(const float3 &sourceDirection, const float3 &targetDirection);

    /// Creates a new float3x3 that performs the rotation expressed by the given quaternion.
    static float3x3 FromQuat(const Quat &orientation);

    /// Converts this rotation matrix to a quaternion.
    /// This function assumes that the matrix is orthonormal (no shear or scaling) and does not perform any mirroring (determinant > 0).
    Quat ToQuat() const;

    /// Creates a new float3x3 as a combination of rotation and scale.
    /** This function creates a new float3x3 M of the form M = R * S, where R is a
        rotation matrix and S a scale matrix. Transforming a vector v using this matrix computes the vector
        v' == M * v == R*S*v == (R * (S * v)), which means that the scale operation is applied to the
        vector first, followed by rotation and finally translation. */
    static float3x3 FromRS(const Quat &rotate, const float3 &scale);
    static float3x3 FromRS(const float3x3 &rotate, const float3 &scale);

    /// Creates a new float3x3 from the given sequence of Euler rotation angles (in radians).
    /** The FromEulerABC function returns a matrix M = A(ea) * B(eb) * C(ec). Rotation
        C is applied first, followed by B and then A. [indexTitle: FromEuler***] */
    static float3x3 FromEulerXYX(float ex, float ey, float ex2);
    static float3x3 FromEulerXZX(float ex, float ez, float ex2); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerYXY(float ey, float ex, float ey2); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerYZY(float ey, float ez, float ey2); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerZXZ(float ez, float ex, float ez2); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerZYZ(float ez, float ey, float ez2); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerXYZ(float ex, float ey, float ez); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerXZY(float ex, float ez, float ey); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerYXZ(float ey, float ex, float ez); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerYZX(float ey, float ez, float ex); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerZXY(float ez, float ex, float ey); ///< [similarOverload: FromEulerXYX] [hideIndex]
    static float3x3 FromEulerZYX(float ez, float ey, float ex); ///< [similarOverload: FromEulerXYX] [hideIndex]

    /// Creates a new transformation matrix that scales by the given factors.
    /// This matrix scales with respect to origin.
    static ScaleOp Scale(float sx, float sy, float sz);
    static ScaleOp Scale(const float3 &scale);

    /// Creates a new float3x3 that scales points along the given axis.
    /** @param axis A normalized direction vector that specifies the direction of scaling.
        @param scalingFactor The amount of scaling to apply along the specified axis. */
    static float3x3 ScaleAlongAxis(const float3 &axis, float scalingFactor);

    /// Creates a new float3x3 that performs uniform scaling by the given amount.
    static ScaleOp UniformScale(float uniformScale);

    /// Returns the scaling performed by this matrix.
    /// GetScale().x specifies the amount of scaling applied to the local x direction vector when it is transformed by this matrix.
    /// i.e. GetScale()[i] equals Col(i).Length();
    float3 GetScale() const;

    /// Produces a matrix that shears along a principal axis.
    /** The shear matrix offsets the two other axes according to the 
        position of the point along the shear axis. [indexTitle: ShearX/Y/Z] */
    static float3x3 ShearX(float yFactor, float zFactor);
    static float3x3 ShearY(float xFactor, float zFactor); ///< [similarOverload: ShearX] [hideIndex]
    static float3x3 ShearZ(float xFactor, float yFactor); ///< [similarOverload: ShearX] [hideIndex]

    /// Creates a new matrix that reflects along the given plane.
    /** Points lying on one side of the plane will end up at the opposite side of the plane, at the same distance of the plane
        they were. */
    static float3x3 Reflect(const Plane &p);

    /// Creates a new float3x3 that performs orthographic projection. [indexTitle: MakeOrthographicProjection/YZ/XZ/XY]
    static float3x3 MakeOrthographicProjection(float nearPlaneDistance, float farPlaneDistance, float horizontalViewportSize, float verticalViewportSize);
    static float3x3 MakeOrthographicProjection(const Plane &target);
    static float3x3 MakeOrthographicProjectionYZ(); ///< [similarOverload: MakeOrthographicProjection] [hideIndex]
    static float3x3 MakeOrthographicProjectionXZ(); ///< [similarOverload: MakeOrthographicProjection] [hideIndex]
    static float3x3 MakeOrthographicProjectionXY(); ///< [similarOverload: MakeOrthographicProjection] [hideIndex]

    /// Computes the covariance matrix of the given set of data points.
//    static float3x3 CovarianceMatrix(const float3 *pointArray, int numPoints);

    /// Returns the given element. [noscript]
    /** Returns a reference to the element at m[row][col] (or "m[y][x]").
        Remember that zero-based indexing is used, so m[0][0] is the upper-left element of this matrix.
        @note You can use the index notation to set elements of the matrix, e.g. m[0][1] = 5.f;
        @note MatrixProxy is a temporary helper class. Do not store references to it, but always
        directly dereference it with the [] operator.
        \example m[0][2] Returns the last element on the first row.
        [Category: Access] */
    MatrixProxy<Cols> &operator[](int row);
    const MatrixProxy<Cols> &operator[](int row) const;

    /// Returns the given element. [noscript]
    /** This function returns the element of this matrix at (row, col)==(i, j)==(y, x).
        If you have a non-const object, you can set values of this matrix through this 
        reference, using the notation m.At(row, col) = someValue; */
    float &At(int row, int col);
    CONST_WIN32 float At(int row, int col) const;

    /// Returns the given row. [noscript]
    /** @param row The zero-based index [0, 3] of the row to get. */
    float3 &Row(int row);
    const float3 &Row(int row) const;

    /// Returns the given column.
    /** @param col The zero-based index [0, 3] of the column to get. */
    CONST_WIN32 float3 Col(int col) const;

    /// Returns the main diagonal.
    /** The main diagonal consists of the elements at m[0][0], m[1][1], m[2][2]. */
    CONST_WIN32 float3 Diagonal() const;

    /// Scales the given row by a scalar.
    void ScaleRow(int row, float scalar);

    /// Scales the given column by a scalar.
    void ScaleCol(int col, float scalar);

    // Returns the local right axis in the post-transformed coordinate space, according to the given convention.
    // @note The returned vector might not be normalized if this matrix contains scaling.
    // @note The basis returned by (Right, Up, Forward) might not be of the same handedness as the
    //       pre-transformed coordinate system, if the matrix contained reflection.
//    template<typename Convention = XposRight_YposUp_ZposForward> float3 Right() const;

    // Returns the local up axis in the post-transformed coordinate space, according to the given convention.
    // @note The returned vector might not be normalized if this matrix contains scaling.
    // @note The basis returned by (Right, Up, Forward) might not be of the same handedness as the
    //       pre-transformed coordinate system, if the matrix contained reflection.
//    template<typename Convention = XposRight_YposUp_ZposForward> float3 Up() const;

    // Returns the local forward axis in the post-transformed coordinate space, according to the given convention.
    // @note The returned vector might not be normalized if this matrix contains scaling.
    // @note The basis returned by (Right, Up, Forward) might not be of the same handedness as the
    //       pre-transformed coordinate system, if the matrix contained reflection.
//    template<typename Convention = XposRight_YposUp_ZposForward> float3 Forward() const;

    /// Returns the local +X/+Y/+Z axis in world space.
    /** This is the same as transforming the vector (1,0,0) by this matrix. [indexTitle: PositiveX/Y/Z] */
    float3 WorldX() const;
    /// Returns the local +Y axis in world space.
    /** This is the same as transforming the vector (0,1,0) by this matrix. [similarOverload: PositiveX] [hideIndex] */
    float3 WorldY() const;
    /// Returns the local +Z axis in world space.
    /** This is the same as transforming the vector (0,0,1) by this matrix. [similarOverload: PositiveX] [hideIndex] */
    float3 WorldZ() const;

    /// Accesses this structure as a float array.
    /// @return A pointer to the upper-left element. The data is contiguous in memory.
    /// ptr[0] gives the element [0][0], ptr[1] is [0][1], ptr[2] is [0][2].
    /// ptr[4] == [1][0], ptr[5] == [1][1], ..., and finally, ptr[15] == [3][3].
    float *ptr();
    /// @return A pointer to the upper-left element . The data is contiguous in memory.
    const float *ptr() const;

    /// Sets the values of the given row.
    /** @param row The index of the row to set, in the range [0-2].
        [Category: Set] */
    void SetRow(int row, float x, float y, float z);
    void SetRow(int row, const float3 &rowVector);
    void SetRow(int row, const float *data);

    /// Sets the values of the given column.
    /// @param column The index of the column to set, in the range [0-2].
    void SetCol(int column, float x, float y, float z);
    void SetCol(int column, const float3 &columnVector);
    void SetCol(int column, const float *data);

    /// Sets all values of this matrix.
    void Set(float _00, float _01, float _02,
             float _10, float _11, float _12,
             float _20, float _21, float _22);

    /// Sets this to be a copy of the matrix rhs.
    void Set(const float3x3 &rhs);

    /// Sets all values of this matrix.
    /// @param values The values in this array will be copied over to this matrix. The source must contain 9 floats in row-major order (the same
    ///        order as the Set() function above has its input parameters in).
    void Set(const float *values);

    /// Sets a single element of this matrix.
    /// @param row The row index of the element to set, in the range [0-2].
    /// @param col The col index of the element to set, in the range [0-2].
    void Set(int row, int col, float value);

    /// Sets this matrix to equal the identity.
    void SetIdentity();

    /// Swaps two columns.
    void SwapColumns(int col1, int col2);

    /// Swaps two rows.
    void SwapRows(int row1, int row2);

    /// Sets this matrix to perform rotation about the positive X axis which passes through
    /// the origin. [similarOverload: SetRotatePart] [hideIndex]
    void SetRotatePartX(float angleRadians);
    /// Sets this matrix to perform rotation about the positive Y axis. [similarOverload: SetRotatePart] [hideIndex]
    void SetRotatePartY(float angleRadians);
    /// Sets this matrix to perform rotation about the positive Z axis. [similarOverload: SetRotatePart] [hideIndex]
    void SetRotatePartZ(float angleRadians);

    /// Sets this matrix to perform rotation about the given axis and angle. [indexTitle: SetRotatePart/X/Y/Z]
    void SetRotatePart(const float3 &axisDirection, float angleRadians);
    /// Sets this matrix to perform the rotation expressed by the given quaternion. 
    void SetRotatePart(const Quat &orientation);

    float3x3 &operator =(const Quat &rhs);
    float3x3 &operator =(const float3x3 &rhs);

    /// Computes the determinant of this matrix. 
    /** If the determinant is nonzero, this matrix is invertible.
        If the determinant is negative, this matrix performs reflection about some axis.
        From http://msdn.microsoft.com/en-us/library/bb204853(VS.85).aspx :
        "If the determinant is positive, the basis is said to be "positively" oriented (or right-handed). 
        If the determinant is negative, the basis is said to be "negatively" oriented (or left-handed)."
        [Category: Compute] */
    float Determinant() const;

//    float2x2 SubMatrix(int i, int j) const;

    /// Returns the adjugate of this matrix.
//    float3x3 Adjugate() const;

    /// Inverts this matrix using Cramer's rule.
    /// @return Returns true on success, false otherwise.
    bool Inverse();

    /// Returns an inverted copy of this matrix. This function uses the Cramer's rule.
    /// If this matrix does not have an inverse, returns the matrix that was the result of running
    /// Gauss's method on the matrix.
    float3x3 Inverted() const;

    /// Inverts a matrix that is a concatenation of only rotation, reflection and scale operations. 
    /// To call this function, the matrix can not contain any projection or shearing operations
    /// about any of the axes (the matrix must preserve all angles, i.e. have orthogonal column vectors). 
    /// This function is faster than the generic matrix Inverse() function.
    /// Returns true on success. On failure, the matrix is not modified. This function fails if any of the
    /// elements of this vector are not finite, or if the matrix contains zero scale.
    bool InverseOrthogonal();

    /// Inverts a matrix that is a concatenation of only rotation, reflection and uniform scale operations. 
    /// To call this function, the matrix can not contain any projection, shearing or non-uniform scaling 
    /// operations about any of the axes.
    /// This function is faster than InverseOrthogonal().
    /// Returns true on success. On failure, the matrix is not modified. This function fails if it contains
    /// a null column vector or if any of the elements of this vector are not finite.
    bool InverseOrthogonalUniformScale();

    /// Inverts a matrix that contains only rotation and/or reflection.
    /// To call this function, the matrix can not contain any shearing, scaling or projection operations
    /// about any of the axes. Always succeeds (or rather, fails to detect if it fails).
    /// This function is faster than InverseOrthogonalUniformScale().
    void InverseOrthonormal();

    /// Transposes this matrix.
    /// This operation swaps all elements with respect to the diagonal.
    void Transpose();

    /// Returns a transposed copy of this matrix.
    float3x3 Transposed() const;

    /// Computes the inverse transpose of this matrix in-place. 
    /** Use the inverse transpose to transform covariant vectors (normal vectors). */
    bool InverseTranspose();

    /// Returns the inverse transpose of this matrix. 
    /** Use that matrix to transform covariant vectors (normal vectors). */
    float3x3 InverseTransposed() const;

    /// Returns the sum of the diagonal elements of this matrix.
    float Trace() const;

    /// Orthonormalizes the basis formed by the column vectors of this matrix.
    void Orthonormalize(int firstColumn, int secondColumn, int thirdColumn);

    /// Removes the scaling performed by this matrix. That is, decomposes this matrix M into a form M = M' * S, where
    /// M' has unitary column vectors and S is a diagonal matrix. Then replaces this matrix with M'.
    /// @note This function assumes that this matrix does not contain projection (the fourth row of this matrix is [0 0 0 1]).
    /// @note This function does not remove reflection (-1 scale along some axis).
    void RemoveScale();

    /// Transforms the given 3-vector by this matrix M, i.e. returns M * (x, y, z). 
    float3 Transform(const float3 &vector) const;
    float3 Transform(float x, float y, float z) const;

    /// Transforms the given 3-vector by this matrix M so that the vector occurs on the left-hand side, i.e.
    /// computes the product lhs * M. This is against the common convention used by this class when transforming
    /// geometrical objects, but this operation is still occasionally useful for other purposes.
    /// (Remember that M * v != v * M in general).
    float3 TransformLeft(const float3 &lhs) const;

    /// Transforms the given 4-vector by this matrix M, i.e. returns M * (x, y, z, w). 
    /// This function ignores the w component of the given input vector. This component is assumed to be either 0 or 1.
    float4 Transform(const float4 &vector) const;

    /// Performs a batch transform of the given array.
    void BatchTransform(float3 *pointArray, int numPoints) const;

    /// Performs a batch transform of the given array.
    void BatchTransform(float3 *pointArray, int numPoints, int stride) const;

    /// Performs a batch transform of the given array.
    /// This function ignores the w component of the input vectors. These components are assumed to be either 0 or 1.
    void BatchTransform(float4 *vectorArray, int numVectors) const;

    /// Performs a batch transform of the given array.
    /// This function ignores the w component of the input vectors. These components are assumed to be either 0 or 1.
    void BatchTransform(float4 *vectorArray, int numVectors, int stride) const;

    /// Multiplies the two matrices.
    float3x3 operator *(const float3x3 &rhs) const;

    /// Converts the quaternion to a float3x3 and multiplies the two matrices together.
    float3x3 operator *(const Quat &rhs) const;

    /// Transforms the given vector by this matrix (in the order M * v).
    float3 operator *(const float3 &rhs) const;

    /// Transforms the given vector by this matrix (in the order M * v).
    /// This function ignores the w component of the given input vector. This component is assumed to be either 0 or 1.
    float4 operator *(const float4 &rhs) const;

    float3x3 operator *(float scalar) const;
    float3x3 operator /(float scalar) const;
    float3x3 operator +(const float3x3 &rhs) const;
    float3x3 operator -(const float3x3 &rhs) const;
    float3x3 operator -() const;

    float3x3 &operator *=(float scalar);
    float3x3 &operator /=(float scalar);
    float3x3 &operator +=(const float3x3 &rhs);
    float3x3 &operator -=(const float3x3 &rhs);

    /// Tests if this matrix does not contain any NaNs or infs.
    /** @return Returns true if the entries of this float3x3 are all finite, and do not contain NaN or infs. 
        [Category: Examine] */
    bool IsFinite() const;

    /// Tests if this is the identity matrix.
    /** @return Returns true if this matrix is the identity matrix, up to the given epsilon. */
    bool IsIdentity(float epsilon = 1e-3f) const;

    /// Tests if this matrix is in lower triangular form.
    /** @return Returns true if this matrix is in lower triangular form, up to the given epsilon. */
    bool IsLowerTriangular(float epsilon = 1e-3f) const;

    /// Tests if this matrix is in upper triangular form.
    /** @return Returns true if this matrix is in upper triangular form, up to the given epsilon. */
    bool IsUpperTriangular(float epsilon = 1e-3f) const;

    /// Tests if this matrix has an inverse.
    /** @return Returns true if this matrix can be inverted, up to the given epsilon. */
    bool IsInvertible(float epsilon = 1e-3f) const;

    /// Tests if this matrix is symmetric (M == M^T).
    /** The test compares the elements for equality, up to the given epsilon. A matrix is symmetric if it is its own transpose. */
    bool IsSymmetric(float epsilon = 1e-3f) const;

    /// Tests if this matrix is skew-symmetric (M == -M^T).
    /** The test compares the floating point elements of this matrix up to the given epsilon. A matrix M is skew-symmetric 
        the identity M=-M^T holds. */
    bool IsSkewSymmetric(float epsilon = 1e-3f) const;

    /// Returns true if this matrix does not perform any scaling.
    /** A matrix does not do any scaling if the column vectors of this 
        matrix are normalized in length, compared to the given epsilon. Note that this matrix may still perform
        reflection, i.e. it has a -1 scale along some axis.
        @note This function only examines the upper 3-by-3 part of this matrix.
        @note This function assumes that this matrix does not contain projection (the fourth row of this matrix is [0 0 0 1]). */
    bool HasUnitaryScale(float epsilonSq = 1e-6f) const;

    /// Returns true if this matrix performs a reflection along some plane.
    /** In 3D space, an even number of reflections corresponds to a rotation about some axis, so a matrix consisting of
        an odd number of consecutive mirror operations can only reflect about one axis. A matrix that contains reflection reverses 
        the handedness of the coordinate system. This function tests if this matrix 
        does perform mirroring. This occurs iff this matrix has a negative determinant. */
    bool HasNegativeScale() const;

    /// Returns true if this matrix contains only uniform scaling, compared to the given epsilon.
    /// @note If the matrix does not really do any scaling, this function returns true (scaling uniformly by a factor of 1).
    bool HasUniformScale(float epsilonSq = 1e-6f) const;

    /// Returns true if the column and row vectors of this matrix are all perpendicular to each other.
    /** @note In math terms, a matrix is orthogonal iff its column and row vectors are orthogonal unit vectors.
        In the terms of this library however, a matrix is orthogonal iff its column and row vectors are orthogonal (no need to be unitary).
        If this function returns true, one can use InverseOrthogonal() to compute the inverse of this matrix, instead
        of the more expensive general Inverse(). If additionally IsUnitaryScale() returns true, then
        it is possible to use InverseOrthonormal() to compute the inverse, which is the fastest way to compute
        an inverse. */
    bool IsOrthogonal(float epsilon = 1e-3f) const;

    /// Returns true if the column and row vectors of this matrix form an orthonormal set.
    /// @note In math terms, there does not exist such a thing as 'orthonormal matrix'. In math terms, a matrix 
    /// is orthogonal iff its column and row vectors are orthogonal *unit* vectors.
    /// In the terms of this library however, a matrix is orthogonal iff its column and row vectors are orthogonal (no need to be unitary),
    /// and a matrix is orthonormal if the column and row vectors are orthonormal.
    bool IsOrthonormal(float epsilon = 1e-3f) const;

    /// Returns true if this float3x3 is equal to the given float3x3, up to given per-element epsilon.
    bool Equals(const float3x3 &other, float epsilon = 1e-3f) const;

#ifdef MATH_ENABLE_STL_SUPPORT
    /// Returns "(m00, m01, m02; m10, m11, m12; m20, m21, m22)".
    std::string ToString() const;

    std::string ToString2() const;
#endif

    /// Extracts the rotation part of this matrix into Euler rotation angles (in radians).
    /// @note It is better to thinkg about the returned float3 as an array of three floats, and
    /// not as a triple of xyz, because e.g. the .y component returned by ToEulerYXZ() does
    /// not return the amount of rotation about the y axis, but contains the amount of rotation
    /// in the second axis, in this case the x axis.
    /// [Category: Extract] [indexTitle: ToEuler***]
    float3 ToEulerXYX() const;
    float3 ToEulerXZX() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerYXY() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerYZY() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerZXZ() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerZYZ() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerXYZ() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerXZY() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerYXZ() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerYZX() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerZXY() const; ///< [similarOverload: ToEulerXYX] [hideIndex]
    float3 ToEulerZYX() const; ///< [similarOverload: ToEulerXYX] [hideIndex]

    /// Returns the scale components of this matrix.
    /** This function decomposes this matrix M into a form M = M' * S, where M' has unitary column vectors and S is a diagonal matrix.
        @return ExtractScale returns the diagonal entries of S, i.e. the scale of the columns of this matrix . If this matrix
        represents a local->world space transformation for an object, then this scale represents a 'local scale', i.e.
        scaling that is performed before translating and rotating the object from its local coordinate system to its world
        position.
        @note This function assumes that this matrix does not contain projection (the fourth row of this matrix is [0 0 0 1]).
        @note This function does not detect and return reflection (-1 scale along some axis). */
    float3 ExtractScale() const;

    /// Decomposes this matrix to rotate and scale parts.
    /** This function decomposes this matrix M to a form M = R * S, where R a rotation matrix and S a
        scale matrix.
        @note Remember that in the convention of this class, transforms are applied in the order M * v, so scale is
        applied first, then rotation, and finally the translation last.
        @note This function assumes that this matrix does not contain projection (the fourth row of this matrix is [0 0 0 1]).
        @param translate [out] This vector receives the translation component this matrix performs. The translation is applied last
            after rotation and scaling.
        @param rotate [out] This object receives the rotation part of this transform.
        @param scale [out] This vector receives the scaling along the local (before transformation by R) X, Y and Z axes 
            performed by this matrix. */
    void Decompose(Quat &rotate, float3 &scale) const;
    void Decompose(float3x3 &rotate, float3 &scale) const;

    float3x3 Mul(const float3x3 &rhs) const;
    float3x4 Mul(const float3x4 &rhs) const;
    float4x4 Mul(const float4x4 &rhs) const;
    float3x3 Mul(const Quat &rhs) const;
    float3 Mul(const float3 &rhs) const;
    float3 MulPos(const float3 &rhs) const { return Mul(rhs); }
    float3 MulDir(const float3 &rhs) const { return Mul(rhs); }

#ifdef OGRE_INTEROP
    float3x3(const Ogre::Matrix3 &m) { Set(&m[0][0]); }
    operator Ogre::Matrix3() { return Ogre::Matrix3(v[0][0], v[0][1], v[0][2], v[1][0], v[1][1], v[1][2], v[2][0], v[2][1], v[2][2]); } 
#endif

#ifdef BULLET_INTEROP
    float3x3(const btMatrix3x3 &m) { Set(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]); }
    operator btMatrix3x3() { return btMatrix3x3(v[0][0], v[0][1], v[0][2], v[1][0], v[1][1], v[1][2], v[2][0], v[2][1], v[2][2]); }
#endif

#ifdef QT_INTEROP
    operator QString() const { return toString(); }
    QString toString() const { return ToString2().c_str(); }
#endif

};

#ifdef MATH_ENABLE_STL_SUPPORT
/// Prints this float3x3 to the given stream.
std::ostream &operator <<(std::ostream &out, const float3x3 &rhs);
#endif

/// Multiplies two transforms together.
float3x3 operator *(const Quat &lhs, const float3x3 &rhs);

/// Transforms the given vector by the given matrix in the order v * M. Note that this form
/// of multiplication is against the convention of this math system for transforming geometrical objects. 
/// Please use the M * v notation instead. (Remember that M * v != v * M in general).
float3 operator *(const float3 &lhs, const float3x3 &rhs);

/// Transforms the given vector by the given matrix in the order v * M. Note that this form
/// of multiplication is against the convention of this math system. Please use the M * v notation instead.
/// (Remember that M * v != v * M in general).
/// This function ignores the w component of the given input vector. This component is assumed to be either 0 or 1.
float4 operator *(const float4 &lhs, const float3x3 &rhs);

#ifdef QT_INTEROP
Q_DECLARE_METATYPE(float3x3)
Q_DECLARE_METATYPE(float3x3*)
#endif
