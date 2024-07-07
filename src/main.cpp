#define SDL_MAIN_HANDLED
//#define GRAPHICS_API_OPENGL_11
//#define GRAPHICS_API_OPENGL_33




#include <math.h>
#include <vector>
#include <algorithm>
#include <utils.hpp>
#include "rlgl.h"
#include "raylib.h"
#include "globals.hpp"
#include <iostream>
#include <filesystem>
#include "fastrender.hpp"
#include <gui.hpp>
#include "fs.hpp"
#include "state.hpp"
#include "zip.h"
#include <condition_variable>
#include <assert.h>
#include "time_util.hpp"
#include <queue>
#include "settingsParser.hpp"
//#include "SDLutils.hpp"


u32 __stacksize__= 512 * 1024;


std::condition_variable cv;

double avgFPS = Global.FPS;
double avgHZ = 1000;

double Mx = 0;
double My = 0;

int VSYNC = 0;

int avgFPSqueueNUM = 0;
int avgHZqueueNUM = 0;
double avgFPSqueueSUM = 0;
double avgHZqueueSUM = 0;
std::queue<double> avgFPSq;
std::queue<double> avgHZq;

bool dumbsleep = false;


//hello from arch!
Globals Global;


void RenderLoop(void *){
    double last = 0;
    C3D_Init(0x100000);//C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    
    std::cout << "WAITING FOR 0.5SECS\n";

    SleepInUs(1*500*1000);

    std::cout << "C2D INIT\n";
    
    std::cout << "parsing the settings.ini file...\n";
    parseSettings();
    
    InitAudioDevice();
    
    if(Global.useTopScreen){
        Global.window = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    }
    else{
        Global.window = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    } 
    Global.gpu_currentRenderTarget = Global.window;
    C2D_Prepare();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_Flush(); 
    C2D_SceneBegin(Global.window);
    C2D_SetTintMode(C2D_TintMult);
    EndBlendMode();
    ClearBackground(Global.Background);
    C2D_Flush();  //test
    C3D_FrameEnd(0);
    std::cout << "Clear first bg\n";

    //RenderTexture2D frameGraph = LoadRenderTexture(512, 512);

    std::cout << "loadframegraph\n";

    //C2D_Prepare();
    //C2D_Flush(); 
    int loc = 0;
    int lastFPS = 0;
    while(!WindowShouldClose() and !Global.stop){
        auto t1 = std::chrono::steady_clock::now();
        last = getTimer();
        //rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

        //std::cout << "begin\n";
        C2D_Prepare();
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_SceneBegin(Global.window);
        
        //C2D_Prepare();
        Global.mutex.lock();
        //LightLock_Lock(&Global.lightlock);
        //ClearBackground(Global.Background);
        if(Global.NeedForBackgroundClear)
            ClearBackground(Global.Background);
        if(Global.GameTextures == -1)
            Global.gameManager->unloadGameTextures();
        else if(Global.GameTextures == 1)
            Global.gameManager->loadGameTextures();
            
        Global.CurrentState->render();
        

        //DrawTextureCenter(Global.cursor, GetMouseX(), GetMouseY() , 0.1f, {255,255,255,255});

        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse(); 




        DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %.3f TPS: %.3f",  avgFPS, avgHZ), {ScaleCordX(5), ScaleCordY(5)}, Scale(10), Scale(1), GREEN);
        if(C3D_GetCmdBufUsage() > 0.8f){
            std::cout << "NEARLY OVERFLOWING THE COMMAND BUFFER, BEWARE: " << C3D_GetCmdBufUsage() * 100 << "%\n";
        }
        C2D_Flush();  //test
        Global.mutex.unlock();
        C3D_FrameEnd(0);

        if(Global.sliderTexNeedDeleting)
            Global.gameManager->unloadSliderTextures();

        std::chrono::duration<double, std::milli> elapsed {std::chrono::steady_clock::now() - t1};
        double fps = (1000.0f / (elapsed.count()));
        lastFPS = fps;
        if(lastFPS > 511)
            lastFPS = 511;
        //if(elapsed.count() > 20 and VSYNC == 0)
        //    std::cout << "dropped frame with " << elapsed.count() << "ms\n";
        avgFPSq.push(fps);
        avgFPSqueueSUM += fps;
        if(avgFPSq.size() > 300){
            avgFPSqueueSUM -= avgFPSq.front();
            avgFPSq.pop();
        }
        avgFPS = avgFPSqueueSUM / (double)(avgFPSq.size());

    }
    //UnloadRenderTexture(frameGraph);
}




int main(){
    
    osSetSpeedupEnable(true);
    gfxInitDefault();
    aptSetSleepAllowed(true);
    consoleGetDefault()->fg = 23;
    if(Global.useTopScreen){
        consoleInit(GFX_BOTTOM, NULL);
    }
    else{
        consoleInit(GFX_TOP, NULL);
    }

    //consoleGetDefault()->flags &= ~CONSOLE_COLOR_BOLD;
	//consoleGetDefault()->flags |= CONSOLE_COLOR_FAINT;
    consoleGetDefault()->fg = 23;
    std::cout << "Loaded gpu\n";

    //SDL_SetMainReady();
    Global.CurrentState = std::make_shared<MainMenu>();
    for(int i = 0; i < Global.GamePath.size(); i++) {
        if (Global.GamePath[i] == '\\')
            Global.GamePath[i] = '/';
    }
    
    //LightLock_Init(&Global.lightlock);

    std::cout << "Loaded gamepath\n";
    //SetTraceLogLevel(LOG_WARNING); //LOG_WARNING
    InitAudioDevice();
    std::cout << linearSpaceFree() << std::endl;    
    Global.linearSpaceFree = linearSpaceFree();
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetAudioStreamBufferSizeDefault(128);
    //InitWindow(640, 480, "osus - amogus");
    
    //SDL_Surface* pIcon = SDL_CreateRGBSurface(0,64,64,32,0,0,0,0);;
    //SDL_SetWindowIcon((SDL_Window*)GetWindowSDL(), pIcon);
    //SDL_FreeSurface(pIcon);
    Global.OsusLogo = LoadTexture("sdmc:/3ds/resources/osus.png");
    std::cout << "Loaded logo\n";
    
    Global.DefaultFont = LoadFont("sdmc:/3ds/resources/telegrama_render.otf");
    std::cout << "Loaded font\n";
    
    

    Global.shdrOutline = LoadShader(0, TextFormat("sdmc:/3ds/resources/shaders/glsl%i/outline.fs", 100));

    Global.shdrTest = LoadShader(TextFormat("sdmc:/3ds/resources/shaders/glsl%i/mcosu.vsh", 330), TextFormat("sdmc:/3ds/resources/shaders/glsl%i/mcosu.fsh", 330));
    

    //Image cus;
    std::string lastPath = Global.Path;
	Global.Path = "sdmc:/3ds/resources/default_skin/";
	std::vector<std::string> files = ls(".png");
	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("cursortrail.png", 0) == 0)
				Global.cursorTrail = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("cursor.png", 0) == 0)
                Global.cursor = LoadTexture((Global.Path + files[i]).c_str());
		}
	}
	files.clear();
    Global.Path = "sdmc:/3ds/resources/skin/";
    files = ls(".png");
	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("cursortrail.png", 0) == 0)
				Global.cursorTrail = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("cursor.png", 0) == 0)
                Global.cursor = LoadTexture((Global.Path + files[i]).c_str());
		}
	}
	files.clear();
    Global.Path = lastPath;
    std::cout << "Loaded skin";

    SetTextureFilter(&Global.DefaultFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(&Global.cursor, TEXTURE_FILTER_BILINEAR);


    SetTextureFilter(&Global.OsusLogo, TEXTURE_FILTER_BILINEAR);

    std::cout << "Loaded all files and filters\n";
    std::cout << "Global.cursor size: " << Global.cursor.width << " x " << Global.cursor.height << " y \n";
    double avgFrameTime;
    HideCursor();
    initMouseTrail();

    std::cout << "Cursor init done\n";

    Global.LastFrameTime = getTimer();
    double lastFrame = getTimer();
    Global.GameTextures = 0;
    std::cout << "Start render loop\n";
    /*while(true){
        PollInputEvents();
        if(IsKeyDown(KEY_SELECT)){
            while(true){
                PollInputEvents();
                if(!IsKeyDown(KEY_SELECT)){
                    break;
                }
                svcSleepThread(10000);
            }
            break;
        }
        svcSleepThread(10000);
    }*/
    //std::thread rend(RenderLoop);
    std::cout << "Free Vram: " << vramSpaceFree() << std::endl;
	std::cout << "Free M_ALL: " << osGetMemRegionFree(MEMREGION_ALL) << "/" << osGetMemRegionSize(MEMREGION_ALL) << std::endl;
	std::cout << "Free M_APP: " << osGetMemRegionFree(MEMREGION_APPLICATION) << "/" << osGetMemRegionSize(MEMREGION_APPLICATION) << std::endl;
	std::cout << "Free M_SYS: " << osGetMemRegionFree(MEMREGION_SYSTEM) << "/" << osGetMemRegionSize(MEMREGION_SYSTEM) << std::endl;
	std::cout << "Free M_BSE: " << osGetMemRegionFree(MEMREGION_BASE) << "/" << osGetMemRegionSize(MEMREGION_BASE) << std::endl;
    std::cout << "Free M_LIN: " << linearSpaceFree() << "/" << Global.linearSpaceFree << std::endl;

    Thread renderThread;
    renderThread = threadCreate(RenderLoop, NULL, 1*1024*1024, 0x29, -1, false);


    
    while(!WindowShouldClose() and aptMainLoop()){
        double timerXXX = getTimer();
        auto t1 = std::chrono::steady_clock::now();
        Global.mutex.lock();
        //LightLock_Lock(&Global.lightlock);
        PollInputEvents();
        GetScale();
        GetMouse();
        GetKeys();
        updateUpDown();
        if(IsKeyDown(KEY_START)){
            break;
        }
        Global.FrameTime = getTimer() - Global.LastFrameTime;
        Global.LastFrameTime = getTimer();
        updateMouseTrail();
        Global.CurrentState->update();
        Global.mutex.unlock();
        //LightLock_Unlock(&Global.lightlock);

        std::chrono::duration<double, std::milli> sleepTime {std::chrono::steady_clock::now() - t1};
        unsigned int sleepTimeInt = (unsigned int)(std::max(0.0, (1000.0/(float)(Global.TPS)) - sleepTime.count()) * 980.0);
        if(!dumbsleep)
            SleepInUs(sleepTimeInt);
        
        while(getTimer() - timerXXX < 1000.0/(float)(Global.TPS) and getTimer() - timerXXX >= 0)
            continue;
        
        std::chrono::duration<double, std::milli> elapsed {std::chrono::steady_clock::now() - t1};
        double hz = (1000.0f / (elapsed.count()));
        avgHZq.push(hz);
        avgHZqueueSUM += hz;
        if(avgHZq.size() > 300){
            avgHZqueueSUM -= avgHZq.front();
            avgHZq.pop();
        }
        avgHZ = avgHZqueueSUM / (double)(avgHZq.size());
    }
    Global.stop = true;
    threadJoin(renderThread, U64_MAX);
    threadFree(renderThread);

    Global.CurrentState->unload();
    UnloadTexture(&Global.OsusLogo);
    UnloadTexture(&Global.cursor);
    UnloadFont(&Global.DefaultFont);
    

    C2D_Fini();
	C3D_Fini();
	gfxExit();
    return 0;
}
