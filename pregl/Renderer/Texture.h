#pragma once

#include <string>

class Texture
{
public:
	Texture() = default;
	Texture(std::string file_path, bool flip_uv);

	bool LoadTexture(std::string file_path, bool flip_uv);
	bool Activate(unsigned int slot = 0) const;
	void Disactivate() const;

	unsigned int GetID() { return mID; }

	void Clear();
	~Texture() = default;
private:
	unsigned int mID = 0;
	bool bLoaded = false;
};