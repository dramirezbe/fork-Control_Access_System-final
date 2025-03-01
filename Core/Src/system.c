#include "system.h"

/**
* @brief Actualiza el estado de la cerradura (LED externo) según la máquina de estados.
*/
void system_state_machine(void)
{
  switch (door_state)
  {
    case STATE_CERRADO:
      HAL_GPIO_WritePin(DOOR_STATUS_GPIO_Port, DOOR_STATUS_Pin, GPIO_PIN_RESET);
      break;
    case STATE_ABIERTO_TEMPORAL:
      HAL_GPIO_WritePin(DOOR_STATUS_GPIO_Port, DOOR_STATUS_Pin, GPIO_PIN_SET);
      // Si han pasado más de 5 segundos, se cierra la puerta
      if (HAL_GetTick() - door_timer > 5000) {
          door_state = STATE_CERRADO;
          printf("Tiempo expirado: Puerta cerrada\r\n");
      }
      break;
    case STATE_ABIERTO_PERMANENTE:
      HAL_GPIO_WritePin(DOOR_STATUS_GPIO_Port, DOOR_STATUS_Pin, GPIO_PIN_SET);
      break;
    default:
      break;
  }
}

/**
* @brief Maneja los eventos provenientes del botón físico (B1).
*        - Una presión simple: si la puerta está cerrada, se abre temporalmente.
*          Si está en modo permanente, se cierra.
*        - Doble presión: se abre en modo permanente.
*/
void system_events_handler_button(uint8_t press_count)
{
  if (press_count == 1) {
    if (door_state == STATE_CERRADO) {
      door_state = STATE_ABIERTO_TEMPORAL;
      door_timer = HAL_GetTick();
      printf("Puerta abierta temporalmente (botón interior)\r\n");
    } else if (door_state == STATE_ABIERTO_PERMANENTE) {
      door_state = STATE_CERRADO;
      printf("Puerta cerrada (botón interior en modo permanente)\r\n");
    }
  } else if (press_count == 2) {
    door_state = STATE_ABIERTO_PERMANENTE;
    printf("Puerta abierta permanentemente (doble presión del botón)\r\n");
  }
}

/**
* @brief Maneja la entrada del teclado hexadecimal.
* Acumula las teclas y, al detectar la tecla terminadora ('#') o haberse alcanzado
* el tamaño máximo, valida la secuencia como contraseña o comando especial.
*/
void system_events_handler_key(uint8_t key)
{
  // Se asume que los caracteres válidos son dígitos, letras (para hexadecimal),
  // y los caracteres especiales '#' y '*'
  if(key_index < KEYPAD_BUFFER_SIZE - 1) {
      key_buffer[key_index++] = key;
  }

  // Se usa el carácter '#' como terminador o si se ha llenado el buffer
  if (key == '#' || key_index >= KEYPAD_BUFFER_SIZE - 1) {
      key_buffer[key_index] = '\0';

      // Comandos de depuración tienen formato "#*O*#" y "#*C*#"
      if(strcmp(key_buffer, "#*O*#") == 0) {
          door_state = STATE_ABIERTO_PERMANENTE;
          printf("Debug: Abrir cerradura (permanente) via Teclado\r\n");
      } else if(strcmp(key_buffer, "#*C*#") == 0) {
          door_state = STATE_CERRADO;
          printf("Debug: Cerrar cerradura via Teclado\r\n");
      } else {
          // Validar contraseña
          if(strcmp(key_buffer, PASSWORD) == 0) {
              door_state = STATE_ABIERTO_TEMPORAL;
              door_timer = HAL_GetTick();
              printf("Acceso concedido: Puerta abierta temporalmente via Teclado\r\n");
          } else {
              printf("Acceso denegado: Clave incorrecta via Teclado\r\n");
          }
      }
      // Reiniciar el buffer
      key_index = 0;
      memset(key_buffer, 0, sizeof(key_buffer));
  }
}

/**
 * @brief Maneja comandos recibidos vía UART (desde PC host o ESP01).
 * Los comandos pueden ser la contraseña o los comandos especiales de depuración.
 */
void system_events_handler_cmd(const char* cmd)
{
  if(strcmp(cmd, "#*O*#") == 0) {
      door_state = STATE_ABIERTO_PERMANENTE;
      printf("Debug: Abrir cerradura (permanente) via UART\r\n");
  } else if(strcmp(cmd, "#*C*#") == 0) {
      door_state = STATE_CERRADO;
      printf("Debug: Cerrar cerradura via UART\r\n");
  } else {
      // Se asume que cualquier otro comando es una posible contraseña
      if(strcmp(cmd, PASSWORD) == 0) {
          door_state = STATE_ABIERTO_TEMPORAL;
          door_timer = HAL_GetTick();
          printf("Acceso concedido: Puerta abierta temporalmente via UART\r\n");
      } else {
          printf("Acceso denegado: Comando desconocido via UART\r\n");
      }
  }
}

void process_uart_commands(ring_buffer_t *rb)
{
  char cmd[32];
  uint8_t i = 0;
  while(ring_buffer_size(rb) > 0) {
      uint8_t ch;
      ring_buffer_read(rb, &ch);
      if(ch == '\r' || ch == '\n') {
          if(i > 0) {
              cmd[i] = '\0';
              system_events_handler_cmd(cmd);
              i = 0;
          }
      } else {
          if(i < sizeof(cmd) - 1) {
              cmd[i++] = ch;
          }
      }
  }
}

/**
 * @brief  Heartbeat function to blink LED2 every 1 second to indicate the system is running
*/
void heartbeat(void)
{
  static uint32_t last_heartbeat = 0;
  if (HAL_GetTick() - last_heartbeat > 1000)
  {
    last_heartbeat = HAL_GetTick();
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}