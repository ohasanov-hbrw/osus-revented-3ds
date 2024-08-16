#include "state.hpp"
#include "fs.hpp"
#include <iostream>
#include "utils.hpp"
#include "gamemanager.hpp"
#include "zip.h"
#include "fastrender.hpp"
#include "rlgl.h"
#include "raylib.h"
#include <dirent.h>
#include <gamefile.hpp>
#include <ctype.h>

PlayMenu::PlayMenu() {
    name = TextBox({320,440}, {520,40}, {0,0,0,0}, "BETA VERSION!", WHITE, 20, 50);
    description = TextBox({320,140}, {520,40}, {240,98,161,255}, "Select a Beatmap to play!", WHITE, 20, 50);
    bg = TextBox({320,240}, {530,290}, {240,98,161,255}, "", {240,98,161,255}, 20, 10);
    back = Button({395,360}, {120,40}, {255,135,198,255}, "Back", BLACK, 20);
    select = Button({520,360}, {120,40}, {255,135,198,255}, "Select", BLACK, 20);
    close = Button({70, 110}, {20,20}, {255,135,198,255}, "x", BLACK, 20);
    skin = Switch({310,350}, {40,20}, RED, GREEN, {255,135,198,255}, BLACK);
    sound = Switch({310,370}, {40,20}, RED, GREEN, {255,135,198,255}, BLACK);
    usedskin = TextBox({180,350}, {190,20}, {240,98,161,255}, "Use default skin", WHITE, 20, 50);
    usedsound = TextBox({180,370}, {190,20}, {240,98,161,255}, "Use default sound", WHITE, 20, 50);
    skin.state = Global.settings.useDefaultSkin;
    sound.state = Global.settings.useDefaultSounds;
}

void PlayMenu::init() {
    std::cout << "loading the playmenu/n";
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    temp = Global.Path;
    Global.Path = Global.BeatmapLocation;

    std::vector<std::string> dir = ls(".osu");
    dir_list = SelectableList({320, 250}, {520, 160}, {255,135,198,255}, dir, BLACK, 20, 20, 65);
}
void PlayMenu::render() {
    //Global.mutex.lock();
    bg.render();
    description.render();
    back.render();
    select.render();
    dir_list.render();
    close.render();
    skin.render();
    sound.render();
    usedskin.render();
    usedsound.render();
    name.render();
    //Global.mutex.unlock();
}
void PlayMenu::update() {
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();
    skin.update();
    sound.update();
    /*if(lastIndex != dir_list.selectedindex){
        name.text = dir_list.objects[dir_list.selectedindex].text;
        name.update();
        lastIndex = dir_list.selectedindex;
    }*/
    if(skin.state != Global.settings.useDefaultSkin)
        Global.settings.useDefaultSkin = skin.state;
    
    if(sound.state != Global.settings.useDefaultSounds)
        Global.settings.useDefaultSounds = sound.state;
    
    if(close.action){
        Global.Path = temp;
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

    if(select.action or dir_list.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){
            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if(Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                lastPos = dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osu");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
                lastIndex = -3;
            }
            else{
                Global.selectedPath = Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
                Global.CurrentLocation = "beatmaps/" + lastPos + "/";
                Global.CurrentState->unload();
                Global.CurrentState.reset(new Game());
                Global.CurrentState->init();
            }
        }
    }
    else if(back.action){
        Global.Path = Global.BeatmapLocation;
        auto dir = ls(".osu");
        dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
        dir_list.init();
    }
}
void PlayMenu::unload() {

}

LoadMenu::LoadMenu() {
    description = TextBox({320,140}, {520,40}, {240,98,161,255}, "Select a .OSZ Beatmap file\n    to extract!", WHITE, 20, 50);
    bg = TextBox({320,240}, {530,290}, {240,98,161,255}, "", {240,98,161,255}, 20, 10);
    back = Button({395,360}, {120,40}, {255,135,198,255}, "Back", BLACK, 20);
    select = Button({520,360}, {120,40}, {255,135,198,255}, "Select", BLACK, 20);
    close = Button({70, 110}, {20,20}, {255,135,198,255}, "x", BLACK, 20);
    auto dir = ls(".osz");
    dir_list = SelectableList({320, 260}, {520, 150}, {255,135,198,255}, dir, BLACK, 20, 20, 60);
    path = TextBox({195,360}, {270,40}, {240,98,161,255}, Global.Path, WHITE, 20, 40);
}

void LoadMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
}

void LoadMenu::render() {
    //Global.mutex.lock();
    bg.render();
    description.render();
    back.render();
    path.render();
    select.render();
    dir_list.render();
    close.render();
    //Global.mutex.unlock();
}
void LoadMenu::update() {
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();
    
    if(close.action){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

    if(select.action or dir_list.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){
            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if(Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osz");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
                path = TextBox(path.position, path.size, path.color, Global.Path, path.textcolor, path.textsize, path.maxlength);
            }
            else{
                Global.selectedPath = Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
                std::string base_file = get_without_ext(Global.selectedPath);
                std::string final_path = Global.GamePath + "/beatmaps/" + base_file;
                create_dir(final_path);
                int arg = 2;
                std::cout << Global.selectedPath.c_str() << std::endl;
                zip_extract(Global.selectedPath.c_str(), final_path.c_str(), on_extract_entry, &arg);
            }
        }
    }
    else if(back.action){
        while(Global.Path.size() > 0){
            if(Global.Path[Global.Path.size()-1] == '/' || Global.Path[Global.Path.size() - 1] == ':'){
                if(Global.Path.size() > 1  && Global.Path.size() != 2)
                    Global.Path.pop_back();
                break;
            }
            Global.Path.pop_back();
        }
        auto dir = ls(".osz");
        dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
        dir_list.init();
        path = TextBox(path.position, path.size, path.color, Global.Path, path.textcolor, path.textsize, path.maxlength);
    }
}
void LoadMenu::unload() {

}

MainMenu::MainMenu() {
    play = Button({250,420}, {120,60}, {255,135,198,255}, "Play", BLACK, 20);
    wip = Button({320,320}, {120,60}, {255,135,198,0}, "WIP", BLACK, 20);
    load = Button({390,420}, {120,60}, {255,135,198,255}, "Load", BLACK, 20);
    volume = TestSlider({510,460}, {240,20}, BLACK, PURPLE, WHITE, WHITE);
}
void MainMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
    Global.useAuto = false;
    volume.location = Global.volume * 100.0f;
}
void MainMenu::update() {
    Global.enableMouse = true;
    play.update();
    wip.update();
    load.update();
    //test.update();
    if(wip.action){
        //Global.CurrentState->unload();
        //Global.CurrentState.reset(new WIPMenu());
        //Global.CurrentState->init();
        std::string temp = Global.Path;
        Global.Path = "sdmc:/3ds/database";

        struct dirent *de;
        DIR *dr = opendir(Global.Path.c_str()); 
        if (dr == NULL){ // opendir returns NULL if couldn't open directory { 
            printf("Could not open current directory: database" ); 
        }
        else{
            while ((de = readdir(dr)) != NULL) {
                 std::remove((Global.Path + "/" + de->d_name).c_str());
            }
            closedir(dr);
        }

        Global.Path = temp;
        Parser parser = Parser();
        for(const auto& p: std::filesystem::recursive_directory_iterator(Global.BeatmapLocation)){
            if(!std::filesystem::is_directory(p)){
                if(p.path().extension().string() == ".osu"){
                    //std::cout << p.path().string() << '\n';
                    GameFile geym;
                    geym = parser.parseMetadata(p.path().string());
                    if(geym.configMetadata.find("BeatmapSetID") == geym.configMetadata.end()){
                        std::cout << "didnt find setid bro, tryin to improvise\n";
                        std::string doublecheck = p.path().parent_path().filename().string();
                        int index = 0;
                        bool works = false;
                        bool startswithnumber = isdigit(doublecheck.at(0));
                        int number = 0;
                        
                        while(true){
                            if(index >= doublecheck.length()){
                                break;
                            }
                            if(!isdigit(doublecheck.at(index))){
                                if(doublecheck[index] == ' '){
                                    if(startswithnumber){
                                        works = true;
                                        break;
                                    }
                                }
                            }
                            else{
                                number *= 10;
                                number += int(doublecheck[index] - '0');
                            }
                            index++;
                            //std::cout << "number: " << number << std::endl;
                        }
                        if(geym.configMetadata.find("Title") == geym.configMetadata.end()){
                            std::cout << "didnt find Setid nor title bro, cant do much\n";
                        }
                        else{
                            std::string title = geym.configMetadata["Title"];
                            if(works){
                                FILE * pFile;
                                pFile = fopen(("sdmc:/3ds/database/" + std::to_string(number) + " {" + title + "}" + ".db").c_str()  ,"a");
                                if(pFile != NULL){
                                    fprintf(pFile, (p.path().string() + " - found\n").c_str());
                                    fclose(pFile);
                                }
                            }
                        }

                    }
                    else{
                        std::string title;
                        if(geym.configMetadata.find("Title") == geym.configMetadata.end()){
                            std::cout << "didnt title bro, setting as Unknown\n";
                            title = "Unknown";
                        }
                        else{
                            title = geym.configMetadata["Title"];
                        }
                        //std::cout << geym.configMetadata["BeatmapSetID"] << std::endl;
                        FILE * pFile;
                        pFile = fopen(("sdmc:/3ds/database/" + geym.configMetadata["BeatmapSetID"] + " {" + title + "}" + ".db").c_str()  ,"a");
                        if(pFile != NULL){
                            fprintf(pFile, (p.path().string() + " - found\n").c_str());
                            fclose(pFile);
                        }
                    }
                }
            }
        }

        return;
    }
    else if(play.action){
        std::cout << "play.action unload" << std::endl;
        Global.CurrentState->unload();
        std::cout << "play.action reset" << std::endl;
        Global.CurrentState.reset(new PlayMenu());
        std::cout << "play.action init" << std::endl;
        Global.CurrentState->init();
        std::cout << "play.action done" << std::endl;
        return;
    }
    else if(load.action){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new LoadMenu());
        Global.CurrentState->init();
        return;
    }

    if(IsKeyDown(KEY_SELECT ))
        volume.update();
    float lastVolume = Global.volume;
    Global.volume = volume.location / 100.0f;
    if(!AreSame(lastVolume, Global.volume)){
        std::cout << "Volume: " << Global.volume << std::endl;
        Global.volumeChanged = true;
    }
}
void MainMenu::render() {
    //Global.mutex.lock();
    DrawTextureCenter(&Global.OsusLogo, 320, 200, 400.0 / (float)Global.OsusLogo.width, WHITE);
    play.render();
    wip.render();
    load.render();
    
    if(IsKeyDown(KEY_SELECT ))
        volume.render();
    //test.render();
    //Global.mutex.unlock();
}

void MainMenu::unload() {
    //NEED FIXING>>> UNLOADING DOESNT STOP RENDEWRING
}

Game::Game() {
    volume = TestSlider({510,460}, {240,20}, BLACK, PURPLE, WHITE, WHITE);
}
void Game::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    initDone = 0;
    Global.LastFrameTime = getTimer();
    std::cout << Global.selectedPath << std::endl;
    std::cout << "Press select to load game" << std::endl;
    while(false){
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
    }
    Global.numberLines = 0;
    Global.parsedLines = 0;
    Global.loadingState = 0;
    initDone = -2;
    initStartTime = getTimer();
    Global.mutex.unlock();
    //LightLock_Unlock(&Global.lightlock);
    Global.gameManager->loadGame(Global.selectedPath);

    Global.gameManager->timingSettingsForHitObject.clear();
    Global.mutex.lock();
    //LightLock_Lock(&Global.lightlock);
    Global.startTime = -5000.0f;
    Global.errorSum = 0;
    Global.errorLast = 0;
    Global.errorDiv = 0;

    volume.location = Global.volume * 100.0f;
    std::cout << volume.location << std::endl;
    std::cout << "done init, press select to continue" << std::endl;

}
void Game::update() {
    if(initDone == 1){
        //Global.enableMouse = false;
        Global.gameManager->run();
        if(IsKeyPressed(KEY_B)){
            Global.CurrentState->unload();
            Global.CurrentState.reset(new PlayMenu());
            Global.CurrentState->init();
            return;
        }
    }
    else{
        if(initDone == 0 or Global.GameTextures == 0){
            initDone = -1;
        }
        if(initDone == -1 and getTimer() - initStartTime > 0.0f){
            std::cout << "init done in " << getTimer() - initStartTime << " secs\n";
            initDone = true;
        }
    }
    if(IsKeyDown(KEY_SELECT ))
        volume.update();
    float lastVolume = Global.volume;
    Global.volume = volume.location / 100.0f;
    if(!AreSame(lastVolume, Global.volume)){
        std::cout << "Volume: " << Global.volume << std::endl;
        Global.volumeChanged = true;
    }
}
void Game::render() {
    
    if(initDone == 1){
        //Global.enableMouse = false;
        Global.gameManager->render();
        //Global.mutex.lock();
        if(IsMusicStreamPlaying(&Global.gameManager->backgroundMusic)){
            DrawTextEx(&Global.DefaultFont, TextFormat("Playing: %.3f/%.3f", (Global.currentOsuTime/1000.0), GetMusicTimeLength(&Global.gameManager->backgroundMusic)), {(int)ScaleCordX(5), (int)ScaleCordY(25)}, Scale(20.05) , Scale(2), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Timer: %.3f ms", getTimer()), {ScaleCordX(5), ScaleCordY(55)}, Scale(10) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Last Error: %.3f ms", Global.errorLast/1000.0f), {ScaleCordX(5), ScaleCordY(65)}, Scale(10) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Avg Time Difference in the First Second: %.3f ms", Global.avgTime), {ScaleCordX(5), ScaleCordY(75)}, Scale(10) , Scale(1), WHITE);
        }
        else{
            DrawTextEx(&Global.DefaultFont, TextFormat("Paused: %.3f/%.3f", GetMusicTimePlayed(&Global.gameManager->backgroundMusic) * 1000000.0f, GetMusicTimeLength(&Global.gameManager->backgroundMusic)), {(int)ScaleCordX(5), (int)ScaleCordY(25)}, Scale(20.05) , Scale(2), WHITE);
            if(Global.errorDiv != 0)
                DrawTextEx(&Global.DefaultFont, TextFormat("Error Avg: %ld ms", (Global.errorSum/Global.errorDiv)/1000), {(int)ScaleCordX(5), (int)ScaleCordY(40)}, Scale(20.05) , Scale(2), WHITE);
        }
        if(GetMusicTimeLength(&Global.gameManager->backgroundMusic) != 0){
            DrawLineEx({0, GetScreenHeight() - Scale(2)}, {GetScreenWidth() * ((Global.currentOsuTime/1000.0) / GetMusicTimeLength(&Global.gameManager->backgroundMusic)), GetScreenHeight() - Scale(2)}, Scale(3), Fade(WHITE, 0.8));
        }
        //Global.mutex.unlock();
    }
    else if(initDone == -1){
        std::string message;
        if(getTimer() - initStartTime < 2000.0f)
            message = "Loaded Game!";
        else if(getTimer() - initStartTime < 2500.0f)
            message = "3...";
        else if(getTimer() - initStartTime < 3000.0f)
            message = "2...";
        else if(getTimer() - initStartTime < 3500.0f)
            message = "1...";
        else if(getTimer() - initStartTime < 4000.0f)
            message = "GO!";
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        //Global.mutex.lock();
        DrawTextEx(&Global.DefaultFont, message.c_str(), {(int)ScaleCordX(320 - message.size() * 7.5f), (int)ScaleCordY(220)}, Scale(20.05), Scale(2), WHITE);
        //Global.mutex.unlock();
    }
    else if(initDone == -2){
        //Global.mutex.lock();
        std::string message;
        message = "Loading Game...";
        
        if(Global.loadingState == 1){
            //std::cout << "Precalculating HitObjects" << std::endl;
            message = "Precalculating HitObjects";
        }
        else if(Global.loadingState == 2){
            //std::cout << "Loading Background Music" << std::endl;
            message = "Loading Background Music";
        }
        else if(Global.loadingState == 3){
            //std::cout << "Loading ComboBreak Sound" << std::endl;
            message = "Loading ComboBreak Sound";
        }
        else if(Global.loadingState == 4){
            //std::cout << "Loading Hit Sounds" << std::endl;
            message = "Loading Hitsound " + std::to_string(Global.parsedLines) + " of " + std::to_string(Global.numberLines);
        }
        else if(Global.loadingState == 5){
            message = "Parsing line " + std::to_string(Global.parsedLines) + " of " + std::to_string(Global.numberLines);
        }
        else if(Global.loadingState == 6){
            message = "Parsing Timing Points";
        }
        else if(Global.loadingState == 7){
            message = "Loading Textures";
        }
        DrawTextEx(&Global.DefaultFont, message.c_str(), {(int)ScaleCordX(320 - message.size() * 7.5f), (int)ScaleCordY(220)}, Scale(20.05), Scale(2), WHITE);
        //Global.mutex.unlock();
    }
    if(IsKeyDown(KEY_SELECT ))
        volume.render();
}

void Game::unload(){
    Global.gameManager->unloadGame();
    Global.NeedForBackgroundClear = true;
}

WIPMenu::WIPMenu() {
    
}

void WIPMenu::init(){
    //index = 0;
    Global.NeedForBackgroundClear = true;
    applyMouse = false;
    std::string temp = Global.Path;
    Global.Path = Path;
    dir.clear();
    dir = ls(".osu");
    //std::cout << "lsdone" << std::endl;
    std::sort(dir.begin(), dir.end());
    CanGoBack = false;
    if(Path != Global.BeatmapLocation + "/"){
        dir.insert(dir.begin(), "Back");
        CanGoBack = true;
    }
    Global.Path = temp;
    logo = LoadTexture("resources/osus.png");
    menu = LoadTexture("resources/menu.png");
    back = LoadTexture("resources/metadata.png");
	SetTextureFilter(&logo, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&back, TEXTURE_FILTER_BILINEAR );
    float time = 0.2f;
    while(time <= 1.0f){
        time += Global.FrameTime / 1000.0f;
        //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
        float weirdSmooth = easeInOutCubic(time);
        angle = -250 + 255 * weirdSmooth;
        GetScale();
        GetMouse();
        GetKeys();
        updateMouseTrail();
        updateUpDown();
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_SceneBegin(Global.window);
        ClearBackground(Global.Background);
        int index = 0;
        float tempangle = angle;
        if(tempangle < 0)
            tempangle -= 10;
        if(tempangle > 0)
            tempangle += 10;
        index = (tempangle) / 20;
        for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
            float tempAngle = angle - i * 20.0f;
            while(tempAngle > 0.0f)
                tempAngle -= 360.0f;
            while(tempAngle < 0.0f)
                tempAngle += 360.0f;
            if(tempAngle < 0)
                tempAngle -= 10;
            if(tempAngle > 0)
                tempAngle += 10;
            int tempindex = (int)(tempAngle / 20) % 18;
            if(tempindex > 9)
                tempindex = 18 - tempindex;
            else
                tempindex = -tempindex;
            index = (tempangle) / 20;
            index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
            
            float tempAngle2 = angle - i * 20.0f;
            
            int offset = 0;
            Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
            int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
            Color temp = WHITE;
            if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                temp = {255,135,198,255};
            DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
            DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
            DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
        }
        DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse();
        DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
        C2D_Flush();  //test
        C3D_FrameEnd(0);
    }
    applyMouse = true;
	//SetTextureFilter(menu, TEXTURE_FILTER_BILINEAR );

}
void WIPMenu::render(){
    //Global.mutex.lock();
    int index = 0;
    float tempangle = angle;
    if(tempangle < 0)
        tempangle -= 10;
    if(tempangle > 0)
        tempangle += 10;
    index = (tempangle) / 20;
    for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
        float tempAngle = angle - i * 20.0f;
        while(tempAngle > 0.0f)
            tempAngle -= 360.0f;
        while(tempAngle < 0.0f)
            tempAngle += 360.0f;
        if(tempAngle < 0)
            tempAngle -= 10;
        if(tempAngle > 0)
            tempAngle += 10;
        int tempindex = (int)(tempAngle / 20) % 18;
        if(tempindex > 9)
            tempindex = 18 - tempindex;
        else
            tempindex = -tempindex;
        index = (tempangle) / 20;
        index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
        
        float tempAngle2 = angle - i * 20.0f;
        
        int offset = 0;
        Vector2 textpos = getPointOnCircle(610, 220, 300, tempAngle2 - 180.0f);
        int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
        Color temp = WHITE;
        if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
            temp = {255,135,198,255};
        DrawTextureOnCircle(&menu, 800, 240, 300, 0.4f, 0, tempAngle2 - 180.0f, temp);
        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15, 7, WHITE);
    }
    
    DrawTextureCenter(&back, 145, 240, 0.45f, Color{255,255,255,255 * easeInOutCubic(animtime)});
    if(TempMeta.size() == 5){
        DrawTextLeft((TempMeta[0]).c_str(), 25, 55, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[1]).c_str(), 25, 70, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[2]).c_str(), 25, 85, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[3]).c_str(), 25, 100, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[4]).c_str(), 25, 115, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
    }

    DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);
    //Global.mutex.unlock();
}
void WIPMenu::update(){
    float clampaccel = 0;
    if(applyMouse){
        accel += (float)(Global.FrameTime / 1000.0f) * (float)(100.0f * -Global.Wheel);
        if(accel > 60.0f)
            accel = 60.0f;
        if(accel < -60.0f)
            accel = -60.0f;
        if(accel < 0.01f and accel > -0.01f)
            accel = 0.0f;
    }
    float floatangle = ((int)posangle) % 20;
    if(floatangle >= 20.0f)
        floatangle -= 20.0f;
    if(floatangle >= 10.0f)
        clampaccel = (float)(Global.FrameTime / 1000.0f) * (float)(2.5f * (20.0f - floatangle));
    else
        clampaccel = -(float)(Global.FrameTime / 1000.0f) * (float)(2.5f * (floatangle));
    
    posangle = angle;
    while(posangle < 0.0f)
        posangle += 360.0f;
    
    int index = 0;
    float tempangle = angle;
    if(tempangle < 0)
        tempangle -= 10;
    if(tempangle > 0)
        tempangle += 10;
    index = (tempangle) / 20;
    index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
    index = index % dir.size();
    
    if(Global.Key1D and Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000})){
        if(Global.MouseInFocus)
            mouseMovement += Global.MousePosition.y - lastMouse;
        if(Global.MouseInFocus)
            absMouseMovement += std::abs(Global.MousePosition.y - lastMouse);
        if(absMouseMovement > 0.5f){
            if(applyMouse)
                angle += (Global.MousePosition.y - lastMouse) / -5.0f;
            moving = true;
        }   
    }
    if(!moving and Global.Key1R and CheckCollisionPointRec(Global.MousePosition, Rectangle{305,198,210,81})){
        if(selectedIndex == index){
            selectedIndex = -1;
            subObjects.clear();
        }
        else
            selectedIndex = index;
        index = 0;
        float tempangle = angle;
        if(tempangle < 0)
            tempangle -= 10;
        if(tempangle > 0)
            tempangle += 10;
        index = (tempangle) / 20;
        selectedAngleIndex = index;
        if(selectedIndex != -1){
            Global.Key1R = false;
            int size = dir[selectedIndex].size();
            if(dir[selectedIndex].size() > 0 and dir[selectedIndex][dir[selectedIndex].size() - 1] == '/'){
                Path += dir[selectedIndex];
                ParseNameFolder(dir[selectedIndex]);
                applyMouse = false;
                float time = 0.2f;
                while(time <= 1.0f){
                    time += (Global.FrameTime / 1000.0f) * 1.0f;
                    //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
                    float weirdSmooth = easeInOutCubic(1.0f-time);
                    angle = -250 + 250 * weirdSmooth;
                    GetScale();
                    GetMouse();
                    GetKeys();
                    updateMouseTrail();
                    updateUpDown();
                    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                    C2D_SceneBegin(Global.window);
                    ClearBackground(Global.Background);
                    int index = 0;
                    float tempangle = angle;
                    if(tempangle < 0)
                        tempangle -= 10;
                    if(tempangle > 0)
                        tempangle += 10;
                    index = (tempangle) / 20;
                    for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
                        float tempAngle = angle - i * 20.0f;
                        while(tempAngle > 0.0f)
                            tempAngle -= 360.0f;
                        while(tempAngle < 0.0f)
                            tempAngle += 360.0f;
                        if(tempAngle < 0)
                            tempAngle -= 10;
                        if(tempAngle > 0)
                            tempAngle += 10;
                        int tempindex = (int)(tempAngle / 20) % 18;
                        if(tempindex > 9)
                            tempindex = 18 - tempindex;
                        else
                            tempindex = -tempindex;
                        index = (tempangle) / 20;
                        index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
                        
                        float tempAngle2 = angle - i * 20.0f;
                        
                        int offset = 0;
                        Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
                        
                        int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
                        Color temp = WHITE;
                        if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                            temp = {255,135,198,255};
                        DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
                    }
                    DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

                    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
                    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
                    renderMouse();
                    DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
                    C2D_Flush();  //test
                    C3D_FrameEnd(0);
                }
                applyMouse = true;
                init();
                index = 0;
            }
            else if(dir[selectedIndex] == "Back" and selectedIndex == 0 and CanGoBack){
                Path.pop_back();
                while(Path[Path.size()-1] != '/'){
                    Path.pop_back();
                }
                if(Path.size() <= (Global.BeatmapLocation + "/").size()){
                    Path = Global.BeatmapLocation + "/";
                }
                selectedIndex = -1;
                std::cout << Path << std::endl;
                applyMouse = false;
                float time = 0.2f;
                while(time <= 1.0f){
                    time += (Global.FrameTime / 1000.0f) * 1.0f;
                    //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
                    float weirdSmooth = easeInOutCubic(1.0f-time);
                    angle = -250 + 250 * weirdSmooth;
                    GetScale();
                    GetMouse();
                    GetKeys();
                    updateMouseTrail();
                    updateUpDown();
                    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                    C2D_SceneBegin(Global.window);
                    ClearBackground(Global.Background);
                    int index = 0;
                    float tempangle = angle;
                    if(tempangle < 0)
                        tempangle -= 10;
                    if(tempangle > 0)
                        tempangle += 10;
                    index = (tempangle) / 20;
                    for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
                        float tempAngle = angle - i * 20.0f;
                        while(tempAngle > 0.0f)
                            tempAngle -= 360.0f;
                        while(tempAngle < 0.0f)
                            tempAngle += 360.0f;
                        if(tempAngle < 0)
                            tempAngle -= 10;
                        if(tempAngle > 0)
                            tempAngle += 10;
                        int tempindex = (int)(tempAngle / 20) % 18;
                        if(tempindex > 9)
                            tempindex = 18 - tempindex;
                        else
                            tempindex = -tempindex;
                        index = (tempangle) / 20;
                        index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
                        
                        float tempAngle2 = angle - i * 20.0f;
                        
                        int offset = 0;
                        Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
                        int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
                        Color temp = WHITE;
                        if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                            temp = {255,135,198,255};
                        DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
                    }
                    DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

                    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
                    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
                    renderMouse();
                    DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
                    C2D_Flush();  //test
                    C3D_FrameEnd(0);
                }
                applyMouse = true;
                init();
                index = 0;
            }
            else if(size >= 4 and dir[selectedIndex][size - 1] == 'u' and dir[selectedIndex][size - 2] == 's' and dir[selectedIndex][size - 3] == 'o' and dir[selectedIndex][size - 4] == '.'){
                ParseNameFile(Path + dir[selectedIndex]);
            }
            selectedIndex = -1;
        }
    }
    if(Global.Key1R and Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000})){
        moving = false;
        if(applyMouse)
            accel += (Global.MousePosition.y - lastMouse) / -10.0f;
        mouseMovement = 0;
        absMouseMovement = 0;
    }
    if(Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000}))
        lastMouse = Global.MousePosition.y;
    //std::cout << accel << std::endl;
    if(applyMouse)
        accel += ((-accel) / 2.0f) * ((float)(Global.FrameTime / 1000.0f) * 8.0f);
    angle += accel;
    if(!moving)
        angle += clampaccel;


    if(AreSame(accel,0.0f) and AreSame(clampaccel,0.0f)){
        if(dir[index] != "Back" and dir[index][dir[index].size() - 1] != '/'){
            renderMetadata = true;
            if(Metadata.size() == 0){
                TempMeta.clear();
                int size = dir[index].size();
                std::vector<std::string> output;
                if(size >= 4 and dir[index][size - 1] == 'u' and dir[index][size - 2] == 's' and dir[index][size - 3] == 'o' and dir[index][size - 4] == '.'){
                    output = ParseNameFile(Path + dir[index]);
                }
                if(output.size() == 5){
                    Metadata.push_back("Title: " + output[0]);
                    Metadata.push_back("Artist: " + output[1]);
                    Metadata.push_back("Creator: " + output[2]);
                    Metadata.push_back("Ver. " + output[3]);
                    Metadata.push_back("ID: " + output[4]);
                    TempMeta = Metadata;
                }
            }
            animtime += (Global.FrameTime / 1000.0f) * 2.0f;
            if(animtime > 1.0f)
                animtime = 1.0f;
        }
    }
    else{
        renderMetadata = false;
        if(Metadata.size() > 0)
            Metadata.clear();
        animtime -= (Global.FrameTime / 1000.0f) * 2.0f;
        if(animtime < 0.0f)
            animtime = 0.0f;
    }

    if(IsKeyPressed(KEY_B) and CanGoBack){
        Path.pop_back();
        while(Path[Path.size()-1] != '/'){
            Path.pop_back();
        }
        if(Path.size() <= (Global.BeatmapLocation + "/").size()){
            Path = Global.BeatmapLocation + "/";
        }
        selectedIndex = -1;
        std::cout << Path << std::endl;
        applyMouse = false;
        float time = 0.2f;
        while(time <= 1.0f){
            time += (Global.FrameTime / 1000.0f) * 1.0f;
            //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
            float weirdSmooth = easeInOutCubic(1.0f-time);
            angle = -250 + 250 * weirdSmooth;
            GetScale();
            GetMouse();
            GetKeys();
            updateMouseTrail();
            updateUpDown();
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_SceneBegin(Global.window);
            ClearBackground(Global.Background);
            int index = 0;
            float tempangle = angle;
            if(tempangle < 0)
                tempangle -= 10;
            if(tempangle > 0)
                tempangle += 10;
            index = (tempangle) / 20;
            for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
                float tempAngle = angle - i * 20.0f;
                while(tempAngle > 0.0f)
                    tempAngle -= 360.0f;
                while(tempAngle < 0.0f)
                    tempAngle += 360.0f;
                if(tempAngle < 0)
                    tempAngle -= 10;
                if(tempAngle > 0)
                    tempAngle += 10;
                int tempindex = (int)(tempAngle / 20) % 18;
                if(tempindex > 9)
                    tempindex = 18 - tempindex;
                else
                    tempindex = -tempindex;
                index = (tempangle) / 20;
                index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
                
                float tempAngle2 = angle - i * 20.0f;
                
                int offset = 0;
                Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
                int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
                Color temp = WHITE;
                if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                    temp = {255,135,198,255};
                DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
            }
            DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

            DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
            DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
            renderMouse();
            DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
            C2D_Flush();  //test
            C3D_FrameEnd(0);
        }
        applyMouse = true;
        init();
    }
    if(IsKeyPressed(KEY_B ) and !CanGoBack){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

}
void WIPMenu::unload(){
    dir.clear();
    subObjects.clear();
    UnloadTexture(&logo);
    UnloadTexture(&back);
    UnloadTexture(&menu);
}


ResultsMenu::ResultsMenu() {
    name = TextBox({320,40}, {520,60}, {0,0,0,0}, "Results!", WHITE, 40, 50);
    close = Button({480, 440}, {160,30}, {255,135,198,255}, "Back to menu", BLACK, 20);
    maxCombo = TextBox({320,80}, {520,60}, {0,0,0,0}, "Maximum Combo: Not available", WHITE, 20, 50);
    hit300 = TextBox({320,120}, {520,60}, {0,0,0,0}, "300s hit: Not available", WHITE, 20, 50);
    hit100 = TextBox({320,160}, {520,60}, {0,0,0,0}, "100s hit: Not available", WHITE, 20, 50);
    hit50 = TextBox({320,200}, {520,60}, {0,0,0,0}, "50s hit: Not available", WHITE, 20, 50);
    hit0 = TextBox({320,240}, {520,60}, {0,0,0,0}, "Misses: Not available", WHITE, 20, 50);
    accuracy = TextBox({320,280}, {520,60}, {0,0,0,0}, "Accuracy: Not available", WHITE, 20, 50);
}

void ResultsMenu::init() {
    std::cout << "loading the scores" << std::endl;;
    std::cout << "Maximum Combo: " + std::to_string(Global.gameManager->maxCombo) << std::endl;
    maxCombo.text = "Maximum Combo: " + std::to_string(Global.gameManager->maxCombo);
    maxCombo.init();
    hit300.text = "300s hit: " + std::to_string(Global.gameManager->hit300s);
    hit300.init();
    hit100.text = "100s hit: " + std::to_string(Global.gameManager->hit100s);
    hit100.init();
    hit50.text = "50s hit: " + std::to_string(Global.gameManager->hit50s);
    hit50.init();
    hit0.text = "0s hit: " + std::to_string(Global.gameManager->hit0s);
    hit0.init();
    float accuracy = ((300.0f * Global.gameManager->hit300s + 10.0f * Global.gameManager->hit100s + 50.0f * Global.gameManager->hit50s) 
    / (300.0f * (Global.gameManager->hit300s + Global.gameManager->hit100s + Global.gameManager->hit50s + Global.gameManager->hit0s))) * 100.0f;
    //accuracy = std::ceil(accuracy * 100.0) / 100.0;
    hit0.text = "Accuracy: " + std::to_string(accuracy) + "%";
    hit0.init();
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    
}
void ResultsMenu::render() {
    close.render();
    name.render();
    maxCombo.render();
    hit300.render();
    hit100.render();
    hit50.render();
    hit0.render();
    //Global.mutex.unlock();
}
void ResultsMenu::update() {
    close.update();
    if(close.action){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new PlayMenu());
        Global.CurrentState->init();
    }
}
void ResultsMenu::unload() {

}