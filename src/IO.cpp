#include <stdlib.h>
#include <stdio.h>

#include <cJSON/cJSON.h>

#include "common.hpp"
#include "IO.hpp"


/**
 * TODO
 */
void IO::loadConfig(const char *path)
{
    // loadJSON(path);
}


char *IO::loadFileBuffer(const char *path)
{
    char *buffer = 0;
    long length;
    FILE * f = fopen(path, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell (f);
        fseek(f, 0, SEEK_SET);

        buffer = (char *)malloc(length);

        if (!buffer) {
            perror("error opening file");
            exit(1);
        }

        fread(buffer, 1, length, f);        
        fclose(f);
    }

    return buffer;
}


void IO::loadJSON(const char *path)
{
    println("loading '%s'... \n", path);
    
    IO::FileBuffer buf(path);

    cJSON *json = cJSON_Parse(buf.getData());

    char *string = cJSON_Print(json);

    // println("%s\n", string);
    println("successfully loaded '%s' \n", path);
    
    free(string);

    cJSON_Delete(json);
}

