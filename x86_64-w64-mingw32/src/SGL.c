#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#include "SGL.h"
#include "SGL_HashMap.h"
#include <stdio.h>
#include <float.h>

const SGL_Color SGL_RED = {.r = 1.0f, .g = 0.0f, .b = 0.0f};
const SGL_Color SGL_GREEN = {.r = 0.0f, .g = 1.0f, .b = 0.0f};
const SGL_Color SGL_BLUE = {.r = 0.0f, .g = 0.0f, .b = 1.0f};

// Engine constants
static const int VERTEX_ARRAY_SIZE = 4;
static const int TRIANGLE_ARRAY_SIZE = 6;

static const float planes_constants[6][4] = {
    {1.0f, 0.0f, 0.0f, -1.0f}, // Left
    {-1.0f, 0.0f, 0.0f, -1.0f}, // Right
    {0.0f, 1.0f, 0.0f, -1.0f}, // Bottom
    {0.0f, -1.0f, 0.0f, -1.0f}, // Top
    {0.0f, 0.0f, 1.0f, -1.0f}, // Near
    {0.0f, 0.0f, -1.0f, -1.0f} // Far
};

float SGL_DegToRad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

float SGL_Cot(float degrees) {
    return 1.0f / tan(SGL_DegToRad(degrees));
}

SGL_Mesh* SGL_CreateMesh(SGL_Vertex vertices[], float_safe_index_t vertices_count, SGL_Triangle triangles[], float_safe_index_t triangles_count, SGL_Vector3 position, SGL_Vector3 orientation, SGL_Vector3 scale) {
    SGL_List *vertices_list = SGL_CreateListFromArray(vertices, vertices_count, sizeof(SGL_Vertex));
    SGL_List *triangles_list = SGL_CreateListFromArray(triangles, triangles_count, sizeof(SGL_Triangle));

    SGL_Mesh *mesh = malloc(sizeof(SGL_Mesh));
    mesh->vertices = vertices_list;

    for (float_safe_index_t i = 0; i < vertices_list->size; i++)
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
    // Free memory of things inside the mesh (vertices and triangles data)
    SGL_FreeList(mesh->vertices, true);
    SGL_FreeList(mesh->triangles, true);
    free(mesh);
}

SGL_Mesh* SGL_CreateCubeMesh(SGL_Vector3 position) {
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

    float_safe_index_t vertices_count = sizeof(vertices) / sizeof(SGL_Vertex);
    float_safe_index_t triangles_count = sizeof(triangles) /sizeof(SGL_Triangle);

    return SGL_CreateMesh(vertices, vertices_count, triangles, triangles_count, position, (SGL_Vector3){.x = 0.0f, .y = 0.0f, .z = 0.0f}, (SGL_Vector3){.x = 0.0f, .y = 0.0f, .z = 0.0f});
}

SGL_Scene* SGL_CreateScene() {
    SGL_Scene* scene = malloc(sizeof(SGL_Scene));
    scene->meshes = SGL_CreateList();
    SGL_Camera *camera = malloc(sizeof(SGL_Camera));
    *camera = (SGL_Camera){
        .near = 0.1f, 
        .far = 100.0f, 
        .fov = 90.0f,
        .position = {0.0f, 0.0f, 0.0f},
        .orientation = {0.0f, 0.0f, 0.0f}
    };
    scene->currentCamera = camera;
    return scene;
}

void SGL_FreeScene(SGL_Scene *scene) {
    SGL_FreeList(scene->meshes, false);
    free(scene->currentCamera);
    free(scene);
}

static void multiply_matrix_with_vertex(float m[16], float_safe_index_t vertex_index, float vertices_data[]) {
    float x = vertices_data[vertex_index];
    float y = vertices_data[vertex_index + 1];
    float z = vertices_data[vertex_index + 2];
    float w = vertices_data[vertex_index + 3];

    vertices_data[vertex_index] = x * m[0] + y * m[4] + z * m[8] + w * m[12];
    vertices_data[vertex_index + 1] = x * m[1] + y * m[5] + z * m[9] + w * m[13];
    vertices_data[vertex_index + 2] = x * m[2] + y * m[6] + z * m[10] + w * m[14];
    vertices_data[vertex_index + 3] = x * m[3] + y * m[7] + z * m[11] + w * m[15];
}

static void multiply_matrix_with_vertices(float m[16], float vertices_data[], float_safe_index_t vertices_size) {
    for (float_safe_index_t i = 0; i < vertices_size / VERTEX_ARRAY_SIZE; i++)
    {
        multiply_matrix_with_vertex(m, i * VERTEX_ARRAY_SIZE, vertices_data);
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
    float aspectRatio = (float)renderer->width / (float)renderer->height;

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
 * Also converts vertices coordinates to world coordinates since all reference with meshes are lost after this. out_size is the length of the flat array,
 * NOT the amount of logical elements inside this array (aka amount of vertices/triangles)
 * 
 * IMPORTANT : THE ** isnt because its an array of pointers, its a pointer of a pointer of an array (so the function can place a pointer of an array inside the pointer you gave)
 * just to clear any confusion!
 * \param meshes List of meshes to convert.
 * \param out_vertices Pointer to the output array of vertices.
 * \param size_vertices Pointer to the size of the output vertices array.
 * \param out_triangles Pointer to the output array of triangles.
 * \param size_triangles Pointer to the size of the output triangles array.
 */
static void convert_scene_to_flat_arrays(SGL_List *meshes, float **out_vertices, float_safe_index_t *size_vertices, float **out_triangles, float_safe_index_t *size_triangles) {
    SGL_List *vertices = SGL_CreateList();
    SGL_List *triangles = SGL_CreateList();

    // Break down all meshes data into two lists
    for (float_safe_index_t i = 0; i < meshes->size; i++)
    {
        SGL_Mesh *mesh = (SGL_Mesh*)SGL_ListGet(meshes, i);
        create_transformation_matrix(mesh->position, mesh->orientation, mesh->scale, mesh->transformation_matrix);

        for (float_safe_index_t j = 0; j < mesh->triangles->size; j++)
        {
            SGL_Triangle *triangle = (SGL_Triangle*)SGL_ListGet(mesh->triangles, j);
            SGL_ListAdd(triangles, triangle);
        }

        for (float_safe_index_t j = 0; j < mesh->vertices->size; j++) {
            SGL_Vertex *vertex = (SGL_Vertex*)SGL_ListGet(mesh->vertices, j);
            SGL_ListAdd(vertices, vertex);
        }
    }

    *size_triangles = triangles->size * TRIANGLE_ARRAY_SIZE;
    *out_triangles = malloc(sizeof(float) * (*size_triangles));

    for (float_safe_index_t i = 0; i < triangles->size; i++)
    {
        SGL_Triangle *triangle = (SGL_Triangle*)SGL_ListGet(triangles, i);
        float_safe_index_t triangle_index = i * TRIANGLE_ARRAY_SIZE;

        (*out_triangles)[triangle_index] = SGL_ListIndexOf(vertices, triangle->vertex1) * TRIANGLE_ARRAY_SIZE;
        (*out_triangles)[triangle_index + 1] = SGL_ListIndexOf(vertices, triangle->vertex2) * TRIANGLE_ARRAY_SIZE;
        (*out_triangles)[triangle_index + 2] = SGL_ListIndexOf(vertices, triangle->vertex3) * TRIANGLE_ARRAY_SIZE;
        (*out_triangles)[triangle_index + 3] = triangle->color.r;
        (*out_triangles)[triangle_index + 4] = triangle->color.g;
        (*out_triangles)[triangle_index + 5] = triangle->color.b;
    }

    *size_vertices = vertices->size * VERTEX_ARRAY_SIZE;
    *out_vertices = malloc(sizeof(float) * (*size_vertices));

    for (float_safe_index_t i = 0; i < vertices->size; i++)
    {
        SGL_Vertex *vertex = (SGL_Vertex*)SGL_ListGet(vertices, i);
        float_safe_index_t vertex_index = i * VERTEX_ARRAY_SIZE;

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
    return *(float_safe_index_t *)a == *(float_safe_index_t *)b;
}

static float_safe_index_t key_sizet_hash_function(void *key) {
    return *(float_safe_index_t *)key;
}

static float_safe_index_t add_vertex(SGL_List *vertices, SGL_HashMap *vertices_index_map, SGL_List *kept_vertices, float_safe_index_t vertices_data_index) {
    void *found = SGL_HashMapGet(vertices_index_map, &vertices_data_index);

    if (found == NULL) {
        float_safe_index_t next_index = kept_vertices->size;

        for (int i = 0; i < VERTEX_ARRAY_SIZE; i++) {
            float *value = malloc(sizeof(float)); // Allocate new memory because new vertices array will be generated, and older array might be destroyed
            *value = *(float*)SGL_ListGet(vertices, vertices_data_index + i);
            SGL_ListAdd(kept_vertices, value);
        }

        // Store the new index in the map
        float_safe_index_t *index_ptr = malloc(sizeof(float_safe_index_t));
        *index_ptr = next_index;
        SGL_HashMapPut(vertices_index_map, &vertices_data_index, index_ptr);

        return next_index;
    } else {
        return *(float_safe_index_t *)found;
    }
}

static float_safe_index_t create_vertex(SGL_List *next_vertices, float vertex[VERTEX_ARRAY_SIZE]) {
    float_safe_index_t next_index = next_vertices->size;
    float *vertex_data = malloc(sizeof(float) * VERTEX_ARRAY_SIZE);

    for (float_safe_index_t i = 0; i < VERTEX_ARRAY_SIZE; i++)
    {
        vertex_data[i] = vertex[i];
        SGL_ListAdd(next_vertices, &vertex_data[i]);
    }

    return next_index;
}

static void get_xyz(float vertices[], float_safe_index_t vertex_index, float out_xyz[3]) {
    for (float_safe_index_t i = 0; i < 3; i++) {
        out_xyz[i] = vertices[vertex_index + i];
    }
}

/**
 * Removes the triangles and their vertices for those facing away from the camera (Triangle facing direction is defined by the order of the vertices in the triangle).
 */
static void cull(float vertices[], float_safe_index_t size_vertices, float triangles[], float_safe_index_t size_triangles, float **out_vertices, float_safe_index_t *out_size_vertices, float **out_triangles, float_safe_index_t *out_size_triangles) {
    SGL_HashMap *vertices_index_map = SGL_CreateHashMap(key_sizet_equals_function, key_sizet_hash_function);
    SGL_List *kept_triangles = SGL_CreateList();
    SGL_List *kept_vertices = SGL_CreateList();

    SGL_List *vertices_list = SGL_CreateListFromArray(vertices, size_vertices, sizeof(float));

    for (float_safe_index_t i = 0; i < size_triangles / TRIANGLE_ARRAY_SIZE; i++)
    {
        float_safe_index_t triangle_index = i * TRIANGLE_ARRAY_SIZE;

        float_safe_index_t vertex1_index = triangles[triangle_index];
        float_safe_index_t vertex2_index = triangles[triangle_index + 1];
        float_safe_index_t vertex3_index = triangles[triangle_index + 2];

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
            add_vertex(vertices_list, vertices_index_map, kept_vertices, vertex1_index);
            add_vertex(vertices_list, vertices_index_map, kept_vertices, vertex2_index);
            add_vertex(vertices_list, vertices_index_map, kept_vertices, vertex3_index);

            // Allocate each float individually for triangle color data
            float *f3 = malloc(sizeof(float));
            float *f4 = malloc(sizeof(float));
            float *f5 = malloc(sizeof(float));
            *f3 = triangles[triangle_index + 3];
            *f4 = triangles[triangle_index + 4];
            *f5 = triangles[triangle_index + 5];
            SGL_ListAdd(kept_triangles, f3);
            SGL_ListAdd(kept_triangles, f4);
            SGL_ListAdd(kept_triangles, f5);
        }
    }

    SGL_FreeList(vertices_list, true);

    *out_size_vertices = kept_vertices->size;
    *out_size_triangles = kept_triangles->size;

    void **vertices_ptrs = SGL_ListToArray(kept_vertices);
    void **triangles_ptrs = SGL_ListToArray(kept_triangles);

    *out_vertices = malloc(sizeof(float) * (*out_size_vertices));
    for (float_safe_index_t i = 0; i < *out_size_vertices; i++)
    {
        (*out_vertices)[i] = *(float*)(vertices_ptrs[i]);
    }
    free(vertices_ptrs);

    *out_triangles = malloc(sizeof(float) * (*out_size_triangles));
    for (float_safe_index_t i = 0; i < *out_size_triangles; i++)
    {
        (*out_triangles)[i] = *(float*)(triangles_ptrs[i]);
    }
    free(triangles_ptrs);

    SGL_FreeHashMap(vertices_index_map, true);
    SGL_FreeList(kept_vertices, true);
    SGL_FreeList(kept_triangles, true);
}

static bool planes_relation(SGL_List *active_vertices, float_safe_index_t vertex_index, int i) {
    float w = *(float*)SGL_ListGet(active_vertices, vertex_index + 3);

    switch (i) {
        case 0: // Left
            return *(float*)SGL_ListGet(active_vertices, vertex_index) >= w;
        case 1: // Right
            return *(float*)SGL_ListGet(active_vertices, vertex_index) <= -w;
        case 2: // Top
            return *(float*)SGL_ListGet(active_vertices, vertex_index + 1) >= w;
        case 3: // Bottom
            return *(float*)SGL_ListGet(active_vertices, vertex_index + 1) <= -w;
        case 4: // Near
            return *(float*)SGL_ListGet(active_vertices, vertex_index + 2) >= w;
        case 5: // Far
            return *(float*)SGL_ListGet(active_vertices, vertex_index + 2) <= -w;
        default:
            return false;
    }
}

static bool get_intersection(SGL_List *active_vertices, float p1_index, float p2_index, int plane_index, float out_intersection[4]) {
    float plane[4];
    memcpy(plane, planes_constants[plane_index], sizeof(float) * 4);

    float x1 = *(float*)SGL_ListGet(active_vertices, p1_index);
    float y1 = *(float*)SGL_ListGet(active_vertices, p1_index + 1);
    float z1 = *(float*)SGL_ListGet(active_vertices, p1_index + 2);
    float w1 = *(float*)SGL_ListGet(active_vertices, p1_index + 3);

    float x2 = *(float*)SGL_ListGet(active_vertices, p2_index);
    float y2 = *(float*)SGL_ListGet(active_vertices, p2_index + 1);
    float z2 = *(float*)SGL_ListGet(active_vertices, p2_index + 2);
    float w2 = *(float*)SGL_ListGet(active_vertices, p2_index + 3);

    float numerator = -(plane[0] * x1 + plane[1] * y1 + plane[2] * z1 + plane[3] * w1);
    float denominator = (plane[0] * x2 + plane[1] * y2 + plane[2] * z2 + plane[3] * w2) + numerator;

    if (abs(denominator) < 1e-6f) {
        return false; // Lines are parallel or coincident
    }

    float t = numerator / denominator;

    out_intersection[0] = x1 + t * (x2 - x1);
    out_intersection[1] = y1 + t * (y2 - y1);
    out_intersection[2] = z1 + t * (z2 - z1);
    out_intersection[3] = w1 + t * (w2 - w1);

    return true;
}

static void create_new_triangle(SGL_List *active_triangles, SGL_List *next_triangles, float v1_index, float v2_index, float v3_index, float_safe_index_t og_tri_index) {
    // Allocate and add each float individually
    float *f0 = malloc(sizeof(float));
    float *f1 = malloc(sizeof(float));
    float *f2 = malloc(sizeof(float));
    float *f3 = malloc(sizeof(float));
    float *f4 = malloc(sizeof(float));
    float *f5 = malloc(sizeof(float));

    *f0 = v1_index;
    *f1 = v2_index;
    *f2 = v3_index;
    *f3 = *(float*)SGL_ListGet(active_triangles, og_tri_index + 3);
    *f4 = *(float*)SGL_ListGet(active_triangles, og_tri_index + 4);
    *f5 = *(float*)SGL_ListGet(active_triangles, og_tri_index + 5);

    SGL_ListAdd(next_triangles, f0);
    SGL_ListAdd(next_triangles, f1);
    SGL_ListAdd(next_triangles, f2);
    SGL_ListAdd(next_triangles, f3);
    SGL_ListAdd(next_triangles, f4);
    SGL_ListAdd(next_triangles, f5);
}

static void clip(float vertices[], float_safe_index_t size_vertices, float triangles[], float_safe_index_t size_triangles, float **out_vertices, float_safe_index_t *out_size_vertices, float **out_triangles, float_safe_index_t *out_size_triangles) {
    SGL_List *active_triangles = SGL_CreateListFromArray(triangles, size_triangles, sizeof(float));
    SGL_List *active_vertices = SGL_CreateListFromArray(vertices, size_vertices, sizeof(float));

    SGL_List *next_triangles = SGL_CreateList();
    SGL_List *next_vertices = SGL_CreateList();

    // Iterate over each plane
    for (float_safe_index_t i = 0; i < 6; i++)
    {
        SGL_HashMap *vertices_index_map = SGL_CreateHashMap(key_sizet_equals_function, key_sizet_hash_function);

        for (float_safe_index_t j = 0; j < active_triangles->size / TRIANGLE_ARRAY_SIZE; j++)
        {
            float_safe_index_t triangle_index = j * TRIANGLE_ARRAY_SIZE;
            float v1_index = *(float*)SGL_ListGet(active_triangles, triangle_index);
            float v2_index = *(float*)SGL_ListGet(active_triangles, triangle_index + 1);
            float v3_index = *(float*)SGL_ListGet(active_triangles, triangle_index + 2);

            SGL_List *inside = SGL_CreateList();
            SGL_List *outside = SGL_CreateList();

            if (planes_relation(active_vertices, v1_index, i)) {
                SGL_ListAdd(inside, &v1_index);
            } else {
                SGL_ListAdd(outside, &v1_index);
            }

            if (planes_relation(active_vertices, v2_index, i)) {
                SGL_ListAdd(inside, &v2_index);
            } else {
                SGL_ListAdd(outside, &v2_index);
            }

            if (planes_relation(active_vertices, v3_index, i)) {
                SGL_ListAdd(inside, &v3_index);
            } else {
                SGL_ListAdd(outside, &v3_index);
            }

            if (inside->size == 3) {
                create_new_triangle(
                    active_triangles, 
                    next_triangles, 
                    (float)add_vertex(active_vertices, vertices_index_map, next_vertices, *(float*)SGL_ListGet(inside, 0)),
                    (float)add_vertex(active_vertices, vertices_index_map, next_vertices, *(float*)SGL_ListGet(inside, 1)),
                    (float)add_vertex(active_vertices, vertices_index_map, next_vertices, *(float*)SGL_ListGet(inside, 2)),
                    triangle_index
                );
            } else if (inside->size == 2) {
                float intersection1[4];
                float intersection2[4];

                get_intersection(active_vertices, *(float*)SGL_ListGet(inside, 0), *(float*)SGL_ListGet(outside, 0), i, intersection1);
                get_intersection(active_vertices, *(float*)SGL_ListGet(inside, 1), *(float*)SGL_ListGet(outside, 0), i, intersection2);

                create_new_triangle(
                    active_triangles, 
                    next_triangles, 
                    (float)add_vertex(active_vertices, vertices_index_map, next_vertices, *(float*)SGL_ListGet(inside, 0)), 
                    (float)add_vertex(active_vertices, vertices_index_map, next_vertices, *(float*)SGL_ListGet(inside, 1)), 
                    (float)create_vertex(next_vertices, intersection1), 
                    triangle_index
                );

                create_new_triangle(
                    active_triangles,
                    next_triangles,
                    (float)add_vertex(active_vertices, vertices_index_map, next_vertices, *(float*)SGL_ListGet(inside, 1)),
                    (float)create_vertex(next_vertices, intersection2),
                    (float)create_vertex(next_vertices, intersection1),
                    triangle_index
                );
            } else if (inside->size == 1) {
                float intersection1[4];
                float intersection2[4];

                get_intersection(active_vertices, *(float*)SGL_ListGet(inside, 0), *(float*)SGL_ListGet(outside, 0), i, intersection1);
                get_intersection(active_vertices, *(float*)SGL_ListGet(inside, 0), *(float*)SGL_ListGet(outside, 1), i, intersection2);

                create_new_triangle(
                    active_triangles,
                    next_triangles,
                    (float)add_vertex(active_vertices, vertices_index_map, next_vertices, *(float*)SGL_ListGet(inside, 0)),
                    (float)create_vertex(next_vertices, intersection2),
                    (float)create_vertex(next_vertices, intersection1),
                    triangle_index
                );
            }
        }

        SGL_FreeHashMap(vertices_index_map, true);
        SGL_FreeList(active_triangles, true);
        active_triangles = next_triangles;
        SGL_FreeList(active_vertices, true);
        active_vertices = next_vertices;
        next_triangles = SGL_CreateList();
        next_vertices = SGL_CreateList();
    }

    *out_size_vertices = active_vertices->size;
    *out_size_triangles = active_triangles->size;

    void **vertices_ptrs = SGL_ListToArray(active_vertices);
    void **triangles_ptrs = SGL_ListToArray(active_triangles);

    *out_vertices = malloc(sizeof(float) * (*out_size_vertices));
    for (float_safe_index_t i = 0; i < *out_size_vertices; i++)
    {
        (*out_vertices)[i] = *(float*)(vertices_ptrs[i]);
    }
    free(vertices_ptrs);

    *out_triangles = malloc(sizeof(float) * (*out_size_triangles));
    for (float_safe_index_t i = 0; i < *out_size_triangles; i++)
    {
        (*out_triangles)[i] = *(float*)(triangles_ptrs[i]);
    }
    free(triangles_ptrs);

    SGL_FreeList(active_triangles, true);
    SGL_FreeList(active_vertices, true);
    SGL_FreeList(next_triangles, true);
    SGL_FreeList(next_vertices, true);
}

/**
 * Used at every step because the vertices and triangles are dynamically allocated.
 */
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

static void apply_perspective_division_clip_vertices(float vertices[], float_safe_index_t vertices_size) {
    for (float_safe_index_t i = 0; i < vertices_size / VERTEX_ARRAY_SIZE; i++)
    {
        float_safe_index_t vertex_index = i * VERTEX_ARRAY_SIZE;
        float w = vertices[vertex_index + 3];

        vertices[vertex_index] /= w;
        vertices[vertex_index + 1] /= w;
        vertices[vertex_index + 2] /= w;
        vertices[vertex_index + 3] /= w;
    }
}

static void map_ndc_vertices_to_screen_coordinates(SGL_Renderer *renderer, float vertices[], float_safe_index_t vertices_size) {
    for (float_safe_index_t i = 0; i < vertices_size / VERTEX_ARRAY_SIZE; i++)
    {
        float_safe_index_t vertex_index = i * VERTEX_ARRAY_SIZE;

        vertices[vertex_index] = (vertices[vertex_index] + 1) / 2 * renderer->width;
        vertices[vertex_index + 1] = (1 - vertices[vertex_index + 1]) / 2 * renderer->height;
    }
}

static bool point_is_in_triangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy, bool is_ccw) {
    float edge_ab = (py - ay) * (bx - ax) - (px - ax) * (by - ay);
    float edge_bc = (py - by) * (cx - bx) - (px - bx) * (cy - by);
    float edge_ca = (py - cy) * (ax - cx) - (px - cx) * (ay - cy);

    if (!is_ccw) {
        edge_ab *= -1;
        edge_bc *= -1;
        edge_ca *= -1;
    }

    return (edge_ab >= 0) && (edge_bc >= 0) && (edge_ca >= 0);
}

static bool is_triangle_ccw(float ax, float ay, float bx, float by, float cx, float cy) {
    float signed_area = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    return signed_area > 0;
}

static bool render_triangles(SGL_Renderer *renderer, float vertices[], float_safe_index_t vertices_size, float triangles[], float_safe_index_t triangles_size) {
    void *pixels;
    int pitch;

    if (!SDL_LockTexture(renderer->texture, NULL, &pixels, &pitch)) {
        SDL_Log("Failed to lock texture: %s\n", SDL_GetError());
        free_sdl(renderer);
        return false;
    }

    // Use example

    //IMPORTANT: ARGB format, use pitch instead of SDL renderer width for getting the index of the pixel in the buffer.
    uint32_t *buffer = (uint32_t *)pixels;

    // Clear to black manually
    for (int y = 0; y < renderer->height; y++) {
        for (int x = 0; x < renderer->width; x++) {
            buffer[y * (pitch / sizeof(uint32_t)) + x] = 0xFF000000;
        }
    }

    int depth_buffer_size = renderer->width * renderer->height;
    float depth_buffer[depth_buffer_size];

    for (int i = 0; i < depth_buffer_size; i++) {
        depth_buffer[i] = FLT_MAX;
    }

    for (float_safe_index_t i = 0; i < triangles_size / TRIANGLE_ARRAY_SIZE; i++) {
        float_safe_index_t triangle_index = i * TRIANGLE_ARRAY_SIZE;

        int v1_index = (int)triangles[triangle_index];
        int v2_index = (int)triangles[triangle_index + 1];
        int v3_index = (int)triangles[triangle_index + 2];

        float v1_x = vertices[v1_index];
        float v1_y = vertices[v1_index + 1];

        float v2_x = vertices[v2_index];
        float v2_y = vertices[v2_index + 1];

        float v3_x = vertices[v3_index];
        float v3_y = vertices[v3_index + 1];

        bool is_ccw = is_triangle_ccw(v1_x, v1_y, v2_x, v2_y, v3_x, v3_y);

        float min_x = floor(MIN(MIN(v1_x, v2_x), v3_x));
        float max_x = floor(MAX(MAX(v1_x, v2_x), v3_x));
        float min_y = floor(MIN(MIN(v1_y, v2_y), v3_y));
        float max_y = floor(MAX(MAX(v1_y, v2_y), v3_y));

        for (float y = min_y; y < max_y; y++) {
            for (float x = min_x; x < max_x; x++) {
                if (point_is_in_triangle(x, y, v1_x, v1_y, v2_x, v2_y, v3_x, v3_y, is_ccw)) {
                    float v1_z = vertices[v1_index + 2];
                    float v2_z = vertices[v2_index + 2];
                    float v3_z = vertices[v3_index + 2];

                    // Computer barycentric coordinates (a1, a2, a3)
                    float denominator = (v2_y - v3_y) * (v1_x - v3_x) + (v3_x - v2_x) * (v1_y - v3_y);

                    float a1 = ((v2_y - v3_y) * (x - v3_x) + (v3_x - v2_x) * (y - v3_y)) / denominator;
                    float a2 = ((v3_y - v1_y) * (x - v3_x) + (v1_x - v3_x) * (y - v3_y)) / denominator;
                    float a3 = 1 - a1 - a2;

                    // Interpolate z value using barycentric coordinates
                    float z = a1 * v1_z + a2 * v2_z + a3 * v3_z;

                    int depth_index = (y * renderer->width + x);
                    if (z < depth_buffer[depth_index]) {
                        depth_buffer[depth_index] = z;

                        int pixel_index = (y * (pitch / sizeof(uint32_t)) + x);

                        /*
                        uint8_t r = (uint8_t)triangles[triangle_index + 3];
                        uint8_t g = (uint8_t)triangles[triangle_index + 4];
                        uint8_t b = (uint8_t)triangles[triangle_index + 5];

                        uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
                        */

                        buffer[pixel_index] = 0xFFFFFF;
                    }
                }
            }
        }
    }

    SDL_UnlockTexture(renderer->texture);
    SDL_RenderTexture(renderer->sdl_renderer, renderer->texture, NULL, NULL);
    SDL_RenderPresent(renderer->sdl_renderer);

    return true;
}

static void stage_debug_print(char *space, float_safe_index_t vertices_size, float_safe_index_t triangles_size, float *vertices, float *triangles) {
    if (vertices_size != 0 || triangles_size != 0) {
        printf("(%s) Vertices: %" PRIu32 ", Triangles: %" PRIu32 "\n\n", space, vertices_size / VERTEX_ARRAY_SIZE, triangles_size / TRIANGLE_ARRAY_SIZE);

        printf("Vertices: \n\n");
        for (float_safe_index_t i = 0; i < vertices_size / VERTEX_ARRAY_SIZE; i++)
        {
            float_safe_index_t vertex_index = i * VERTEX_ARRAY_SIZE;
            printf("%" PRIu32 " - x: %f, y: %f, z: %f, w: %f\n", i, vertices[vertex_index], vertices[vertex_index + 1], vertices[vertex_index + 2], vertices[vertex_index + 3]);
        }

        printf("\n\n");
    }
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
    float_safe_index_t vertices_size, triangles_size;
    convert_scene_to_flat_arrays(renderer->scene->meshes, &vertices, &vertices_size, &triangles, &triangles_size);

    // World space -> View space
    float view_matrix[16];
    create_view_matrix(renderer->scene->currentCamera, view_matrix);
    multiply_matrix_with_vertices(view_matrix, vertices, vertices_size);

    // TODO : Fix culling
    // Cull backface triangles
    float *culled_vertices, *culled_triangles;
    float_safe_index_t culled_vertices_size, culled_triangles_size;
    cull(vertices, vertices_size, triangles, triangles_size, &culled_vertices, &culled_vertices_size, &culled_triangles, &culled_triangles_size);

    stage_debug_print("View space: After culling", culled_vertices_size, culled_triangles_size, culled_vertices, culled_triangles);

    // Free world space data (the moment you don't need it anymore, don't wait and forget)
    free_pipeline_step(vertices, triangles);

    // View space -> Clip space
    float projection_matrix[16];
    create_projection_matrix(renderer, renderer->scene->currentCamera, projection_matrix);
    multiply_matrix_with_vertices(projection_matrix, culled_vertices, culled_vertices_size);

    // Clip triangles
    float *clipped_vertices, *clipped_triangles;
    float_safe_index_t clipped_vertices_size, clipped_triangles_size;
    clip(culled_vertices, culled_vertices_size, culled_triangles, culled_triangles_size, &clipped_vertices, &clipped_vertices_size, &clipped_triangles, &clipped_triangles_size);

    // Free view space data
    free_pipeline_step(culled_vertices, culled_triangles);

    // Clip space -> NDC space
    apply_perspective_division_clip_vertices(clipped_vertices, clipped_vertices_size);

    // NDC space -> Screen space
    map_ndc_vertices_to_screen_coordinates(renderer, clipped_vertices, clipped_vertices_size);

    // Rasterization
    render_triangles(renderer, clipped_vertices, clipped_vertices_size, clipped_triangles, clipped_triangles_size);

    // Free screen space data
    free_pipeline_step(clipped_vertices, clipped_triangles);

    return true;
}