#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

void FixDefaultTexture();

int main(int argc, char** argv) {
    
    float scaleFactor = 0.5;
    const char* colorImgPath = "res/elephant.png";
    const char* depthImgPath = "res/elephant_depth.png";
    
    Image colorImg = LoadImage(colorImgPath);
    if (colorImg.data == NULL) {
        fprintf(stderr, "ERROR: Could not load color image `%s`\n", colorImgPath);
        return 1;
    }

    Image depthImg = LoadImage(depthImgPath);
    if (depthImg.data == NULL) {
        fprintf(stderr, "ERROR: Could not load depth image `%s`\n", depthImgPath);
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(colorImg.width * scaleFactor, colorImg.height * scaleFactor, "Autostereogram");

    FixDefaultTexture();

    
    // Load shaders
    Shader mangoShader = LoadShader(NULL, "shader/mango.glsl");
    
    // Load / Create Textures
    Texture2D colorTexture = LoadTextureFromImage(colorImg);

    int width, height;

    while(!WindowShouldClose()) {
        BeginDrawing();
            width  = GetRenderWidth();
            height = GetRenderHeight();
            BeginShaderMode(mangoShader);
                DrawRectangle(0, 0, width, height, WHITE);
            EndShaderMode();
            DrawTextureEx(colorTexture, (Vector2){0, 0}, 0, scaleFactor, WHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void FixDefaultTexture() {
    // Set default internal texture (1px white) and rectangle to be used for shapes drawing -> Otherwise UVs will not work for rectangles
    Texture2D defaultTexture = {
        .format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .width = 1,
        .height = 1,
        .mipmaps = 0,
        .id = rlGetTextureIdDefault()
    };
    SetShapesTexture(defaultTexture, (Rectangle){ 0.0f, 0.0f, 1.0f, 1.0f });
}

// Load a render texture with 16 bits per channel
RenderTexture2D LoadRenderTexture16(int width, int height) {
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(width, height); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R16G16B16, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R16G16B16;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}
