

#ifndef _QCAR_CUBE_SHADERS_H_
#define _QCAR_CUBE_SHADERS_H_

#ifndef USE_OPENGL_ES_1_1

static const char* vertex_shader_literal = " \
  \
attribute vec4 vertexPosition; \
attribute vec4 vertexNormal; \
attribute vec2 vertexTexCoord; \
 \
varying vec2 texCoord; \
varying vec4 normal; \
 \
uniform mat4 modelViewProjectionMatrix; \
 \
void main() \
{ \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
} \
";


static const char* fragment_shader_literal = " \
 \
precision mediump float; \
 \
varying vec2 texCoord; \
varying vec4 normal; \
 \
uniform sampler2D texSampler2D; \
 \
void main() \
{ \
   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \
} \
";

//vec4(1.0, 0.0, 0.0, 1.0); red ambient shader (make everything 1 colo)

#endif

#endif // _QCAR_CUBE_SHADERS_H_
