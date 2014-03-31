#ifndef __SHADERS_H__
#define __SHADERS_H__
#include "cocos2d.h"

#define PHONG_SHADER_KEY "cc3Phong"
#define ADVANCED_SHADER_KEY "cc3Advanced"

using namespace cocos2d;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#include "precompiled-phong-shader.h"
#else
#include "phong-shader.h"
#endif

#define kCCVertexAttrib_Normals 4

#define INIT_PHONG_WP8(ccglProgram) \
ccglProgram->initWithPrecompiledByteArray(phong,sizeof(phong)); \
ccglProgram->updateUniforms(); 

#define INIT_PHONG_GLSL(ccglProgram) \
ccglProgram->initWithVertexShaderByteArray(glslPhongVert,glslPhongFrag); \
ccglProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position); \
ccglProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords); \
ccglProgram->link(); \
ccglProgram->updateUniforms();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#define MESH_INIT_PHONG(ccglProgram) INIT_PHONG_WP8(ccglProgram)
#else
#define MESH_INIT_PHONG(ccglProgram) INIT_PHONG_GLSL(ccglProgram)
#endif

#endif