#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./json.h"

const char *read_file(const char *path) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    fprintf(stderr, "Expected file \"%s\" not found", path);
    return NULL;
  }
  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *buffer = malloc(len + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Unable to allocate memory for file");
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, len, file);
  buffer[len] = '\0';
  return (const char *)buffer;
}

static const char *default_file = "..\\multidim_arr.json";

int main(int argc, char **argv)
{
    const char *file_name;
    if (argc > 0)
    {
        file_name = argv[1];
    }
    else
    {
        file_name = default_file;
    }

    const char *json = read_file("");
    if (json == NULL)
    {
        return -1;
    }

    result(json_element) element_result = json_parse(json);

    free((void *)json);

    if (result_is_err(json_element)(&element_result))
    {
        typed(json_error) error = result_unwrap_err(json_element)(&element_result);
        fprintf(stderr, "Error parsing JSON: %s\n", json_error_to_string(error));
        return -1;
    }
    typed(json_element) element = result_unwrap(json_element)(&element_result);

    // json_print(&element, 2);
    json_free(&element);

    return 0;
}
