#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Rabbit Rules */
/*
    Rabbits can move N/E/W/S, but not diagonally.
    In each gen, rabbits attempt to move themselves to an empty adjacent cell. If
     there are many empty adjacent cells, they choose one accordingly to a
     common criteria for selecting adjacent cells (see below). If there is no
     empty adjacent cell, they stay in the same place.
    Rabbits can procreate whenever GEN_PROC_RABBITS generations have passed since
     they were born or since they last procreated. Whenever a rabbit reaches such
     age (to procreate) and makes a move, it leaves in its last position a new rabbit
     and both rabbits' procreation age is set to zero.
*/

/* Fox Rules */
/*
    Foxes can move horizontally or vertically, but not diagonally.
    In each generation, foxes try to eat a rabbit by moving to an adjacent cell
     that is occupied by a rabbit. If there are many adjacent cells occupied with
     rabbits, they choose one accordingly to the common criteria for selecting
     adjacent cells (see below). If there is no adjacent cell occupied by a rabbit,
     foxes attempt to move themselves to an empty adjacent cell by following the
     same criteria. If there is no adjacent cell occupied with a rabbit or empty,
     they stay in the same place.
    Foxes starve and die whenever GEN_FOOD_FOXES generations have passed since
     they were born or since they last ate a rabbit. Foxes die after not finding a
     rabbit to eat and before attempting to move themselves to an empty adjacent cell.
    Foxes can procreate whenever GEN_PROC_FOXES generations have passed since they
     were born or since they last procreated. Whenever a fox reaches such age
     (to procreate) and makes a move, it leaves in its last position a new fox and
     both foxes' procreation age is set to zero.
*/

/* Rock Rules */
/*
    Rocks do not move and no animal can occupy its space.
*/

/* Criteria for Selecting Adjacent Cells */
/*
    By following the clockwise order, start numbering from 0 the possible P cells
     to where a fox/rabbit can move (adjacent N/E/S/W cells).
    Let G represent the current generation and (X,Y) represent the cell coordinates
     where a fox/rabbit is positioned in the ecosystem space, then the adjacent
     cell to be chosen is determined by (G+X+Y) mod P. Assume that the initial
     generation is number 0 and that the world origin is (0,0).
*/

enum ObjectType
{
    OBJECT_TYPE_NONE = 0,
    OBJECT_TYPE_ROCK,
    OBJECT_TYPE_RABBIT,
    OBJECT_TYPE_FOX,
};

typedef enum ObjectType ObjectType;

struct WorldObject
{
    // object type
    ObjectType      type;
    // generations since the object last ate, only used for OBJECT_TYPE_FOX
    unsigned int    last_ate;
    // generation counter for procreation
    unsigned int    gen_proc;
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

    // grid ptr, size n_rows] * n_cols
    WorldObject* grid;
};

typedef struct World World;

int World_CoordsToIdx(World const* world, int x, int y);
void World_SetObjectType(World* world, int idx, ObjectType obj_type);
void World_SetObject(World* world, int idx, WorldObject* obj);
WorldObject* World_GetObject(World const* world, int idx);
ObjectType World_GetObjectType(World const* world, int idx);
void World_Print(World const* world);
void World_PrettyPrint(World const* world);

int World_CoordsToIdx(World const* world, int x, int y)
{
    return x * world->n_rows + y;
}

void World_SetObjectType(World* world, int idx, ObjectType obj_type)
{
    WorldObject* obj = World_GetObject(world, idx);
    obj->type = obj_type;
}

void World_SetObject(World* world, int idx, WorldObject* obj)
{
    WorldObject* local_obj = World_GetObject(world, idx);
    (*local_obj) = (*obj);
}

WorldObject* World_GetObject(World const* world, int idx)
{
    return &world->grid[idx];
}

ObjectType World_GetObjectType(World const* world, int idx)
{
    WorldObject* obj = World_GetObject(world, idx);
    return obj->type;
}

void World_Print(World const* world)
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

void World_PrettyPrint(World const* world)
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

void print_usage();
int read_world_from_file(const char* file_str, World* world);

int choose_move(World const* world, int gen, WorldObject const* obj,
    int x, int y, ObjectType target_type)
{
    static int const directions[4][2] = {
        // north      east      south      west
        { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 }
    };

    int viable_mask = 0;
    for (int i = 0; i < 4; ++i)
    {
        int const coord_x = x + directions[i][0];
        int const coord_y = y + directions[i][1];
        if (coord_x < 0 || coord_x >= world->n_rows ||
            coord_y < 0 || coord_y >= world->n_cols)
            continue;

        int idx = World_CoordsToIdx(world, coord_x, coord_y);
        WorldObject const* local_obj = World_GetObject(world, idx);
        if (local_obj->type == target_type)
            viable_mask |= 1 << i;
    }

    if (!viable_mask)
        return -1;

    int const p = __builtin_popcount(viable_mask);
    int path_choice = (gen + x + y) % p;

    // from viable_mask and path_choice we can use a lookup table
    //  to determine which path to go, instead of iteratively figuring it out
    static int const lookuptable[16][4] = {
        { -1, -1, -1, -1 }, // 0
        {  0, -1, -1, -1 }, // 1
        {  1, -1, -1, -1 }, // 2
        {  0,  1, -1, -1 }, // 3
        {  2, -1, -1, -1 }, // 4
        {  0,  2, -1, -1 }, // 5
        {  1,  2, -1, -1 }, // 6
        {  0,  1,  2, -1 }, // 7
        {  3, -1, -1, -1 }, // 8
        {  0,  3, -1, -1 }, // 9
        {  1,  3, -1, -1 }, // 10
        {  0,  1,  3, -1 }, // 11
        {  2,  3, -1, -1 }, // 12
        {  0,  2,  3, -1 }, // 13
        {  1,  2,  3, -1 }, // 14
        {  0,  1,  2,  3 }, // 15
    };

    int i = lookuptable[viable_mask][path_choice];
    return World_CoordsToIdx(world, x + directions[i][0], y + directions[i][1]);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    World world;
    if (read_world_from_file(argv[1], &world))
    {
        fprintf(stderr, "Failed while reading input file '%s'\n", argv[1]);
        return 1;
    }

    printf("Generation 0\n");
    World_PrettyPrint(&world);

    size_t grid_size = world.n_rows * world.n_cols * sizeof(WorldObject);
    WorldObject* out_grid = (WorldObject*)malloc(grid_size);

    int const n_gen = world.n_gen;
    for (int gen = 0; gen < n_gen; ++gen)
    {
        memcpy(out_grid, world.grid, grid_size);

        // process rabbits
        for (int x = 0; x < world.n_rows; ++x)
        {
            for (int y = 0; y < world.n_rows; ++y)
            {
                int idx = World_CoordsToIdx(&world, x, y);
                WorldObject* obj = World_GetObject(&world, idx);
                if (obj->type != OBJECT_TYPE_RABBIT)
                    continue;

                ++obj->gen_proc;
                int const can_proc = obj->gen_proc > world.gen_proc_rabbits;

                int loc_idx = choose_move(&world, gen, obj, x, y, OBJECT_TYPE_NONE);
                if (loc_idx >= 0)
                {
                    // reset proc age since we were able to move
                    if (can_proc)
                        obj->gen_proc = 0;

                    // move obj to loc_idx
                    // conflict rules say the one with the older procreation age stays
                    WorldObject* local_object = &out_grid[loc_idx];
                    if (local_object->type == OBJECT_TYPE_RABBIT)
                    {
                        if (obj->gen_proc > local_object->gen_proc)
                            (*local_object) = (*obj);
                    }
                    else
                        (*local_object) = (*obj);

                    if (can_proc)
                    {
                        // procreation, leave rabbit in place
                        out_grid[idx] = (*obj);
                    }
                    else
                        out_grid[idx].type = OBJECT_TYPE_NONE;

                    continue;
                }

                // failed to move, stay in same place
                out_grid[idx] = (*obj);
            }
        }

        memcpy(world.grid, out_grid, grid_size);

        // process foxes
        for (int x = 0; x < world.n_rows; ++x)
        {
            for (int y = 0; y < world.n_cols; ++y)
            {
                int idx = World_CoordsToIdx(&world, x, y);
                WorldObject* obj = World_GetObject(&world, idx);
                if (obj->type != OBJECT_TYPE_FOX)
                    continue;

                ++obj->gen_proc;
                int const can_proc = obj->gen_proc > world.gen_proc_foxes;

                // search for a rabbit
                int rabbit_loc_idx = choose_move(&world, gen, obj, x, y, OBJECT_TYPE_RABBIT);
                if (rabbit_loc_idx >= 0)
                {
                    // reset proc age since we were able to move
                    if (can_proc)
                        obj->gen_proc = 0;

                    // found a rabbit, eat it up
                    obj->last_ate = 0;
                    // move fox to rabbit location
                    WorldObject* local_object = &out_grid[rabbit_loc_idx];
                    (*local_object) = (*obj);

                    if (can_proc)
                    {
                        // procreation, leave fox in place
                        out_grid[idx] = (*local_object);
                    }
                    else
                        out_grid[idx].type = OBJECT_TYPE_NONE;

                    continue; // that's all folks
                }

                // no rabbit found, die if too much time passed since last gen
                if (++obj->last_ate >= world.gen_food_foxes)
                {
                    out_grid[idx].type = OBJECT_TYPE_NONE; // death
                    continue;
                }

                int loc_idx = choose_move(&world, gen, obj, x, y, OBJECT_TYPE_NONE);
                if (loc_idx >= 0)
                {
                    // reset proc age since we were able to move
                    if (can_proc)
                        obj->gen_proc = 0;

                    // move fox to location
                    WorldObject* local_object = &out_grid[loc_idx];

                    // overriding another fox, keep the one with older procreation age
                    // or if gen_proc is equal, the least hungry one
                    if (local_object->type == OBJECT_TYPE_FOX)
                    {
                        if (obj->gen_proc > local_object->gen_proc)
                            (*local_object) = (*obj);
                        else if (obj->gen_proc == local_object->gen_proc &&
                            obj->last_ate < local_object->last_ate)
                            (*local_object) = (*obj);
                    }
                    else
                        (*local_object) = (*obj);

                    if (can_proc)
                    {
                        // procreation, leave fox in place
                        // it doesn't inherit father's last_ate
                        out_grid[idx] = (*obj);
                        out_grid[idx].last_ate = 0;
                    }
                    else
                        out_grid[idx].type = OBJECT_TYPE_NONE;

                    continue;
                }

                // failed to move, stay in same place
                out_grid[idx] = (*obj);
            }
        }

        memcpy(world.grid, out_grid, grid_size);

        --world.n_gen;

        printf("\nGeneration %d\n", gen + 1);
        World_PrettyPrint(&world);
    }

    World_Print(&world);

    free(world.grid);
    return 0;
}

void print_usage()
{
    printf("Usage: ./ecosystem $infile\n");
}

int read_world_from_file(const char* file_str, World* world)
{
    FILE* file = fopen(file_str, "r");
    if (!file)
        return 1;

    if (fscanf(file, "%d ", &world->gen_proc_rabbits) <= 0 ||
        fscanf(file, "%d ", &world->gen_proc_foxes) <= 0 ||
        fscanf(file, "%d ", &world->gen_food_foxes) <= 0 ||
        fscanf(file, "%d ", &world->n_gen) <= 0 ||
        fscanf(file, "%d ", &world->n_rows) <= 0 ||
        fscanf(file, "%d ", &world->n_cols) <= 0)
        return 1;

    // initialize world grid
    size_t grid_size = world->n_rows * world->n_cols * sizeof(WorldObject);
    world->grid = (WorldObject*)malloc(grid_size);
    if (!world->grid)
        return 1;

    for (int y = 0; y < world->n_rows; ++y)
    {
        for (int x = 0; x < world->n_cols; ++x)
        {
            int idx = World_CoordsToIdx(world, x, y);
            WorldObject* obj = World_GetObject(world, idx);
            obj->type = OBJECT_TYPE_NONE;
            obj->last_ate = 0;
        }
    }

    // fill grid with objects
    int n_objects;
    if (fscanf(file, "%d\n", &n_objects) <= 0)
        return 1;

    for (int i = 0; i < n_objects; ++i)
    {
        char buffer[100];
        int x;
        int y;

        if (fscanf(file, "%s ", buffer) <= 0 ||
            fscanf(file, "%d ", &x) <= 0 ||
            fscanf(file, "%d", &y) <= 0)
            return 1;

        ObjectType obj_type = OBJECT_TYPE_NONE;
        if (strcmp(buffer, "ROCK") == 0)
            obj_type = OBJECT_TYPE_ROCK;
        else if (strcmp(buffer, "RABBIT") == 0)
            obj_type = OBJECT_TYPE_RABBIT;
        else if (strcmp(buffer, "FOX") == 0)
            obj_type = OBJECT_TYPE_FOX;
        else
            return 1;

        if (x >= world->n_rows ||
            y >= world->n_cols)
            return 1;

        int idx = World_CoordsToIdx(world, x, y);
        World_SetObjectType(world, idx, obj_type);
    }

    fclose(file);
    return 0;
}
