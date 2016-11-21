#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    OBJECT_NONE = 0,
    OBJECT_ROCK,
    OBJECT_RABBIT,
    OBJECT_FOX,
};

typedef enum ObjectType ObjectType;

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
    ObjectType* grid;
};

typedef struct World World;

void World_SetObject(World* world, int x, int y, ObjectType obj)
{
    int idx = x + y * world->n_rows;
    world->grid[idx] = obj;
}

ObjectType World_GetObject(World const* world, int x, int y)
{
    int idx = x + y * world->n_rows;
    return world->grid[idx];
}

void World_Print(World const* world)
{
    // print leading '====='
    for (int i = 0; i < world->n_cols; ++i)
        printf("=");

    printf("\n");

    for (int x = 0; x < world->n_cols; ++x)
    {
        for (int y = 0; y < world->n_rows; ++y)
        {
            ObjectType obj = World_GetObject(world, x, y);

            switch (obj)
            {
                default:
                case OBJECT_NONE:
                    printf(" ");
                    break;
                case OBJECT_ROCK:
                    printf("*");
                    break;
                case OBJECT_RABBIT:
                    printf("R");
                    break;
                case OBJECT_FOX:
                    printf("F");
                    break;
            }
        }

        printf("\n");
    }

    // print trailing '====='
    for (int i = 0; i < world->n_cols; ++i)
        printf("=");

    printf("\n");
}

void print_usage();
int read_world_from_file(const char* file_str, World* world);

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
    size_t grid_size = world->n_rows * world->n_cols * sizeof(ObjectType);
    world->grid = (ObjectType*)malloc(grid_size);
    if (!world->grid)
        return 1;

    for (int y = 0; y < world->n_rows; ++y)
    {
        for (int x = 0; x < world->n_cols; ++x)
            World_SetObject(world, x, y, OBJECT_NONE);
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

        ObjectType obj = OBJECT_NONE;
        if (strcmp(buffer, "ROCK") == 0)
            obj = OBJECT_ROCK;
        else if (strcmp(buffer, "RABBIT") == 0)
            obj = OBJECT_RABBIT;
        else if (strcmp(buffer, "FOX") == 0)
            obj = OBJECT_FOX;
        else
            return 1;

        if (x >= world->n_cols ||
            y >= world->n_rows)
            return 1;

        World_SetObject(world, x, y, obj);
    }

    fclose(file);
    return 0;
}
