#include "Texture.h"
#include <stb_image/stb_image.h>

#include "ErrorAssertion.h"

Texture::Texture(std::string file_path, bool flip_uv)
{
	LoadTexture(file_path, flip_uv);
}

bool Texture::LoadTexture(std::string file_path, bool flip_uv)
{
	stbi_set_flip_vertically_on_load(flip_uv);

	int width;
	int height;
	int bit_depth;
	unsigned char* image_buffer = stbi_load(file_path.c_str(), &width, &height, &bit_depth, 4);

	if (!image_buffer)
	{
		printf("Faild to Load Image from file: %s\n", file_path.c_str());
		bLoaded = false;
		return false;
	}

	glGenTextures(1, &mID);
	glBindTexture(GL_TEXTURE_2D, mID);

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer));

	glGenerateMipmap(GL_TEXTURE_2D);

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	if (image_buffer)
		stbi_image_free(image_buffer);

	bLoaded = true;
	return true;
}

bool Texture::Activate(unsigned int slot) const
{
	if (!bLoaded)
		return false;
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, mID));
	return true;
}

void Texture::Disactivate() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::Clear()
{
	GLCall(glDeleteTextures(1, &mID));
	mID = 0;
}
