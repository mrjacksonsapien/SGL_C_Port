#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "SGL.h"
#include "SGL_HashMap.h"
#include <stdio.h>

const SGL_Color SGL_RED = {.r = 1.0f, .g = 0.0f, .b = 0.0f};
const SGL_Color SGL_GREEN = {.r = 0.0f, .g = 1.0f, .b = 0.0f};
const SGL_Color SGL_BLUE = {.r = 0.0f, .g = 0.0f, .b = 1.0f};

// Engine constants
static const int VERTEX_ARRAY_SIZE = 4;
static const int TRIANGLE_ARRAY_SIZE = 6;

float SGL_DegToRad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

float SGL_Cot(float degrees) {
    return 1.0f * tan(SGL_DegToRad(degrees));
}

SGL_Mesh* SGL_CreateMesh(SGL_Vertex vertices[], size_t vertices_count, SGL_Triangle triangles[], size_t triangles_count, SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale) {
    SGL_List *vertices_list = SGL_CreateListFromArray(vertices, vertices_count, sizeof(SGL_Vertex));
    SGL_List *triangles_list = SGL_CreateListFromArray(triangles, triangles_count, sizeof(SGL_Triangle));

    SGL_Mesh *mesh = malloc(sizeof(SGL_Mesh));
    mesh->vertices = vertices_list;

    for (size_t i = 0; i < vertices_list->size; i++)
    {
        SGL_Vertex *vertex = (SGL_Vertex *)vertices_list->items[i];
        vertex->mesh = mesh;
    }

    mesh->triangles = triangles_list;
    mesh->position = position;
    mesh->orientation = orientation;
    mesh->scale = scale;

    return mesh;
}

void SGL_FreeMesh(SGL_Mesh *mesh) {
    SGL_FreeList(mesh->vertices, true); // Free memory of things inside the mesh (vertices and triangles data)
    SGL_FreeList(mesh->triangles, true);
    free(mesh);
}

SGL_Mesh* SGL_CreateCubeMesh(SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale) {
    SGL_Vertex vertices[] = {
        {.position = {-1.0f, 1.0f, -1.0f}},
        {.position = {1.0f, 1.0f, -1.0f}},
        {.position = {1.0f, -1.0f, -1.0f}},
        {.position = {-1.0f, -1.0f, -1.0f}},
        {.position = {-1.0f, 1.0f, 1.0f}},
        {.position = {1.0f, 1.0f, 1.0f}},
        {.position = {1.0f, -1.0f, 1.0f}},
        {.position = {-1.0f, -1.0f, 1.0f}}
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

    return SGL_CreateMesh(vertices, vertices_count, triangles, triangles_count, position, orientation, scale);
}

SGL_Scene* SGL_CreateScene() {
    SGL_Scene* scene = malloc(sizeof(SGL_Scene));
    scene->meshes = SGL_CreateList();
    SGL_Camera *camera = malloc(sizeof(SGL_Camera));
    *camera = (SGL_Camera){
        .near = 0.1f, 
        .far = 100.0f, 
        .position = (SGL_Vector3){.x = 0.0f, .y = 0.0f, .z = 0.0f},
        .orientation = (SGL_Vector3){.x = 0.0f, .y = 0.0f, .z = 0.0f}
    };
    scene->currentCamera = camera;
    return scene;
}

void SGL_FreeScene(SGL_Scene *scene) {
    SGL_FreeList(scene->meshes, false);
    free(scene->currentCamera);
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
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, y, z, 1.0f
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_scale_matrix(float x, float y, float z, float out[16]) {
    float mat[16] = {
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_pitch_matrix(float pitch_degrees, float out[16]) {
    float pitch_radians = SGL_DegToRad(pitch_degrees);
    float c = cos(pitch_radians);
    float s = sin(pitch_radians);

    float mat[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, c, -s, 0.0f,
        0.0f, s,  c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_yaw_matrix(float yaw_degrees, float out[16]) {
    float yaw_radians = SGL_DegToRad(yaw_degrees);
    float c = cos(yaw_radians);
    float s = sin(yaw_radians);

    float mat[16] = {
         c, 0.0f, s, 0.0f,
         0.0f, 1.0f, 0, 0.0f,
        -s, 0.0f, c, 0.0f,
         0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy(out, mat, sizeof(float) * 16);
}

static void create_roll_matrix(float roll_degrees, float out[16]) {
    float roll_radians = SGL_DegToRad(roll_degrees);
    float c = cos(roll_radians);
    float s = sin(roll_radians);

    float mat[16] = {
         c, -s, 0.0f, 0.0f,
         s,  c, 0.0f, 0.0f,
         0.0f,  0.0f, 1.0f, 0.0f,
         0.0f,  0.0f, 0.0f, 1.0f
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

struct SGL_Renderer {
    SDL_Window *window;
    bool is_full_screen;
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
        640, 
        480,
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
    free_sdl(renderer);
    free(renderer);
}

void SGL_RendererSetScene(SGL_Renderer *renderer, SGL_Scene *scene) {
    renderer->scene = scene;
}

SDL_Window* SGL_RendererGetWindow(SGL_Renderer *renderer) {
    return renderer->window;
}

static void create_projection_matrix(SGL_Renderer *renderer, SGL_Camera *camera, float out[16]) {
    float aspectRatio = renderer->width * renderer->height;

    float mat[16] = {
        SGL_Cot(camera->fov / 2.0f) / aspectRatio, 0.0f, 0.0f, 0.0f,
        0.0f, SGL_Cot(camera->fov / 2.0f), 0.0f, 0.0f,
        0.0f, 0.0f, -(camera->far / (camera->far - camera->near)), -1.0f,
        0.0f, 0.0f, (camera->far * camera->near) / (camera->far - camera->near), 0.0f
    };

    memcpy(out, mat, sizeof(float) * 16);
}

/**
 * Local space-> world space: Converts OOP-like structure into 2 flat arrays (vertices and triangles) for faster computing in the pipeline.
 * Also converts vertices coordinates to world coordinates since all reference with meshes are lost after this.
 * 
 * IMPORTANT : THE ** isnt because its an array of pointers, its a pointer of a pointer of an array (so the function can place a pointer of an array inside the pointer you gave)
 * just to clear any confusion!
 * \param meshes List of meshes to convert.
 * \param out_vertices Pointer to the output array of vertices.
 * \param size_vertices Pointer to the size of the output vertices array.
 * \param out_triangles Pointer to the output array of triangles.
 * \param size_triangles Pointer to the size of the output triangles array.
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

    SGL_FreeList(vertices, false);
    SGL_FreeList(triangles, false);
}

static bool key_sizet_equals_function(void *a, void *b) {
    return *(size_t *)a == *(size_t *)b;
}

static size_t key_sizet_hash_function(void *key) {
    return *(size_t *)key;
}

static size_t add_vertex(float vertices[], SGL_HashMap *vertices_index_map, SGL_List *kept_vertices, size_t vertices_data_index) {
    void *found = SGL_HashMapGet(vertices_index_map, &vertices_data_index);

    if (found == NULL) {
        size_t next_index = kept_vertices->size;

        for (int i = 0; i < VERTEX_ARRAY_SIZE; i++) {
            float *value = malloc(sizeof(float)); // Allocate new memory because new vertices array will be generated, and older array might be destroyed
            *value = vertices[vertices_data_index + i];
            SGL_ListAdd(kept_vertices, value);
        }

        // Store the new index in the map
        size_t *index_ptr = malloc(sizeof(size_t));
        *index_ptr = next_index;
        SGL_HashMapPut(vertices_index_map, &vertices_data_index, index_ptr);

        return next_index;
    } else {
        return *(size_t *)found;
    }
}

static void get_xyz(float vertices[], size_t vertex_index, float out_xyz[3]) {
    for (size_t i = 0; i < 3; i++) {
        out_xyz[i] = vertices[vertex_index + i];
    }
}

/**
 * Removes the triangles and their vertices for those facing away from the camera (Triangle facing direction is defined by the order of the vertices in the triangle).
 * Function will free allocated memory from the previous arrays.
 */
static void cull(float vertices[], size_t size_vertices, float triangles[], size_t size_triangles, float **out_vertices, size_t *out_size_vertices, float **out_triangles, size_t *out_size_triangles) {
    SGL_HashMap *vertices_index_map = SGL_CreateHashMap(key_sizet_equals_function, key_sizet_hash_function);
    SGL_List *kept_triangles = SGL_CreateList();
    SGL_List *kept_vertices = SGL_CreateList();

    for (size_t i = 0; i < size_triangles / TRIANGLE_ARRAY_SIZE; i++)
    {
        size_t triangle_index = i * TRIANGLE_ARRAY_SIZE;

        size_t vertex1_index = triangles[triangle_index];
        size_t vertex2_index = triangles[triangle_index + 1];
        size_t vertex3_index = triangles[triangle_index + 2];

        float vertex1_xyz[3], vertex2_xyz[3], vertex3_xyz[3];
        get_xyz(vertices, vertex1_index, vertex1_xyz);
        get_xyz(vertices, vertex2_index, vertex2_xyz);
        get_xyz(vertices, vertex3_index, vertex3_xyz);

        // CCW convention
        float edge_ax = vertex2_xyz[0] - vertex1_xyz[0];
        float edge_ay = vertex2_xyz[1] - vertex1_xyz[1];
        float edge_az = vertex2_xyz[2] - vertex1_xyz[2];

        float edge_bx = vertex3_xyz[0] - vertex1_xyz[0];
        float edge_by = vertex3_xyz[1] - vertex1_xyz[1];
        float edge_bz = vertex3_xyz[2] - vertex1_xyz[2];

        float normal_x = (edge_ay * edge_bz) - (edge_az * edge_by);
        float normal_y = (edge_az * edge_bx) - (edge_ax * edge_bz);
        float normal_z = (edge_ax * edge_by) - (edge_ay * edge_bx);

        float view_direction_x = vertex1_xyz[0];
        float view_direction_y = vertex1_xyz[1];
        float view_direction_z = vertex1_xyz[2];

        float dot = (normal_x * view_direction_x) + (normal_y * view_direction_y) + (normal_z * view_direction_z);

        if (dot < 0) {
            add_vertex(vertices, vertices_index_map, kept_vertices, vertex1_index);
            add_vertex(vertices, vertices_index_map, kept_vertices, vertex2_index);
            add_vertex(vertices, vertices_index_map, kept_vertices, vertex3_index);

            float *triangles_data_456 = malloc(sizeof(float) * 3);
            triangles_data_456[0] = triangles[triangle_index + 3];
            triangles_data_456[1] = triangles[triangle_index + 4];
            triangles_data_456[2] = triangles[triangle_index + 5];

            SGL_ListAdd(kept_triangles, &triangles_data_456[0]);
            SGL_ListAdd(kept_triangles, &triangles_data_456[1]);
            SGL_ListAdd(kept_triangles, &triangles_data_456[2]);
        }
    }

    *out_size_vertices = kept_vertices->size;
    *out_size_triangles = kept_triangles->size;

    void **vertices_ptrs = SGL_ListToArray(kept_vertices);
    void **triangles_ptrs = SGL_ListToArray(kept_triangles);

    *out_vertices = malloc(sizeof(float) * (*out_size_vertices));
    for (size_t i = 0; i < *out_size_vertices; i++)
    {
        (*out_vertices)[i] = *(float*)(vertices_ptrs[i]);
    }
    free(vertices_ptrs);

    *out_triangles = malloc(sizeof(float) * (*out_size_triangles));
    for (size_t i = 0; i < *out_size_triangles; i++)
    {
        (*out_triangles)[i] = *(float*)(triangles_ptrs[i]);
    }
    free(triangles_ptrs);

    SGL_FreeHashMap(vertices_index_map);
    SGL_FreeList(kept_vertices, true);
    SGL_FreeList(kept_triangles, true);
}

static void clip(float vertices[], size_t size_vertices, float triangles[], size_t size_triangles, float **out_vertices, size_t *out_size_vertices, float **out_triangles, size_t *out_size_triangles) {
    // TODO
}

static void free_pipeline_step(float *vertices, float *triangles) {
    free(vertices);
    free(triangles);
}

static bool handle_sdl_events(SGL_Renderer *renderer, SDL_Event *event) {
    switch (event->type)
    {
        case SDL_EVENT_QUIT:
            return false;
        case SDL_EVENT_WINDOW_RESIZED:
            if (!resize_texture(renderer)) {
                return false;
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event->key.key == SDLK_F11) {
                renderer->is_full_screen = !renderer->is_full_screen;
                SDL_SetWindowFullscreen(renderer->window, renderer->is_full_screen ? SDL_WINDOW_FULLSCREEN : 0);
            }
            break;
        default:
            break;
    }

    return true;
}

static bool draw(SGL_Renderer *renderer) {
    void *pixels;
    int pitch;

    if (!SDL_LockTexture(renderer->texture, NULL, &pixels, &pitch)) {
        SDL_Log("Failed to lock texture: %s\n", SDL_GetError());
        free_sdl(renderer);
        return false;
    }

    //IMPORTANT: ARGB format, use pitch instead of SDL renderer width for getting the index of the pixel in the buffer.
    uint32_t *buffer = (uint32_t *)pixels;

    // Clear to black manually
    for (int y = 0; y < renderer->height; y++) {
        for (int x = 0; x < renderer->width; x++) {
            buffer[y * (pitch / sizeof(uint32_t)) + x] = 0xFF000000;
        }
    }

    // Render example (Red square)
    for (int y = 100; y < 200; y++) {
        for (int x = 100; x < 200; x++) {
            buffer[y * (pitch / sizeof(uint32_t)) + x] = 0xFFFF0000;
        }
    }

    SDL_UnlockTexture(renderer->texture);
    SDL_RenderTexture(renderer->sdl_renderer, renderer->texture, NULL, NULL);
    SDL_RenderPresent(renderer->sdl_renderer);

    return true;
}

bool SGL_Render(SGL_Renderer *renderer, SDL_Event *event) {
    if (!handle_sdl_events(renderer, event)) {
        return false;
    }

    if (renderer->scene->meshes->size == 0) {
        return true; // Skip pipeline
    }

    // Convert scene into flat arrays for vertices and triangles and local space -> world space
    float *vertices, *triangles;
    size_t vertices_size, triangles_size;
    convert_scene_to_flat_arrays(renderer->scene->meshes, &vertices, &vertices_size, &triangles, &triangles_size);

    // World space -> View space
    float view_matrix[16];
    create_view_matrix(renderer->scene->currentCamera, view_matrix);
    multiply_matrix_with_vertices(view_matrix, vertices, vertices_size);

    // Cull backface triangles
    float *culled_vertices, *culled_triangles;
    size_t culled_vertices_size, culled_triangles_size;
    cull(vertices, vertices_size, triangles, triangles_size, &culled_vertices, &culled_vertices_size, &culled_triangles, &culled_triangles_size);

    // Free world space data (the moment you don't need it anymore, don't wait and forget)
    free_pipeline_step(vertices, triangles);

    // View space -> Clip space
    float projection_matrix[16];
    create_projection_matrix(renderer, renderer->scene->currentCamera, projection_matrix);
    multiply_matrix_with_vertices(projection_matrix, culled_vertices, culled_vertices_size);

    // Clip triangles
    float *clipped_vertices, *clipped_triangles;
    size_t clipped_vertices_size, clipped_triangles_size;
    clip(culled_vertices, culled_vertices_size, culled_triangles, culled_triangles_size, &clipped_vertices, &clipped_vertices_size, &clipped_triangles, &clipped_triangles_size);

    // Free view space data
    free_pipeline_step(culled_vertices, culled_triangles);

    // Rasterization
    return draw(renderer);
}