/**
Javascript SGL library ported to C with SDL3. Goal here is again to bring more 
performance since the pipeline is 100% sequential (No GPU) and running it in a 
browser slows things down a bit. This port tries to stay as close as possible to the original
OOP structure but some extra things were added to make it work on C (eg: The data structures).
Also notice how now every "class" and "methods" have SGL_ in front. This was used to avoid mixing up
with other libraries like SDL for example. Methods name are structured as follow SGL_[Name of the class][Method name]
except for constructors where it goes as follow: SGL_Create[Name of the class]/SGL_Free[Name of the class].
Pointers passed as arguments for structs are not freed automatically most of the time when calling a free method of a struct. It's your
responsability (no garbage collector like in javascript lol :\. It will be mentionned when memory is managed). Since C has a different paradigm, you'll
have to handle a few SDL events yourself in the while loop. The goal here was really to abstract away
as much as possible from the SDL library but sadly some things still need to be done. Anyways everything is in the demo.

NOTICE: The MakeFile compiles this code for SDL3 x86_64-w64-mingw32.
You can change the target as long as you include SDL3 (you will need to have the folder for the right target with the right .dll)
*/

#ifndef SGL_h
#define SGL_h

#include <math.h>
#include <SDL3/SDL.h>

#include "SGL_List.h"

#ifdef __cplusplus
extern "C" {
#endif

// Utilities functions (Originally SGLMath)

/**
 * Converts degrees to radians.
 * \returns Radians
 */
float SGL_DegToRad(float degrees);
/**
 * Gives the cotangent of the opening of a frustum.
 * \param degrees Opening of the frustum in degrees
 * \returns Cotangent
 */
float SGL_Cot(float degrees);

/**
 * Block of xyz coordinates (used for position, scale, rotation, etc.)
 */
typedef struct {
    float x;
    float y;
    float z;
} SGL_Vector3;

/**
 * Color
 */
typedef struct {
    float r;
    float g;
    float b;
} SGL_Color;

extern const SGL_Color SGL_RED;
extern const SGL_Color SGL_GREEN;
extern const SGL_Color SGL_BLUE;

/**
 * Camera inside SGL_Scene.
 */
typedef struct {
    float near;
    float far;
    float fov;
    SGL_Vector3 position;
    SGL_Vector3 orientation;
} SGL_Camera;

/**
 * Scene containing all the meshes and the camera.
 */
typedef struct {
    SGL_List *meshes;
    SGL_Camera *currentCamera;
} SGL_Scene;

SGL_Scene* SGL_CreateScene();
/**
 * Meshes added in the scene will not be freed. Meshes are managed by the user.
 */
void SGL_FreeScene(SGL_Scene *scene);

/**
 * Contains the mesh's properties, it's vertices and triangles. The transformation_matrix is computed automatically, not input required.
 */
typedef struct {
    SGL_Vector3 position;
    SGL_Vector3 orientation;
    SGL_Vector3 scale;
    SGL_List *vertices;
    SGL_List *triangles;
    float transformation_matrix[16];
} SGL_Mesh;

/**
 * Contains it's position and a pointer to the mesh it was placed in (used for converting position from local space to world space).
 */
typedef struct {
    SGL_Vector3 position;
    SGL_Mesh *mesh;
} SGL_Vertex;

/**
 * Contains pointers to 3 vertices and it's color.
 */
typedef struct {
    SGL_Vertex *vertex1;
    SGL_Vertex *vertex2;
    SGL_Vertex *vertex3;
    SGL_Color color;
} SGL_Triangle;

SGL_Mesh* SGL_CreateMesh(SGL_Vertex vertices[], float_safe_index_t vertices_count, SGL_Triangle triangles[], float_safe_index_t triangles_count, SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale);
/**
 * Free a mesh passed as argument (All vertices and triangles are freed too).
 */
void SGL_FreeMesh(SGL_Mesh *mesh);

// Mesh Templates
SGL_Mesh* SGL_CreateCubeMesh(SGL_Vector3 position);

/**
 * Renderer containing the SDL_Window, SDL_Renderer and SDL_Texture buffer. Members were hidden to
 * abstract away the SDL library as much as possible.
 */
typedef struct SGL_Renderer SGL_Renderer;

/**
 * Creates SGL renderer, initializes SDL and all necessary components.
 * \param name Name of the SDL_Window
 * \param scene Pointer to SGL scene to render
 */
SGL_Renderer* SGL_CreateRenderer(const char *name, SGL_Scene *scene);
void SGL_FreeRenderer(SGL_Renderer *renderer);
/**
 * Renders the scene passed as an argument at creation.
 * \param event Pointer to polled SDL_Event
 */
bool SGL_Render(SGL_Renderer *renderer, SDL_Event *event);
/**
 * Change scene for this renderer.
 */
void SGL_RendererSetScene(SGL_Renderer *renderer, SGL_Scene *scene);
/**
 * Get the SDL window (only if you know what you're doing). I created the abstraction
 * layer so you don't have to deal with the window itself but if you wanna tweak it and add your own stuff feel free.
 */
SDL_Window* SGL_RendererGetWindow(SGL_Renderer *renderer);

#ifdef __cplusplus
}
#endif

#endif