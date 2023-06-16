#include "knuxfanscreen.h"
#include "titlescreen.h"
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h> 
}
//#include <libavutil/frame.h>
#include <SDL2/SDL.h>
//The following is a super bastardized version of SDLTris missing a lot of it's core functions "for the funny"
std::vector<Mix_Music*> generateMusic(std::string path);
std::vector<Mix_Chunk*> generateSounds(std::string path);
std::vector<SDL_Texture*> generateSurfaces(std::string path, SDL_Renderer* renderer);
bool hasEnding(std::string const& fullString, std::string const& ending);
bool compareFunction (std::string a, std::string b) {return a<b;} 
bool bgCompare (bg a, bg b) {return a.name<b.name;} 
static Uint32 next_time;
void display(AVCodecContext*, AVPacket*, AVFrame*,
    SDL_Texture*, SDL_Texture*, SDL_Renderer*, double);
Uint32 time_left(void)
{
    Uint32 now;

    now = SDL_GetTicks();
    if(next_time <= now)
        return 0;
    else
        return next_time - now;
}
#define TICK_INTERVAL    7
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
int main() {
    AVFormatContext *pFormatCtx;
    int vidId = -1, audId = -1;
    double fpsrendering = 0.0;
    AVCodecContext *vidCtx, *audCtx;
    const  AVCodec *vidCodec, *audCodec;
    AVCodecParameters *vidpar, *audpar;
    AVFrame *vframe, *aframe;
    AVPacket *packet;
        pFormatCtx = avformat_alloc_context();
    char bufmsg[1024];
    if (avformat_open_input(&pFormatCtx, "./test.mp4", NULL, NULL) < 0) {
        sprintf(bufmsg, "Cannot open %s", "./test.mp4");
        perror(bufmsg);
        return 1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        perror("Cannot find stream info. Quitting.");
        return 1;
    }
    bool foundVideo = false, foundAudio = false;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        AVCodecParameters *localparam = pFormatCtx->streams[i]->codecpar;
        const AVCodec *localcodec = avcodec_find_decoder(localparam->codec_id);
        if (localparam->codec_type == AVMEDIA_TYPE_VIDEO && !foundVideo) {
            vidCodec = localcodec;
            vidpar = localparam;
            vidId = i;
            AVRational rational = pFormatCtx->streams[i]->avg_frame_rate;
            fpsrendering = 1.0 / ((double)rational.num / (double)(rational.den));
            foundVideo = true;
        } else if (localparam->codec_type == AVMEDIA_TYPE_AUDIO && !foundAudio) {
            audCodec = localcodec;
            audpar = localparam;
            audId = i;
            foundAudio = true;
        }
        if (foundVideo && foundAudio) { break; }
    }
    vidCtx = avcodec_alloc_context3(vidCodec);
    audCtx = avcodec_alloc_context3(audCodec);
    if (avcodec_parameters_to_context(vidCtx, vidpar) < 0) {
        perror("vidCtx");
        return 1;
    }
    if (avcodec_parameters_to_context(audCtx, audpar) < 0) {
        perror("audCtx");
        return 1;
    }
    if (avcodec_open2(vidCtx, vidCodec, NULL) < 0) {
        perror("vidCtx");
        return 1;
    }
    if (avcodec_open2(audCtx, audCodec, NULL) < 0) {
        perror("audCtx");
        return 1;
    }

    vframe = av_frame_alloc();
    aframe = av_frame_alloc();
    packet = av_packet_alloc();
    int swidth = vidpar->width;
    int sheight = vidpar->height;
    srand((unsigned)time(0));

    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;

        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Knuxfan's Tetriminos", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;

        SDL_Quit();

        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }
    //Initialize SDL_mixer
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    SDL_Texture* rendertext = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 640,480);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING | SDL_TEXTUREACCESS_TARGET,
        swidth, sheight);
    if (!texture) {
        perror("texture");
        return 1;
    }
    if(SDL_QueryTexture(rendertext, NULL, NULL, NULL, NULL) < 0) {
        printf("Failed to create render plane.\n");
        std::cout << SDL_GetError() << "\n";
    };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, 640, 480);
    std::vector<font*> fonts;
    fonts.push_back(new font("8x8font",renderer));
    fonts.push_back(new font("8x16font",renderer));
    fonts.push_back(new font("small8x8font",renderer));

    std::vector<SDL_Texture*> textures = generateSurfaces("./sprites/", renderer); //DOES THIS CODE EVEN WORK??? WHOOOO KNOWWWSSS?!?!?!?!
    std::vector<Mix_Music*> music = generateMusic("./music/");
    std::vector<Mix_Chunk*> sound = generateSounds("./sound/");
    std::vector<bg> backgrounds;
    rapidxml::file<> bgFile("./backgrounds/backgrounds.xml");
    rapidxml::xml_document<> bgDoc;
    bgDoc.parse<0>(bgFile.data());
    int bgs = atoi(bgDoc.first_node("bgs")->value());
    for(int i = 0; i < bgs; i++) {
            rapidxml::file<> bgFile("./backgrounds/backgrounds.xml");
            rapidxml::xml_document<> bgDoc;
            bgDoc.parse<0>(bgFile.data());
            std::string tmp = "bg" + std::to_string(i);
            std::cout << tmp << "\n";
            std::string bgPath = bgDoc.first_node(tmp.c_str())->value();
            std::cout << "loading background " << bgPath << "\n";
            //std::cout << "HELP ME:" << p.path().filename() << "\n";
            bg backg(bgPath, false, renderer);
            backgrounds.push_back(backg);
    }
    int knxfnbg = std::rand() % backgrounds.size();
    knuxfanscreen screen = knuxfanscreen(renderer, textures, backgrounds, sound.data(),0, fonts[2]);
    titlescreen* title = new titlescreen(renderer, window, &backgrounds[0], textures, music.data(), sound.data(), 0, fonts);
    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    float deltaTime = 0;
    int gamemode = 0;
    bool quit = false;
    SDL_Event event;
    float _fps = 0;
    Mix_HaltMusic();
    Mix_PlayMusic(music[0],-1);
    int gamescene = 0;
    double timer = 0;
    while(!quit) {
                auto t1 = std::chrono::high_resolution_clock::now();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if(event.type == SDL_KEYDOWN && gamemode == 1) {
                title->keyPressed(event.key.keysym.sym);
            }
        }
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = (float)((NOW - LAST) * 1000 / (float)SDL_GetPerformanceFrequency());
        //SDL_SetRenderTarget(renderer, rendertext);
        // SDL_RenderClear(renderer);
        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        switch(gamemode) {
            case 0:  {
                screen.logic(deltaTime);
                screen.render();
                if(screen.endlogic()) {
                    gamemode++;
                }
                break;
            }
            case 1: {
                title->logic(deltaTime);
                title->render();
                int logicret = title->endlogic();
                if (logicret == 1) {
                    Mix_HaltMusic();
                    Mix_PlayChannel(-1,sound[5],0);
                    gamemode++;
                }
                break;
            }
            case 2: {
                switch(gamescene) {
                    case 0: {
                        SDL_RenderClear(renderer);
                        SDL_RenderCopy(renderer,textures[13],NULL,NULL);
                        timer += deltaTime/5;
                        if(timer >= 1400) {
                            gamescene++;
                            timer = 0;
                            Mix_PlayChannel(-1,sound[6],0);
                        }
                        SDL_RenderPresent(renderer);
                        break;
                    }
                    case 1: {
                        SDL_RenderClear(renderer);
                        SDL_RenderCopy(renderer,textures[1],NULL,NULL);
                        timer += deltaTime/5;
                        if(timer >= 1400) {
                            Mix_PlayMusic(music[1],0);
                            gamescene++;
                            timer = 0;
                        }
                        SDL_RenderPresent(renderer);

                        break;
                    }
                    case 2: {
                        while (av_read_frame(pFormatCtx, packet) >= 0) {               
                            while (SDL_PollEvent(&event)) {
                                if (event.type == SDL_QUIT) {
                                    exit(0);
                                }
                            }
                            if (packet->stream_index == vidId) {
                            display(vidCtx, packet, vframe,
                            texture, textures[2], renderer, fpsrendering);

                            } 
                        }
                    }
                }
            }
        }


        // SDL_SetRenderTarget(renderer, NULL);
        // SDL_RenderClear(renderer);
        //SDL_RenderCopy(renderer, rendertext, NULL,NULL);
        
        SDL_RenderPresent(renderer);

        //SDL_Delay(time_left());
        next_time += TICK_INTERVAL;

        auto t2 = std::chrono::high_resolution_clock::now();
        Uint64 ms_int = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        
        _fps =  (1000000 / ms_int);
        //std::cout << "Frame pushed! FPS: " << _fps << "\n";
    }
}

std::vector<Mix_Music*> generateMusic(std::string path) {
    rapidxml::file<> musFile((path+"/music.xml").c_str());
    rapidxml::xml_document<> musDoc;
    musDoc.parse<0>(musFile.data());
    std::vector<Mix_Music*> music;
    Mix_Music* temp;
    int musics = atoi(musDoc.first_node("songs")->value());
    for(int i = 0; i < musics; i++) {
            std::string tmp = "mus" + std::to_string(i);
            std::cout << tmp << "\n";
            std::string musPath = path + "/" + musDoc.first_node(tmp.c_str())->value();
            temp = Mix_LoadMUS(musPath.c_str());
            if (!temp) {
                printf("Failed to load music at %s: %s\n", musPath, SDL_GetError());
                return music;
            }
            music.push_back(temp);
            printf("Successfully loaded music at %s\n", musPath.c_str());

    }
    return music;
}


std::vector<Mix_Chunk*> generateSounds(std::string path) {
    rapidxml::file<> musFile((path+"/sound.xml").c_str());
    rapidxml::xml_document<> musDoc;
    musDoc.parse<0>(musFile.data());
    std::vector<Mix_Chunk*> music;
    Mix_Chunk* temp;
    int musics = atoi(musDoc.first_node("sounds")->value());
    for(int i = 0; i < musics; i++) {
            std::string tmp = "snd" + std::to_string(i);
            std::cout << tmp << "\n";
            std::string musPath = path + "/" + musDoc.first_node(tmp.c_str())->value();
            temp = Mix_LoadWAV(musPath.c_str());
            if (!temp) {
                printf("Failed to load music at %s: %s\n", musPath, SDL_GetError());
                return music;
            }
            music.push_back(temp);
            printf("Successfully loaded music at %s\n", musPath.c_str());

    }
    return music;
}

bool hasEnding(std::string const& fullString, std::string const& ending) { //thank you kdt on Stackoverflow, its late at night and you helped me out https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

std::vector<SDL_Texture*> generateSurfaces(std::string path, SDL_Renderer* renderer) {
    std::vector<SDL_Surface*> surfaces;
    std::vector<SDL_Texture*> textures;
        rapidxml::file<> spriteFile("./sprites/sprites.xml");
    rapidxml::xml_document<> spriteDoc;
    spriteDoc.parse<0>(spriteFile.data());
    int sprites = atoi(spriteDoc.first_node("sprites")->value());

    for(int i = 0; i < sprites; i++) {
        char buff[12];
        snprintf(buff, sizeof(buff), "%02d", i);
        std::string temppath = path + "/" + buff + ".bmp";

        SDL_Surface* temp = SDL_LoadBMP(temppath.c_str());
        if (!temp) {
            printf("Failed to load image at %s: %s\n", temppath, SDL_GetError());
        }
        surfaces.push_back(temp);
        printf("Successfully loaded sprite at %s\n", temppath.c_str());
    }
    for (SDL_Surface* surf : surfaces) {
        SDL_Texture* temp = SDL_CreateTextureFromSurface(renderer, surf);
        if(temp != NULL) {
            textures.push_back(temp);
            printf("pushed texture!!\n");
            SDL_FreeSurface(surf);
        }
        else {
            fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
        }
    }
    return textures;

}

void display(AVCodecContext* ctx, AVPacket* pkt, AVFrame* frame,
    SDL_Texture* texture, SDL_Texture* tvoverlay, SDL_Renderer* renderer, double fpsrend)
{
    time_t start = time(NULL);
    if (avcodec_send_packet(ctx, pkt) < 0) {
        perror("send packet");
        return;
    }
    if (avcodec_receive_frame(ctx, frame) < 0) {
        perror("receive frame");
        return;
    }
    int framenum = ctx->frame_number;
    if ((framenum % 1000) == 0) {
        printf("Frame %d (size=%d pts %d dts %d key_frame %d"
            " [ codec_picture_number %d, display_picture_number %d\n",
            framenum, frame->pkt_size, frame->pts, frame->pkt_dts, frame->key_frame,
            frame->coded_picture_number, frame->display_picture_number);
    }
    SDL_UpdateYUVTexture(texture, NULL,
        frame->data[0], frame->linesize[0],
        frame->data[1], frame->linesize[1],
        frame->data[2], frame->linesize[2]);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderCopy(renderer, tvoverlay, NULL, NULL);

    SDL_RenderPresent(renderer);
    time_t end = time(NULL);
    double diffms = difftime(end, start) / 1000.0;
    if (diffms < fpsrend) {
        uint32_t diff = (uint32_t)((fpsrend - diffms) * 1000);
        printf("diffms: %f, delay time %d ms.\n", diffms, diff);
        SDL_Delay(diff);
    }
}