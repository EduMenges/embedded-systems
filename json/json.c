#include "json.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Determines whether a character `ch` is whitespace
 */
#define is_whitespace(ch) (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')

#ifdef JSON_SKIP_WHITESPACE
void json_skip_whitespace(typed(json_string) * str_ptr) {
  while (is_whitespace(**str_ptr))
    (*str_ptr)++;
}
#else
#define json_skip_whitespace(arg)
#endif

#ifdef JSON_DEBUG
#define log(str, ...) printf(str "\n", ##__VA_ARGS__)
void json_debug_print(typed(json_string) str, typed(size) len) {
  for (size_t i = 0; i < len; i++) {
    if (str[i] == '\0')
      break;

    putchar(str[i]);
  }
  printf("\n");
}
#else
#define log(str, ...)
#endif

#define define_result_type(name)                                               \
  result(name) result_ok(name)(typed(name) value) {                            \
    result(name) retval = {                                                    \
        .is_ok = _true,                                                         \
        .inner =                                                               \
            {                                                                  \
                .value = value,                                                \
            },                                                                 \
    };                                                                         \
    return retval;                                                             \
  }                                                                            \
  result(name) result_err(name)(typed(json_error) err) {                       \
    result(name) retval = {                                                    \
        .is_ok = _false,                                                        \
        .inner =                                                               \
            {                                                                  \
                .err = err,                                                    \
            },                                                                 \
    };                                                                         \
    return retval;                                                             \
  }                                                                            \
  typed(json_boolean) result_is_ok(name)(result(name) * result) {              \
    return result->is_ok;                                                      \
  }                                                                            \
  typed(json_boolean) result_is_err(name)(result(name) * result) {             \
    return !result->is_ok;                                                     \
  }                                                                            \
  typed(name) result_unwrap(name)(result(name) * result) {                     \
    return result->inner.value;                                                \
  }                                                                            \
  typed(json_error) result_unwrap_err(name)(result(name) * result) {           \
    return result->inner.err;                                                  \
  }

/**
 * @brief Allocate `count` number of items of `type` in memory
 * and return the pointer to the newly allocated memory
 */
#define allocN(type, count) (type *)malloc((count) * sizeof(type))

/**
 * @brief Allocate an item of `type` in memory and return the
 * pointer to the newly allocated memory
 */
#define alloc(type) allocN(type, 1)

/**
 * @brief Re-allocate `count` number of items of `type` in memory
 * and return the pointer to the newly allocated memory
 */
#define reallocN(ptr, type, count) (type *)realloc(ptr, (count) * sizeof(type))

/**
 * @brief Parses a JSON element {json_element_t} and moves the string
 * pointer to the end of the parsed element
 */
static result(json_entry) json_parse_entry(json_string_t *);

/**
 * @brief Guesses the element type at the start of a string
 */
static result(json_element_type) json_guess_element_type(json_string_t);

/**
 * @brief Whether a token represents a string. Like '"'
 */
static _bool json_is_string(char);

/**
 * @brief Whether a token represents a number. Like '0'
 */
static _bool json_is_number(char);

/**
 * @brief Whether a token represents a object. Like '"'
 */
static _bool json_is_object(char);

/**
 * @brief Whether a token represents a array. Like '['
 */
static _bool json_is_array(char);

/**
 * @brief Whether a token represents a boolean. Like 't'
 */
static _bool json_is_boolean(char);

/**
 * @brief Whether a token represents a null. Like 'n'
 */
static _bool json_is_null(char);

/**
 * @brief Parses a JSON element value {json_element_value_t} based
 * on the `type` parameter passed and moves the string pointer
 * to end of the parsed element
 */
static result(json_element_value)
    json_parse_element_value(json_string_t *, json_element_type_t);

/**
 * @brief Parses a `String` {json_string_t} and moves the string
 * pointer to the end of the parsed string
 */
static result(json_element_value) json_parse_string(json_string_t *);

/**
 * @brief Parses a `Number` {json_number_t} and moves the string
 * pointer to the end of the parsed number
 */
static result(json_element_value) json_parse_number(json_string_t *);

/**
 * @brief Parses a `Object` {json_object_t} and moves the string
 * pointer to the end of the parsed object
 */
static result(json_element_value) json_parse_object(json_string_t *);

static uint64_t json_key_hash(json_string_t);

/**
 * @brief Parses a `Array` {json_array_t} and moves the string
 * pointer to the end of the parsed array
 */
static result(json_element_value) json_parse_array(json_string_t *);

/**
 * @brief Parses a `Boolean` {json_boolean_t} and moves the string
 * pointer to the end of the parsed boolean
 */
static result(json_element_value) json_parse_boolean(json_string_t *);

/**
 * @brief Skips a Key-Value pair
 *
 * @return true If a valid entry is skipped
 * @return false If entry was invalid (still skips)
 */
static _bool json_skip_entry(json_string_t *);

/**
 * @brief Skips an element value
 *
 * @return true If a valid element is skipped
 * @return false If element was invalid (still skips)
 */
static _bool json_skip_element_value(json_string_t *,
                                    json_element_type_t);

/**
 * @brief Skips a string value
 *
 * @return true If a valid string is skipped
 * @return false If string was invalid (still skips)
 */
static _bool json_skip_string(json_string_t *);

/**
 * @brief Skips a number value
 *
 * @return true If a valid number is skipped
 * @return false If number was invalid (still skips)
 */
static _bool json_skip_number(json_string_t *);

/**
 * @brief Skips an object value
 *
 * @return true If a valid object is skipped
 * @return false If object was invalid (still skips)
 */
static _bool json_skip_object(json_string_t *);

/**
 * @brief Skips an array value
 *
 * @return true If a valid array is skipped
 * @return false If array was invalid (still skips)
 */
static _bool json_skip_array(json_string_t *);

/**
 * @brief Skips a boolean value
 *
 * @return true If a valid boolean is skipped
 * @return false If boolean was invalid (still skips)
 */
static _bool json_skip_boolean(json_string_t *);

/**
 * @brief Moves a JSON string pointer beyond any whitespace
 */
// static void json_skip_whitespace_actual(json_string_t *);

/**
 * @brief Moves a JSON string pointer beyond `null` literal
 *
 */
static void json_skip_null(json_string_t *);

/**
 * @brief Prints a JSON element {json_element_t} type
 */
static void json_print_element(json_element_t *, int, int);

/**
 * @brief Prints a `String` {json_string_t} type
 */
static void json_print_string(json_string_t);

/**
 * @brief Prints a `Number` {json_number_t} type
 */
static void json_print_number(json_number_t);

/**
 * @brief Prints an `Object` {json_object_t} type
 */
static void json_print_object(json_object_t *, int, int);

/**
 * @brief Prints an `Array` {json_array_t} type
 */
static void json_print_array(json_array_t *, int, int);

/**
 * @brief Prints a `Boolean` {json_boolean_t} type
 */
static void json_print_boolean(json_boolean_t);

/**
 * @brief Frees a `String` (json_string_t) from memory
 */
static void json_free_string(json_string_t);

/**
 * @brief Frees an `Object` (json_object_t) from memory
 */
static void json_free_object(json_object_t *);

/**
 * @brief Frees an `Array` (json_array_t) from memory
 */
static void json_free_array(json_array_t *);

/**
 * @brief Utility function to convert an escaped string to a formatted string
 */
static result(json_string)
    json_unescape_string(json_string_t, size_t);

/**
 * @brief Offset to the last `"` of a JSON string
 */
static size_t json_string_len(json_string_t);

result(json_element) json_parse(json_string_t json_str) {
  if (json_str == NULL) {
    return result_err(json_element)(JSON_ERROR_EMPTY);
  }

  size_t len = strlen(json_str);
  if (len == 0) {
    return result_err(json_element)(JSON_ERROR_EMPTY);
  }

  result_try(json_element, json_element_type, type,
             json_guess_element_type(json_str));
  result_try(json_element, json_element_value, value,
             json_parse_element_value(&json_str, type));

  const json_element_t element = {
      .type = type,
      .value = value,
  };

  return result_ok(json_element)(element);
}

result(json_entry) json_parse_entry(json_string_t * str_ptr) {
  result_try(json_entry, json_element_value, key, json_parse_string(str_ptr));
  json_skip_whitespace(str_ptr);

  // Skip the ':' delimiter
  (*str_ptr)++;

  json_skip_whitespace(str_ptr);

  result(json_element_type) type_result = json_guess_element_type(*str_ptr);
  if (result_is_err(json_element_type)(&type_result)) {
    free((void *)key.as_string);
    return result_map_err(json_entry, json_element_type, &type_result);
  }
  json_element_type_t type =
      result_unwrap(json_element_type)(&type_result);

  result(json_element_value) value_result =
      json_parse_element_value(str_ptr, type);
  if (result_is_err(json_element_value)(&value_result)) {
    free((void *)key.as_string);
    return result_map_err(json_entry, json_element_value, &value_result);
  }
  json_element_value_t value =
      result_unwrap(json_element_value)(&value_result);

  json_entry_t entry = {
      .key = key.as_string,
      .element =
          {
              .type = type,
              .value = value,
          },
  };

  return result_ok(json_entry)(entry);
}

result(json_element_type) json_guess_element_type(json_string_t str) {
  const char ch = *str;
  json_element_type_t type;

  if (json_is_string(ch))
    type = JSON_ELEMENT_TYPE_STRING;
  else if (json_is_object(ch))
    type = JSON_ELEMENT_TYPE_OBJECT;
  else if (json_is_array(ch))
    type = JSON_ELEMENT_TYPE_ARRAY;
  else if (json_is_null(ch))
    type = JSON_ELEMENT_TYPE_NULL;
  else if (json_is_number(ch))
    type = JSON_ELEMENT_TYPE_NUMBER;
  else if (json_is_boolean(ch))
    type = JSON_ELEMENT_TYPE_BOOLEAN;
  else
    return result_err(json_element_type)(JSON_ERROR_INVALID_TYPE);

  return result_ok(json_element_type)(type);
}

_bool json_is_string(char ch) { return ch == '"'; }

_bool json_is_number(char ch) {
  return (ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '.' ||
         ch == 'e' || ch == 'E';
}

_bool json_is_object(char ch) { return ch == '{'; }

_bool json_is_array(char ch) { return ch == '['; }

_bool json_is_boolean(char ch) { return ch == 't' || ch == 'f'; }

_bool json_is_null(char ch) { return ch == 'n'; }

result(json_element_value)
    json_parse_element_value(json_string_t * str_ptr,
                             json_element_type_t type) {
  switch (type) {
  case JSON_ELEMENT_TYPE_STRING:
    return json_parse_string(str_ptr);
  case JSON_ELEMENT_TYPE_NUMBER:
    return json_parse_number(str_ptr);
  case JSON_ELEMENT_TYPE_OBJECT:
    return json_parse_object(str_ptr);
  case JSON_ELEMENT_TYPE_ARRAY:
    return json_parse_array(str_ptr);
  case JSON_ELEMENT_TYPE_BOOLEAN:
    return json_parse_boolean(str_ptr);
  case JSON_ELEMENT_TYPE_NULL:
    json_skip_null(str_ptr);
    return result_err(json_element_value)(JSON_ERROR_EMPTY);
  default:
    return result_err(json_element_value)(JSON_ERROR_INVALID_TYPE);
  }
}

result(json_element_value) json_parse_string(json_string_t * str_ptr) {
  // Skip the first '"' character
  (*str_ptr)++;

  size_t len = json_string_len(*str_ptr);
  if (len == 0) {
    // Skip the end quote
    (*str_ptr)++;
    return result_err(json_element_value)(JSON_ERROR_EMPTY);
  }

  result_try(json_element_value, json_string, output,
             json_unescape_string(*str_ptr, len));

  // Skip to beyond the string
  (*str_ptr) += len + 1;

  json_element_value_t retval = {0};
  retval.as_string = output;

  return result_ok(json_element_value)(retval);
}

result(json_element_value) json_parse_number(json_string_t * str_ptr) {
  json_string_t temp_str = *str_ptr;
  _bool has_decimal = _false;

  while (json_is_number(*temp_str)) {
    if (*temp_str == '.') {
      has_decimal = _true;
    }

    temp_str++;
  }

  json_number_t number = {0};
  json_number_value_t val = {0};

  if (has_decimal) {
    errno = 0;

    val.as_double = strtod(*str_ptr, (char **)str_ptr);

    number.type = JSON_NUMBER_TYPE_DOUBLE;
    number.value = val;

    if (errno == ERANGE)
      return result_err(json_element_value)(JSON_ERROR_INVALID_VALUE);
  } else {
    errno = 0;

    val.as_long = strtol(*str_ptr, (char **)str_ptr, 10);

    number.type = JSON_NUMBER_TYPE_LONG;
    number.value = val;

    if (errno == ERANGE)
      return result_err(json_element_value)(JSON_ERROR_INVALID_VALUE);
  }

  json_element_value_t retval = {0};
  retval.as_number = number;

  return result_ok(json_element_value)(retval);
}

result(json_element_value) json_parse_object(json_string_t * str_ptr) {
  json_string_t temp_str = *str_ptr;

  // ******* First find the number of valid entries *******
  // Skip the first '{' character
  temp_str++;

  json_skip_whitespace(&temp_str);

  if (*temp_str == '}') {
    // Skip the end '}' in the actual pointer
    (*str_ptr) = temp_str + 1;
    return result_err(json_element_value)(JSON_ERROR_EMPTY);
  }

  size_t count = 0;

  while (*temp_str != '\0') {
    // Skip any accidental whitespace
    json_skip_whitespace(&temp_str);

    // If the entry could be skipped
    if (json_skip_entry(&temp_str)) {
      count++;
    }

    // Skip any accidental whitespace
    json_skip_whitespace(&temp_str);

    if (*temp_str == '}')
      break;

    // Skip the ',' to move to the next entry
    temp_str++;
  }

  if (count == 0)
    return result_err(json_element_value)(JSON_ERROR_EMPTY);

  // ******* Initialize the hash map *******
  // Now we have a perfectly sized hash map
  json_entry_t **entries = allocN(json_entry_t *, count);
  size_t i;
  for (i = 0; i < count; i++)
    entries[i] = NULL;

  // Skip the first '{' character
  (*str_ptr)++;

  json_skip_whitespace(str_ptr);

  while (**str_ptr != '\0') {
    // Skip any accidental whitespace
    json_skip_whitespace(str_ptr);
    result(json_entry) entry_result = json_parse_entry(str_ptr);

    if (result_is_ok(json_entry)(&entry_result)) {
      json_entry_t entry = result_unwrap(json_entry)(&entry_result);
      uint64_t bucket = json_key_hash(entry.key) % count;

      // Bucket size is exactly count. So there will be at max
      // count misses in the worst case
      size_t i;
      for (i = 0; i < count; i++) {
        if (entries[bucket] == NULL) {
          json_entry_t *temp_entry = alloc(json_entry_t);
          memcpy(temp_entry, &entry, sizeof(json_entry_t));
          entries[bucket] = temp_entry;
          break;
        }

        bucket = (bucket + 1) % count;
      }
    }

    // Skip any accidental whitespace
    json_skip_whitespace(str_ptr);

    if (**str_ptr == '}')
      break;

    // Skip the ',' to move to the next entry
    (*str_ptr)++;
  }

  // Skip the '}' closing brace
  (*str_ptr)++;

  json_object_t *object = alloc(json_object_t);
  object->count = count;
  object->entries = entries;

  json_element_value_t retval = {0};
  retval.as_object = object;

  return result_ok(json_element_value)(retval);
}

uint64_t json_key_hash(json_string_t str) {
  uint64_t hash = 0;

  while (*str != '\0')
    hash += (hash * 31) + *str++;

  return hash;
}

result(json_element_value) json_parse_array(json_string_t * str_ptr) {
  // Skip the starting '[' character
  (*str_ptr)++;

  json_skip_whitespace(str_ptr);

  // Unfortunately the array is empty
  if (**str_ptr == ']') {
    // Skip the end ']'
    (*str_ptr)++;
    return result_err(json_element_value)(JSON_ERROR_EMPTY);
  }

  size_t count = 0;
  json_element_t *elements = NULL;

  while (**str_ptr != '\0') {
    json_skip_whitespace(str_ptr);

    // Guess the type
    result(json_element_type) type_result = json_guess_element_type(*str_ptr);
    if (result_is_ok(json_element_type)(&type_result)) {
      json_element_type_t type =
          result_unwrap(json_element_type)(&type_result);

      // Parse the value based on guessed type
      result(json_element_value) value_result =
          json_parse_element_value(str_ptr, type);
      if (result_is_ok(json_element_value)(&value_result)) {
        json_element_value_t value =
            result_unwrap(json_element_value)(&value_result);

        count++;
        elements = reallocN(elements, json_element_t, count);
        elements[count - 1].type = type;
        elements[count - 1].value = value;
      }

      json_skip_whitespace(str_ptr);
    }

    // Reached the end
    if (**str_ptr == ']')
      break;

    // Skip the ','
    (*str_ptr)++;
  }

  // Skip the ']' closing array
  (*str_ptr)++;

  if (count == 0)
    return result_err(json_element_value)(JSON_ERROR_EMPTY);

  json_array_t *array = alloc(json_array_t);
  array->count = count;
  array->elements = elements;

  json_element_value_t retval = {0};
  retval.as_array = array;

  return result_ok(json_element_value)(retval);
}

result(json_element_value) json_parse_boolean(json_string_t * str_ptr) {
  json_boolean_t output;

  switch (**str_ptr) {
  case 't':
    output = _true;
    (*str_ptr) += 4;
    break;

  case 'f':
    output = _false;
    (*str_ptr) += 5;
    break;
  }
  
  json_element_value_t retval = {0};
  retval.as_boolean = output;

  return result_ok(json_element_value)(retval);
}

result(json_element)
    json_object_find(json_object_t * obj, json_string_t key) {
  if (key == NULL || strlen(key) == 0)
    return result_err(json_element)(JSON_ERROR_INVALID_KEY);

  uint64_t bucket = json_key_hash(key) % obj->count;

  // Bucket size is exactly obj->count. So there will be at max
  // obj->count misses in the worst case
  size_t i;
  for (i = 0; i < obj->count; i++) {
    json_entry_t *entry = obj->entries[bucket];
    if (strcmp(key, entry->key) == 0)
      return result_ok(json_element)(entry->element);

    bucket = (bucket + 1) % obj->count;
  }

  return result_err(json_element)(JSON_ERROR_INVALID_KEY);
}

_bool json_skip_entry(json_string_t * str_ptr) {
  json_skip_string(str_ptr);

  json_skip_whitespace(str_ptr);

  // Skip the ':' delimiter
  (*str_ptr)++;

  json_skip_whitespace(str_ptr);

  result(json_element_type) type_result = json_guess_element_type(*str_ptr);
  if (result_is_err(json_element_type)(&type_result))
    return _false;

  json_element_type_t type =
      result_unwrap(json_element_type)(&type_result);

  return json_skip_element_value(str_ptr, type);
}

_bool json_skip_element_value(json_string_t * str_ptr,
                             json_element_type_t type) {
  switch (type) {
  case JSON_ELEMENT_TYPE_STRING:
    return json_skip_string(str_ptr);
  case JSON_ELEMENT_TYPE_NUMBER:
    return json_skip_number(str_ptr);
  case JSON_ELEMENT_TYPE_OBJECT:
    return json_skip_object(str_ptr);
  case JSON_ELEMENT_TYPE_ARRAY:
    return json_skip_array(str_ptr);
  case JSON_ELEMENT_TYPE_BOOLEAN:
    return json_skip_boolean(str_ptr);
  case JSON_ELEMENT_TYPE_NULL:
    json_skip_null(str_ptr);
    return _false;

  default:
    return _false;
  }
}

_bool json_skip_string(json_string_t * str_ptr) {
  // Skip the initial '"'
  (*str_ptr)++;

  // Find the length till the last '"'
  size_t len = json_string_len(*str_ptr);

  // Skip till the end of the string
  (*str_ptr) += len + 1;

  return len > 0;
}

_bool json_skip_number(json_string_t * str_ptr) {
  while (json_is_number(**str_ptr)) {
    (*str_ptr)++;
  }

  return _true;
}

_bool json_skip_object(json_string_t * str_ptr) {
  // Skip the first '{' character
  (*str_ptr)++;

  json_skip_whitespace(str_ptr);

  if (**str_ptr == '}') {
    // Skip the end '}'
    (*str_ptr)++;
    return _false;
  }

  while (**str_ptr != '\0') {
    // Skip any accidental whitespace
    json_skip_whitespace(str_ptr);

    json_skip_entry(str_ptr);

    // Skip any accidental whitespace
    json_skip_whitespace(str_ptr);

    if (**str_ptr == '}')
      break;

    // Skip the ',' to move to the next entry
    (*str_ptr)++;
  }

  // Skip the '}' closing brace
  (*str_ptr)++;

  return _true;
}

_bool json_skip_array(json_string_t * str_ptr) {
  // Skip the starting '[' character
  (*str_ptr)++;

  json_skip_whitespace(str_ptr);

  // Unfortunately the array is empty
  if (**str_ptr == ']') {
    // Skip the end ']'
    (*str_ptr)++;
    return _false;
  }

  while (**str_ptr != '\0') {
    json_skip_whitespace(str_ptr);

    // Guess the type
    result(json_element_type) type_result = json_guess_element_type(*str_ptr);
    if (result_is_ok(json_element_type)(&type_result)) {
      json_element_type_t type =
          result_unwrap(json_element_type)(&type_result);

      // Parse the value based on guessed type
      json_skip_element_value(str_ptr, type);

      json_skip_whitespace(str_ptr);
    }

    // Reached the end
    if (**str_ptr == ']')
      break;

    // Skip the ','
    (*str_ptr)++;
  }

  // Skip the ']' closing array
  (*str_ptr)++;

  return _true;
}

_bool json_skip_boolean(json_string_t * str_ptr) {
  switch (**str_ptr) {
  case 't':
    (*str_ptr) += 4;
    return _true;

  case 'f':
    (*str_ptr) += 5;
    return _true;
  }

  return _false;
}

void json_skip_null(json_string_t * str_ptr) { (*str_ptr) += 4; }

void json_print(json_element_t * element, int indent) {
  json_print_element(element, indent, 0);
}

void json_print_element(json_element_t * element, int indent,
                        int indent_level) {

  switch (element->type) {
  case JSON_ELEMENT_TYPE_STRING:
    json_print_string(element->value.as_string);
    break;
  case JSON_ELEMENT_TYPE_NUMBER:
    json_print_number(element->value.as_number);
    break;
  case JSON_ELEMENT_TYPE_OBJECT:
    json_print_object(element->value.as_object, indent, indent_level);
    break;
  case JSON_ELEMENT_TYPE_ARRAY:
    json_print_array(element->value.as_array, indent, indent_level);
    break;
  case JSON_ELEMENT_TYPE_BOOLEAN:
    json_print_boolean(element->value.as_boolean);
    break;
  case JSON_ELEMENT_TYPE_NULL:
    break;
    // Do nothing
  }
}

void json_print_string(json_string_t string) { printf("\"%s\"", string); }

void json_print_number(json_number_t number) {
  switch (number.type) {
  case JSON_NUMBER_TYPE_DOUBLE:
    printf("%f", number.value.as_double);
    break;

  case JSON_NUMBER_TYPE_LONG:
    printf("%ld", number.value.as_long);
    break;
  }
}

void json_print_object(json_object_t * object, int indent,
                       int indent_level) {
  printf("{\n");

  size_t i;
  for (i = 0; i < object->count; i++) {
  	int j;
    for (j = 0; j < indent * (indent_level + 1); j++)
      printf(" ");

    json_entry_t *entry = object->entries[i];

    json_print_string(entry->key);
    printf(": ");
    json_print_element(&entry->element, indent, indent_level + 1);

    if (i != object->count - 1)
      printf(",");
    printf("\n");
  }

  int j;
  for (j = 0; j < indent * indent_level; j++)
    printf(" ");
  printf("}");
}

void json_print_array(json_array_t * array, int indent, int indent_level) {
  printf("[\n");

  {
	  size_t i;
	  for (i = 0; i < array->count; i++) {
	    json_element_t element = array->elements[i];
		int j;
	    for (j = 0; j < indent * (indent_level + 1); j++)
	      printf(" ");
	    json_print_element(&element, indent, indent_level + 1);

	    if (i != array->count - 1)
	      printf(",");
	    printf("\n");
	  }
  }

  int i;
  for (i = 0; i < indent * indent_level; i++)
    printf(" ");
  printf("]");
}

void json_print_boolean(json_boolean_t boolean) {
  printf("%s", boolean ? "true" : "false");
}

void json_free(json_element_t * element) {
  switch (element->type) {
  case JSON_ELEMENT_TYPE_STRING:
    json_free_string(element->value.as_string);
    break;

  case JSON_ELEMENT_TYPE_OBJECT:
    json_free_object(element->value.as_object);
    break;

  case JSON_ELEMENT_TYPE_ARRAY:
    json_free_array(element->value.as_array);
    break;

  case JSON_ELEMENT_TYPE_NUMBER:
  case JSON_ELEMENT_TYPE_BOOLEAN:
  case JSON_ELEMENT_TYPE_NULL:
    // Do nothing
    break;
  }
}

void json_free_string(json_string_t string) { free((void *)string); }

void json_free_object(json_object_t * object) {
  if (object == NULL)
    return;

  if (object->count == 0) {
    free(object);
    return;
  }

  size_t i;
  for (i = 0; i < object->count; i++) {
    json_entry_t *entry = object->entries[i];

    if (entry != NULL) {
      free((void *)entry->key);
      json_free(&entry->element);
      free(entry);
    }
  }

  free(object->entries);
  free(object);
}

void json_free_array(json_array_t * array) {
  if (array == NULL)
    return;

  if (array->count == 0) {
    free(array);
    return;
  }

  // Recursively free each element in the array
  size_t i;
  for (i = 0; i < array->count; i++) {
    json_element_t element = array->elements[i];
    json_free(&element);
  }

  // Lastly free
  free(array->elements);
  free(array);
}

json_string_t json_error_to_string(json_error_t error) {
  switch (error) {
  case JSON_ERROR_EMPTY:
    return "Empty";
  case JSON_ERROR_INVALID_KEY:
    return "Invalid key";
  case JSON_ERROR_INVALID_TYPE:
    return "Invalid type";
  case JSON_ERROR_INVALID_VALUE:
    return "Invalid value";

  default:
    return "Unknown error";
  }
}

size_t json_string_len(json_string_t str) {
  size_t len = 0;

  json_string_t iter = str;
  while (*iter != '\0') {
    if (*iter == '\\')
      iter += 2;

    if (*iter == '"') {
      len = iter - str;
      break;
    }

    iter++;
  }

  return len;
}

result(json_string)
    json_unescape_string(json_string_t str, size_t len) {
  size_t count = 0;
  json_string_t iter = str;

  while ((size_t)(iter - str) < len) {
    if (*iter == '\\')
      iter++;

    count++;
    iter++;
  }

  char *output = allocN(char, count + 1);
  size_t offset = 0;
  iter = str;

  while ((size_t)(iter - str) < len) {
    if (*iter == '\\') {
      iter++;

      switch (*iter) {
      case 'b':
        output[offset] = '\b';
        break;
      case 'f':
        output[offset] = '\f';
        break;
      case 'n':
        output[offset] = '\n';
        break;
      case 'r':
        output[offset] = '\r';
        break;
      case 't':
        output[offset] = '\t';
        break;
      case '"':
        output[offset] = '"';
        break;
      case '\\':
        output[offset] = '\\';
        break;
      default:
        return result_err(json_string)(JSON_ERROR_INVALID_VALUE);
      }
    } else {
      output[offset] = *iter;
    }

    offset++;
    iter++;
  }

  output[offset] = '\0';
  return result_ok(json_string)((json_string_t)output);
}

define_result_type(json_element_type)
define_result_type(json_element_value)
define_result_type(json_element)
define_result_type(json_entry)
define_result_type(json_string)
define_result_type(size)

