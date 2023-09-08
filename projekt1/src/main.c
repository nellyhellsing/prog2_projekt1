/*********************************************************************************
 * Fyra lysdioder är anslutna till pin 17, 22, 23 och 24. Samt två tryckknappar
 * till pin 25 och 27. Varje tryckknapp styr två lysdioder via var sin tråd, där
 * lysdioderna togglas mellan att blinka och vara släkta vid stigande flank.
 * 
 * Vid varje toggling av lysdioderna sker en utskrift i terminalen gällande vilka
 * lysdioder som tänds/släcks.
*********************************************************************************/

#include <gpiod_utils.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>

struct thread_args
{
    struct gpiod_line* led_t1;
    struct gpiod_line* led_t2;
    struct gpiod_line* button;
    uint16_t blink_speed_ms;
    uint8_t last_input_value;
    bool enabled;
};

void* run_thread(void* arg)
{
    struct thread_args* self = (struct thread_args*)(arg);
    while(1)
    {
        
        if (gpiod_line_event_detected(self->button, GPIOD_LINE_EDGE_RISING, &self->last_input_value))
        {
            self->enabled = !self->enabled;
            semaphore_reserve(0);
            if(self->enabled)
            {
                printf("LEDs connected to pin %hu and %hu now enabled!\n", 
                gpiod_line_offset(self->led_t1),gpiod_line_offset(self->led_t2));
            }
            else
            {
               printf("LEDs connected to pin %hu and %hu now disable!\n", 
               gpiod_line_offset(self->led_t1),gpiod_line_offset(self->led_t2)); 
            }
            delay_ms(10);
            semaphore_release(0);
        
        }
        
        if (self->enabled)
        {
            gpiod_line_set_value(self->led_t1, 1);
            gpiod_line_set_value(self->led_t2, 0);
            delay_ms(self->blink_speed_ms);
            gpiod_line_set_value(self->led_t1, 0);
            gpiod_line_set_value(self->led_t2, 1);
            delay_ms(self->blink_speed_ms);

        }
        else
        {
            gpiod_line_set_value(self->led_t1, 0);
            gpiod_line_set_value(self->led_t2, 0);
        }
        
    }
    return 0;
}

int main(void)
{
    struct gpiod_line* led1 = gpiod_line_new(17, GPIOD_LINE_DIRECTION_OUT);
    struct gpiod_line* led2 = gpiod_line_new(22, GPIOD_LINE_DIRECTION_OUT);
    struct gpiod_line* led3 = gpiod_line_new(23, GPIOD_LINE_DIRECTION_OUT);
    struct gpiod_line* led4 = gpiod_line_new(24, GPIOD_LINE_DIRECTION_OUT);
    struct gpiod_line* button1 = gpiod_line_new(25, GPIOD_LINE_DIRECTION_IN);
    struct gpiod_line* button2 = gpiod_line_new(27, GPIOD_LINE_DIRECTION_IN);

    struct thread_args args1 = {led1, led2, button1, 100, 0, false };
    struct thread_args args2 = {led3, led4, button2, 500, 0, false };
    pthread_t t1, t2;

    pthread_create(&t1, 0, run_thread, &args1);
    pthread_create(&t2, 0, run_thread, &args2);
    pthread_join(t1, 0);
    pthread_join(t2, 0);
    return 0;
}