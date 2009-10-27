#ifndef EEPROM_H
#define EEPROM_H
#include <avr/eeprom.h>

#define KEYMAP_POINTER_FROM_INDEX(I)  ((uint8_t *)0+(((I)-1)*sizeof(Storedmap)))
#define KEYMAP_POINTER_NULL           ((void *)(E2END+1))

#ifndef MAXIMUM_KEYMAP_INDEX
#define MAXIMUM_KEYMAP_INDEX ((E2END+1)/sizeof(Storedmap))
#endif /*! MAXIMUM_KEYMAP_INDEX */
#endif /* !EEPROM_H */
