#pragma once

#include "Vector/Vector3.h"
#include "Quaternion/Quaternion.h"

struct QuaternionTransform {
    Vector3 scale;
    Quaternion rotate;
    Vector3 translate;
};
