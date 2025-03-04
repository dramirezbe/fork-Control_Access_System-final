#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "ring_buffer.h"


// Definiciones para el sistema



// Enumeración para los estados de la cerradura
typedef enum {
  STATE_CERRADO,
  STATE_ABIERTO_TEMPORAL,
  STATE_ABIERTO_PERMANENTE
} door_state_t;

// Declaración de variables globales (si es necesario que sean accedidas en otros módulos)
extern door_state_t door_state;
extern uint32_t door_timer;
extern char key_buffer[KEYPAD_BUFFER_SIZE];
extern uint8_t key_index;

// Prototipos de funciones públicas
void system_state_machine(void);
void system_events_handler_button(uint8_t press_count);
void system_events_handler_key(uint8_t key);
void system_events_handler_cmd(const char* cmd);
void process_uart_commands(ring_buffer_t *rb);
void heartbeat(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_H
