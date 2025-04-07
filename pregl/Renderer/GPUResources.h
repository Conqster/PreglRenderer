#pragma once
#include "Core/NonCopyable.h"

#include <stdint.h>
//using TexWrapMode = uint8_t;


namespace GPUResource {


	enum class TexWrapMode : uint8_t
	{
		REPEAT,
		CLAMP, //default clamp to border
	};

	enum class TexFilterMode : uint8_t
	{
		NEAREST,   //GL_NEAREST for both MIN and MAG
		LINEAR,   //GL_LINEAR for both MIN and MAG
	};

	enum class IMGFormat : uint8_t
	{
		RGB,
		RGBA,

		DEPTH,

		RGB16F,
		RGBA16F,

		RGB32F,
		RGBA32F,

		RGBA16,
	};

	enum class DataType : uint8_t
	{
		FLOAT,
		UNSIGNED_BYTE
	};

	enum class TextureType : uint8_t
	{
		DEFAULT,
		BASE_MAP,
		NORMAL_MAP,
		SHADOW_MAP,
		RENDER
	};

	struct TextureParameter
	{
		IMGFormat imgFormat = IMGFormat::RGBA;
		TextureType textureType = TextureType::DEFAULT;
			
		TexWrapMode wrapMode = TexWrapMode::REPEAT;
		TexFilterMode filterMode = TexFilterMode::LINEAR;
		DataType pxDataType = DataType::UNSIGNED_BYTE; //pixel data data type
	};

	///////////////////////////////////////////////////////////////////////////////
	// TEXTURE
	///////////////////////////////////////////////////////////////////////////////
	class Texture
	{
	private:
		unsigned int mID = 0;
		int mWidth = 0;
		int mHeight = 0;

		//image / colour format
		IMGFormat mFormat = IMGFormat::RGB;
		TextureType mType = TextureType::DEFAULT;

		//hack to know if already loaded
		bool mLoaded = false;
	public:
		Texture() = default;
		Texture(unsigned int width, unsigned int height, TextureParameter parameter = {});
		void Generate(unsigned int weight, unsigned int height, TextureParameter parameter = {});
		Texture(const char* file_path, bool flip_uv) { GenerateFromFile(file_path, flip_uv); };
		bool GenerateFromFile(const char* file_path, bool flip_uv, TextureParameter parameter = {});

		unsigned int GetID() { return mID; }
		TextureType& GetType() { return mType; }

		void Bind() const;

		void Activate(unsigned int slot = 0) const;
		void Disactivate() const;

		~Texture() { Clear(); }
		void Clear();
	};


	/////////////////////////////////////////////////////////////////////////////
	// SHADOW MAP
	/////////////////////////////////////////////////////////////////////////////
	class ShadowMap
	{
	public:
		ShadowMap() = default;
		explicit ShadowMap(unsigned int in_size); //<-- prevent implicit int conversion to ShadowMap
		void Generate(unsigned int size);
		void Write();
		void Read(unsigned int slot = 0) const;
		inline unsigned int GetImageID() { return mTexture.GetID(); }

		void Destroy();
	private:
		unsigned int mID = 0;
		Texture mTexture;
		//union size with texture width or heigt
		int mSize = 0;
	};  //ShadowMap


	/////////////////////////////////////////////////////////////////////////////
	// FRAME BUFFER
	/////////////////////////////////////////////////////////////////////////////
	class Framebuffer : NonCopyable
	{
	public:
		Framebuffer() = default;
		Framebuffer(unsigned int width, unsigned int height);
		~Framebuffer() { Delete(); }
		bool Generate(unsigned int width, unsigned int height);
		bool ResizeBuffer(unsigned int width, unsigned int height);

		void Bind();
		void UnBind();
		/// <summary>
		/// Deletes Framebuffer and its resources
		/// </summary>
		void Delete();
		/// <summary>
		/// Deletes Framebuffer but returns Img Texture (Render Texture)
		/// </summary>
		/// <returns></returns>
		Texture Delete_FBO_GetTexture();

		void BindTexture(unsigned int slot = 0);

		inline unsigned int GetWidth() const { return mWidth; }
		inline unsigned int GetHeight() const { return mHeight; }
		inline unsigned int GetRenderTextureGPU_ID() { return mRenderTexture.GetID(); }
		inline Texture GetRenderTexture() { return mRenderTexture; }


	private:
		unsigned int mWidth, 
					 mHeight = 256;

		unsigned int mID,
			mRenderbufferID;
		
		Texture mRenderTexture;
	};

	/////////////////////////////////////////////////////////////////////////////
	// UNIFORM BUFFER
	/////////////////////////////////////////////////////////////////////////////
	class UniformBuffer : NonCopyable
	{
	private:
		unsigned int mID = 0;
	public:
		UniformBuffer() = default;
		UniformBuffer(signed long long int size);
		~UniformBuffer();

		void Generate(signed long long int size);
		void Bind() const;
		void UnBind() const;
		void SetBufferSubData(const void* data, unsigned long long int size, unsigned int offset) const;
		void BindBufferRndIdx(const unsigned int block_idx, unsigned long long int size, unsigned int offset) const;
		void SetSubDataByID(const void* data, signed long long int size, unsigned int offset) const;

		void Delete();
	}; //Uniform Buffer class 



} //GPUResource namespace