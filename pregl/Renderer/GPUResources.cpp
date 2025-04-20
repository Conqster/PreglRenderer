#include "GPUResources.h"
#include <stb_image/stb_image.h>

#include "GLErrorAssertion.h"
#include "Core/Log.h"

namespace GPUResource {

	namespace ToOpenGL {

		static GLint Format(IMGFormat format)
		{
			switch (format)
			{
				case IMGFormat::RGB: return GL_RGB;
				case IMGFormat::RGBA: return GL_RGBA;

				case IMGFormat::DEPTH: return GL_DEPTH_COMPONENT;

				case IMGFormat::RGB16F: return GL_RGB16F;
				case IMGFormat::RGBA16F: return GL_RGBA16F;

				case IMGFormat::RGB32F: return GL_RGB32F;
				case IMGFormat::RGBA32F: return GL_RGBA32F;

				case IMGFormat::RGBA16: return GL_RGBA16;

				case IMGFormat::RED: return GL_RED;
				case IMGFormat::RG: return GL_RG;
			}

			DEBUG_LOG_ERROR("[TEXTURE]: Format not supported yet !!!!!!!!!!!!!!!!!!!!!!!!!!!");
			return GL_RGB;
		}


		static GLint TexWrap(TexWrapMode mode)
		{
			switch (mode)
			{
				case TexWrapMode::REPEAT: return GL_REPEAT;
				case TexWrapMode::CLAMP: return GL_CLAMP_TO_BORDER;
			}

			DEBUG_LOG_ERROR("[TO OPENGL WRAP]: Wrap mode not supported yet !!!!!!!!!!!!!!!!!!!!!!!!!!!");
			return GL_REPEAT;
		}


		static GLint TexFilter(TexFilterMode mode)
		{
			switch (mode)
			{
				case TexFilterMode::LINEAR: return GL_LINEAR;
				case TexFilterMode::NEAREST: return GL_NEAREST;
			}

			DEBUG_LOG_ERROR("[TO OPENGL FILTER]: Filter mode not supported yet !!!!!!!!!!!!!!!!!!!!!!!!!!!");
			return GL_LINEAR;
		}

		static GLint Type(DataType type)
		{
			switch (type)
			{
			case DataType::UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
			case DataType::FLOAT: return GL_FLOAT;
			}

			DEBUG_LOG_ERROR("[TO OPENGL FILTER]: Filter mode not supported yet !!!!!!!!!!!!!!!!!!!!!!!!!!");
			return GL_LINEAR;
		}
	} // To OpenGL


	///////////////////////////////////////////////////////////////////////////////
	// TEXTURE
	///////////////////////////////////////////////////////////////////////////////
	Texture::Texture(unsigned int width, unsigned int height, TextureParameter parameter) :
					mWidth(width), mHeight(height), mInternalFormat(parameter.imgInternalFormat)
	{
		Generate(width, height, nullptr, parameter);
		DEBUG_LOG_STATUS("[GPUResource]: New Texture Object x: ", width, ", ", height);
	}

	Texture::Texture(unsigned int width, unsigned int height, const void* pixels_data, TextureParameter parameter)
	{
		Generate(width, height, pixels_data, parameter);
		DEBUG_LOG_STATUS("[GPUResource]: New Texture Object with pixel data x: ", width, ", ", height);
	}

	void Texture::Generate(unsigned int width, unsigned int height, const void* pixels_data, TextureParameter parameter)
	{

		mWidth = width;
		mHeight = height;

		//quick check 
		if (parameter.useEqualFormat)
			parameter.format = parameter.imgInternalFormat;


		//check if its already generated on GPU 
		if (mLoaded)
		{
			DEBUG_LOG_WARNING("[GPUResource - Texture]: Texture already loaded.");
			//void* old_px_data = nullptr;
			//void* old_px_data = new unsigned char[mWidth * mHeight * Utilities::IMGFormatChannelCount(mParameter.imgInternalFormat)];
			void* old_px_data = RetrieveGPUImageBuffer();
			//RetrieveGPUImageBuffer(old_px_data);
			GLCall(glBindTexture(GL_TEXTURE_2D, mID));
			//glGetTexImage(GL_TEXTURE_2D, 0, ToOpenGL::Format(mParameter.imgInternalFormat), ToOpenGL::Type(mParameter.pxDataType), old_px_data);
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, ToOpenGL::Format(parameter.imgInternalFormat), width, height, 0, ToOpenGL::Format(parameter.format), ToOpenGL::Type(parameter.pxDataType), old_px_data));
			//if (old_px_data)
				//delete old_px_data;
		}
		else
		{
			GLCall(glGenTextures(1, &mID));
			GLCall(glBindTexture(GL_TEXTURE_2D, mID)); 
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, ToOpenGL::Format(parameter.imgInternalFormat), width, height, 0, ToOpenGL::Format(parameter.format), ToOpenGL::Type(parameter.pxDataType), pixels_data));
		}

		mInternalFormat = parameter.imgInternalFormat;
		mType = parameter.textureType;
		mParameter = parameter; //<- update parameter (??? after GPU retrival if needed)


		//wrap
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToOpenGL::TexWrap(parameter.wrapMode)));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToOpenGL::TexWrap(parameter.wrapMode)));
		//filter
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToOpenGL::TexFilter(parameter.filterMode)));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToOpenGL::TexFilter(parameter.filterMode)));
	
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
		DEBUG_LOG_STATUS("[GPUResource]: New Texture Object ID: ", mID, " , ", mWidth, " , ", mHeight);

		mLoaded = true;
	}

	bool Texture::GenerateFromFile(const char* file_path, bool flip_uv, TextureParameter parameter)
	{
		stbi_set_flip_vertically_on_load(flip_uv);


		//mFilePath = std::string(file_path);
		mFilePath = file_path;
		mParameter = parameter;
		int bit_depth;
		unsigned char* image_buffer = stbi_load(file_path, &mWidth, &mHeight, &bit_depth, 4);


		if (!image_buffer)
		{
			PGL_ASSERT_WARN(image_buffer, "Failed to load Image from file to buffer, PATH: ", file_path);
			return false;
		}

		mInternalFormat = parameter.imgInternalFormat;
		mType = parameter.textureType;
		//check if its already generated on GPU 
		if (mLoaded)
		{
			DEBUG_LOG_WARNING("[GPUResource - Texture]: Texture already loaded.");
		}
		else
		{
			GLCall(glGenTextures(1, &mID));
		}
		glBindTexture(GL_TEXTURE_2D, mID);

		//wrap
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToOpenGL::TexWrap(parameter.wrapMode)));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToOpenGL::TexWrap(parameter.wrapMode)));
		//filter
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToOpenGL::TexFilter(parameter.filterMode)));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToOpenGL::TexFilter(parameter.filterMode)));

		if (parameter.useEqualFormat)
		{
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, ToOpenGL::Format(mInternalFormat), mWidth, mHeight, 0, ToOpenGL::Format(mInternalFormat), ToOpenGL::Type(parameter.pxDataType), image_buffer));
		}
		else
		{
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, ToOpenGL::Format(mInternalFormat), mWidth, mHeight, 0, ToOpenGL::Format(parameter.format), ToOpenGL::Type(parameter.pxDataType), image_buffer));
		}
		
		glGenerateMipmap(GL_TEXTURE_2D);

		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		if (image_buffer)
			stbi_image_free(image_buffer);

		DEBUG_LOG_STATUS("[GPUResource]: New Texture Object ID: ", mID, " , ", mWidth, " , ", mHeight);
		mLoaded = true;
		return true;
	}

	//void Texture::RetrieveGPUImageBuffer(void*& ptr_img_buffer)
	unsigned char* Texture::RetrieveGPUImageBuffer()
	{
		//need to know previous texture image data not incoming data 
		if (mLoaded)
		{
			//unsigned char* img_buffer = new unsigned char[mWidth * mHeight * Utilities::IMGFormatChannelCount(mParameter.imgInternalFormat)];
			//unsigned char* img_buffer;
			unsigned char* img_buffer = new unsigned char[sizeof(float) * mWidth * mHeight * Utilities::IMGFormatChannelCount(mParameter.imgInternalFormat)];
			Bind();
			//glGetTexImage(GL_TEXTURE_2D, 0, ToOpenGL::Format(mParameter.imgInternalFormat), ToOpenGL::Type(mParameter.pxDataType), (void*)img_buffer);
			GLCall(glGetTexImage(GL_TEXTURE_2D, 0, ToOpenGL::Format(mParameter.imgInternalFormat), ToOpenGL::Type(mParameter.pxDataType), img_buffer));
			DEBUG_LOG_STATUS("[TEXTURE ID ", mID, "]: Successfully retrived texture image from GPU Texture buffer. Width: ", mWidth, ", Height: ", mHeight, ".");
			return img_buffer;
			Disactivate();
		}
		DEBUG_LOG_WARNING("[TEXTURE]: Trying to access GPU texture image data that does not exist");
		return nullptr;
	}

	void Texture::Bind() const
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, mID));
	}

	void Texture::Activate(unsigned int slot) const
	{
		GLCall(glActiveTexture(GL_TEXTURE0 + slot));
		GLCall(glBindTexture(GL_TEXTURE_2D, mID));
	}

	void Texture::Disactivate() const
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void Texture::Clear()
	{
		if (mLoaded)
		{
			GLCall(glDeleteTextures(1, &mID));
			mID = 0;
			DEBUG_LOG_STATUS("[GPUResource]: Cleared Texture Object x: ", mWidth, ", ", mHeight, " From Image (Map).");
			mLoaded = false;
		}
	}


	/////////////////////////////////////////////////////////////////////////////
	// FRAMEBUFFER
	////////////////////////////////////////////////////////////////////////////
	Framebuffer::Framebuffer(unsigned int width, unsigned int height)
	{
		Generate(width, height);
	}

	bool Framebuffer::Generate(unsigned int width, unsigned int height)
	{
		glGenFramebuffers(1, &mID);
		glBindFramebuffer(GL_FRAMEBUFFER, mID);

		///////////////////////////////////////////////////////////////////////
		// create colour attachment texture for frame buffer
		///////////////////////////////////////////////////////////////////////
		//Texture Parameter
		TextureParameter tex_parameter{
			IMGFormat::RGB,			//imgInternalFormat
			TextureType::RENDER,	//textureType

			TexWrapMode::CLAMP,		//wrapMode
			TexFilterMode::LINEAR,	//filterMode
			DataType::FLOAT,		//pxDataType
		};

		mWidth = width;
		mHeight = height;
		mRenderTexture.Generate(width, height, nullptr, tex_parameter);

		//attach this new texture(fboTex) to the framebuffer FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderTexture.GetID(), 0);


		///////////////////////////////////////////////////////////////////////
		// create a render buffer object for depth and stencil 
		///////////////////////////////////////////////////////////////////////
		glGenRenderbuffers(1, &mRenderbufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRenderbufferID);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			DEBUG_LOG_WARNING("[GPUResource - FRAMEBUFFER]:   Framebuffer did not complete!!!!");
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			return false;
		}

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		DEBUG_LOG_STATUS("[GPUResource - FRAMEBUFFER]: Framebuffer Generated, width: ", mWidth, "height: ", mHeight);
		return true;
	}

	bool Framebuffer::ResizeBuffer(unsigned int width, unsigned int height)
	{
		DEBUG_LOG_STATUS("[GPUResource - FRAMEBUFFER - RESIZE]: Resizing FBO from width: ", mWidth, "height: ", mHeight);
		if (mWidth == width && mHeight == height)
		{
			DEBUG_LOG_WARNING("[GPUResource - FRAMEBUFFER - RESIZE]: No need to Resize, FBO width: ", mWidth, "height: ", mHeight);
			return false;
		}
		
		mWidth = width;
		mHeight = height;
		//Texture Parameter
		TextureParameter tex_parameter{
						IMGFormat::RGB,			//imgInternalFormat
						TextureType::RENDER,	//textureType

						TexWrapMode::CLAMP,		//wrapMode
						TexFilterMode::LINEAR,	//filterMode
						DataType::FLOAT,		//pxDataType
		};


		//texture checks if the same texture gpu id was created before,
		//if so a new is not generate but old one parameters gets modified
		mRenderTexture.Generate(mWidth, mHeight, nullptr, tex_parameter);

		//resize render buffer
		glBindRenderbuffer(GL_RENDERBUFFER, mRenderbufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			DEBUG_LOG_WARNING("[GPUResource - FRAMEBUFFER - RESIZE]:   Framebuffer did not complete!!!!");
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			return false;
		}
		glBindRenderbuffer(GL_RENDERBUFFER, 0);


		DEBUG_LOG_STATUS("[GPUResource - FRAMEBUFFER - RESIZE]: Framebuffer Resized, width: ", mWidth, ", height: ", (int)mHeight);
		return true;
	}

	void Framebuffer::Bind()
	{
		glViewport(0, 0, mWidth, mHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, mID);
	}

	void Framebuffer::UnBind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Delete()
	{
		if (mID != 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &mID);

			glDeleteRenderbuffers(1, &mRenderbufferID);
			DEBUG_LOG_STATUS("[GPUResource - FRAMEBUFFER]: Framebuffer Deleted.");
			mID = 0;
			mRenderbufferID = 0;
			mRenderTexture.Clear();
		}
	}

	Texture Framebuffer::Delete_FBO_GetTexture()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &mID);

		glDeleteRenderbuffers(1, &mRenderbufferID);

		DEBUG_LOG_STATUS("[GPUResource - FRAMEBUFFER]: Framebuffer Deleted, and tried retriving Render Texture.");
		
		mID = 0;
		mRenderbufferID = 0;
		return mRenderTexture;
	}

	void Framebuffer::BindTexture(unsigned int slot)
	{
		mRenderTexture.Activate(slot);
	}

	/////////////////////////////////////////////////////////////////////////////
	// SHADOW MAP
	/////////////////////////////////////////////////////////////////////////////
	ShadowMap::ShadowMap(unsigned int in_size) : mSize(in_size)
	{
		Generate(in_size);
	}

	void ShadowMap::Generate(unsigned int size)
	{
		GLCall(glGenFramebuffers(1, &mID));

		//generate img to render to (colour attachment/render texture) 
		//Texture Parameter
		TextureParameter tex_parameter{
			IMGFormat::DEPTH,			//imgInternalFormat
			TextureType::SHADOW_MAP,	//textureType

			TexWrapMode::CLAMP,		//wrapMode
			TexFilterMode::NEAREST,	//filterMode
			DataType::FLOAT,			//pxDataType
		};
		mTexture.Generate(size, size, nullptr, tex_parameter);
		mSize = size;
		//just incase texture is un binded
		mTexture.Bind();

		float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mID));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTexture.GetID(), 0));
		GLCall(glDrawBuffer(GL_NONE));
		GLCall(glReadBuffer(GL_NONE));


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			DEBUG_LOG_WARNING("[GPUResource - FRAMEBUFFER]:  Shadow map FBO did not complete!!!!");
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			return;
		}

		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		DEBUG_LOG_STATUS("[GPUResource - FRAMEBUFFER]: Shadow Map Generated", (int)mSize);
	}

	void ShadowMap::Write()
	{
		glViewport(0, 0, mSize, mSize);
		glBindFramebuffer(GL_FRAMEBUFFER, mID);
	}

	void ShadowMap::Read(unsigned int slot) const
	{
		mTexture.Activate(slot);
	}

	void ShadowMap::Destroy()
	{
		GLCall(glDeleteFramebuffers(1, &mID));
		mTexture.Clear();
		mID = 0;
		DEBUG_LOG_STATUS("[GPUResource - FRAMEBUFFER]: Shadow Map Destory.", (int)mSize);
	}


	/////////////////////////////////////////////////////////////////////////////
	// UNIFORM BUFFER
	/////////////////////////////////////////////////////////////////////////////
	UniformBuffer::UniformBuffer(signed long long int size)
	{
		Generate(size);
	}
	UniformBuffer::~UniformBuffer()
	{
		Delete();
	}


	void UniformBuffer::Generate(signed long long int size)
	{
		GLCall(glGenBuffers(1, &mID));

		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, mID));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		//TRY TO COMMENT OUT BELOW LATER 
		// need to configure which block to bind to see BindBufferRndIdx
		//GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_ID, 0, size));

		DEBUG_LOG_STATUS("[GPUResource - UNIFROMBUFFER]: buffer generated id: ", mID, ", size: ", (int)size);
		mLoaded = true;
	}
	void UniformBuffer::Bind() const
	{
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, mID));
	}
	void UniformBuffer::UnBind() const
	{
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}
	void UniformBuffer::SetBufferSubData(const void* data, unsigned long long int size, unsigned int offset) const
	{
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
	}
	void UniformBuffer::BindBufferRndIdx(const unsigned int block_idx, unsigned long long int size, unsigned int offset) const
	{
		GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, block_idx, mID, offset, size));
	}
	void UniformBuffer::SetSubDataByID(const void* data, signed long long int size, unsigned int offset) const
	{
		GLCall(glNamedBufferSubData(mID, offset, size, data));
	}
	void UniformBuffer::Delete()
	{
		if (mLoaded)
		{
			DEBUG_LOG_STATUS("[GPUResource - UNIFROMBUFFER]: deleting buffer: ", mID);
			GLCall(glDeleteBuffers(1, &mID));
			mLoaded = false;
		}
	}



} //GPUResource namespace
 