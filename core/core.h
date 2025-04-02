//
//  core.h
//  FFMPEG OpenGL
//
//  Created by Dmitri Wamback on 2025-03-30.
//

#ifndef core_h
#define core_h

extern "C" {

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

}

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>

#include <GL/glew.h>
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::mutex frameMutex;
std::atomic<bool> isRunning(true);
uint8_t* frameData;

#include "render/shader.h"
#include "render/texture.h"

Texture texture;
Shader shader;
GLFWwindow* window;

bool firstFrame = false;

#include "object/plane.h"
#include "object/camera.h"


void loadFrames(int* _width, int* _height, bool runForever) {
    avformat_network_init();
    const char* videoPath = "/Users/dmitriwamback/Documents/Projects/FFMPEG OpenGL/FFMPEG OpenGL/test.mp4";
    
    AVFormatContext* formatCtx = avformat_alloc_context();
    
    if (avformat_open_input(&formatCtx, videoPath, nullptr, nullptr) != 0) {
        std::cout << "failed to load image\n";
    }
    
    avformat_find_stream_info(formatCtx, nullptr);
    int videoStream = -1;
    
    for (uint32_t i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    
    if (videoStream == -1) {
        return;
    }
    
    AVCodecParameters* codecParameters  = formatCtx->streams[videoStream]->codecpar;
    const AVCodec* pCodec               = avcodec_find_decoder(codecParameters->codec_id);
    AVCodecContext* codecCtx            = avcodec_alloc_context3(pCodec);
    
    avcodec_parameters_to_context(codecCtx, codecParameters);
    avcodec_open2(codecCtx, pCodec, nullptr);
    
    int width = codecCtx->width, height = codecCtx->height;
    
    *_width = width;
    *_height = height;
    
    frameData = static_cast<uint8_t*>(malloc(width * height * 3));
    
    AVFrame *frame = av_frame_alloc(), *frameRGB = av_frame_alloc();
    SwsContext* swsCtx = sws_getContext(width, height, codecCtx->pix_fmt, width, height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    
    int nbBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(nbBytes);
    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);
    
    AVPacket* packet = av_packet_alloc();
        
    do {
        while (av_read_frame(formatCtx, packet) >= 0 && isRunning) {
            
            if (packet->stream_index != videoStream) continue;
            
            if (avcodec_send_packet(codecCtx, packet) != 0) continue;
            if (avcodec_receive_frame(codecCtx, frame) != 0) continue;
            
            sws_scale(swsCtx, frame->data, frame->linesize, 0, height, frameRGB->data, frameRGB->linesize);
            
            memcpy(frameData, frameRGB->data[0], nbBytes);
            if (!runForever) {
                return;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(7));
            av_packet_unref(packet);
        }
        
        av_seek_frame(formatCtx, videoStream, 0, AVSEEK_FLAG_BACKWARD);
        
    } while (runForever && isRunning);
    
    av_frame_free(&frame);
    av_frame_free(&frameRGB);
    av_packet_free(&packet);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    av_free(buffer);
}


void initialize() {
    
    
    if (!glfwInit()) {
        throw std::runtime_error("Couldn't initialize glfw");
    }
    
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
    window = glfwCreateWindow(1200, 800, "FFMPEG OpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    Camera::Initialize();
    
    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    int width, height;
    loadFrames(&width, &height, false);
    texture = Texture::LoadTextureWithBytes(frameData, width, height);
    
    std::thread video = std::thread(loadFrames, &width, &height, true);
    
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    shader = Shader::Create("/Users/dmitriwamback/Documents/Projects/FFMPEG OpenGL/FFMPEG OpenGL/shader/main");
    
    Plane plane = Plane::Create();
    
    while (!glfwWindowShouldClose(window)) {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.3, 0.3, 0.3, 0.0);
        
        glm::vec4 movement = glm::vec4(0.0f);

        movement.z = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ?  0.05f : 0;
        movement.w = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? -0.05f : 0;
        movement.x = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ?  0.05f : 0;
        movement.y = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? -0.05f : 0;
        
        float up = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? 0.05f : 0;
        float down = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? -0.05f : 0;
        
        camera.Update(movement, up, down);
        std::cout << camera.position.x << " " << camera.position.y << " " << camera.position.z << '\n';
        
        shader.Use();
        shader.SetMatrix4("projection", camera.projection);
        shader.SetMatrix4("lookAt", camera.lookAt);
        
        texture.Bind();
        plane.Render(shader);
        
        texture.UpdateWithBytes(frameData);
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    isRunning = false;
    video.join();
}

#endif /* core_h */
