/**
 * @file ring_buffer.c
 * @brief Circular buffer (ring buffer) implementation with overwrite capability
 * @author dramirezbe
 * @date Feb 11, 2025
 * 
 * This implementation overwrites the oldest data when writing to a full buffer.
 * Suitable for embedded systems using STM32 HAL.
 */

 #include "ring_buffer.h"
 #include <stdint.h>
 
 /**
  * @brief Initialize ring buffer structure
  * @param rb        Pointer to ring buffer instance
  * @param mem_add   Memory address for buffer storage
  * @param capacity  Maximum number of elements the buffer can hold
  */
 void ring_buffer_init(ring_buffer_t *rb, uint8_t *mem_add, uint8_t capacity) {
     rb->buffer   = mem_add;
     rb->capacity = capacity;
     rb->head     = 0;
     rb->tail     = 0;
     rb->is_full  = 0;
 }
 
 /**
  * @brief Reset buffer to empty state
  * @param rb Pointer to ring buffer instance
  */
 void ring_buffer_reset(ring_buffer_t *rb) {
     rb->head    = 0;
     rb->tail    = 0;
     rb->is_full = 0;
 }
 
 /**
  * @brief Calculate current number of elements in buffer
  * @param rb Pointer to ring buffer instance
  * @return Number of elements stored in buffer
  */
 uint8_t ring_buffer_size(ring_buffer_t *rb) {
     if (rb->is_full) {
         return rb->capacity;
     }
     
     return (rb->head >= rb->tail) ? (rb->head - rb->tail) 
                                  : (rb->capacity - rb->tail + rb->head);
 }
 
 /**
  * @brief Check if buffer is full
  * @param rb Pointer to ring buffer instance
  * @return 1 if full, 0 otherwise
  */
 uint8_t ring_buffer_is_full(ring_buffer_t *rb) {
     return rb->is_full;
 }
 
 /**
  * @brief Check if buffer is empty
  * @param rb Pointer to ring buffer instance
  * @return 1 if empty, 0 otherwise
  */
 uint8_t ring_buffer_is_empty(ring_buffer_t *rb) {
     return (!rb->is_full && (rb->head == rb->tail));
 }
 
 /**
  * @brief Write data to buffer (overwrites oldest data if full)
  * @param rb    Pointer to ring buffer instance
  * @param data  Byte to write to buffer
  * 
  * @note When buffer is full, writing new data will:
  *       1. Overwrite the oldest data
  *       2. Advance both head and tail pointers
  *       3. Maintain full state
  */
 void ring_buffer_write(ring_buffer_t *rb, uint8_t data) {
     rb->buffer[rb->head] = data;
     rb->head = (rb->head + 1) % rb->capacity;
 
     if (rb->is_full) {
         rb->tail = (rb->tail + 1) % rb->capacity;
     }
 
     rb->is_full = (rb->head == rb->tail);
 }
 
 /**
  * @brief Read data from buffer
  * @param rb    Pointer to ring buffer instance
  * @param byte  Pointer to store read byte
  * @return 1 if read successful, 0 if buffer empty
  * 
  * @note Reading from buffer will:
  *       1. Clear full status if buffer was full
  *       2. Advance tail pointer
  *       3. Preserve data until overwritten
  */
 uint8_t ring_buffer_read(ring_buffer_t *rb, uint8_t *byte) {
     if (ring_buffer_is_empty(rb)) {
         return 0;
     }
 
     *byte = rb->buffer[rb->tail];
     rb->tail = (rb->tail + 1) % rb->capacity;
     rb->is_full = 0;
     return 1;
 }