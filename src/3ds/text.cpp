#include "text.h"
#include "rlgl.h"
#include "string.h"
#include "utils.hpp"


#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "raymath.h"


const char *TextFormat(const char *text, ...){
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    int requiredByteCount = vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    // If requiredByteCount is larger than the MAX_TEXT_BUFFER_LENGTH, then overflow occured
    if (requiredByteCount >= MAX_TEXT_BUFFER_LENGTH)
    {
        // Inserting "..." at the end of the string to mark as truncated
        char *truncBuffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
        sprintf(truncBuffer, "...");
    }

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type){
    GlyphInfo *chars = NULL;
    if (fileData != NULL){
        bool genFontChars = false;
        stbtt_fontinfo fontInfo = { 0 };

        if (stbtt_InitFont(&fontInfo, (unsigned char *)fileData, 0)){
            float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);
            int ascent, descent, lineGap;
            stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
            codepointCount = (codepointCount > 0)? codepointCount : 95;
            if (codepoints == NULL){
                codepoints = (int *)malloc(codepointCount*sizeof(int));
                for (int i = 0; i < codepointCount; i++) codepoints[i] = i + 32;
                genFontChars = true;
            }
            chars = (GlyphInfo *)calloc(codepointCount, sizeof(GlyphInfo));

            for (int i = 0; i < codepointCount; i++){
                int chw = 0, chh = 0;   // Character width and height (on generation)
                int ch = codepoints[i];  // Character value to get info for
                chars[i].value = ch;
                int index = stbtt_FindGlyphIndex(&fontInfo, ch);
                if (index > 0){
                    switch (type){
                        case FONT_DEFAULT:
                        case FONT_BITMAP: chars[i].image.data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY); break;
                        case FONT_SDF: if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, FONT_SDF_CHAR_PADDING, FONT_SDF_ON_EDGE_VALUE, FONT_SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offsetX, &chars[i].offsetY); break;
                        default: break;
                    }

                    if (chars[i].image.data != NULL){
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);

                        // Load characters images
                        chars[i].image.width = chw;
                        chars[i].image.height = chh;
                        chars[i].image.mipmaps = 1;
                        chars[i].image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
                        chars[i].offsetY += (int)((float)ascent*scaleFactor);
                    }
                    if (ch == 32)
                    {
                        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advanceX, NULL);
                        chars[i].advanceX = (int)((float)chars[i].advanceX*scaleFactor);

                        Image imSpace = {
                            .data = calloc(chars[i].advanceX*fontSize, 2),
                            .width = chars[i].advanceX,
                            .height = fontSize,
                            .mipmaps = 1,
                            .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
                        };

                        chars[i].image = imSpace;
                    }

                    if (type == FONT_BITMAP){
                        for (int p = 0; p < chw*chh; p++){
                            if (((unsigned char *)chars[i].image.data)[p] < FONT_BITMAP_ALPHA_THRESHOLD) ((unsigned char *)chars[i].image.data)[p] = 0;
                            else ((unsigned char *)chars[i].image.data)[p] = 255;
                        }
                    }
                }
            }
        }
        if(genFontChars)
            free(codepoints);
    }
    return chars;
}

Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount){
    Font font = { 0 };

    char fileExtLower[16] = { 0 };
    strncpy(fileExtLower, TextToLower(fileType), 16 - 1);

    font.baseSize = fontSize;
    font.glyphCount = (codepointCount > 0)? codepointCount : 95;
    font.glyphPadding = 0;
    if (TextIsEqual(fileExtLower, ".ttf") || TextIsEqual(fileExtLower, ".otf")){
        font.glyphs = LoadFontData(fileData, dataSize, font.baseSize, codepoints, font.glyphCount, FONT_DEFAULT);
    }
    if (font.glyphs != NULL)
    {
        font.glyphPadding = FONT_TTF_DEFAULT_CHARS_PADDING;

        Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, font.glyphPadding, 0);
        font.texture = LoadTextureFromImage(&atlas);

        // Update glyphs[i].image to use alpha, required to be used on ImageDrawText()
        for (int i = 0; i < font.glyphCount; i++)
        {
            UnloadImage(&font.glyphs[i].image);
            font.glyphs[i].image = ImageFromImage(&atlas, font.recs[i]);
        }

        UnloadImage(&atlas);
    }
    return font;
}

Font LoadFont(const char *fileName){
    Font font = { 0 };
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);
    if (fileData != NULL){
        font = LoadFontFromMemory(GetFileExtension(fileName), fileData, dataSize, FONT_TTF_DEFAULT_SIZE, NULL, FONT_TTF_DEFAULT_NUMCHARS);
        UnloadFileData(fileData);
    }
    return font;
}

Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod){
    Image atlas = { 0 };
    if (glyphs == NULL){
        return atlas;
    }
    *glyphRecs = NULL;
    glyphCount = (glyphCount > 0)? glyphCount : 95;
    Rectangle *recs = (Rectangle *)malloc(glyphCount*sizeof(Rectangle));
    int totalWidth = 0;
    int maxGlyphWidth = 0;
    for (int i = 0; i < glyphCount; i++){
        if (glyphs[i].image.width > maxGlyphWidth) maxGlyphWidth = glyphs[i].image.width;
        totalWidth += glyphs[i].image.width + 2*padding;
    }
    int paddedFontSize = fontSize + 2*padding;
    // No need for a so-conservative atlas generation
    float totalArea = totalWidth*paddedFontSize*1.2f;
    float imageMinSize = sqrtf(totalArea);
    int imageSize = (int)powf(2, ceilf(logf(imageMinSize)/logf(2)));

    if (totalArea < ((imageSize*imageSize)/2)){
        atlas.width = imageSize;    // Atlas bitmap width
        atlas.height = imageSize/2; // Atlas bitmap height
    }
    else{
        atlas.width = imageSize;   // Atlas bitmap width
        atlas.height = imageSize;  // Atlas bitmap height
    }

    atlas.data = (unsigned char *)calloc(1, atlas.width*atlas.height);   // Create a bitmap to store characters (8 bpp)
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    atlas.mipmaps = 1;

    if (packMethod == 0){
        int offsetX = padding;
        int offsetY = padding;

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < glyphCount; i++){
            // Check remaining space for glyph
            if (offsetX >= (atlas.width - glyphs[i].image.width - 2*padding)){
                offsetX = padding;
                offsetY += (fontSize + 2*padding);

                if (offsetY > (atlas.height - fontSize - padding)){
                    for(int j = i + 1; j < glyphCount; j++){
                        recs[j].x = 0;
                        recs[j].y = 0;
                        recs[j].width = 0;
                        recs[j].height = 0;
                    }
                    break;
                }
            }

            // Copy pixel data from glyph image to atlas
            for (int y = 0; y < glyphs[i].image.height; y++){
                for (int x = 0; x < glyphs[i].image.width; x++){
                    ((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = ((unsigned char *)glyphs[i].image.data)[y*glyphs[i].image.width + x];
                }
            }
            recs[i].x = (float)offsetX;
            recs[i].y = (float)offsetY;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;
            offsetX += (glyphs[i].image.width + 2*padding);
        }
    }
    else if (packMethod == 1){
        stbrp_context *context = (stbrp_context *)malloc(sizeof(*context));
        stbrp_node *nodes = (stbrp_node *)malloc(glyphCount*sizeof(*nodes));

        stbrp_init_target(context, atlas.width, atlas.height, nodes, glyphCount);
        stbrp_rect *rects = (stbrp_rect *)malloc(glyphCount*sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < glyphCount; i++){
            rects[i].id = i;
            rects[i].w = glyphs[i].image.width + 2*padding;
            rects[i].h = glyphs[i].image.height + 2*padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(context, rects, glyphCount);

        for (int i = 0; i < glyphCount; i++){
            // It returns char rectangles in atlas
            recs[i].x = rects[i].x + (float)padding;
            recs[i].y = rects[i].y + (float)padding;
            recs[i].width = (float)glyphs[i].image.width;
            recs[i].height = (float)glyphs[i].image.height;

            if (rects[i].was_packed){
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < glyphs[i].image.height; y++){
                    for (int x = 0; x < glyphs[i].image.width; x++)
                    {
                        ((unsigned char *)atlas.data)[(rects[i].y + padding + y)*atlas.width + (rects[i].x + padding + x)] = ((unsigned char *)glyphs[i].image.data)[y*glyphs[i].image.width + x];
                    }
                }
            }
        }

        free(rects);
        free(nodes);
        free(context);
    }
    unsigned char *dataGrayAlpha = (unsigned char *)malloc(atlas.width*atlas.height*sizeof(unsigned char)*2); // Two channels

    for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2){
        dataGrayAlpha[k] = 255;
        dataGrayAlpha[k + 1] = ((unsigned char *)atlas.data)[i];
    }

    free(atlas.data);
    atlas.data = dataGrayAlpha;
    atlas.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;

    *glyphRecs = recs;

    return atlas;
}

const char *TextToLower(const char *text){
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);
    if (text != NULL){
        for (int i = 0; (i < MAX_TEXT_BUFFER_LENGTH - 1) && (text[i] != '\0'); i++){
            if ((text[i] >= 'A') && (text[i] <= 'Z')) buffer[i] = text[i] + 32;
            else buffer[i] = text[i];
        }
    }

    return buffer;
}

unsigned int TextLength(const char *text){
    unsigned int length = 0;

    if (text != NULL){
        // NOTE: Alternative: use strlen(text)

        while (*text++) length++;
    }

    return length;
}

int GetCodepointNext(const char *text, int *codepointSize){
    const char *ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    *codepointSize = 1;

    // Get current codepoint and bytes processed
    if (0xf0 == (0xf8 & ptr[0])){
        // 4 byte UTF-8 codepoint
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepointSize = 4;
    }
    else if (0xe0 == (0xf0 & ptr[0])){
        // 3 byte UTF-8 codepoint */
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepointSize = 3;
    }
    else if (0xc0 == (0xe0 & ptr[0])){
        // 2 byte UTF-8 codepoint
        if((ptr[1] & 0xC0) ^ 0x80) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepointSize = 2;
    }
    else if (0x00 == (0x80 & ptr[0])){
        // 1 byte UTF-8 codepoint
        codepoint = ptr[0];
        *codepointSize = 1;
    }

    return codepoint;
}

int GetGlyphIndex(Font *font, int codepoint){
    //No support for unordered glyphs yet
    int index = 0;
    index = codepoint - 32;
    return index;
}

void DrawTextCodepoint(Font *font, int codepoint, Vector2 position, float fontSize, Color tint){
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scaleFactor = fontSize/font->baseSize;     // Character quad scaling factor

    // Character destination rectangle on screen
    // NOTE: We consider glyphPadding on drawing
    Rectangle dstRec = { position.x + font->glyphs[index].offsetX*scaleFactor - (float)font->glyphPadding*scaleFactor,
                      position.y + font->glyphs[index].offsetY*scaleFactor - (float)font->glyphPadding*scaleFactor,
                      (font->recs[index].width + 2.0f*font->glyphPadding)*scaleFactor,
                      (font->recs[index].height + 2.0f*font->glyphPadding)*scaleFactor };

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font->recs[index].x - (float)font->glyphPadding, font->recs[index].y - (float)font->glyphPadding,
                         font->recs[index].width + 2.0f*font->glyphPadding, font->recs[index].height + 2.0f*font->glyphPadding };

    // Draw the character texture on the screen
    DrawTexturePro(&font->texture, srcRec, dstRec, (Vector2){ 0, 0 }, 0.0f, tint);
    
}

void DrawTextEx(Font *font, const char *text, Vector2 position, float fontSize, float spacing, Color tint){
    if (font->texture.id == 0) return;

    int size = TextLength(text);    // Total size in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font->baseSize;         // Character quad scaling factor

    for (int i = 0; i < size;){
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n'){
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            int textLineSpacing = 15;
            textOffsetY += textLineSpacing;
            textOffsetX = 0.0f;
        }
        else{
            if ((codepoint != ' ') && (codepoint != '\t')){
                DrawTextCodepoint(font, codepoint, (Vector2){ position.x + textOffsetX, position.y + textOffsetY }, fontSize, tint);
            }
            if (font->glyphs[index].advanceX == 0)
                textOffsetX += ((float)font->recs[index].width*scaleFactor + spacing);
            else
                textOffsetX += ((float)font->glyphs[index].advanceX*scaleFactor + spacing);
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

Vector2 MeasureTextEx(Font *font, const char *text, float fontSize, float spacing)
{
    Vector2 textSize = { 0 };

    if ((font->texture.id == 0) || (text == NULL)) return textSize;

    int size = TextLength(text);    // Get size in bytes of text
    int tempByteCounter = 0;        // Used to count longer text line num chars
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float textHeight = fontSize;
    float scaleFactor = fontSize/(float)font->baseSize;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < size;)
    {
        byteCounter++;

        int next = 0;
        letter = GetCodepointNext(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        i += next;

        if (letter != '\n')
        {
            if (font->glyphs[index].advanceX != 0) textWidth += font->glyphs[index].advanceX;
            else textWidth += (font->recs[index].width + font->glyphs[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;

            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            int textLineSpacing = 15;
            textHeight += (float)textLineSpacing;
        }

        if (tempByteCounter < byteCounter) tempByteCounter = byteCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    textSize.x = tempTextWidth*scaleFactor + (float)((tempByteCounter - 1)*spacing);
    textSize.y = textHeight;

    return textSize;
}

void UnloadFontData(GlyphInfo *glyphs, int glyphCount){
    if (glyphs != NULL){
        for (int i = 0; i < glyphCount; i++) UnloadImage(&glyphs[i].image);
        free(glyphs);
    }
}

void UnloadFont(Font *font){
    UnloadFontData(font->glyphs, font->glyphCount);
    UnloadTexture(&font->texture);
    free(font->recs);
}

void DrawTextPro(Font *font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint){
    //not good but meh
    DrawTextEx(font, text, (Vector2){ position.x - origin.x, position.y - origin.y}, fontSize, spacing, tint);
}