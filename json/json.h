#ifndef JSON
#define JSON

#include <stddef.h>

#ifndef __cplusplus
typedef unsigned int _bool;
#define _true (1)
#define _false (0)
#endif

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;
#else
#include <stddint.h>
#endif

#define typed(name) name##_t

typedef const char *json_string_t;
typedef _bool json_boolean_t;

typedef union json_number_value_u json_number_value_t;
typedef signed long json_number_long_t;
typedef double json_number_double_t;
typedef struct json_number_s json_number_t;
typedef union json_element_value_u json_element_value_t;
typedef struct json_element_s json_element_t;
typedef struct json_entry_s json_entry_t;
typedef struct json_object_s json_object_t;
typedef struct json_array_s json_array_t;

#define result(name) name##_result_t
#define result_ok(name) name##_result_ok
#define result_err(name) name##_result_err
#define result_is_ok(name) name##_result_is_ok
#define result_is_err(name) name##_result_is_err
#define result_unwrap(name) name##_result_unwrap
#define result_unwrap_err(name) name##_result_unwrap_err
#define result_map_err(outer_name, inner_name, value)                          \
  result_err(outer_name)(result_unwrap_err(inner_name)(value))
#define result_try(outer_name, inner_name, lvalue, rvalue)                     \
  result(inner_name) lvalue##_result = rvalue;                                 \
  if (result_is_err(inner_name)(&lvalue##_result))                             \
    return result_map_err(outer_name, inner_name, &lvalue##_result);           \
  const typed(inner_name) lvalue = result_unwrap(inner_name)(&lvalue##_result);
#define declare_result_type(name)                                              \
  typedef struct name##_result_s {                                             \
    typed(json_boolean) is_ok;                                                 \
    union {                                                                    \
      typed(name) value;                                                       \
      typed(json_error) err;                                                   \
    } inner;                                                                   \
  } result(name);                                                              \
  result(name) result_ok(name)(typed(name));                                   \
  result(name) result_err(name)(typed(json_error));                            \
  typed(json_boolean) result_is_ok(name)(result(name) *);                      \
  typed(json_boolean) result_is_err(name)(result(name) *);                     \
  typed(name) result_unwrap(name)(result(name) *);                             \
  typed(json_error) result_unwrap_err(name)(result(name) *);

typedef enum json_element_type_e {
  JSON_ELEMENT_TYPE_STRING = 0,
  JSON_ELEMENT_TYPE_NUMBER,
  JSON_ELEMENT_TYPE_OBJECT,
  JSON_ELEMENT_TYPE_ARRAY,
  JSON_ELEMENT_TYPE_BOOLEAN,
  JSON_ELEMENT_TYPE_NULL
} json_element_type_t;

typedef enum json_number_type_e {
  JSON_NUMBER_TYPE_LONG = 0,
  JSON_NUMBER_TYPE_DOUBLE,
} json_number_type_t;

union json_number_value_u {
  json_number_long_t as_long;
  json_number_double_t as_double;
};

struct json_number_s {
  json_number_type_t type;
  json_number_value_t value;
};

union json_element_value_u {
  json_string_t as_string;
  json_number_t as_number;
  json_object_t * as_object;
  json_array_t * as_array;
  json_boolean_t as_boolean;
};

struct json_element_s {
  json_element_type_t type;
  json_element_value_t value;
};

struct json_entry_s {
  json_string_t key;
  json_element_t element;
};

struct json_object_s {
  size_t count;
  json_entry_t * *entries;
};

struct json_array_s {
  size_t count;
  json_element_t * elements;
};

typedef enum json_error_e {
  JSON_ERROR_EMPTY = 0,
  JSON_ERROR_INVALID_TYPE,
  JSON_ERROR_INVALID_KEY,
  JSON_ERROR_INVALID_VALUE
} json_error_t;

declare_result_type(json_element_type)
declare_result_type(json_element_value)
declare_result_type(json_element)
declare_result_type(json_entry)
declare_result_type(json_string)
declare_result_type(size)

/**
 * @brief Parses a JSON string into a JSON element {json_element_t}
 * with a fallible `result` type
 *
 * @param json_str The raw JSON string
 * @return The parsed {json_element_t} wrapped in a `result` type
 */
result(json_element) json_parse(json_string_t json_str);

/**
 * @brief Tries to get the element by key. If not found, returns
 * a {JSON_ERROR_INVALID_KEY} error
 *
 * @param object The object to find the key in
 * @param key The key of the element to be found
 * @return Either a {json_element_t} or {json_error_t}
 */
result(json_element)
    json_object_find(json_object_t * object, json_string_t key);

/**
 * @brief Prints a JSON element {json_element_t} with proper
 * indentation
 *
 * @param indent The number of spaces to indent each level by
 */
void json_print(json_element_t * element, int indent);

/**
 * @brief Frees a JSON element {json_element_t} from memory
 *
 * @param element The JSON element {json_element_t} to free
 */
void json_free(json_element_t * element);

/**
 * @brief Returns a string representation of JSON error {json_error_t} type
 *
 * @param error The JSON error enum {json_error_t} type
 * @return The string representation
 */
json_string_t json_error_to_string(json_error_t error);

#endif
