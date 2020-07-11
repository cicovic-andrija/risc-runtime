/* File: devices.h */
/* Peripheral devices emulation. */

#ifndef DEVICES_H
#define DEVICES_H

#define INPUT_BUFFER_SIZE 1024

/* Function signal_output_device sends a byte to the output device if
 * any data was written to memory address OUTPUT_DEVICE_ADDRESS. */
void signal_output_device(void);

/* Function poll_input_device checks with the input device if new byte
 * is ready at the input, to be stored at memory addresss INPUT_DEVICE_ADDRESS. */
void poll_input_device(void);

/* Function init_time initializes CPU timer. */
void init_timer(void);

/* Function poll_timer checks with the timer
 * if TIMER_PERIOD_IN_SEC seconds has passed. */
void poll_timer(void);

#endif /* DEVICES_H */


