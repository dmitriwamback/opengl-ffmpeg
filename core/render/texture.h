//
//  texture.h
//  FFMPEG OpenGL
//
//  Created by Dmitri Wamback on 2025-03-30.
//

#ifndef texture_h
#define texture_h

class Texture {
public:
    uint32_t textureID;
    int width, height;
    
    static Texture LoadTextureWithBytes(unsigned char* bytes, int width, int height);
    void Bind();
    void Unbind();
};

Texture Texture::LoadTextureWithBytes(unsigned char* bytes, int width, int height) {
    
    Texture texture = Texture();
    
    glGenTextures(1, &texture.textureID);
    glBindTexture(GL_TEXTURE_2D, texture.textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    texture.width = width;
    texture.height = height;
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, bytes);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

void Texture::Bind() {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

#endif /* texture_h */
