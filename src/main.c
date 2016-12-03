#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Defines.h"
#include "World.h"

void print_usage();
World* read_world_from_file(const char* file_str);

WorldObjectPos* choose_move(World const* world, uint32 gen, WorldObject const* obj,
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
        int idx = World_CoordsToIdx(world, coord_x, coord_y);
        WorldObjectPos const* local_obj = World_GetObject(world, idx);
        if (local_obj->first.type == target_type)
            viable_mask |= 1 << i;
    }

    if (!viable_mask)
        return nullptr;

    int const p = __builtin_popcount(viable_mask);
    int path_choice = (gen + x + y) % p;

    // from viable_mask and path_choice we can use a lookup table
    //  to determine which path to go, instead of iteratively figuring it out
    static int const lookuptable[16][4] = {

        {  9,  9,  9,  9 }, // 0
        {  0,  9,  9,  9 }, // 1
        {  1,  9,  9,  9 }, // 2
        {  0,  1,  9,  9 }, // 3
        {  2,  9,  9,  9 }, // 4
        {  0,  2,  9,  9 }, // 5
        {  1,  2,  9,  9 }, // 6
        {  0,  1,  2,  9 }, // 7
        {  3,  9,  9,  9 }, // 8
        {  0,  3,  9,  9 }, // 9
        {  1,  3,  9,  9 }, // 10
        {  0,  1,  3,  9 }, // 11
        {  2,  3,  9,  9 }, // 12
        {  0,  2,  3,  9 }, // 13
        {  1,  2,  3,  9 }, // 14
        {  0,  1,  2,  3 }, // 15
    };

    int i = lookuptable[viable_mask][path_choice];
    int idx = World_CoordsToIdx(world, x + directions[i][0], y + directions[i][1]);
    return World_GetObject(world, idx);
}

void print_usage()
{
    printf("Usage: ./ecosystem $infile [options]\n");
    printf("Options:\n");
    printf("'--test test_file' uses world in test_file to compare with output world, exit error 1 if not equal\n");
    printf("'--verbose' prints each world generation\n");
    printf("'--no-output' silences default output, don't use with --verbose\n");
    printf("'--help' prints this usage message\n");
}

int main(int argc, char** argv)
{
    const char* input_world_file = argc > 1 ? argv[1] : NULL;
    const char* output_test_file = NULL;
    int verbose = 0;
    int no_output = 0;

    // process program options
    for (int i = 1; i < argc; ++i)
    {
        char const* arg = argv[i];
        if (strcmp(arg, "--test") == 0)
        {
            ++i;
            if (i >= argc)
            {
                LOG_ERROR("--test option: missing test_file arg");
                return 1;
            }

            no_output = 1; // no normal output with test case, only passed/failed msg
            output_test_file = argv[i];
        }
        else if (strcmp(arg, "--verbose") == 0)
            verbose = 1;
        else if (strcmp(arg, "--no-output") == 0)
            no_output = 1;
        else if (strcmp(arg, "--help") == 0)
        {
            print_usage();
            return 0;
        }
    }

    if (input_world_file == NULL)
    {
        print_usage();
        return 1;
    }

    World* world = read_world_from_file(input_world_file);
    if (!world)
    {
        LOG_ERROR("failed while reading input file '%s'", input_world_file);
        return 1;
    }

    if (verbose)
    {
        printf("Generation 0\n");
        World_PrettyPrint(world);
    }

    uint64 const n_gen = world->n_gen;
    for (uint64 gen = 0; gen < n_gen; ++gen)
    {
        // process rabbits
        for (int x = 0; x < world->n_rows; ++x)
        {
            for (int y = 0; y < world->n_rows; ++y)
            {
                int idx = World_CoordsToIdx(world, x, y);
                WorldObjectPos* obj_pos = World_GetObject(world, idx);
                WorldObject* obj = &(obj_pos->first);
                if (obj->type != OBJECT_TYPE_RABBIT)
                    continue;

                ++obj->gen_proc;

                WorldObjectPos* local_obj_pos = choose_move(world, gen, obj, x, y, OBJECT_TYPE_NONE);
                if (local_obj_pos)
                {
                    int const can_proc = obj->gen_proc > world->gen_proc_rabbits;

                    // reset proc age since we were able to move
                    if (can_proc)
                        obj->gen_proc = 0;

                    // move obj to loc_idx
                    // conflict rules say the one with the older procreation age stays
                    WorldObject* local_obj = &(local_obj_pos->second);
                    if (local_obj->type == OBJECT_TYPE_RABBIT)
                    {
                        if (obj->gen_proc > local_obj->gen_proc)
                            (*local_obj) = (*obj);
                    }
                    else
                        (*local_obj) = (*obj);

                    // procreation, leave rabbit in place
                    if (can_proc)
                        obj_pos->second = (*obj);
                    else
                        obj_pos->second.type = OBJECT_TYPE_NONE;

                    continue;
                }

                // failed to move, stay in same place
                obj_pos->second = (*obj);
            }
        }

        World_UpdateGrid(world);

        // process foxes
        for (int x = 0; x < world->n_rows; ++x)
        {
            for (int y = 0; y < world->n_cols; ++y)
            {
                int idx = World_CoordsToIdx(world, x, y);
                WorldObjectPos* obj_pos = World_GetObject(world, idx);
                WorldObject* obj = &(obj_pos->first);
                if (obj->type != OBJECT_TYPE_FOX)
                    continue;

                ++obj->gen_proc;
                int const can_proc = obj->gen_proc > world->gen_proc_foxes;

                // search for a rabbit
                WorldObjectPos* local_obj_pos = choose_move(world, gen, obj, x, y, OBJECT_TYPE_RABBIT);
                if (local_obj_pos)
                {
                    // reset proc age since we were able to move
                    if (can_proc)
                        obj->gen_proc = 0;

                    // found a rabbit, eat it up
                    obj->last_ate = 0;
                    // move fox to rabbit location
                    WorldObject* local_obj = &(local_obj_pos->second);
                    (*local_obj) = (*obj);

                    // // procreation, leave fox in place
                    if (can_proc)
                        obj_pos->second = (*local_obj);
                    else
                        obj_pos->second.type = OBJECT_TYPE_NONE;

                    continue; // that's all folks
                }

                // no rabbit found, die if too much time passed since last gen
                if (++obj->last_ate >= world->gen_food_foxes)
                {
                    obj_pos->second.type = OBJECT_TYPE_NONE; // death
                    continue;
                }

                local_obj_pos = choose_move(world, gen, obj, x, y, OBJECT_TYPE_NONE);
                if (local_obj_pos)
                {
                    // reset proc age since we were able to move
                    if (can_proc)
                        obj->gen_proc = 0;

                    // move fox to location
                    WorldObject* local_obj = &(local_obj_pos->second);

                    // overriding another fox, keep the one with older procreation age
                    // or if gen_proc is equal, the least hungry one
                    if (local_obj->type == OBJECT_TYPE_FOX)
                    {
                        if (obj->gen_proc > local_obj->gen_proc)
                            (*local_obj) = (*obj);
                        else if (obj->gen_proc == local_obj->gen_proc &&
                            obj->last_ate < local_obj->last_ate)
                            (*local_obj) = (*obj);
                    }
                    else
                        (*local_obj) = (*obj);

                    // procreation, leave fox in place
                    // it doesn't inherit father's last_ate
                    if (can_proc)
                    {
                        obj_pos->second = (*obj);
                        obj_pos->second.last_ate = 0;
                    }
                    else
                        obj_pos->second.type = OBJECT_TYPE_NONE;

                    continue;
                }

                // failed to move, stay in same place
                obj_pos->second = (*obj);
            }
        }

        World_UpdateGrid(world);

        --world->n_gen;

        if (verbose)
        {
            printf("\nGeneration %lu\n", gen + 1);
            World_PrettyPrint(world);
        }
    }

    if (no_output == 0)
        World_Print(world);

    int exit_code = 0;

    if (output_test_file)
    {
        World* test_world = read_world_from_file(output_test_file);
        if (!test_world)
        {
            LOG_ERROR("failed while reading test file '%s'", output_test_file);
            return 1;
        }

        exit_code = World_Compare(world, test_world);
        if (exit_code > 0)
            printf("Failed test for world size %dx%d\n", world->n_rows, world->n_cols);
        else
            printf("Passed test for world size %dx%d\n", world->n_rows, world->n_cols);

        World_Delete(test_world);
    }

    World_Delete(world);
    return exit_code;
}

World* read_world_from_file(const char* file_str)
{
    FILE* file = fopen(file_str, "r");
    if (!file)
        return nullptr;

    uint32 gen_proc_rabbits;
    uint32 gen_proc_foxes;
    uint32 gen_food_foxes;
    uint32 n_gen;
    uint32 n_rows;
    uint32 n_cols;

    if (fscanf(file, "%u ", &gen_proc_rabbits) <= 0 ||
        fscanf(file, "%u ", &gen_proc_foxes) <= 0 ||
        fscanf(file, "%u ", &gen_food_foxes) <= 0 ||
        fscanf(file, "%u ", &n_gen) <= 0 ||
        fscanf(file, "%u ", &n_rows) <= 0 ||
        fscanf(file, "%u ", &n_cols) <= 0)
        return nullptr;

    World* world = World_New(gen_proc_rabbits, gen_proc_foxes, gen_food_foxes,
        n_gen, n_rows, n_cols);

    // fill grid with objects
    uint32 n_objects;
    if (fscanf(file, "%d\n", &n_objects) <= 0)
    {
        World_Delete(world);
        return nullptr;
    }

    for (uint32 i = 0; i < n_objects; ++i)
    {
        char buffer[100];
        uint32 x;
        uint32 y;

        if (fscanf(file, "%s ", buffer) <= 0 ||
            fscanf(file, "%u ", &x) <= 0 ||
            fscanf(file, "%u", &y) <= 0)
        {
            World_Delete(world);
            return nullptr;
        }

        ObjectType obj_type = OBJECT_TYPE_NONE;
        if (strcmp(buffer, "ROCK") == 0)
            obj_type = OBJECT_TYPE_ROCK;
        else if (strcmp(buffer, "RABBIT") == 0)
            obj_type = OBJECT_TYPE_RABBIT;
        else if (strcmp(buffer, "FOX") == 0)
            obj_type = OBJECT_TYPE_FOX;
        else
        {
            World_Delete(world);
            return nullptr;
        }

        if (x >= n_rows ||
            y >= n_cols)
        {
            World_Delete(world);
            return nullptr;
        }

        int idx = World_CoordsToIdx(world, x, y);
        WorldObjectPos* obj = World_GetObject(world, idx);
        obj->first.type = obj_type;
        obj->second.type = obj_type;
    }

    fclose(file);
    return world;
}
