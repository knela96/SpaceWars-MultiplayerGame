#include "Networks.h"


extern ID3D11Device *g_pd3dDevice;

bool ModuleTextures::init()
{
	return true;
}

bool ModuleTextures::cleanUp()
{
	for (auto &texture : _textures)
	{
		if (texture.shaderResource != nullptr)
		{
			texture.shaderResource->Release();
			texture.shaderResource = nullptr;
			texture.filename = "";
			texture.size = vec2{ -1.0f , -1.0f };
			texture.used = false;
		}
	}

	return true;
}

Texture* ModuleTextures::loadTexture(const char *filename)
{
	Texture & texture = getTextureSlotForFilename(filename);

	if (texture.shaderResource == nullptr)
	{
		int width, height;
		texture.shaderResource = loadD3DTextureFromFile(filename, &width, &height);
		if (texture.shaderResource == nullptr)
		{
			texture.used = false;
			return nullptr;
		}

		// If not found, add the texture into the first empty slot
		texture.filename = filename;
		texture.size = vec2{ (float)width, (float)height };
		texture.used = true;
	}

	return &texture;
}

Texture * ModuleTextures::loadTexture(void * pixels, int width, int height)
{
	ID3D11ShaderResourceView *shaderResource = loadD3DTextureFromPixels(pixels, width, height);
	if (shaderResource == nullptr) { return nullptr; }

	Texture & texture = getTextureSlotForFilename("###EMPTY_TEXTURE###");
	texture.shaderResource = shaderResource;
	texture.filename = "";
	texture.size = vec2{ (float)width, (float)height };
	texture.used = true;
	return &texture;
}

void ModuleTextures::freeTexture(Texture* tex)
{
	if (tex != nullptr)
	{
		for (auto &texture : _textures)
		{
			if (texture.shaderResource == tex->shaderResource)
			{
				texture.shaderResource->Release();
				texture.shaderResource = nullptr;
				texture.filename = "";
				texture.size = vec2{ -1.0f, -1.0f };
				texture.used = false;
				break;
			}
		}
	}
}

ID3D11ShaderResourceView* ModuleTextures::loadD3DTextureFromFile(const char * filename, int * width, int * height)
{
	ID3D11ShaderResourceView *shaderResourceView = nullptr;

	// Read image pixels
	int nchannels;           // NOTE(jesus): nchanels would be the number of channels without forcing bytes_per_pixel
	int bytes_per_pixel = 4; // NOTE(jesus): 4 is the desired (and final) number of channels
	unsigned char *pixels = stbi_load((char *)filename, width, height, &nchannels, bytes_per_pixel);
	if (pixels == nullptr) {
		LOG("ModuleTextures::loadTexture() - stbi_load() failed.");
		return NULL;
	}

	//// Swap R and B channels (RGB -> BGR)
	//unsigned int size_in_pixels = (*width) * (*height);
	//for (unsigned int i = 0; i < size_in_pixels; ++i)
	//{
	//	unsigned char *pixel = pixels + i * 4 * sizeof(unsigned char);
	//	unsigned char tmp = pixel[0];
	//	pixel[0] = pixel[2];
	//	pixel[2] = tmp;
	//}

	shaderResourceView = loadD3DTextureFromPixels(pixels, *width, *height);

	// Free image pixels
	stbi_image_free(pixels);

	return shaderResourceView;
}

ID3D11ShaderResourceView * ModuleTextures::loadD3DTextureFromPixels(void * pixels, int width, int height)
{
	ID3D11ShaderResourceView *shaderResourceView = nullptr;
	ID3D11Texture2D* handle = nullptr;

	// Create texture description
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1; // NOTE(jesus): 0 means full mipmap
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	//desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create texture in graphics system
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	if (g_pd3dDevice->CreateTexture2D(&desc, &subResource, &handle) < 0) {
		LOG("ModuleTextures::loadTexture() - g_pd3dDevice->CreateTexture() failed.");
		return NULL;
	}

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	if (g_pd3dDevice->CreateShaderResourceView(handle, &srvDesc, &shaderResourceView) != S_OK) {
		LOG("ModuleTextures::loadTexture() - g_pd3dDevice->CreateShaderResourceView() failed.");
		handle->Release();
		return NULL;
	}
	handle->Release();

	return shaderResourceView;
}

Texture & ModuleTextures::getTextureSlotForFilename(const char *filename)
{
	// Protect concurrent access to this shared resource...
	static std::mutex mtx;
	std::unique_lock<std::mutex> lock(mtx);

	// Try to find an existing texture
	for (auto &texture : _textures)
	{
		if (strcmp(texture.filename, filename) == 0)
		{
			return texture;
		}
	}

	// Find the first empty slot
	for (auto &texture : _textures)
	{
		if (strcmp(texture.filename, "") == 0 && !texture.used)
		{
			texture.used = true;
			return texture;
		}
	}

	assert(nullptr && "ModuleTextures::getTextureSlotForFilename() - no more room for textures");

	// Else... add a new texture slot
	static Texture texture;
	return texture;
}
