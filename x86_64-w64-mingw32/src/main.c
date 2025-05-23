#include <stdio.h>
#include "SGL.h"

int main(int argc, char* argv[]) {
    SGL_Scene *scene = SGL_CreateScene();
    SGL_Renderer *renderer = SGL_CreateRenderer("SGL C Demo", scene);

    if (!renderer) {
        printf("Couldn't start SGL.");
        return 1;
    }

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (!SGL_Render(renderer, &event)) {
                running = false;
                break;
            }
            // Add other events check here
        }

        if (!running) break;

        // Logic (moving things, etc.)
    }

    SGL_FreeRenderer(renderer);
    SGL_FreeScene(scene);

    return 0;
}
