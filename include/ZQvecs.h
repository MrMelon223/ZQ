#ifndef ZQVECS_H
#define ZQVECS_H

#include "glad/glad.h"
#include "GL/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"
//#include <sycl/sycl.hpp>
#include <cstdint>

typedef glm::vec2 vec2_t;
typedef glm::vec2 uv_t;

typedef glm::vec3 vec3_t;
typedef glm::vec4 vec4_t;

typedef glm::dvec3 dvec3_t;
typedef glm::dvec4 dvec4_t;

typedef glm::u8vec4 color_t;

typedef glm::i64vec2 dim_t;
typedef struct {
	uint32_t x, y, z;
} tri_t;

typedef glm::mat3 mat3_t;
typedef glm::mat4 mat4_t;
typedef glm::f64mat4 dmat4_t;

typedef int8_t char_t;
typedef uint8_t uchar_t;
typedef int16_t short_t;
typedef uint16_t ushort_t;
typedef int32_t int_t;
typedef uint32_t uint_t;
typedef int64_t long_t;
typedef uint64_t ulong_t;

#endif