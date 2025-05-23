/*
Javascript SGL library ported to C with SDL3. Goal here is again to bring more 
performance since the pipeline is 100% sequential (No GPU) and running it in a 
browser slows things down a bit. This port tries to stay as close as possible to the original
OOP structure but some extra things were added to make it work on C (eg: the SGL_List struct).
Also notice how now every "class" and "methods" have SGL_ in front. This was used to avoid mixing up
with other libraries like SDL for example. Methods name are structured as follow SGL_[Name of the class][Method name]
except for constructors where it goes as follow: SGL_Create[Name of the class]/SGL_Free[Name of the class].
Pointers passed as arguments for structs are not freed automatically when calling a free method of a struct. It's your
responsability (no garbage collector like in javascript lol :\). Since C has a different paradigm, you'll
have to handle a few SDL events yourself in the while loop. The goal here was really to abstract away
as much as possible from the SDL library but sadly some things still need to be done. Anyways everything is in the demo.

NOTICE: The MakeFile compiles this code for SDL3 x86_64-w64-mingw32.
You can change the target as long as it's SDL3 (You can basically detach the SGL.h and SGL.c file from this project
to use it for other targets of SDL3)
*/

#ifndef SGL_h
#define SGL_h

#include <SDL3/SDL_events.h>

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
 * Resizable array used for any type. Warning: Putting different types of data can work
 * but it is recommended to stick with only one.
 */
typedef struct {
    void **items;
    size_t size;
    size_t capacity;
} SGL_List;

SGL_List* SGL_CreateList();
SGL_List* SGL_CreateListFromArray(void **array, size_t count);
void SGL_ListAdd(SGL_List *list, void *item);
void* SGL_ListGet(SGL_List *list, size_t index);
size_t SGL_ListIndexOf(SGL_List *list, void *item);
void SGL_ListRemove(SGL_List *list, size_t index);
void** SGL_ListToArray(SGL_List *list);
void SGL_FreeList(SGL_List *list);

typedef struct {
    float x;
    float y;
    float z;
} SGL_Vector3;

typedef struct {
    float r;
    float g;
    float b;
} SGL_Color;

const SGL_Color SGL_RED = (SGL_Color){.r = 1.0f, .g = 0.0f, .b = 0.0f};
const SGL_Color SGL_GREEN = (SGL_Color){.r = 0.0f, .g = 1.0f, .b = 0.0f};
const SGL_Color SGL_BLUE = (SGL_Color){.r = 0.0f, .g = 0.0f, .b = 1.0f};

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
void SGL_FreeScene(SGL_Scene *scene);

/**
 * Contains it's position, the vertices and triangles.
 */
typedef struct {
    SGL_Vector3 position;
    SGL_Vector3 orientation;
    SGL_Vector3 scale;
    SGL_List *vertices;
    SGL_List *triangles;
    float transformation_matrix[16];
} SGL_Mesh;

SGL_Mesh* SGL_CreateMesh(SGL_List *vertices, SGL_List *triangles, SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale);

// Mesh Templates
SGL_Mesh* SGL_CreateCubeMesh(SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale);

/**
 * Contains pointers to 3 vertices and it's color.
 */
typedef struct {
    SGL_Vertex *vertex1;
    SGL_Vertex *vertex2;
    SGL_Vertex *vertex3;
    SGL_Color color;
} SGL_Triangle;

/**
 * Contains it's position and a pointer to the mesh it was placed in (used for converting position from local space to world space).
 */
typedef struct {
    SGL_Vector3 position;
    SGL_Mesh *mesh;
} SGL_Vertex;

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

#endif