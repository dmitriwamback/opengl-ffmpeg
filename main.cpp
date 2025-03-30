//
//  main.cpp
//  FFMPEG OpenGL
//
//  Created by Dmitri Wamback on 2025-03-30.
//

#include <iostream>
extern "C" {

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

}

int main(int argc, const char * argv[]) {
    
    avformat_network_init();
    std::cout << "Hello, World!\n";
    
    return 0;
}
