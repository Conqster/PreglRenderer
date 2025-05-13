#pragma once
#include "Core/NonCopyable.h"

#include <stdint.h>
#include <array>
#include <string>

#include <vector>



namespace GPUResource {


	enum class TexWrapMode : uint8_t
	{
		REPEAT,
		CLAMP_EDGE, //default clamp to EDGE
		CLAMP_BORDER, //default clamp to border

		COUNT
	};

	enum class TexFilterMode : uint8_t
	{
		NEAREST,   //GL_NEAREST for both MIN and MAG
		LINEAR,   //GL_LINEAR for both MIN and MAG

		COUNT
	};

	enum class IMGFormat : uint8_t
	{
		RGB,
		RGBA,
		RGBA8,
		RGBA16,
		RGB16F,
		RGBA16F,
		RGB32F,
		RGBA32F,
		RED,
		R16F,
		R32F,
		RG,
		DEPTH_COMPONENT,
		DEPTH24_STENCIL8,

		COUNT,
	};


	enum class AttachmentType : uint8_t
	{
		DEPTH,
		STENCIL,
		DEPTH_STENCIL,

		COUNT
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
		RENDER,
		UTILITIES,

		COUNT,
	};



	struct TextureParameter
	{
		IMGFormat imgInternalFormat = IMGFormat::RGBA;
		TextureType textureType = TextureType::DEFAULT;
			
		TexWrapMode wrapMode = TexWrapMode::REPEAT;
		TexFilterMode filterMode = TexFilterMode::LINEAR;
		DataType pxDataType = DataType::UNSIGNED_BYTE; //pixel data data type
		
		bool useEqualFormat = true;
		IMGFormat format = IMGFormat::RGBA;
	};

	struct RenderbufferParameter
	{
		bool bHasRenderbuffer = true;
		IMGFormat storageBufferInternalFormat = IMGFormat::DEPTH24_STENCIL8;
		AttachmentType attachment = AttachmentType::DEPTH_STENCIL;
	};


	namespace Utilities {

		
		constexpr const char* TextureTypeToString(TextureType type)
		{
			switch (type)
			{
			case GPUResource::TextureType::DEFAULT:
				return "DEFAULT";
				break;
			case GPUResource::TextureType::BASE_MAP:
				return "BASE_MAP";
				break;
			case GPUResource::TextureType::NORMAL_MAP:
				return "NORMAL_MAP";
				break;
			case GPUResource::TextureType::SHADOW_MAP:
				return "SHADOW_MAP";
				break;
			case GPUResource::TextureType::RENDER:
				return "RENDER";
				break;
			case GPUResource::TextureType::UTILITIES:
				return "UTILITIES";
				break;
			default:
				return "UNKNOWN";
				break;
			}
		}

		static bool IsFormatFloatPoint(IMGFormat format)
		{
			if (format >= IMGFormat::RGBA16)
				return true;
			return false;
		}

		static int IMGFormatChannelCount(IMGFormat format)
		{
			switch (format)
			{
			case GPUResource::IMGFormat::RGB:
				return 3;
				break;
			case GPUResource::IMGFormat::RGBA:
				return 4;
				break;
			case GPUResource::IMGFormat::DEPTH_COMPONENT:
				return 1;
				break;
			case GPUResource::IMGFormat::RGBA16:
				return 4;
				break;
			case GPUResource::IMGFormat::RGB16F:
				return 3;
				break;
			case GPUResource::IMGFormat::RGBA16F:
				return 4;
				break;
			case GPUResource::IMGFormat::RGB32F:
				return 3;
				break;
			case GPUResource::IMGFormat::RGBA32F:
				return 4;
				break;
			case GPUResource::IMGFormat::RED:
				return 1;
				break;
			case GPUResource::IMGFormat::R16F:
				return 1;
				break;
			case GPUResource::IMGFormat::RG:
				return 2;
				break;
			default:
				
				return 4;
				break;
			}
			return 4;
		}

		static std::array<const char*, static_cast<size_t>(TextureType::COUNT)> TextureTypesToStringArray()
		{
			return{ "DEFAULT", "BASE_MAP", "NORMAL_MAP", "SHADOW_MAP", "RENDER", "UTILITIES" };
		}

		static std::array<const char*, static_cast<size_t>(IMGFormat::COUNT)> ImgFormatToStringArray()
		{
			return { "RGB", "RGBA", "RGBA8", "RGBA16", "RGB16F", "RGBA16F", "RGB32F", "RGBA32F", "RED", "R16F", "R32F", "RG", "DEPTH_COMPONENT", "DEPTH24_STENCIL8"};
		}

		//static std::array<std::string, static_cast<size_t>(TexWrapMode::COUNT)> TextureWrapModeToStringArray()
		static std::array<const char*, static_cast<size_t>(TexWrapMode::COUNT)> TextureWrapModeToStringArray()
		{
			return { "REPEAT", "CLAMP_EDGE" };
		}

		static std::array<const char*, static_cast<size_t>(TexFilterMode::COUNT)> TextureFilterModeToStringArray()
		{
			return { "NEAREST", "LINEAR" };
		}
	} //Utilities namespace

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
		IMGFormat mInternalFormat = IMGFormat::RGB;
		TextureType mType = TextureType::DEFAULT;

		//hack to know if already loaded
		bool mLoaded = false;
		std::string mFilePath;
		TextureParameter mParameter;
	public:
		Texture() = default;
		Texture(unsigned int width, unsigned int height, TextureParameter parameter = {});
		Texture(unsigned int width, unsigned int height, const void* pixels_data, TextureParameter parameter = {});
		void Generate(unsigned int weight, unsigned int height, const void* pixels_data, TextureParameter parameter = {});
		Texture(const char* file_path, bool flip_uv) { GenerateFromFile(file_path, flip_uv); };
		bool GenerateFromFile(const char* file_path, bool flip_uv, TextureParameter parameter = {});


		unsigned int GetID() { return mID; }
		TextureType& GetType() { return mType; }
		IMGFormat& GetFormat() { return mInternalFormat; }
		std::string& GetImgPath() { return mFilePath; }
		//send a copy you do not want modification of texture data to prevent error
		TextureParameter GetParameter() { return mParameter; }

		int GetWidth() { return mWidth; }
		int GetHeight() { return mHeight; }
		unsigned char* RetrieveGPUImageBuffer();
		//void RetrieveGPUImageBuffer(void*& ptr_img_buffer);

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
		bool Generate(unsigned int width, unsigned int height, RenderbufferParameter render_buf_para = {}, TextureParameter tex_parameter = {
																															IMGFormat::RGB,			//imgInternalFormat
																															TextureType::RENDER,	//textureType

																															TexWrapMode::CLAMP_EDGE,		//wrapMode
																															TexFilterMode::LINEAR,	//filterMode
																															DataType::FLOAT,		//pxDataType
																															});

		bool ResizeBuffer(unsigned int width, unsigned int height);
		void ResizeBuffer2(unsigned int width, unsigned int height);
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
		RenderbufferParameter mRenderbufferPara;
	};

	/////////////////////////////////////////////////////////////////////////////
	// MultiRenderTarget
	/////////////////////////////////////////////////////////////////////////////
	class MultiRenderTarget : NonCopyable
	{
	public:
		MultiRenderTarget() = default;
		MultiRenderTarget(unsigned int width, unsigned int height, unsigned int count = 3, RenderbufferParameter render_buf_para = {}, TextureParameter render_target_tex_para_config[] = {});
		~MultiRenderTarget() { Delete(); }
		bool Generate(unsigned int width, unsigned int height, unsigned int count = 3, RenderbufferParameter render_buf_para = {}, TextureParameter render_target_tex_para_config[] = nullptr);

		void ResizeBuffer(unsigned int width, unsigned int height);
		void Bind();
		void UnBind();
		/// <summary>
		/// Deletes Framebuffer and its resources
		/// </summary>
		void Delete();

		void BindTextureIdx(unsigned int idx, unsigned int slot = 0);

		inline unsigned int GetWidth() const { return mWidth; }
		inline unsigned int GetHeight() const { return mHeight; }
		inline unsigned int GetRenderTargetTextureGPU_ID(unsigned int idx = 0) { return mRenderTextures[static_cast<size_t>(idx)].GetID(); }
		inline Texture GetRenderTargetTexture(unsigned int idx = 0) { return mRenderTextures[static_cast<size_t>(idx)]; }
		inline std::vector<Texture>& GetRenderTargetTextures() { return mRenderTextures; }

	private:
		unsigned int mWidth,
			mHeight = 256;

		unsigned int mID,
			mRenderbufferID;

		std::vector<Texture> mRenderTextures;
		RenderbufferParameter mRenderbufferPara;
	};

	/////////////////////////////////////////////////////////////////////////////
	// UNIFORM BUFFER
	/////////////////////////////////////////////////////////////////////////////
	class UniformBuffer : NonCopyable
	{
	private:
		unsigned int mID = 0;
		//utility flag
		bool mLoaded = false;
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