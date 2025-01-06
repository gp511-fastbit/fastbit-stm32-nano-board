/*
 * app.h
 *
 *  Created on: Oct 19, 2024
 *      Author: Shreyas Acharya, BHRATI SOFTWARE
 */

#ifndef INC_APP_H_
#define INC_APP_H_
#include <stdint.h>

typedef enum {
  home,
  us,
  science,
  arts,
  world
} MenuState;

void sh1106_scroll_text_left(const char* text, uint16_t delay_ms);
void update_home_page();
void main_app();

#endif /* INC_APP_H_ */
