#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "SGL.h"

const SGL_Color SGL_RED = (SGL_Color){.r = 1.0f, .g = 0.0f, .b = 0.0f};
const SGL_Color SGL_GREEN = (SGL_Color){.r = 0.0f, .g = 1.0f, .b = 0.0f};
const SGL_Color SGL_BLUE = (SGL_Color){.r = 0.0f, .g = 0.0f, .b = 1.0f};

// Engine constants
static const int VERTEX_ARRAY_SIZE = 4;
static const int TRIANGLE_ARRAY_SIZE = 6;

float SGL_DegToRad(float degrees) {
    return degrees * (M_PI / 180);
}

float SGL_Cot(float degrees) {
    return 1.0f * tan(SGL_DegToRad(degrees));
}

// SGL_Mesh
SGL_Mesh* SGL_CreateMesh(SGL_List *vertices, SGL_List *triangles, SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale) {
    SGL_Mesh *mesh = malloc(sizeof(SGL_Mesh));
    mesh->vertices = vertices;

    for (size_t i = 0; i < vertices->size; i++)
    {
        SGL_Vertex *vertex = (SGL_Vertex *)vertices->items[i];
        vertex->mesh = mesh;
    }

    mesh->triangles = triangles;
    mesh->position = position;
    mesh->orientation = orientation;
    mesh->scale = scale;
    return mesh;
}

// Mesh template
SGL_Mesh* SGL_CreateCubeMesh(SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale) {
    SGL_Vertex vertices[] = {
        {.position = (SGL_Vector3){-1, 1, -1}},
        {.position = (SGL_Vector3){1, 1, -1}},
        {.position = (SGL_Vector3){1, -1, -1}},
        {.position = (SGL_Vector3){-1, -1, -1}},
        {.position = (SGL_Vector3){-1, 1, 1}},
        {.position = (SGL_Vector3){1, 1, 1}},
        {.position = (SGL_Vector3){1, -1, 1}},
        {.position = (SGL_Vector3){-1, -1, 1}}
    };

    SGL_Triangle triangles[] = {
        {.vertex1 = &vertices[0], .vertex2 = &vertices[1], .vertex3 = &vertices[2], .color = SGL_BLUE},
        {.vertex1 = &vertices[0], .vertex2 = &vertices[1], .vertex3 = &vertices[3], .color = SGL_BLUE},
        {.vertex1 = &vertices[6], .vertex2 = &vertices[5], .vertex3 = &vertices[4], .color = SGL_BLUE},
        {.vertex1 = &vertices[7], .vertex2 = &vertices[6], .vertex3 = &vertices[4], .color = SGL_BLUE},
        {.vertex1 = &vertices[5], .vertex2 = &vertices[1], .vertex3 = &vertices[0], .color = SGL_RED},
        {.vertex1 = &vertices[4], .vertex2 = &vertices[5], .vertex3 = &vertices[0], .color = SGL_RED},
        {.vertex1 = &vertices[3], .vertex2 = &vertices[2], .vertex3 = &vertices[6], .color = SGL_RED},
        {.vertex1 = &vertices[5], .vertex2 = &vertices[6], .vertex3 = &vertices[7], .color = SGL_RED},
        {.vertex1 = &vertices[3], .vertex2 = &vertices[4], .vertex3 = &vertices[0], .color = SGL_GREEN},
        {.vertex1 = &vertices[4], .vertex2 = &vertices[3], .vertex3 = &vertices[7], .color = SGL_GREEN},
        {.vertex1 = &vertices[1], .vertex2 = &vertices[5], .vertex3 = &vertices[6], .color = SGL_GREEN},
        {.vertex1 = &vertices[6], .vertex2 = &vertices[2], .vertex3 = &vertices[1], .color = SGL_GREEN}
    };

    size_t vertices_count = sizeof(vertices) / sizeof(SGL_Vertex);
    size_t triangles_count = sizeof(triangles) /sizeof(SGL_Triangle);

    void *vertices_ptrs[vertices_count];
    void *triangles_ptrs[triangles_count];

    for (size_t i = 0; i < vertices_count; i++)
    {
        vertices_ptrs[i] = &vertices[i];
    }

    for (size_t i = 0; i < triangles_count; i++)
    {
        triangles_ptrs[i] = &triangles[i];
    }

    return SGL_CreateMesh(SGL_CreateListFromArray(vertices_ptrs, vertices_count), SGL_CreateListFromArray(triangles_ptrs, triangles_count), position, orientation, scale);
}

// SGL_Scene
SGL_Scene* SGL_CreateScene() {
    SGL_Scene* scene = malloc(sizeof(SGL_Scene));
    scene->meshes = SGL_CreateList();
    SGL_Camera camera = (SGL_Camera){
        .near = 0.1f, 
        .far = 100.0f, 
        .position = (SGL_Vector3){.x = 0, .y = 0, .z = 0},
        .orientation = (SGL_Vector3){.x = 0, .y = 0, .z = 0}
    };
    scene->currentCamera = &camera;
    return scene;
}

void SGL_FreeScene(SGL_Scene *scene) {
    SGL_FreeList(scene->meshes);
    free(scene);
}

// Matrices

static void multiply_matrix_with_vertex(float m[16], size_t vertex_index, float vertices_data[]) {
    float x = vertices_data[vertex_index];
    float y = vertices_data[vertex_index + 1];
    float z = vertices_data[vertex_index + 2];
    float w = vertices_data[vertex_index + 3];

    vertices_data[vertex_index] = x * m[0] + y * m[4] + z * m[8] + w * m[12];
    vertices_data[vertex_index + 1] = x * m[1] + y * m[5] + z * m[9] + w * m[13];
    vertices_data[vertex_index + 2] = x * m[2] + y * m[6] + z * m[10] + w * m[14];
    vertices_data[vertex_index + 3] = x * m[3] + y * m[7] + z * m[11] + w * m[15];
}

static void multiply_matrix_with_vertices(float m[16], float vertices_data[], size_t vertices_size) {
    for (size_t i = 0; i < vertices_size; i++)
    {
        multiply_matrix_with_vertex(m, i, vertices_data);
    }
}

static void multiply_4x4_matrix(float a[16], float b[16], float out[16]) {
    for (int i = 0; i < 4; i++)
    {
        int offset_a = i * 4;
        for (int j = 0; j < 4; j++)
        {
            int offset_b = j;
            out[offset_a + j] = a[offset_a] * b[offset_b] +
            a[offset_a + 1] * b[4 + offset_b] + 
            a[offset_a + 2] * b[8 + offset_b] + 
            a[offset_a + 3] * b[12 + offset_b];
        }
    }
}

static void create_translation_matrix(float x, float y, float z, float out[16]) {
    float mat[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_scale_matrix(float x, float y, float z, float out[16]) {
    float mat[16] = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_pitch_matrix(float pitch_degrees, float out[16]) {
    float pitch_radians = SGL_DegToRad(pitch_degrees);
    float c = cos(pitch_radians);
    float s = sin(pitch_radians);

    float mat[16] = {
        1, 0, 0, 0,
        0, c, -s, 0,
        0, s,  c, 0,
        0, 0, 0, 1
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_yaw_matrix(float yaw_degrees, float out[16]) {
    float yaw_radians = SGL_DegToRad(yaw_degrees);
    float c = cos(yaw_radians);
    float s = sin(yaw_radians);

    float mat[16] = {
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_roll_matrix(float roll_degrees, float out[16]) {
    float roll_radians = SGL_DegToRad(roll_degrees);
    float c = cos(roll_radians);
    float s = sin(roll_radians);

    float mat[16] = {
         c, -s, 0, 0,
         s,  c, 0, 0,
         0,  0, 1, 0,
         0,  0, 0, 1
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_euler_matrix(float x, float y, float z, float out[16]) {
    float yaw_matrix[16];
    float pitch_matrix[16];
    float roll_matrix[16];

    float yaw_x_pitch_matrix[16];
    float yaw_x_pitch_x_roll_matrix[16];

    create_yaw_matrix(y, yaw_matrix);
    create_pitch_matrix(x, pitch_matrix);
    create_roll_matrix(z, roll_matrix);

    multiply_4x4_matrix(yaw_matrix, pitch_matrix, yaw_x_pitch_matrix);
    multiply_4x4_matrix(yaw_x_pitch_matrix, roll_matrix, yaw_x_pitch_x_roll_matrix);

    memcpy(out, yaw_x_pitch_x_roll_matrix, sizeof(float) * 16);
}

static void create_transformation_matrix(SGL_Vector3 position, SGL_Vector3 rotation, SGL_Vector3 scale, float out[16]) {
    float scale_matrix[16];
    float rotation_matrix[16];
    float translation_matrix[16];

    float scale_x_rotation_matrix[16];
    float scale_x_rotation_x_translation[16];

    create_scale_matrix(scale.x, scale.y, scale.z, scale_matrix);
    create_euler_matrix(rotation.x, rotation.y, rotation.z, rotation_matrix);
    create_translation_matrix(position.x, position.y, position.z, translation_matrix);

    multiply_4x4_matrix(scale_matrix, rotation_matrix, scale_x_rotation_matrix);
    multiply_4x4_matrix(scale_x_rotation_matrix, translation_matrix, scale_x_rotation_x_translation);

    memcpy(out, scale_x_rotation_x_translation, sizeof(float) * 16);
}

static void create_view_matrix(SGL_Camera *camera, float out[16]) {
    float translation_matrix[16];
    float euler_matrix[16];

    float translation_x_euler_matrix[16];

    create_translation_matrix(camera->position.x, camera->position.y, camera->position.z, translation_matrix);
    create_euler_matrix(camera->orientation.x, camera->orientation.y, camera->orientation.z, euler_matrix);
    multiply_4x4_matrix(translation_matrix, euler_matrix, translation_x_euler_matrix);

    memcpy(out, translation_x_euler_matrix, sizeof(float) * 16);
}

// SGL_Renderer
struct SGL_Renderer {
    SDL_Window *window;
    SDL_Renderer *sdl_renderer;
    SDL_Texture *texture;
    SGL_Scene *scene;
    int width;
    int height;
};

static void free_sdl(SGL_Renderer *renderer) {
    // Free SDL memory
    SDL_DestroyTexture(renderer->texture);
    SDL_DestroyRenderer(renderer->sdl_renderer);
    SDL_DestroyWindow(renderer->window);
    SDL_Quit();
}

static bool resize_texture(SGL_Renderer *renderer) {
    int new_width, new_height;
    SDL_GetWindowSize(renderer->window, &new_width, &new_height);
    SDL_DestroyTexture(renderer->texture);
    renderer->texture = SDL_CreateTexture(renderer->sdl_renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             new_width, new_height);
    if (!renderer->texture) {
         SDL_Log("Texture creation failed: %s\n", SDL_GetError());
         free_sdl(renderer);
         return false;
    }

    renderer->width = new_width;
    renderer->height = new_height;

    return true;
}

SGL_Renderer* SGL_CreateRenderer(const char *name, SGL_Scene *scene) {
    SDL_Init(SDL_INIT_VIDEO);

    SGL_Renderer *renderer = malloc(sizeof(SGL_Renderer));

    renderer->window = SDL_CreateWindow(
        name,
        640, 480,
        SDL_WINDOW_RESIZABLE
    );

    if (!renderer->window) {
        SDL_Log("Window creation failed: %s\n", SDL_GetError());
        free_sdl(renderer);
        return NULL;
    }

    renderer->sdl_renderer = SDL_CreateRenderer(renderer->window, NULL);

    if (!renderer->sdl_renderer) {
        SDL_Log("Renderer creation failed: %s\n", SDL_GetError());
        free_sdl(renderer);
        return NULL;
    }
    
    if (!resize_texture(renderer)) {
        free_sdl(renderer);
        return NULL;
    }

    renderer->scene = scene;

    return renderer;
}

void SGL_FreeRenderer(SGL_Renderer *renderer) {
    free(renderer);
}

static void create_projection_matrix(SGL_Renderer *renderer, SGL_Camera *camera, float out[16]) {
    float aspectRatio = renderer->width * renderer->height;

    float mat[16] = {
         SGL_Cot(camera->fov / 2) / aspectRatio, 0, 0, 0,
         0, SGL_Cot(camera->fov / 2), 0, 0,
         0, 0, -(camera->far / (camera->far - camera->near)), -1,
         0, 0, (camera->far * camera->near) / (camera->far - camera->near), 0
    };

    memcpy(out, mat, sizeof(float) * 16);
}

/**
 * Step 1 (Local space to world space): Converts OOP-like structure into 2 flat arrays (vertices and triangles) for faster computing in the pipeline.
 * Also converts vertices coordinates to world coordinates since all reference with meshes are lost after this.
 */
static void convert_scene_to_flat_arrays(SGL_List *meshes, float **out_vertices, size_t *size_vertices, float **out_triangles, size_t *size_triangles) {
    SGL_List *vertices = SGL_CreateList();
    SGL_List *triangles = SGL_CreateList();

    // Break down all meshes data into two lists
    for (size_t i = 0; i < meshes->size; i++)
    {
        SGL_Mesh *mesh = (SGL_Mesh*)SGL_ListGet(meshes, i);
        create_transformation_matrix(mesh->position, mesh->orientation, mesh->scale, mesh->transformation_matrix);

        for (size_t j = 0; j < mesh->triangles->size; j++)
        {
            SGL_Triangle *triangle = (SGL_Triangle*)SGL_ListGet(mesh->triangles, j);
            SGL_ListAdd(triangles, triangle);
        }

        for (size_t j = 0; j < mesh->vertices->size; j++) {
            SGL_Vertex *vertex = (SGL_Vertex*)SGL_ListGet(mesh->vertices, j);
            SGL_ListAdd(vertices, vertex);
        }
    }

    *size_triangles = triangles->size * TRIANGLE_ARRAY_SIZE;
    *out_triangles = malloc(sizeof(float) * (*size_triangles));

    for (size_t i = 0; i < triangles->size; i++)
    {
        SGL_Triangle *triangle = (SGL_Triangle*)SGL_ListGet(triangles, i);
        size_t triangle_index = i * TRIANGLE_ARRAY_SIZE;

        (*out_triangles)[triangle_index] = SGL_ListIndexOf(vertices, triangle->vertex1) * TRIANGLE_ARRAY_SIZE;
        (*out_triangles)[triangle_index + 1] = SGL_ListIndexOf(vertices, triangle->vertex2) * TRIANGLE_ARRAY_SIZE;
        (*out_triangles)[triangle_index + 2] = SGL_ListIndexOf(vertices, triangle->vertex3) * TRIANGLE_ARRAY_SIZE;
        (*out_triangles)[triangle_index + 3] = triangle->color.r;
        (*out_triangles)[triangle_index + 4] = triangle->color.g;
        (*out_triangles)[triangle_index + 5] = triangle->color.b;
    }

    *size_vertices = vertices->size * VERTEX_ARRAY_SIZE;
    *out_vertices = malloc(sizeof(float) * (*size_vertices));

    for (size_t i = 0; i < vertices->size; i++)
    {
        SGL_Vertex *vertex = (SGL_Vertex*)SGL_ListGet(vertices, i);
        size_t vertex_index = i * VERTEX_ARRAY_SIZE;

        // Conversion from local space to world space
        float vertex_data[] = {vertex->position.x, vertex->position.y, vertex->position.z, 1};
        multiply_matrix_with_vertex(vertex->mesh->transformation_matrix, 0, vertex_data);

        (*out_vertices)[vertex_index] = vertex_data[0];
        (*out_vertices)[vertex_index + 1] = vertex_data[1];
        (*out_vertices)[vertex_index + 2] = vertex_data[2];
        (*out_vertices)[vertex_index + 3] = vertex_data[3];
    }
}

static void cull(float vertices[], size_t size_vertices, float triangles[], size_t size_triangles, float **out_vertices, size_t *out_size_vertices, float **out_triangles, size_t *out_size_triangles) {
    // TODO
}

bool SGL_Render(SGL_Renderer *renderer, SDL_Event *event) {
    switch (event->type)
    {
        case SDL_EVENT_QUIT:
            free_sdl(renderer);
            return false;
        case SDL_EVENT_WINDOW_RESIZED:
            if (!resize_texture(renderer)) {
                return false;
            }
            break;
        default:
            break;
    }

    // TODO : Pre-processing

    void *pixels;
    int pitch;

    if (!SDL_LockTexture(renderer->texture, NULL, &pixels, &pitch)) {
        SDL_Log("Failed to lock texture: %s\n", SDL_GetError());
        free_sdl(renderer);
        return false;
    }

    // Draw manually to buffer
    uint32_t *buffer = (uint32_t *)pixels;

    for (int y = 0; y < renderer->height; y++) {
        for (int x = 0; x < renderer->width; x++) {
            // TODO : Draw triangles
        }
    }

    SDL_UnlockTexture(renderer->texture);
    SDL_RenderTexture(renderer->sdl_renderer, renderer->texture, NULL, NULL);
    SDL_RenderPresent(renderer->sdl_renderer);

    return true;
}