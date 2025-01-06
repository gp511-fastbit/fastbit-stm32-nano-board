/*
 * parser.c
 *
 *  Created on: Oct 17, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#include <stdio.h>
#include <string.h>

//void parse_JSON(const char *data, char *output_buffer, size_t buffer_size) {
//  // Find the beginning of the JSON data by locating the first '{'
//  const char *json_start = strchr(data, '{');
//  if (json_start == NULL) {
//      snprintf(output_buffer, buffer_size, "No JSON data found.\n");
//      return;
//  }
//
//  const char *title_key = "\"title\":\"";
//  const char *abstract_key = "\"abstract\":\"";
//  char title[256];
//  char abstract[512];
//  size_t buffer_used = 0;
//
//  const char *title_pos = json_start;
//  const char *abstract_pos = json_start;
//
//  // Loop to extract all titles and abstracts from the JSON
//  while ((title_pos = strstr(title_pos, title_key)) != NULL) {
//      title_pos += strlen(title_key);  // Move past the "title":" key
//      const char *title_end = strchr(title_pos, '"');  // Find the end of the title
//      if (title_end) {
//          size_t title_length = title_end - title_pos;
//          strncpy(title, title_pos, title_length);
//          title[title_length] = '\0';  // Null-terminate the string
//
//          // Now find the abstract related to this title
//          abstract_pos = strstr(title_end, abstract_key);
//          if (abstract_pos) {
//              abstract_pos += strlen(abstract_key);  // Move past the "abstract":" key
//              const char *abstract_end = strchr(abstract_pos, '"');  // Find the end of the abstract
//              if (abstract_end) {
//                  size_t abstract_length = abstract_end - abstract_pos;
//                  strncpy(abstract, abstract_pos, abstract_length);
//                  abstract[abstract_length] = '\0';  // Null-terminate the string
//
//                  // Format and store the extracted title and abstract into the output buffer
//                  int chars_written = snprintf(output_buffer + buffer_used, buffer_size - buffer_used,
//                                               "Title: %s\nAbstract: %s\n\n", title, abstract);
//
//                  // Check for buffer overflow
//                  if (chars_written < 0 || (size_t)chars_written >= buffer_size - buffer_used) {
//                      // Truncate and break if the buffer is full
//                      snprintf(output_buffer + buffer_used, buffer_size - buffer_used, "Buffer full\n");
//                      break;
//                  }
//                  buffer_used += chars_written;
//              }
//          }
//      }
//      title_pos = title_end;  // Move forward in the data string
//  }
//}
//
//
//
void parse_JSON(const char *data, char *output_buffer, size_t buffer_size) {
  // Find the beginning of the JSON data by locating the first '{'
  const char *json_start = strchr(data, '{');
  if (json_start == NULL) {
      snprintf(output_buffer, buffer_size, "No JSON data found.\n");
      return;
  }

  const char *title_key = "\"title\":\"";
  char title[256];
  size_t buffer_used = 0;

  const char *title_pos = json_start;

  // Loop to extract all titles from the JSON
  while ((title_pos = strstr(title_pos, title_key)) != NULL) {
      title_pos += strlen(title_key);  // Move past the "title":" key
      const char *title_end = strchr(title_pos, '"');  // Find the end of the title
      if (title_end) {
          size_t title_length = title_end - title_pos;
          strncpy(title, title_pos, title_length);
          title[title_length] = '\0';  // Null-terminate the string

          // Format and store the extracted title into the output buffer
          int chars_written = snprintf(output_buffer + buffer_used, buffer_size - buffer_used,
                                       "Title: %s\n", title);

          // Check for buffer overflow
          if (chars_written < 0 || (size_t)chars_written >= buffer_size - buffer_used) {
              // Truncate and break if the buffer is full
              snprintf(output_buffer + buffer_used, buffer_size - buffer_used, "Buffer full\n");
              break;
          }
          buffer_used += chars_written;
      }
      title_pos = title_end;  // Move forward in the data string
  }
}
