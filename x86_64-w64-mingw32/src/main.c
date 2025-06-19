#include <stdio.h>
#include "SGL.h"

int main(int argc, char* argv[]) {
    SGL_Scene *scene = SGL_CreateScene(); // Create scene attached to renderer
    SGL_Renderer *renderer = SGL_CreateRenderer("SGL C Demo", scene);

    if (!renderer) {
        printf("Couldn't start SGL.");
        return 1;
    }

    // Add meshes to scene
    SGL_Mesh *cube = SGL_CreateCubeMesh((SGL_Vector3){.x = 0, .y = 0, .z = 3}, (SGL_Vector3){.x = 0, .y = 0, .z = 0}, (SGL_Vector3){.x = 1, .y = 1, .z = 1});
    SGL_ListAdd(scene->meshes, cube);

    bool running = true;

    while (running) {
        SDL_Event event;

        /**
         * Manually poll events here to be able to read for user inputs, etc. and send event to SGL_Render() first
         * since it handles the window, if the function returns false, then you know the SDL program stopped
         * and you should too (depending on what you're doing of course)
         */
        while (SDL_PollEvent(&event)) {
            if (!SGL_Render(renderer, &event)) {
                running = false;
                break;
            }
            // Add other events check here for event logic (eg: character moving based on user input)
        }

        if (!running) break;

        // Repeating logic (eg: animal moving)
    }

    // Don't forget to free your stuff here!
    SGL_FreeRenderer(renderer);
    SGL_FreeScene(scene);
    SGL_FreeMesh(cube);

    return 0;
}
