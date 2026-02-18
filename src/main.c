#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

int main(int argc, char* argv[]) {
    printf("=== Library Test ===\n");

    // Test SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL2 init failed: %s\n", SDL_GetError());
        return -1;
    }
    printf("✓ SDL2 initialized\n");

    // Test FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        printf("FreeType init failed\n");
        return -1;
    }
    printf("✓ FreeType initialized\n");
    FT_Done_FreeType(ft);

    // Test OpenGL window
    SDL_Window* window = SDL_CreateWindow(
        "Game Engine Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Window creation failed\n");
        return -1;
    }
    printf("✓ Window created\n");

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    // Test GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("GLAD initialization failed\n");
        return -1;
    }
    printf("✓ GLAD loaded OpenGL\n");
    printf("✓ OpenGL Version: %s\n", glGetString(GL_VERSION));

    printf("\n✅ ALL LIBRARIES WORKING!\n");
    printf("Press any key to close...\n");
    getchar();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
