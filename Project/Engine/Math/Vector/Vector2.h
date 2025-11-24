#pragma once

struct Vector2 {
    float x, y;

    // スカラー乗算(Vector2 * float)
    Vector2 operator*(float scalar) const {
        return { x * scalar, y * scalar };
    }

    // スカラー除算(Vector2 / float)
    Vector2 operator/(float scalar) const {
        return { x / scalar, y / scalar };
    }

    // ベクトル加算(Vector2 + Vector2)
    Vector2 operator+(const Vector2& v) const {
        return { x + v.x, y + v.y };
    }

    // ベクトル減算(Vector2 - Vector2)
    Vector2 operator-(const Vector2& v) const {
        return { x - v.x, y - v.y };
    }

    // 複合代入演算子
    Vector2& operator+=(const Vector2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    Vector2& operator-=(const Vector2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
};

// スカラーとベクトルの乗算 (float * Vector2)
inline Vector2 operator*(float scalar, const Vector2& v) {
    return { scalar * v.x, scalar * v.y };
}
