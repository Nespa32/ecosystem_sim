
#ifndef __WORLD_H
#define __WORLD_H

#include <stdio.h>

enum
{
    OBJECT_TYPE_NONE = 0,
    OBJECT_TYPE_ROCK,
    OBJECT_TYPE_RABBIT,
    OBJECT_TYPE_FOX,
};

// bit of a hack, since we can't have unsigned enums
typedef unsigned char ObjectType;

struct WorldObject
{
    // object type
    ObjectType      type : 2;
    // generations since the object last ate, only used for OBJECT_TYPE_FOX
    unsigned char   last_ate : 6;
    // generation counter for procreation
    unsigned char   gen_proc : 8;
};

typedef struct WorldObject WorldObject;

struct World
{
    // world configs
    int gen_proc_rabbits;
    int gen_proc_foxes;
    int gen_food_foxes;
    int n_gen;
    int n_rows;
    int n_cols;

    size_t grid_size;
    // grid ptr, size n_rows * n_cols
    WorldObject* grid;
    // helper used to compute next grid, also size n_rows * n_cols
    WorldObject* next_grid;
};

typedef struct World World;

World* World_New(int gen_proc_rabbits, int gen_proc_foxes, int gen_food_foxes,
    int n_gen, int n_rows, int n_cols);
void World_Delete(World* world);
int World_CoordsToIdx(World const* world, int x, int y);
void World_SetObjectType(World* world, int idx, ObjectType obj_type);
void World_SetObject(World* world, int idx, WorldObject* obj);
WorldObject* World_GetObject(World const* world, int idx);
ObjectType World_GetObjectType(World const* world, int idx);
void World_Print(World const* world);
void World_PrettyPrint(World const* world);
int World_Compare(World const* left, World const* right);

inline World* World_New(int gen_proc_rabbits, int gen_proc_foxes, int gen_food_foxes,
    int n_gen, int n_rows, int n_cols)
{
    size_t grid_size = n_rows * n_cols * sizeof(WorldObject);
    // do a single malloc
    // this only works because WorldObject elements are 1-byte aligned
    size_t world_size = sizeof(World) +     // World size
        grid_size +                         // World.grid size
        grid_size;                          // World.next_grid size

    char* m = malloc(world_size);
    World* world = (World*)m;
    world->gen_proc_rabbits = gen_proc_rabbits;
    world->gen_proc_foxes = gen_proc_foxes;
    world->gen_food_foxes = gen_food_foxes;
    world->n_gen = n_gen;
    world->n_rows = n_rows;
    world->n_cols = n_cols;
    world->grid_size = grid_size;
    world->grid = (WorldObject*)(m + sizeof(World));
    world->next_grid = (WorldObject*)(m + sizeof(World) + grid_size);

    memset(world->grid, 0x0, world->grid_size);
    memset(world->next_grid, 0x0, world->grid_size);
    return world;
}

inline void World_Delete(World* world)
{
    // free the single malloc
    free(world);
}

inline int World_CoordsToIdx(World const* world, int x, int y)
{
    return x * world->n_rows + y;
}

inline void World_SetObjectType(World* world, int idx, ObjectType obj_type)
{
    WorldObject* obj = World_GetObject(world, idx);
    obj->type = obj_type;
}

inline void World_SetObject(World* world, int idx, WorldObject* obj)
{
    WorldObject* local_obj = World_GetObject(world, idx);
    (*local_obj) = (*obj);
}

inline WorldObject* World_GetObject(World const* world, int idx)
{
    return &world->grid[idx];
}

inline ObjectType World_GetObjectType(World const* world, int idx)
{
    WorldObject* obj = World_GetObject(world, idx);
    return obj->type;
}

inline void World_Print(World const* world)
{
    int n_objs = 0;
    for (int x = 0; x < world->n_rows; ++x)
    {
        for (int y = 0; y < world->n_cols; ++y)
        {
            int idx = World_CoordsToIdx(world, x, y);
            if (World_GetObjectType(world, idx) != OBJECT_TYPE_NONE)
                ++n_objs;
        }
    }

    printf("%d %d %d %d %d %d %d\n",
        world->gen_proc_rabbits, world->gen_proc_foxes,
        world->gen_food_foxes, world->n_gen, world->n_rows,
        world->n_cols, n_objs);

    for (int x = 0; x < world->n_rows; ++x)
    {
        for (int y = 0; y < world->n_cols; ++y)
        {
            int idx = World_CoordsToIdx(world, x, y);
            ObjectType obj_type = World_GetObjectType(world, idx);
            if (obj_type == OBJECT_TYPE_NONE)
                continue;

            const char* obj_name = "ROCK";
            if (obj_type == OBJECT_TYPE_RABBIT)
                obj_name = "RABBIT";
            else if (obj_type == OBJECT_TYPE_FOX)
                obj_name = "FOX";

            printf("%s %d %d\n", obj_name, x, y);
        }
    }
}

inline void World_PrettyPrint(World const* world)
{
    // print leading '====='
    for (int i = 0; i < world->n_cols + 2; ++i)
        printf("-");

    printf("\n");

    for (int x = 0; x < world->n_rows; ++x)
    {
        printf("|");

        for (int y = 0; y < world->n_cols; ++y)
        {
            int idx = World_CoordsToIdx(world, x, y);
            ObjectType obj_type = World_GetObjectType(world, idx);

            switch (obj_type)
            {
                default:
                case OBJECT_TYPE_NONE:
                    printf(" ");
                    break;
                case OBJECT_TYPE_ROCK:
                    printf("*");
                    break;
                case OBJECT_TYPE_RABBIT:
                    printf("R");
                    break;
                case OBJECT_TYPE_FOX:
                    printf("F");
                    break;
            }
        }

        printf("|\n");
    }

    // print trailing '====='
    for (int i = 0; i < world->n_cols + 2; ++i)
        printf("-");

    printf("\n");
}

inline int World_Compare(World const* left, World const* right)
{
    if (left->gen_proc_rabbits != right->gen_proc_rabbits ||
        left->gen_proc_foxes != right->gen_proc_foxes ||
        left->gen_food_foxes != right->gen_food_foxes ||
        left->n_gen != right->n_gen ||
        left->n_rows != right->n_rows ||
        left->n_cols != right->n_cols)
        return 1;

    if ((left->grid == NULL) != (right->grid == NULL))
        return 1;

    if (left->grid == NULL)
        return 1;

    for (int x = 0; x < left->n_rows; ++x)
    {
        for (int y = 0; y < left->n_cols; ++y)
        {
            int idx = World_CoordsToIdx(left, x, y);
            WorldObject const* left_obj = World_GetObject(left, idx);
            WorldObject const* right_obj = World_GetObject(right, idx);
            if (left_obj->type != right_obj->type)
                return 1;
        }
    }

    return 0;
}

#endif // __WORLD_H
