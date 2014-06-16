#ifndef __SHADERS_H__
#define __SHADERS_H__
#include "cocos2d.h"

#define PHONG_SHADER_KEY "cc3Phong"
#define PHONG_SHADER_TEXTURE_KEY "cc3PhongTexture"
#define PHONG_SHADER_TEXTURE_TO_ALPHA_KEY "cc3PhongTextureToAlpha"
#define PHONG_SHADER_ANIMATED_KEY "cc3PhongAnimated"
#define PHONG_SHADER_TEXTURE_ANIMATED_KEY "cc3PhongTextureAnimated"
#define ADVANCED_SHADER_KEY "cc3Advanced"

using namespace cocos2d;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#ifndef PRECOMPILED_SHADERS
#define PRECOMPILED_SHADERS
#include "precompiled-phong-shader.h"
#include "precompiled-phong-texture-shader.h"
#include "precompiled-phong-texture-to-alpha-shader.h"
#include "precompiled-phong-animated-shader.h"
#include "precompiled-phong-texture-animated-shader.h"
#endif
#else
#ifndef PRECOMPILED_SHADERS
#define PRECOMPILED_SHADERS
#include "phong-shader.h"
#endif
#endif

#define kCCVertexAttrib_Normals 4

// Material

#define INIT_PHONG_WP8(ccglProgram) \
ccglProgram->initWithPrecompiledByteArray(phong,sizeof(phong)); \
ccglProgram->updateUniforms(); 

#define INIT_PHONG_GLSL(ccglProgram) \
ccglProgram->initWithVertexShaderByteArray(glslPhongVert,glslPhongFrag); \
ccglProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position); \
ccglProgram->link(); \
ccglProgram->updateUniforms();

// Material + Texture

#define INIT_PHONG_TEXTURE_WP8(ccglProgram) \
ccglProgram->initWithPrecompiledByteArray(phongTexture,sizeof(phongTexture)); \
ccglProgram->updateUniforms(); 

#define INIT_PHONG_TEXTURE_GLSL(ccglProgram) \
ccglProgram->initWithVertexShaderByteArray(glslPhongVertTexture,glslPhongFragTexture); \
ccglProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position); \
ccglProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords); \
ccglProgram->link(); \
ccglProgram->updateUniforms();

// Material + Animated Texture to alpha

#define INIT_PHONG_TEXTURE_TO_ALPHA_WP8(ccglProgram) \
ccglProgram->initWithPrecompiledByteArray(phongTextureToAlpha,sizeof(phongTextureToAlpha)); \
ccglProgram->updateUniforms(); 

#define INIT_PHONG_TEXTURE_TO_ALPHA_GLSL(ccglProgram) \
ccglProgram->initWithVertexShaderByteArray(glslPhongVertTexture,glslPhongFragTextureToAlpha); \
ccglProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position); \
ccglProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords); \
ccglProgram->link(); \
ccglProgram->updateUniforms();

// Material + Animation

#define INIT_PHONG_ANIMATED_WP8(ccglProgram) \
ccglProgram->initWithPrecompiledByteArray(phongAnimated,sizeof(phongAnimated)); \
ccglProgram->updateUniforms(); 

#define INIT_PHONG_ANIMATED_GLSL(ccglProgram) \
ccglProgram->initWithVertexShaderByteArray(glslPhongVertAnimated,glslPhongFragAnimated); \
ccglProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position); \
ccglProgram->link(); \
ccglProgram->updateUniforms();

// Material + Texture + Animation

#define INIT_PHONG_TEXTURE_ANIMATED_WP8(ccglProgram) \
ccglProgram->initWithPrecompiledByteArray(phongTextureAnimated,sizeof(phongTextureAnimated)); \
ccglProgram->updateUniforms(); 

#define INIT_PHONG_TEXTURE_ANIMATED_GLSL(ccglProgram) \
ccglProgram->initWithVertexShaderByteArray(glslPhongVertTextureAnimated,glslPhongFragTextureAnimated); \
ccglProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position); \
ccglProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords); \
ccglProgram->link(); \
ccglProgram->updateUniforms();

//initializers

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#define MESH_INIT_PHONG(ccglProgram) INIT_PHONG_WP8(ccglProgram)
#else
#define MESH_INIT_PHONG(ccglProgram) INIT_PHONG_GLSL(ccglProgram)
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#define MESH_INIT_PHONG_TEXTURE(ccglProgram) INIT_PHONG_TEXTURE_WP8(ccglProgram)
#else
#define MESH_INIT_PHONG_TEXTURE(ccglProgram) INIT_PHONG_TEXTURE_GLSL(ccglProgram)
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#define MESH_INIT_PHONG_TEXTURE_TO_ALPHA(ccglProgram) INIT_PHONG_TEXTURE_TO_ALPHA_WP8(ccglProgram)
#else
#define MESH_INIT_PHONG_TEXTURE_TO_ALPHA(ccglProgram) INIT_PHONG_TEXTURE_TO_ALPHA_GLSL(ccglProgram)
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#define MESH_INIT_PHONG_ANIMATED(ccglProgram) INIT_PHONG_ANIMATED_WP8(ccglProgram)
#else
#define MESH_INIT_PHONG_ANIMATED(ccglProgram) INIT_PHONG_ANIMATED_GLSL(ccglProgram)
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
#define MESH_INIT_PHONG_TEXTURE_ANIMATED(ccglProgram) INIT_PHONG_TEXTURE_ANIMATED_WP8(ccglProgram)
#else
#define MESH_INIT_PHONG_TEXTURE_ANIMATED(ccglProgram) INIT_PHONG_TEXTURE_ANIMATED_GLSL(ccglProgram)
#endif

#endif