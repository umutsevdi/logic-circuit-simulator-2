#include <filesystem>
#include <fstream>
#include "components.h"

static std::map<std::string, lcs::ui::ImageHandle> _TEXTURE_MAP;
namespace lcs::ui {

const ImageHandle* get_texture(const std::string& key)
{
    auto p = _TEXTURE_MAP.find(key);
    if (p == _TEXTURE_MAP.end()) {
        if (load_texture(
                key, (fs::CACHE / (base64_encode(key) + ".jpeg")).string())) {
            return get_texture(key);
        };
        return nullptr;
    }
    return &p->second;
}

} // namespace lcs::ui

#ifdef _WIN32
#include <windows.h>
#endif
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <GL/gl.h>
#include <stb_image.h>
// Simple helper function to load an image into a OpenGL texture with common
// settings
bool lcs::ui::load_texture(
    const std::string& key, std::vector<unsigned char>& buffer)
{
    // Load from file
    int image_width           = 0;
    int image_height          = 0;
    unsigned char* image_data = stbi_load_from_memory(
        buffer.data(), buffer.size(), &image_width, &image_height, nullptr, 4);
    if (image_data == nullptr) {
        return false;
    }

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
    _TEXTURE_MAP.insert_or_assign(key,
        lcs::ui::ImageHandle {
            (uint32_t)image_texture, image_width, image_height });
    return true;
}

bool lcs::ui::load_texture(const std::string& key, const std::string& file_path)
{
    std::ifstream infile { file_path, std::ios::binary | std::ios::ate };
    if (!infile) {
        return false;
    }

    std::streamsize file_size = infile.tellg();
    if (file_size < 0) {
        return false;
    }
    infile.seekg(0, std::ios::beg);
    std::vector<unsigned char> data;
    data.resize(file_size);
    if (!infile.read(reinterpret_cast<char*>(data.data()), file_size)) {
        return false;
    }

    return load_texture(key, data);
}
