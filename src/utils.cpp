#include <utils.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include <globals.hpp>
#include <gamemanager.hpp>
#include <iostream>
#include <ctype.h>
#include <parser.hpp>

void updateUpDown(){
    //Get the current state of the mouse wheel
    Global.Wheel = 0; //GetMouseWheelMove();
    if(IsKeyPressed(KEY_DDOWN )){
        //If the down key is pressed, start a timer so that we can simulate a fast mouse wheel movement
        Global.Wheel = -1;
        Global.FrameTimeCounterWheel = -170;
    }
    if(IsKeyPressed(KEY_DUP )){
        //The same deal as above but for the up key
        Global.Wheel = 1;
        Global.FrameTimeCounterWheel = -170;
    }
    Global.FrameTimeCounterWheel += Global.FrameTime;
    while(Global.FrameTimeCounterWheel > 50.0f){
        //If the keys are still down, trigger a wheel movement every 50 milliseconds
        Global.FrameTimeCounterWheel -= 50.0f;
        if(IsKeyDown(KEY_DUP ))
            Global.Wheel = 1;
        if(IsKeyDown(KEY_DDOWN ))
            Global.Wheel = -1;
    }
}

void GetScale(){
    //Get the scale and also get the current offset for the zero point of the game area
    Global.Scale = std::min(GetScreenWidth()/640.0f, GetScreenHeight()/480.0f);
    Global.ZeroPoint = {GetScreenWidth() / 2.0f - (Global.Scale * 320.0f), GetScreenHeight() / 2.0f - (Global.Scale * 240.0f)};
}

void GetMouse(){
    //Get the mouse position and also check if it is in the game area
    Global.MouseInFocus = CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()});
    if(!Global.MouseInFocus){
        Global.MousePosition = {0,0};
    }
    else {
        if(Global.useAuto)
            Global.MousePosition = Global.AutoMousePosition;
        else
            Global.MousePosition = (Vector2){(GetMouseX() - Global.ZeroPoint.x) / Global.Scale, (GetMouseY() - Global.ZeroPoint.y) / Global.Scale};
    }
}

void GetKeys(){
    //Get all of the keys an store this data into some variables
    if(IsKeyPressed(KEY_L ) or (Global.enableMouse and IsMouseButtonPressed(SDL_BUTTON_LEFT)))
        Global.Key1P = true;
    else
        Global.Key1P = false;
    if(IsKeyPressed(KEY_R ) or (Global.enableMouse and IsMouseButtonPressed(SDL_BUTTON_RIGHT)))
        Global.Key2P = true;
    else
        Global.Key2P = false;
    if(IsKeyDown(KEY_L ) or (Global.enableMouse and IsMouseButtonDown(SDL_BUTTON_LEFT)))
        Global.Key1D = true;
    else
        Global.Key1D = false;
    if(IsKeyDown(KEY_R ) or (Global.enableMouse and IsMouseButtonDown(SDL_BUTTON_RIGHT)))
        Global.Key2D = true;
    else
        Global.Key2D = false;
    if(IsKeyReleased(KEY_L ) or (Global.enableMouse and IsMouseButtonReleased(SDL_BUTTON_LEFT)))
        Global.Key1R = true;
    else
        Global.Key1R = false;
    if(IsKeyReleased(KEY_R ) or (Global.enableMouse and IsMouseButtonReleased(SDL_BUTTON_RIGHT)))
        Global.Key2R = true;
    else
        Global.Key2R = false;
}

float Scale(float a){
    //Basic scaling function
    return a * Global.Scale;
}

Rectangle ScaleRect(Rectangle a){
    //Scale the size and also move the rectangle depending on the offset
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y, a.width * Global.Scale, a.height * Global.Scale};
}

Vector2 ScaleCords(Vector2 a){
    //Scale so that the coordinates are correct for our window size and also move them based on the offset
    return {a.x * Global.Scale + Global.ZeroPoint.x, a.y * Global.Scale + Global.ZeroPoint.y};
}

float ScaleCordX(float a){
    //Do the same thing as the ScaleCords function but only for the x axis
    return a * Global.Scale + Global.ZeroPoint.x;
}

float ScaleCordY(float a){
    //Do the same thing as the ScaleCords function but only for the y axis
    return a * Global.Scale + Global.ZeroPoint.y;
}

Vector2 GetCenter(Rectangle a){
    //Get the center of a rectangle
    return {a.x + a.width / 2, a.y + a.height / 2};
}
Vector2 GetRaylibOrigin(Rectangle a){
    //Move the coordinates so that the center of the rectangle is where we want it to be for the rendering process
    return {a.x - a.width / 2, a.y - a.height / 2};
}

Rectangle GetRaylibOriginR(Rectangle a){
    //Do the same thing as the RaylibOrigin function but this time just take the whole rectangle
    return {a.x - a.width / 2, a.y - a.height / 2, a.width, a.height};
}

float clip( float n, float lower, float upper ){
    //I have no idea what this does but it works
    n = ( n > lower ) * n + !( n > lower ) * lower;
    return ( n < upper ) * n + !( n < upper ) * upper;
}

Vector2 lerp(Vector2 a, Vector2 b, float t){
    //Thank you geeksforgeeks
    return { .x = (1 - t) * a.x + t * b.x, .y = (1 - t) * a.y + t * b.y};
}

Vector2 vectorize(float i) {
    //Really dumb but raylib needs it
    return Vector2{i, i};
}

float distance(Vector2 &p0, Vector2 &p1){
    //Its a bit heavy of a process but we dont do this frequently anyway
    return std::sqrt(std::pow(std::abs(p0.x - p1.x),2) + std::pow(std::abs(p0.y - p1.y),2));
}

int Search(std::vector<float> arr, float x,int l,int r) {
    //Basic binary search implementation
    if (r >= l) {
        int mid = (l + r) / 2;
        if (arr[mid] == x || l==r)
            return mid;
        if (arr[mid] > x)
            return Search(arr, x,l, mid - 1);
        else 
            return Search(arr, x,mid + 1, r);
    }
    else
        return l;
}

void DrawTextureCenter(Texture2D *tex, float x, float y, float s, Color color){
    if(tex->id == 0)
        return;
    //DrawTextureEx(tex, ScaleCords(GetRaylibOrigin({x,y,tex.width*s,tex.height*s})), 0, Scale(s), color);
    C2D_ImageTint c2dTint;
    //Maybe needs fixing, idk how blend and alpha work
    c2dTint.corners[0] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
    c2dTint.corners[1] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
    c2dTint.corners[2] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
    c2dTint.corners[3] = {C2D_Color32(color.r, color.g, color.b, color.a), 1.0f};
    /*if(tex.width == 48){
        std::cout << tex.subtex.width << tex.subtex.height << std::endl;
    }*/
    C2D_Prepare();
    C2D_DrawImageAt(C2D_Image{&tex->tex, &tex->subtex}, ScaleCordX(x - ((tex->width * s) / 2.0f)), ScaleCordY(y - ((tex->height * s) / 2.0f)), 0.0f, &c2dTint, Scale(s), Scale(s));
    C2D_Flush();  //test
}

void DrawTextureSlider(Texture2D *tex, float x, float y, Color color, float s){
    //Same thing as the DrawTextureCenter() function above
    DrawTextureEx(tex, ScaleCords({x-s/2.0f-4.0f*Global.sliderTexSize,y-s/2.0f-4.0f*Global.sliderTexSize}),0,Scale(1.0f/Global.sliderTexSize), color);
}

/*void DrawTextureRotate(Texture2D tex, float x, float y, float s, float r, Color color){
    //Same thing as the DrawTextureCenter() function above
    DrawTexturePro(tex, Rectangle{0,0,tex.width,tex.height}, Rectangle{ScaleCordX(x),ScaleCordY(y),Scale(tex.width*s),Scale(tex.height*s)}, Vector2{Scale(tex.width*s/2.0f), Scale(tex.height*s/2.0f)}, r, color);
}*/

int nthDigit(int v, int n){
    //Find the nth digit of a number... Dumb but this is probably one of the best ways :D
    while(n > 0){
        v /= 10;
        n--;
    }
    return "0123456789"[v % 10] - '0';
}

void DrawCNumbersCenter(int n, float x, float y, float s, Color color){
    //I will need to fix this function but currently it works good enough
    GameManager* gm = GameManager::getInstance();
    int digits = log10(n) + 1;
    int i = (digits - 1) * 18;
    for(int k = 0; k < digits; k++){
        DrawTextureCenter(&gm->numbers[nthDigit(n, digits-k-1)], x - (float)i * s + k * 18 * s * 2, y, s*4, color);
        C2D_Flush(); 
    }
}
std::string getSampleSetFromInt(int s){
    //simple map for sound loading
    if (s == 1) return "normal"; 
    else if (s == 2) return "soft"; 
    else if (s == 3) return "drum"; 
}

void DrawSpinnerMeter(Texture2D *tex, float per){
    //currently broken, TODO
    per = clip(per, 0.001f, 0.999f);
    float x = 0;
    float y = 0;
    float ratio = (float)tex->width / (float)tex->height;
    float defaultRatio = 640.0f / 480.0f;
    if(defaultRatio > ratio){
        x = 640.0f - 480.0f * ratio;
    }
    else{
        y = 480.0f - 640.0f / ratio;
    }
    Rectangle source = {0,tex->height*(1.0f-per),tex->width, tex->height*per};
    DrawTexturePro(tex, Rectangle{0,0,tex->width, tex->height}, ScaleRect(Rectangle{0+x/2.0f,0+y/2.0f,640-x,480-y}), Vector2{0,0}, 0, BLACK);
    DrawTexturePro(tex, source, ScaleRect(Rectangle{0+x/2.0f,(480.0f-y)*(1.0f-per)+y/2.0f,640-x,(480.0f-y)*per}), Vector2{0,0}, 0, WHITE);
}

void DrawSpinnerBack(Texture2D *tex, Color color){
    //currently broken, TODO
    float x = 0;
    float y = 0;
    float ratio = (float)tex->width / (float)tex->height;
    float defaultRatio = 640.0f / 480.0f;
    if(defaultRatio > ratio){
        x = 640.0f - 480.0f * ratio;
    }
    else{
        y = 480.0f - 640.0f / ratio;
    }
    Rectangle source = {0,0,tex->width, tex->height};
    DrawTexturePro(tex, source, ScaleRect(Rectangle{0+x/2.0f,y/2.0f,640-x,(480.0f-y)}), Vector2{0,0}, 0, color);
}

Vector2 getPointOnCircle(float x, float y, float radius, float angle){
    //gets a point on a circle with a defined radius
    angle= (angle * M_PI) / 180;
    float xdiff = radius * cos(angle);
    float ydiff = radius * sin(angle);
    return Vector2{x + xdiff, y + ydiff};
}

void DrawTextureOnCircle(Texture2D *tex, float x, float y, float rad, float s, float r, float ang, Color color){
    //draw texture on a circular path
    Vector2 pos = getPointOnCircle(x, y, rad, ang);
    DrawTextureRotate(tex, pos.x, pos.y, s, r, color);
}

void DrawTextCenter(const char *text, float x, float y, float s, Color color){
    //draw centered text
    Vector2 size = MeasureTextEx(&Global.DefaultFont, text, s, 1);
    DrawTextPro(&Global.DefaultFont, text, ScaleCords(Vector2{x - size.x / 2.0f, y - size.y / 2.0f}), Vector2{0,0}, 0, Scale(s), Scale(1), color);
}

void DrawTextLeft(const char *text, float x, float y, float s, Color color){
    //draw text LTR
    Vector2 size = MeasureTextEx(&Global.DefaultFont, text, s, 1);
    DrawTextPro(&Global.DefaultFont, text, ScaleCords(Vector2{x, y - size.y / 2.0f}), Vector2{0,0}, 0, Scale(s), Scale(1), color);
}

void DrawCNumbersLeft(int n, float x, float y, float s, Color color){
    //I will need to fix this function but currently it works good enough
    GameManager* gm = GameManager::getInstance();
    int digits = log10(n) + 1;
    int i = (digits - 1) * 18;
    for(int k = 0; k < digits; k++){
        DrawTextureCenter(&gm->numbers[nthDigit(n, digits-k-1)], x + k * 18 * s * 2, y, s*4, color);
    }
}

float easeInOutCubic(float x){
    //ease function
    //return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3) / 2.0f;
    /*x = x / 2;
    x += 0.25;
    return ((x * x * (3.0f - 2.0f * x)) - 0.25) * 2.0f;*/
    //return 1.0f - ((1.0f - x) * (1.0f - x));
    return x;
    //return std::sin((x * M_PI) / 2.0);
}

float easeOutQuad(float x){
    //ease function
    //return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3) / 2.0f;
    /*x = x / 2;
    x += 0.25;
    return ((x * x * (3.0f - 2.0f * x)) - 0.25) * 2.0f;*/
    return 1.0f - ((1.0f - x) * (1.0f - x));
    //return x;
    //return std::sin((x * M_PI) / 2.0);
}

bool AreSame(double a, double b){
    //float errors
    return std::fabs(a - b) < 0.0001f;
}

std::vector<std::string> ParseNameFolder(std::string folder){
    //parse the normal osu folder names, they give a lot of info WIP
    std::vector<std::string> output;
    if(folder[folder.size() - 1] == '/'){
        folder.pop_back();
    }
    int end = folder.size() - 1;
    int begin = folder.size() - 1;
    while(begin >= 0 and folder[begin] != '-')
        begin--;
    begin = std::max(0, begin);
    if(folder[begin] == '-'){
        output.push_back("");
        for(int i = begin + 2; i <= end; i++)
            output[output.size()-1].push_back(folder[i]);
        end = std::max(0, begin - 2);
        begin = std::max(0, begin - 2);
        while(begin >= 0 and !isdigit(folder[begin]))
            begin--;
        begin = std::max(0, begin);
        if(isdigit(folder[begin])){
            output.push_back("");
            for(int i = begin + 2; i <= end; i++)
                output[output.size()-1].push_back(folder[i]);
            end = std::max(0, begin);
            begin = std::max(0, begin);
            begin = 0;
            if(isdigit(folder[begin])){
                output.push_back("");
                for(int i = begin; i <= end; i++)
                    output[output.size()-1].push_back(folder[i]);
                end = std::max(0, begin - 2);
                begin = std::max(0, begin - 2);
            }
        }
        else if (begin == 0){
            output.push_back("");
            for(int i = begin; i <= end; i++)
                output[output.size()-1].push_back(folder[i]);
            end = std::max(0, begin);
            begin = std::max(0, begin);
        }
    }
    else{
        output.push_back(folder);
    }

    if(output.size() == 0){
        output.push_back(folder);
    }
    return output;
}

std::vector<std::string> ParseNameFile(std::string file){
    //get get metadata from file WIP
    std::vector<std::string> output;
    Parser parser = Parser();
    GameFile geym = parser.parseMetadata(file);
    output.push_back(geym.configMetadata["Title"]);
    output.push_back(geym.configMetadata["Artist"]);
    output.push_back(geym.configMetadata["Creator"]);
    output.push_back(geym.configMetadata["Version"]);
    output.push_back(geym.configMetadata["BeatmapSetID"]);
    return output;
}

void initTimer(){
    //init the timer
    Global.start = std::chrono::steady_clock::now();
}

void pauseTimer(){
    //currently not in use
}

void resumeTimer(){
    //currently not in use
}


double getTimer(){
    //get the current time elapsed
    Global.end = std::chrono::steady_clock::now();
    return (Global.end - Global.start).count() / 1000000.0;
}

void addOffsetTimer(unsigned long long int time){
    //currently not in use
    Global.pausedFor += time;
}

void updateTimer(){
    //currently not in use
}

bool IsTextureReady(Texture2D *texture){
    // TODO: Validate maximum texture size supported by GPU?

    return ((texture->id > 0) &&         // Validate OpenGL id
            (texture->width > 0) &&
            (texture->height > 0) &&     // Validate texture size
            (texture->format > 0) &&     // Validate texture pixel format
            (texture->mipmaps > 0));     // Validate texture mipmaps (at least 1 for basic mipmap level)
}

bool IsRenderTextureReady(RenderTexture2D *target){
    return (target->texture.id > 0);
}

float getAngle(Vector2 p1, Vector2 p2){
    //get angle from two points
    return atan2(p1.y - p2.y, p1.x - p2.x);
}

std::vector<std::string> getAudioFilenames(int timingSet, int timingSampleIndex, int defaultSampleSet, int normalSet, int additionSet, int hitSound, int index, std::string filename){
    //simple function to get filenames from basic integers
    int defaultSampleSetForObject = 0;
    int defaultSampleIndexForObject = timingSampleIndex;

    if(timingSet == 1)
        defaultSampleSetForObject = 0;
    else if(timingSet == 2)
        defaultSampleSetForObject = 1;
    else if(timingSet == 3)
        defaultSampleSetForObject = 2;
    else
        defaultSampleSetForObject = defaultSampleSet;
    
    int NormalSetForObject = 0;
    
    int HitSoundForObject = 0;
    int SampleIndexForObject = 0;

    if(normalSet == 1)
        NormalSetForObject = 0;
    else if(normalSet == 2)
        NormalSetForObject = 1;
    else if(normalSet == 3)
        NormalSetForObject = 2;
    else
        NormalSetForObject = defaultSampleSetForObject;
    
    
    if(hitSound == 2)
        HitSoundForObject = 1;
    else if(hitSound == 4)
        HitSoundForObject = 2;
    else if(hitSound == 8)
        HitSoundForObject = 3;
    else
        HitSoundForObject = 0;
    
    if(index != 0)
        SampleIndexForObject = index;
    else
        SampleIndexForObject = defaultSampleIndexForObject;
    
    std::string HitSoundIndex = "";

    if(!(SampleIndexForObject == 0)){
        HitSoundIndex = std::to_string(SampleIndexForObject);
    }

    std::string NormalFileName;

    if(NormalSetForObject == 0)
        NormalFileName = "normal-hitnormal";
    else if(NormalSetForObject == 1)
        NormalFileName = "soft-hitnormal";
    else
        NormalFileName = "drum-hitnormal";

    std::vector<std::string> out;
    
    out.push_back(NormalFileName + HitSoundIndex);
    out.push_back(NormalFileName);



    std::string AdditionFilename;

    int AdditionSetForObject = 0;

    if(additionSet == 1)
        AdditionSetForObject = 0;
    else if(additionSet == 2)
        AdditionSetForObject = 1;
    else if(additionSet == 3)
        AdditionSetForObject = 2;
    else
        AdditionSetForObject = NormalSetForObject;

    if(AdditionSetForObject == 0)
        AdditionFilename = "normal-hit";
    else if(AdditionSetForObject == 1)
        AdditionFilename = "soft-hit";
    else
        AdditionFilename = "drum-hit";

    if(filename.size() > 4){
        if(filename[filename.size() - 5] == '.'){
            for(int i = 0; i < 4; i++)
                filename.pop_back();
        }
        out.push_back(filename);
        out.push_back(AdditionFilename);
    }
    else {
        if((hitSound & (1 << 1)) != 0){
            out.push_back(AdditionFilename + "whistle" + HitSoundIndex);
            out.push_back(AdditionFilename + "whistle");
        }
        if((hitSound & (1 << 2)) != 0){
            out.push_back(AdditionFilename + "finish" + HitSoundIndex);
            out.push_back(AdditionFilename + "finish");
        }
        if((hitSound & (1 << 3)) != 0){
            out.push_back(AdditionFilename + "clap" + HitSoundIndex);
            out.push_back(AdditionFilename + "clap");
        }
    }

    return out;
}


unsigned char *LoadFileData(const char *fileName, int *dataSize){
    unsigned char *data = NULL;
    *dataSize = 0;
    if (fileName != NULL){
        FILE *file = fopen(fileName, "rb");
        if (file != NULL){
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);     // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);
            if (size > 0){
                data = (unsigned char *)malloc(size*sizeof(unsigned char));
                if (data != NULL){
                    size_t count = fread(data, sizeof(unsigned char), size, file);
                    if (count > 2147483647){
                        free(data);
                        data = NULL;
                    }
                    else{
                        *dataSize = (int)count;
                    }
                }
            }
            fclose(file);
        }
    }

    return data;
}


void UnloadFileData(unsigned char *data){
    free(data);
}

bool TextIsEqual(const char *text1, const char *text2){
    bool result = false;

    if ((text1 != NULL) && (text2 != NULL)){
        if (strcmp(text1, text2) == 0) result = true;
    }

    return result;
}

const char *GetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return dot;
}

bool IsFileExtension(const char *fileName, const char *ext){
    #define MAX_FILE_EXTENSION_LENGTH  16

    bool result = false;
    const char *fileExt = GetFileExtension(fileName);

    if (fileExt != NULL){
        if (strcmp(fileExt, ext) == 0) result = true;
    }

    return result;
}