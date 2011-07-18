/** @file
    @author Jukka Jyl�nki

    This work is copyrighted material and may NOT be used for any kind of commercial or 
    personal advantage and may NOT be copied or redistributed without prior consent
    of the author(s). 
*/
#include "StableHeaders.h"

#include <string.h>

#include "MathFunc.h"
#include "float3.h"
#include "float4.h"
#include "float3x3.h"
#include "float3x4.h"
#include "float4x4.h"
#include "Matrix.inl"
#include "Quat.h"
#include "TransformOps.h"

float4x4::float4x4(float _00, float _01, float _02, float _03,
                   float _10, float _11, float _12, float _13,
                   float _20, float _21, float _22, float _23,
                   float _30, float _31, float _32, float _33)
{
    Set(_00, _01, _02, _03, 
        _10, _11, _12, _13, 
        _20, _21, _22, _23, 
        _30, _31, _32, _33);
}

float4x4::float4x4(const float3x3 &other)
{
    SetRow3(0, other.Row(0));
    SetRow3(1, other.Row(1));
    SetRow3(2, other.Row(2));
    SetRow(3, 0, 0, 0, 1);
    SetCol3(3, 0, 0, 0);
}

float4x4::float4x4(const float3x4 &other)
{
    SetRow(0, other.Row(0));
    SetRow(1, other.Row(1));
    SetRow(2, other.Row(2));
    SetRow(3, 0, 0, 0, 1);
}

float4x4::float4x4(const float4 &col0, const float4 &col1, const float4 &col2, const float4 &col3)
{
    SetCol(0, col0);
    SetCol(1, col1);
    SetCol(2, col2);
    SetCol(3, col3);
}

float4x4::float4x4(const Quat &orientation)
{
    SetRotatePart(orientation);
    SetRow(3, 0, 0, 0, 1);
    SetCol3(3, 0, 0, 0);
}

float4x4::float4x4(const Quat &orientation, const float3 &translation)
{
    SetRotatePart(orientation);
    SetTranslatePart(translation);
    SetRow(3, 0, 0, 0, 1);
}

TranslateOp float4x4::Translate(float tx, float ty, float tz)
{
    return TranslateOp(tx, ty, tz);
}

TranslateOp float4x4::Translate(const float3 &offset)
{
    return TranslateOp(offset);
}

float4x4 float4x4::RotateX(float angleRadians)
{
    float4x4 r;
    r.SetRotatePartX(angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return r;
}

float4x4 float4x4::RotateX(float angleRadians, const float3 &pointOnAxis)
{
    float4x4 r;
    r.SetRotatePartX(angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return float4x4::Translate(pointOnAxis) * r * float4x4::Translate(-pointOnAxis);
}

float4x4 float4x4::RotateY(float angleRadians)
{
    float4x4 r;
    r.SetRotatePartY(angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return r;
}

float4x4 float4x4::RotateY(float angleRadians, const float3 &pointOnAxis)
{
    float4x4 r;
    r.SetRotatePartY(angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return float4x4::Translate(pointOnAxis) * r * float4x4::Translate(-pointOnAxis);
}

float4x4 float4x4::RotateZ(float angleRadians)
{
    float4x4 r;
    r.SetRotatePartZ(angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return r;
}

float4x4 float4x4::RotateZ(float angleRadians, const float3 &pointOnAxis)
{
    float4x4 r;
    r.SetRotatePartZ(angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return float4x4::Translate(pointOnAxis) * r * float4x4::Translate(-pointOnAxis);
}

float4x4 float4x4::RotateAxisAngle(const float3 &axisDirection, float angleRadians)
{
    float4x4 r;
    r.SetRotatePart(axisDirection, angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return r;
}

float4x4 float4x4::RotateAxisAngle(const float3 &axisDirection, float angleRadians, const float3 &pointOnAxis)
{
    float4x4 r;
    r.SetRotatePart(axisDirection, angleRadians);
    r.SetRow(3, 0, 0, 0, 1);
    r.SetCol3(3, 0, 0, 0);
    return float4x4::Translate(pointOnAxis) * r * float4x4::Translate(-pointOnAxis);
}

float4x4 float4x4::RotateFromTo(const float3 &sourceDirection, const float3 &targetDirection)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::RotateFromTo(const float3 &sourceDirection, const float3 &targetDirection, const float3 &centerPoint)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::RotateFromTo(const float3 &centerPoint, const float3 &sourceDirection, const float3 &targetDirection,
    const float3 &sourceDirection2, const float3 &targetDirection2)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::FromQuat(const Quat &orientation)
{
    return float4x4(orientation);
}

float4x4 float4x4::FromQuat(const Quat &orientation, const float3 &pointOnAxis)
{
    return float4x4::Translate(pointOnAxis) * float4x4(orientation) * float4x4::Translate(-pointOnAxis);
}

float4x4 float4x4::FromTRS(const float3 &translate, const Quat &rotate, const float3 &scale)
{
    return float4x4::Translate(translate) * float4x4(rotate) * float4x4::Scale(scale);
}

float4x4 float4x4::FromTRS(const float3 &translate, const float3x3 &rotate, const float3 &scale)
{
    return float4x4::Translate(translate) * float4x4(rotate) * float4x4::Scale(scale);
}

float4x4 float4x4::FromTRS(const float3 &translate, const float3x4 &rotate, const float3 &scale)
{
    return float4x4::Translate(translate) * float4x4(rotate) * float4x4::Scale(scale);
}

float4x4 float4x4::FromTRS(const float3 &translate, const float4x4 &rotate, const float3 &scale)
{
    return float4x4::Translate(translate) * rotate * float4x4::Scale(scale);
}

float4x4 float4x4::FromEulerXYX(float x2, float y, float x)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerXYX(r, x2, y, x);
    assert(r.Equals(float4x4::RotateX(x2) * float4x4::RotateY(y) * float4x4::RotateX(x)));
    return r;
}

float4x4 float4x4::FromEulerXZX(float x2, float z, float x)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerXZX(r, x2, z, x);
    assert(r.Equals(float4x4::RotateX(x2) * float4x4::RotateZ(z) * float4x4::RotateX(x)));
    return r;
}

float4x4 float4x4::FromEulerYXY(float y2, float x, float y)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerYXY(r, y2, x, y);
    assert(r.Equals(float4x4::RotateY(y2) * float4x4::RotateX(x) * float4x4::RotateY(y)));
    return r;
}

float4x4 float4x4::FromEulerYZY(float y2, float z, float y)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerYZY(r, y2, z, y);
    assert(r.Equals(float4x4::RotateY(y2) * float4x4::RotateZ(z) * float4x4::RotateY(y)));
    return r;
}

float4x4 float4x4::FromEulerZXZ(float z2, float x, float z)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerZXZ(r, z2, x, z);
    assert(r.Equals(float4x4::RotateZ(z2) * float4x4::RotateX(x) * float4x4::RotateZ(z)));
    return r;
}

float4x4 float4x4::FromEulerZYZ(float z2, float y, float z)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerZYZ(r, z2, y, z);
    assert(r.Equals(float4x4::RotateZ(z2) * float4x4::RotateY(y) * float4x4::RotateZ(z)));
    return r;
}

float4x4 float4x4::FromEulerXYZ(float x, float y, float z)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerXYZ(r, x, y, z);
    assert(r.Equals(float4x4::RotateX(x) * float4x4::RotateY(y) * float4x4::RotateX(z)));
    return r;
}

float4x4 float4x4::FromEulerXZY(float x, float z, float y)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerXZY(r, x, z, y);
    assert(r.Equals(float4x4::RotateX(x) * float4x4::RotateZ(z) * float4x4::RotateY(y)));
    return r;
}

float4x4 float4x4::FromEulerYXZ(float y, float x, float z)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerYXZ(r, y, x, z);
    assert(r.Equals(float4x4::RotateY(y) * float4x4::RotateX(x) * float4x4::RotateZ(z)));
    return r;
}

float4x4 float4x4::FromEulerYZX(float y, float z, float x)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerYZX(r, y, z, x);
    assert(r.Equals(float4x4::RotateY(y) * float4x4::RotateZ(z) * float4x4::RotateX(x)));
    return r;
}

float4x4 float4x4::FromEulerZXY(float z, float x, float y)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerZXY(r, z, x, y);
    assert(r.Equals(float4x4::RotateZ(z) * float4x4::RotateX(x) * float4x4::RotateY(y)));
    return r;
}

float4x4 float4x4::FromEulerZYX(float z, float y, float x)
{
    float4x4 r;
    r.SetTranslatePart(0,0,0);
    r.SetRow(3, 0,0,0,1);
    Set3x3PartRotateEulerZYX(r, z, y, x);
    assert(r.Equals(float4x4::RotateZ(z) * float4x4::RotateY(y) * float4x4::RotateX(x)));
    return r;
}


ScaleOp float4x4::Scale(float sx, float sy, float sz)
{
    return ScaleOp(sx, sy, sz);
}

ScaleOp float4x4::Scale(const float3 &scale)
{
    return ScaleOp(scale);
}

float4x4 float4x4::Scale(const float3 &scale, const float3 &scaleCenter)
{
    return Translate(scaleCenter).ToFloat4x4() * Scale(scale) * Translate(-scaleCenter);
}

float4x4 float4x4::ScaleAlongAxis(const float3 &axis, float scalingFactor)
{
    return Scale(axis * scalingFactor);
}

float4x4 float4x4::ScaleAlongAxis(const float3 &axis, float scalingFactor, const float3 &scaleCenter)
{
    return Translate(scaleCenter).ToFloat4x4() * Scale(axis * scalingFactor) * Translate(-scaleCenter);
}

ScaleOp float4x4::UniformScale(float uniformScale)
{
    return ScaleOp(uniformScale, uniformScale, uniformScale);
}

float3 float4x4::GetScale() const
{
    return float3(Col3(0).Length(), Col3(1).Length(), Col3(2).Length());
}

float4x4 float4x4::ShearX(float yFactor, float zFactor)
{
    return float4x4(1.f, yFactor, zFactor, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f);
}

float4x4 float4x4::ShearY(float xFactor, float zFactor)
{
    return float4x4(1.f, 0.f, 0.f, 0.f,
                    xFactor, 1.f, zFactor, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f);
}

float4x4 float4x4::ShearZ(float xFactor, float yFactor)
{
    return float4x4(1.f, 0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    xFactor, yFactor, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f);
}

float4x4 float4x4::Reflect(const Plane &p)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::MakePerspectiveProjection(float nearPlaneDistance, float farPlaneDistance, float horizontalFov, float verticalFov)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::MakeOrthographicProjection(float nearPlaneDistance, float farPlaneDistance, float horizontalViewportSize, float verticalViewportSize)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::MakeOrthographicProjection(const Plane &target)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::MakeOrthographicProjectionYZ()
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::MakeOrthographicProjectionXZ()
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::MakeOrthographicProjectionXY()
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 float4x4::ComplementaryProjection() const
{
	assume(IsIdempotent()); 

    return float4x4::identity - *this;
}

MatrixProxy<float4x4::Cols> &float4x4::operator[](int row)
{
    assert(row >= 0);
    assert(row < Rows);

    return *(reinterpret_cast<MatrixProxy<Cols>*>(v[row]));
}

const MatrixProxy<float4x4::Cols> &float4x4::operator[](int row) const
{
    assert(row >= 0);
    assert(row < Rows);

    return *(reinterpret_cast<const MatrixProxy<Cols>*>(v[row]));
}

float &float4x4::At(int row, int col)
{
    return v[row][col];
}

CONST_WIN32 float float4x4::At(int row, int col) const
{
    return v[row][col];
}

float4 &float4x4::Row(int row)
{
    return reinterpret_cast<float4 &>(v[row]);
}

const float4 &float4x4::Row(int row) const
{
    return reinterpret_cast<const float4 &>(v[row]);
}

float3 &float4x4::Row3(int row)
{
    return reinterpret_cast<float3 &>(v[row]);
}

const float3 &float4x4::Row3(int row) const
{
    return reinterpret_cast<const float3 &>(v[row]);
}

CONST_WIN32 float4 float4x4::Col(int col) const
{
    return float4(v[0][col], v[1][col], v[2][col], v[3][col]);
}

CONST_WIN32 float3 float4x4::Col3(int col) const
{
    return float3(v[0][col], v[1][col], v[2][col]);
}

CONST_WIN32 float4 float4x4::Diagonal() const
{
    return float4(v[0][0], v[1][1], v[2][2], v[3][3]);
}

CONST_WIN32 float3 float4x4::Diagonal3() const
{
    return float3(v[0][0], v[1][1], v[2][2]);
}

void float4x4::ScaleRow3(int row, float scalar)
{
    Row3(row) *= scalar;
}

void float4x4::ScaleRow(int row, float scalar)
{
    Row(row) *= scalar;
}

void float4x4::ScaleCol3(int col, float scalar)
{
    v[0][col] *= scalar;
    v[1][col] *= scalar;
    v[2][col] *= scalar;
}

void float4x4::ScaleCol(int col, float scalar)
{
    v[0][col] *= scalar;
    v[1][col] *= scalar;
    v[2][col] *= scalar;
    v[3][col] *= scalar;
}

const float3x3 float4x4::Float3x3Part() const
{
    return float3x3(v[0][0], v[0][1], v[0][2],
                    v[1][0], v[1][1], v[1][2],
                    v[2][0], v[2][1], v[2][2]);
}

float3x4 &float4x4::Float3x4Part()
{
    return reinterpret_cast<float3x4 &>(*this);
}

const float3x4 &float4x4::Float3x4Part() const
{
    return reinterpret_cast<const float3x4 &>(*this);
}

CONST_WIN32 float3 float4x4::TranslatePart() const
{
    return Col3(3);
}

CONST_WIN32 float3x3 float4x4::RotatePart() const
{
    return Float3x3Part();
}

float3 float4x4::WorldX() const
{
    return Col3(0);
}

float3 float4x4::WorldY() const
{
    return Col3(1);
}

float3 float4x4::WorldZ() const
{
    return Col3(2);
}

float *float4x4::ptr()
{
    return reinterpret_cast<float *>(v);
}

const float *float4x4::ptr() const
{ 
    return reinterpret_cast<const float *>(v);
}

void float4x4::SetRow3(int row, const float3 &rowVector)
{
    v[row][0] = rowVector.x;
    v[row][1] = rowVector.y;
    v[row][2] = rowVector.z;
}

void float4x4::SetRow3(int row, const float *data)
{
    assert(data);
    v[row][0] = data[0];
    v[row][1] = data[1];
    v[row][2] = data[2];
}

void float4x4::SetRow3(int row, float m_r0, float m_r1, float m_r2)
{
    v[row][0] = m_r0;
    v[row][1] = m_r1;
    v[row][2] = m_r2;
}

void float4x4::SetRow(int row, const float3 &rowVector, float m_r3)
{
    v[row][0] = rowVector.x;
    v[row][1] = rowVector.y;
    v[row][2] = rowVector.z;
    v[row][3] = m_r3;
}

void float4x4::SetRow(int row, const float4 &rowVector)
{
    v[row][0] = rowVector.x;
    v[row][1] = rowVector.y;
    v[row][2] = rowVector.z;
    v[row][3] = rowVector.w;
}

void float4x4::SetRow(int row, const float *data)
{
    assert(data);
    v[row][0] = data[0];
    v[row][1] = data[1];
    v[row][2] = data[2];
    v[row][3] = data[3];
}

void float4x4::SetRow(int row, float m_r0, float m_r1, float m_r2, float m_r3)
{
    v[row][0] = m_r0;
    v[row][1] = m_r1;
    v[row][2] = m_r2;
    v[row][3] = m_r3;
}

void float4x4::SetCol3(int column, const float3 &columnVector)
{
    v[0][column] = columnVector.x;
    v[1][column] = columnVector.y;
    v[2][column] = columnVector.z;
}

void float4x4::SetCol3(int column, const float *data)
{
    assert(data);
    v[0][column] = data[0];
    v[1][column] = data[1];
    v[2][column] = data[2];
    v[3][column] = data[3];
}

void float4x4::SetCol3(int column, float m_0c, float m_1c, float m_2c)
{
    v[0][column] = m_0c;
    v[1][column] = m_1c;
    v[2][column] = m_2c;
}

void float4x4::SetCol(int column, const float3 &columnVector, float m_3c)
{
    v[0][column] = columnVector.x;
    v[1][column] = columnVector.y;
    v[2][column] = columnVector.z;
    v[3][column] = m_3c;
}

void float4x4::SetCol(int column, const float4 &columnVector)
{
    v[0][column] = columnVector.x;
    v[1][column] = columnVector.y;
    v[2][column] = columnVector.z;
    v[3][column] = columnVector.w;
}

void float4x4::SetCol(int column, const float *data)
{
    assert(data);
    v[0][column] = data[0];
    v[1][column] = data[1];
    v[2][column] = data[2];
    v[3][column] = data[3];
}

void float4x4::SetCol(int column, float m_0c, float m_1c, float m_2c, float m_3c)
{
    v[0][column] = m_0c;
    v[1][column] = m_1c;
    v[2][column] = m_2c;
    v[3][column] = m_3c;
}

void float4x4::Set(float _00, float _01, float _02, float _03,
                   float _10, float _11, float _12, float _13,
                   float _20, float _21, float _22, float _23,
                   float _30, float _31, float _32, float _33)
{
    v[0][0] = _00; v[0][1] = _01; v[0][2] = _02; v[0][3] = _03;
    v[1][0] = _10; v[1][1] = _11; v[1][2] = _12; v[1][3] = _13;
    v[2][0] = _20; v[2][1] = _21; v[2][2] = _22; v[2][3] = _23;
    v[3][0] = _30; v[3][1] = _31; v[3][2] = _32; v[3][3] = _33;
}

void float4x4::Set(const float4x4 &rhs)
{
    Set(rhs.ptr());
}

void float4x4::Set(const float *values)
{
    memcpy(ptr(), values, sizeof(float) * Rows * Cols);
}

void float4x4::Set(int row, int col, float value)
{
    assume(0 <= row && row <= 3);
    assume(0 <= col && col <= 3);
    v[row][col] = value;
}

void float4x4::SetIdentity()
{
    Set(1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1);
}

void float4x4::Set3x3Part(const float3x3 &r)
{
    v[0][0] = r[0][0]; v[0][1] = r[0][1]; v[0][2] = r[0][2];
    v[1][0] = r[1][0]; v[1][1] = r[1][1]; v[1][2] = r[1][2];
    v[2][0] = r[2][0]; v[2][1] = r[2][1]; v[2][2] = r[2][2];
}

void float4x4::Set3x4Part(const float3x4 &r)
{
    v[0][0] = r[0][0]; v[0][1] = r[0][1]; v[0][2] = r[0][2]; v[0][3] = r[0][3];
    v[1][0] = r[1][0]; v[1][1] = r[1][1]; v[1][2] = r[1][2]; v[1][3] = r[1][3];
    v[2][0] = r[2][0]; v[2][1] = r[2][1]; v[2][2] = r[2][2]; v[2][3] = r[2][3];
}

void float4x4::SwapColumns(int col1, int col2)
{
    Swap(v[0][col1], v[0][col2]);
    Swap(v[1][col1], v[1][col2]);
    Swap(v[2][col1], v[2][col2]);
    Swap(v[3][col1], v[3][col2]);
}

void float4x4::SwapColumns3(int col1, int col2)
{
    Swap(v[0][col1], v[0][col2]);
    Swap(v[1][col1], v[1][col2]);
    Swap(v[2][col1], v[2][col2]);
}

void float4x4::SwapRows(int row1, int row2)
{
    Swap(v[row1][0], v[row2][0]);
    Swap(v[row1][1], v[row2][1]);
    Swap(v[row1][2], v[row2][2]);
    Swap(v[row1][3], v[row2][3]);
}

void float4x4::SwapRows3(int row1, int row2)
{
    Swap(v[row1][0], v[row2][0]);
    Swap(v[row1][1], v[row2][1]);
    Swap(v[row1][2], v[row2][2]);
}

void float4x4::SetTranslatePart(float tx, float ty, float tz)
{
    SetCol3(3, tx, ty, tz);
}

void float4x4::SetTranslatePart(const float3 &offset)
{
    SetCol3(3, offset);
}

void float4x4::SetRotatePartX(float angle)
{
    Set3x3PartRotateX(*this, angle);
}

void float4x4::SetRotatePartY(float angle)
{
    Set3x3PartRotateY(*this, angle);
}

void float4x4::SetRotatePartZ(float angle)
{
    Set3x3PartRotateZ(*this, angle);
}

void float4x4::SetRotatePart(const float3 &a, float angle)
{
    assert(a.IsNormalized());

    const float c = Cos(angle);
    const float c1 = (1.f-c);
    const float s = Sin(angle);

    v[0][0] = c+c1*a.x*a.x;
    v[1][0] = c1*a.x*a.y+s*a.z;
    v[2][0] = c1*a.x*a.z-s*a.y;

    v[0][1] = c1*a.x*a.y-s*a.z;
    v[1][1] = c+c1*a.y*a.y;
    v[2][1] = c1*a.y*a.z+s*a.x;

    v[0][2] = c1*a.x*a.z+s*a.y;
    v[1][2] = c1*a.y*a.z-s*a.x;
    v[2][2] = c+c1*a.z*a.z;
}

void float4x4::SetRotatePart(const Quat &q)
{
    SetMatrixRotatePart(*this, q);
}

float4x4 float4x4::LookAt(const float3 &localForward, const float3 &targetDirection, const float3 &localUp, const float3 &worldUp, bool rightHanded)
{
    assume(false && "Not implemented!");
    return float4x4(); ///\todo
}

float4x4 &float4x4::operator =(const float3x3 &rhs)
{
    SetRotatePart(rhs);
    SetTranslatePart(0,0,0);
    SetRow(3, 0,0,0,1);
    return *this;
}

float4x4 &float4x4::operator =(const float3x4 &rhs)
{
    Float3x4Part() = rhs;
    SetRow(3, 0,0,0,1);
    return *this;
}

float4x4 &float4x4::operator =(const float4x4 &rhs)
{
    memcpy(this, &rhs, sizeof(rhs));
    return *this;
}

float float4x4::Determinant3() const
{
    const float a = v[0][0];
    const float b = v[0][1];
    const float c = v[0][2];
    const float d = v[1][0];
    const float e = v[1][1];
    const float f = v[1][2];
    const float g = v[2][0];
    const float h = v[2][1];
    const float i = v[2][2];

    return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g;
}

float float4x4::Determinant4() const
{
    return v[0][0] * Minor(0,0) - v[0][1] * Minor(0,1) + v[0][2] * Minor(0,2) - v[0][3] * Minor(0,3);
}

#define SKIPNUM(val, skip) (val < skip ? val : skip + 1)

float3x3 float4x4::SubMatrix(int i, int j) const
{
    int r0 = SKIPNUM(0, i);
    int r1 = SKIPNUM(1, i);
    int r2 = SKIPNUM(2, i);
    int c0 = SKIPNUM(0, j);
    int c1 = SKIPNUM(1, j);
    int c2 = SKIPNUM(2, j);

    return float3x3(v[r0][c0], v[r0][c1], v[r0][c2],
                    v[r1][c0], v[r1][c1], v[r1][c2],
                    v[r2][c0], v[r2][c1], v[r2][c2]);
}

float float4x4::Minor(int i, int j) const
{
    int r0 = SKIPNUM(0, i);
    int r1 = SKIPNUM(1, i);
    int r2 = SKIPNUM(2, i);
    int c0 = SKIPNUM(0, j);
    int c1 = SKIPNUM(1, j);
    int c2 = SKIPNUM(2, j);

    float a = v[r0][c0];
    float b = v[r0][c1];
    float c = v[r0][c2];
    float d = v[r1][c0];
    float e = v[r1][c1];
    float f = v[r1][c2];
    float g = v[r2][c0];
    float h = v[r2][c1];
    float k = v[r2][c2];

    return a*e*k + b*f*g + c*d*h - a*f*h - b*d*k - c*e*g;
}

float4x4 float4x4::Adjugate() const
{
    float4x4 a;
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            a[y][x] = (((x+y) & 1) != 0) ? -Minor(y, x) : Minor(y, x);

    return a;
}

bool float4x4::CholeskyDecompose(float4x4 &outL) const
{
    return CholeskyDecomposeMatrix(*this, outL);
}

bool float4x4::LUDecompose(float4x4 &outLower, float4x4 &outUpper) const
{
    return LUDecomposeMatrix(*this, outLower, outUpper);
}

bool float4x4::Inverse()
{
    return InverseMatrix(*this);
}

float4x4 float4x4::Inverted() const
{
    float4x4 copy = *this;
    copy.Inverse();
    return copy;
}

bool float4x4::InverseOrthogonal()
{
    assume(IsOrthogonal3());
    Swap(v[0][1], v[1][0]);
    Swap(v[0][2], v[2][0]);
    Swap(v[1][2], v[2][1]);
    float scale1 = sqrtf(1.f / float3(v[0][0], v[0][1], v[0][2]).LengthSq());
    float scale2 = sqrtf(1.f / float3(v[1][0], v[1][1], v[1][2]).LengthSq());
    float scale3 = sqrtf(1.f / float3(v[2][0], v[2][1], v[2][2]).LengthSq());

    v[0][0] *= scale1; v[0][1] *= scale2; v[0][2] *= scale3;
    v[1][0] *= scale1; v[1][1] *= scale2; v[1][2] *= scale3;
    v[2][0] *= scale1; v[2][1] *= scale2; v[2][2] *= scale3;

    SetTranslatePart(TransformDir(-v[0][3], -v[1][3], -v[2][3]));

    return true;
}

bool float4x4::InverseOrthogonalUniformScale()
{
    assume(IsOrthogonal3());
    assume(HasUniformScale());
    Swap(v[0][1], v[1][0]);
    Swap(v[0][2], v[2][0]);
    Swap(v[1][2], v[2][1]);
    const float scale = sqrtf(1.f / float3(v[0][0], v[0][1], v[0][2]).LengthSq());

    v[0][0] *= scale; v[0][1] *= scale; v[0][2] *= scale;
    v[1][0] *= scale; v[1][1] *= scale; v[1][2] *= scale;
    v[2][0] *= scale; v[2][1] *= scale; v[2][2] *= scale;

    SetTranslatePart(TransformDir(-v[0][3], -v[1][3], -v[2][3]));

    return true;
}

void float4x4::InverseOrthonormal()
{
    assume(IsOrthonormal3());
    Swap(v[0][1], v[1][0]);
    Swap(v[0][2], v[2][0]);
    Swap(v[1][2], v[2][1]);
    SetTranslatePart(TransformDir(-v[0][3], -v[1][3], -v[2][3]));
}

void float4x4::Transpose()
{
    Swap(v[0][1], v[1][0]);
    Swap(v[0][2], v[2][0]);
    Swap(v[0][3], v[3][0]);
    Swap(v[1][2], v[2][1]);
    Swap(v[1][3], v[3][1]);
    Swap(v[2][3], v[3][2]);
}

float4x4 float4x4::Transposed() const
{
    float4x4 copy = *this;
    copy.Transpose();
    return copy;
}

bool float4x4::InverseTranspose()
{
    bool success = Inverse();
    Transpose();
    return success;
}

float4x4 float4x4::InverseTransposed() const
{
    float4x4 copy = *this;
    copy.Transpose();
    copy.Inverse();
    return copy;
}

float float4x4::Trace() const
{
    return v[0][0] + v[1][1] + v[2][2] + v[3][3];
}

void float4x4::Orthogonalize3(int firstColumn, int secondColumn, int thirdColumn)
{
    assume(false && "Not implemented!");
    ///\todo
}

void float4x4::Orthonormalize3(int firstColumn, int secondColumn, int thirdColumn)
{
    assume(false && "Not implemented!");
    ///\todo
}

void float4x4::RemoveScale()
{
    float x = Row3(0).Normalize();
    float y = Row3(1).Normalize();
    float z = Row3(2).Normalize();
    assume(x != 0 && y != 0 && z != 0 && "float4x4::RemoveScale failed!");
}

/// Algorithm from Eric Lengyel's Mathematics for 3D Game Programming & Computer Graphics, 2nd Ed.
void float4x4::Pivot()
{
	int row = 0;

	for(int col = 0; col < Cols; ++col)
	{
		int greatest = row;

		// find the row k with k >= 1 for which Mkj has the largest absolute value.
		for(int i = row; i < Rows; ++i)
			if (Abs(v[i][col]) > Abs(v[greatest][col]))
				greatest = i;

		if (!EqualAbs(v[greatest][col], 0))
		{
			if (row != greatest)
				SwapRows(row, greatest); // the greatest now in row

			ScaleRow(row, 1.f/v[row][col]);

			for(int r = 0; r < Rows; ++r)
				if (r != row)
                    SetRow(r, Row(r) - Row(row) * v[r][col]);

			++row;
		}
	}
}

float3 float4x4::TransformPos(const float3 &pointVector) const
{
    return TransformPos(pointVector.x, pointVector.y, pointVector.z);
}

float3 float4x4::TransformPos(float x, float y, float z) const
{
    return float3(DOT4POS_xyz(Row(0), x,y,z),
                  DOT4POS_xyz(Row(1), x,y,z),
                  DOT4POS_xyz(Row(2), x,y,z));
}

float3 float4x4::TransformDir(const float3 &directionVector) const
{
    return TransformDir(directionVector.x, directionVector.y, directionVector.z);
}

float3 float4x4::TransformDir(float x, float y, float z) const
{
    return float3(DOT4DIR_xyz(Row(0), x,y,z),
                  DOT4DIR_xyz(Row(1), x,y,z),
                  DOT4DIR_xyz(Row(2), x,y,z));
}

float4 float4x4::Transform(const float4 &vector) const
{
    return float4(DOT4(Row(0), vector),
                  DOT4(Row(1), vector),
                  DOT4(Row(2), vector),
                  DOT4(Row(3), vector));
}

void float4x4::TransformPos(float3 *pointArray, int numPoints) const
{
    for(int i = 0; i < numPoints; ++i)
        pointArray[i] = this->TransformPos(pointArray[i]);
}

void float4x4::TransformPos(float3 *pointArray, int numPoints, int strideBytes) const
{
    u8 *data = reinterpret_cast<u8*>(pointArray);
    for(int i = 0; i < numPoints; ++i)
    {
        float3 *v = reinterpret_cast<float3*>(data);
        *v = this->TransformPos(*v);
        data += strideBytes;
    }        
}

void float4x4::TransformDir(float3 *dirArray, int numVectors) const
{
    for(int i = 0; i < numVectors; ++i)
        dirArray[i] = this->TransformDir(dirArray[i]);
}

void float4x4::TransformDir(float3 *dirArray, int numVectors, int strideBytes) const
{
    u8 *data = reinterpret_cast<u8*>(dirArray);
    for(int i = 0; i < numVectors; ++i)
    {
        float3 *v = reinterpret_cast<float3*>(data);
        *v = this->TransformDir(*v);
        data += strideBytes;
    }        
}

void float4x4::Transform(float4 *vectorArray, int numVectors) const
{
    for(int i = 0; i < numVectors; ++i)
        vectorArray[i] = *this * vectorArray[i];
}

void float4x4::Transform(float4 *vectorArray, int numVectors, int strideBytes) const
{
    u8 *data = reinterpret_cast<u8*>(vectorArray);
    for(int i = 0; i < numVectors; ++i)
    {
        float4 *v = reinterpret_cast<float4*>(data);
        *v = *this * *v;
        data += strideBytes;
    }        
}

float4x4 float4x4::operator *(const float3x3 &rhs) const
{
    float4x4 r;
    const float *c0 = rhs.ptr();
    const float *c1 = rhs.ptr() + 1;
    const float *c2 = rhs.ptr() + 2;
    r[0][0] = DOT3STRIDED(v[0], c0, 3);
    r[0][1] = DOT3STRIDED(v[0], c1, 3);
    r[0][2] = DOT3STRIDED(v[0], c2, 3);
    r[0][3] = v[0][3];

    r[1][0] = DOT3STRIDED(v[1], c0, 3);
    r[1][1] = DOT3STRIDED(v[1], c1, 3);
    r[1][2] = DOT3STRIDED(v[1], c2, 3);
    r[1][3] = v[1][3];

    r[2][0] = DOT3STRIDED(v[2], c0, 3);
    r[2][1] = DOT3STRIDED(v[2], c1, 3);
    r[2][2] = DOT3STRIDED(v[2], c2, 3);
    r[2][3] = v[2][3];

    r[3][0] = DOT3STRIDED(v[3], c0, 3);
    r[3][1] = DOT3STRIDED(v[3], c1, 3);
    r[3][2] = DOT3STRIDED(v[3], c2, 3);
    r[3][3] = v[3][3];

    return r;
}

float4x4 float4x4::operator *(const float3x4 &rhs) const
{
    float4x4 r;
    const float *c0 = rhs.ptr();
    const float *c1 = rhs.ptr() + 1;
    const float *c2 = rhs.ptr() + 2;
    const float *c3 = rhs.ptr() + 3;
    r[0][0] = DOT3STRIDED(v[0], c0, 4);
    r[0][1] = DOT3STRIDED(v[0], c1, 4);
    r[0][2] = DOT3STRIDED(v[0], c2, 4);
    r[0][3] = DOT3STRIDED(v[0], c3, 4) + v[0][3];

    r[1][0] = DOT3STRIDED(v[1], c0, 4);
    r[1][1] = DOT3STRIDED(v[1], c1, 4);
    r[1][2] = DOT3STRIDED(v[1], c2, 4);
    r[1][3] = DOT3STRIDED(v[1], c3, 4) + v[1][3];

    r[2][0] = DOT3STRIDED(v[2], c0, 4);
    r[2][1] = DOT3STRIDED(v[2], c1, 4);
    r[2][2] = DOT3STRIDED(v[2], c2, 4);
    r[2][3] = DOT3STRIDED(v[2], c3, 4) + v[2][3];

    r[3][0] = DOT3STRIDED(v[3], c0, 4);
    r[3][1] = DOT3STRIDED(v[3], c1, 4);
    r[3][2] = DOT3STRIDED(v[3], c2, 4);
    r[3][3] = DOT3STRIDED(v[3], c3, 4) + v[3][3];

    return r;
}

float4x4 float4x4::operator *(const float4x4 &rhs) const
{
    float4x4 r;
    const float *c0 = rhs.ptr();
    const float *c1 = rhs.ptr() + 1;
    const float *c2 = rhs.ptr() + 2;
    const float *c3 = rhs.ptr() + 3;
    r[0][0] = DOT4STRIDED(v[0], c0, 4);
    r[0][1] = DOT4STRIDED(v[0], c1, 4);
    r[0][2] = DOT4STRIDED(v[0], c2, 4);
    r[0][3] = DOT4STRIDED(v[0], c3, 4);

    r[1][0] = DOT4STRIDED(v[1], c0, 4);
    r[1][1] = DOT4STRIDED(v[1], c1, 4);
    r[1][2] = DOT4STRIDED(v[1], c2, 4);
    r[1][3] = DOT4STRIDED(v[1], c3, 4);

    r[2][0] = DOT4STRIDED(v[2], c0, 4);
    r[2][1] = DOT4STRIDED(v[2], c1, 4);
    r[2][2] = DOT4STRIDED(v[2], c2, 4);
    r[2][3] = DOT4STRIDED(v[2], c3, 4);

    r[3][0] = DOT4STRIDED(v[3], c0, 4);
    r[3][1] = DOT4STRIDED(v[3], c1, 4);
    r[3][2] = DOT4STRIDED(v[3], c2, 4);
    r[3][3] = DOT4STRIDED(v[3], c3, 4);

    return r;
}

float4x4 float4x4::operator *(const Quat &rhs) const
{
    float3x3 rot(rhs);
    return *this * rot;
}

float4 float4x4::operator *(const float4 &rhs) const
{
    return this->Transform(rhs);
}

float4x4 float4x4::operator *(float scalar) const
{
    float4x4 r = *this;
    r *= scalar;
    return r;
}

float4x4 float4x4::operator /(float scalar) const
{
    assume(!EqualAbs(scalar, 0));
    float4x4 r = *this;
    r /= scalar;
    return r;
}

float4x4 float4x4::operator +(const float4x4 &rhs) const
{
    float4x4 r = *this;
    r += rhs;
    return r;
}

float4x4 float4x4::operator -(const float4x4 &rhs) const
{
    float4x4 r = *this;
    r -= rhs;
    return r;
}

float4x4 float4x4::operator -() const
{
    float4x4 r;
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            r[y][x] = -v[y][x];
    return r;
}

float4x4 &float4x4::operator *=(float scalar)
{
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            v[y][x] *= scalar;

    return *this;
}

float4x4 &float4x4::operator /=(float scalar)
{
    assume(!EqualAbs(scalar, 0));
    float invScalar = 1.f / scalar;
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            v[y][x] *= invScalar;

    return *this;
}

float4x4 &float4x4::operator +=(const float4x4 &rhs)
{
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            v[y][x] += rhs[y][x];

    return *this;
}

float4x4 &float4x4::operator -=(const float4x4 &rhs)
{
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            v[y][x] -= rhs[y][x];

    return *this;
}

bool float4x4::IsFinite() const
{
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            if (!isfinite(v[y][x]))
                return false;
    return true;
}

bool float4x4::IsIdentity(float epsilon) const
{
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            if (!EqualAbs(v[y][x], (x == y) ? 1.f : 0.f, epsilon))
                return false;

    return true;
}

bool float4x4::IsLowerTriangular(float epsilon) const
{
    return EqualAbs(v[0][1], 0.f, epsilon)
        && EqualAbs(v[0][2], 0.f, epsilon)
        && EqualAbs(v[0][3], 0.f, epsilon)
        && EqualAbs(v[1][2], 0.f, epsilon)
        && EqualAbs(v[1][3], 0.f, epsilon)
        && EqualAbs(v[2][3], 0.f, epsilon);
}

bool float4x4::IsUpperTriangular(float epsilon) const
{
    return EqualAbs(v[1][0], 0.f, epsilon)
        && EqualAbs(v[2][0], 0.f, epsilon)
        && EqualAbs(v[3][0], 0.f, epsilon)
        && EqualAbs(v[2][1], 0.f, epsilon)
        && EqualAbs(v[3][1], 0.f, epsilon)
        && EqualAbs(v[3][2], 0.f, epsilon);
}

bool float4x4::IsInvertible(float epsilon) const
{
    ///\todo Optimize.
    float4x4 copy = *this;
    return copy.Inverse();
}

bool float4x4::IsSymmetric(float epsilon) const
{
    for(int y = 0; y < Rows; ++y)
        for(int x = y+1; x < Cols; ++x)
            if (!EqualAbs(v[y][x], v[x][y], epsilon))
                return false;
    return true;
}

bool float4x4::IsSkewSymmetric(float epsilon) const
{
    for(int y = 0; y < Rows; ++y)
        for(int x = y; x < Cols; ++x)
            if (!EqualAbs(v[y][x], -v[x][y], epsilon))
                return false;
    return true;
}

bool float4x4::IsIdempotent(float epsilon) const
{
    float4x4 m2 = *this * *this;
    return this->Equals(m2, epsilon);
}

bool float4x4::HasUnitaryScale(float epsilon) const
{
    float3 scale = ExtractScale();
    return scale.Equals(1.f, 1.f, 1.f, epsilon);
}

bool float4x4::HasNegativeScale() const
{
    return Determinant3() < 0.f;
}

bool float4x4::HasUniformScale(float epsilon) const
{
    float3 scale = ExtractScale();
    return EqualAbs(scale.x, scale.y, epsilon) && EqualAbs(scale.x, scale.z, epsilon);
}

bool float4x4::IsOrthogonal3(float epsilon) const
{
    return Row3(0).IsPerpendicular(Row3(1), epsilon)
        && Row3(0).IsPerpendicular(Row3(2), epsilon)
        && Row3(1).IsPerpendicular(Row3(2), epsilon);
}

bool float4x4::IsOrthonormal3(float epsilon) const
{
    ///\todo Epsilon magnitudes don't match.
    return IsOrthogonal3(epsilon) && Row3(0).IsNormalized(epsilon) && Row3(1).IsNormalized(epsilon) && Row3(2).IsNormalized(epsilon);
}

bool float4x4::Equals(const float4x4 &other, float epsilon) const
{
    for(int y = 0; y < Rows; ++y)
        for(int x = 0; x < Cols; ++x)
            if (!EqualAbs(v[y][x], other[y][x], epsilon))
                return false;
    return true;
}

bool float4x4::ContainsProjection(float epsilon) const
{
    return Row(3).Equals(0.f, 0.f, 0.f, 1.f, epsilon) == false;
}

#ifdef MATH_ENABLE_STL_SUPPORT
std::string float4x4::ToString() const
{
    char str[256];
    sprintf(str, "(%.2f, %.2f, %.2f, %.2f) (%.2f, %.2f, %.2f, %.2f) (%.2f, %.2f, %.2f, %.2f) (%.2f, %.2f, %.2f, %.2f)", 
        v[0][0], v[0][1], v[0][2], v[0][3],
        v[1][0], v[1][1], v[1][2], v[1][3],
        v[2][0], v[2][1], v[2][2], v[2][3],
        v[3][0], v[3][1], v[3][2], v[3][3]);

    return std::string(str);
}

std::string float4x4::ToString2() const
{
    char str[256];
    sprintf(str, "float3x4(X:(%.2f,%.2f,%.2f,%.2f) Y:(%.2f,%.2f,%.2f,%.2f) Z:(%.2f,%.2f,%.2f,%.2f), Pos:(%.2f,%.2f,%.2f,%.2f))", 
        v[0][0], v[1][0], v[2][0], v[3][0],
        v[0][1], v[1][1], v[2][1], v[3][1],
        v[0][2], v[1][2], v[2][2], v[3][2],
        v[0][3], v[1][3], v[2][3], v[3][3]);

    return std::string(str);
}
#endif

float3 float4x4::ToEulerXYX() const { float3 f; ExtractEulerXYX(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerXZX() const { float3 f; ExtractEulerXZX(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerYXY() const { float3 f; ExtractEulerYXY(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerYZY() const { float3 f; ExtractEulerYZY(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerZXZ() const { float3 f; ExtractEulerZXZ(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerZYZ() const { float3 f; ExtractEulerZYZ(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerXYZ() const { float3 f; ExtractEulerXYZ(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerXZY() const { float3 f; ExtractEulerXZY(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerYXZ() const { float3 f; ExtractEulerYXZ(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerYZX() const { float3 f; ExtractEulerYZX(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerZXY() const { float3 f; ExtractEulerZXY(*this, f[0], f[1], f[2]); return f; }
float3 float4x4::ToEulerZYX() const { float3 f; ExtractEulerZYX(*this, f[0], f[1], f[2]); return f; }

float3 float4x4::ExtractScale() const
{
    return float3(Col3(0).Length(), Col3(1).Length(), Col3(2).Length());
}

void float4x4::Decompose(float3 &translate, Quat &rotate, float3 &scale) const
{
    assume(this->IsOrthogonal3());

    float3x3 r;
    Decompose(translate, r, scale);
    rotate = Quat(r);

    // Test that composing back yields the original float4x4.
    assume(float4x4::FromTRS(translate, rotate, scale).Equals(*this, 0.1f));
}

void float4x4::Decompose(float3 &translate, float3x3 &rotate, float3 &scale) const
{
    assume(this->IsOrthogonal3());

    assume(Row(3).Equals(0,0,0,1));
    Float3x4Part().Decompose(translate, rotate, scale);

    // Test that composing back yields the original float4x4.
    assume(float4x4::FromTRS(translate, rotate, scale).Equals(*this, 0.1f));
}

void float4x4::Decompose(float3 &translate, float3x4 &rotate, float3 &scale) const
{
    assume(this->IsOrthogonal3());

    float3x3 r;
    Decompose(translate, r, scale);
    rotate.SetRotatePart(r);
    rotate.SetTranslatePart(0,0,0);

    // Test that composing back yields the original float4x4.
    assume(float4x4::FromTRS(translate, rotate, scale).Equals(*this, 0.1f));
}

void float4x4::Decompose(float3 &translate, float4x4 &rotate, float3 &scale) const
{
    assume(this->IsOrthogonal3());

    float3x3 r;
    Decompose(translate, r, scale);
    rotate.SetRotatePart(r);
    rotate.SetTranslatePart(0,0,0);
    rotate.SetRow(3, 0, 0, 0, 1);

    // Test that composing back yields the original float4x4.
    assume(float4x4::FromTRS(translate, rotate, scale).Equals(*this, 0.1f));
}

#ifdef MATH_ENABLE_STL_SUPPORT
std::ostream &operator <<(std::ostream &out, const float4x4 &rhs)
{
    out << rhs.ToString();
    return out;
}
#endif

float4x4 operator *(const Quat &lhs, const float4x4 &rhs)
{
    float3x3 rot(lhs);
    return rot * rhs;
}

float4x4 operator *(const float3x3 &lhs, const float4x4 &rhs)
{
    float4x4 r;

    const float *c0 = rhs.ptr();
    const float *c1 = rhs.ptr() + 1;
    const float *c2 = rhs.ptr() + 2;
    const float *c3 = rhs.ptr() + 3;
    r[0][0] = DOT3STRIDED(lhs[0], c0, 4);
    r[0][1] = DOT3STRIDED(lhs[0], c1, 4);
    r[0][2] = DOT3STRIDED(lhs[0], c2, 4);
    r[0][3] = DOT3STRIDED(lhs[0], c3, 4);

    r[1][0] = DOT3STRIDED(lhs[1], c0, 4);
    r[1][1] = DOT3STRIDED(lhs[1], c1, 4);
    r[1][2] = DOT3STRIDED(lhs[1], c2, 4);
    r[1][3] = DOT3STRIDED(lhs[1], c3, 4);

    r[2][0] = DOT3STRIDED(lhs[2], c0, 4);
    r[2][1] = DOT3STRIDED(lhs[2], c1, 4);
    r[2][2] = DOT3STRIDED(lhs[2], c2, 4);
    r[2][3] = DOT3STRIDED(lhs[2], c3, 4);

    r[3][0] = rhs[3][0];
    r[3][1] = rhs[3][1];
    r[3][2] = rhs[3][2];
    r[3][3] = rhs[3][3];

    return r;
}

float4x4 operator *(const float3x4 &lhs, const float4x4 &rhs)
{
    float4x4 r;
    const float *c0 = rhs.ptr();
    const float *c1 = rhs.ptr() + 1;
    const float *c2 = rhs.ptr() + 2;
    const float *c3 = rhs.ptr() + 3;
    r[0][0] = DOT4STRIDED(lhs[0], c0, 4);
    r[0][1] = DOT4STRIDED(lhs[0], c1, 4);
    r[0][2] = DOT4STRIDED(lhs[0], c2, 4);
    r[0][3] = DOT4STRIDED(lhs[0], c3, 4);

    r[1][0] = DOT4STRIDED(lhs[1], c0, 4);
    r[1][1] = DOT4STRIDED(lhs[1], c1, 4);
    r[1][2] = DOT4STRIDED(lhs[1], c2, 4);
    r[1][3] = DOT4STRIDED(lhs[1], c3, 4);

    r[2][0] = DOT4STRIDED(lhs[2], c0, 4);
    r[2][1] = DOT4STRIDED(lhs[2], c1, 4);
    r[2][2] = DOT4STRIDED(lhs[2], c2, 4);
    r[2][3] = DOT4STRIDED(lhs[2], c3, 4);

    r[3][0] = rhs[3][0];
    r[3][1] = rhs[3][1];
    r[3][2] = rhs[3][2];
    r[3][3] = rhs[3][3];

    return r;
}

float4 operator *(const float4 &lhs, const float4x4 &rhs)
{
    return float4(DOT4STRIDED(lhs, rhs.ptr(), 4),
                  DOT4STRIDED(lhs, rhs.ptr()+1, 4),
                  DOT4STRIDED(lhs, rhs.ptr()+2, 4),
                  DOT4STRIDED(lhs, rhs.ptr()+3, 4));
}

float4x4 float4x4::Mul(const float3x3 &rhs) const { return *this * rhs; }
float4x4 float4x4::Mul(const float3x4 &rhs) const { return *this * rhs; }
float4x4 float4x4::Mul(const float4x4 &rhs) const { return *this * rhs; }
float4x4 float4x4::Mul(const Quat &rhs) const { return *this * rhs; }
float3 float4x4::MulPos(const float3 &pointVector) const { return this->TransformPos(pointVector); }
float3 float4x4::MulDir(const float3 &directionVector) const { return this->TransformDir(directionVector); }
float4 float4x4::Mul(const float4 &vector) const { return *this * vector; }

const float4x4 float4x4::zero     = float4x4(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
const float4x4 float4x4::identity = float4x4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
const float4x4 float4x4::nan = float4x4(FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN, FLOAT_NAN);
