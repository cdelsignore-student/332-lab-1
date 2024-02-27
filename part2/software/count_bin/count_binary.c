#include "alt_types.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "system.h"
#include <stdio.h>
#include <unistd.h>

#define STUDENT_ID_1 30800923                                                   // INT      used in determining max count
#define STUDENT_ID_2 33891211                                                   // INT      used in determining max count
#define LOOP_TIME_SEC 60                                                        // INT      target time in seconds for count to reach max
#define DELAY_PRESCALER 3.5                                                     // FLOAT    divisor used to calibrate DE1_SoC sleep timing

static alt_u8 count;                                                            // Holds the current count
volatile int edge_capture;                                                      // Holds the value of the button pio edge capture reg

#ifdef BUTTON_PIO_BASE
    // =======================================================================
    // static void handle_button_interrupts(void* context, alt_u32 id)
    // 
    // Sets *context to the value in the button edge capture register and then
    // clears the reg to continue normal program execution
    // =======================================================================
    #ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
        static void handle_button_interrupts(void* context)                     // For enchanced interrupts
    #else
        static void handle_button_interrupts(void* context, alt_u32 id)         // For legacy interrupts
    #endif
    {
        volatile int* edge_capture_ptr = (volatile int*) context;               // Volatile to avoid compiler optimization
        *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);   // Store value from button edge capture reg
        IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0);                    // Clear button edge capture reg
        IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);                       // Read PIO to delay ISR exit and prevent spurious interrupt
    }

    // =======================================================================
    // static void init_button_pio()
    // 
    // Initializes button PIO and interrupts for the three input switches
    // =======================================================================
    static void init_button_pio()
    {
        void* edge_capture_ptr = (void*) &edge_capture;                         // Cast to match alt_irq_register() function
        IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_BASE, 0x7);                  // Enable button interrupts.
        IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0x0);                  // Clear button edge capture reg

        // Register the interrupt handler.
        #ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
            alt_ic_isr_register(BUTTON_PIO_IRQ_INTERRUPT_CONTROLLER_ID,         // For enchanced interrupts
                BUTTON_PIO_IRQ, handle_button_interrupts, edge_capture_ptr, 0x0);
        #else
            alt_irq_register( BUTTON_PIO_IRQ, edge_capture_ptr,                 // For legacy interrupts
                handle_button_interrupts);
        #endif
    }
#endif

#ifdef SEVEN_SEG_PIO_BASE
    // =======================================================================
    // static void sevenseg_set_dec(int dec) 
    // 
    // Displays dec on the 2-digit seven segment display                 
    // =======================================================================
    static void sevenseg_set_dec(int dec)
    {
        static alt_u8 segments[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66,            // Bit representations of numbers for 7 segment display
                                       0x6D, 0x7D, 0x07, 0x7F, 0x6F };          // 0-9, indexed respectively

        unsigned int data = (segments[dec%10] | segments[dec/10] << 8);         // Packs [Ones][Tens] bit information (note integer division)
        IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_PIO_BASE, data);                  // Pushes data to the seven segment PIO
    }
#endif

// ===========================================================================
// static void initial_message(int max, unsigned int sec, unsigned int usec)
// 
// Prints some information to the console at the start of a counting loop
// ===========================================================================
static void initial_message(int max, unsigned int sec, unsigned int usec)
{
    printf("\n\n*****************************\n");
    printf("* Hello from Nios II!       *\n");
    printf("* Counting from 00 to %d    *\n", max);
    printf("* With %u sec %u usec delay *\n", sec, usec);
    printf("*****************************\n");
}

// ===========================================================================
// static void show_count_led()
// 
// Displays the current count in binary represenatation on board's led strip
// ===========================================================================
static void show_count_led()
{
#ifdef LED_PIO_BASE
    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, count);                           // Push data to LED PIO
#endif
}

// ===========================================================================
// static void show_count_sevenseg()
// 
// Displays the current count in decimal represenatation on board's first two
// seven segment displays 
// ===========================================================================
static void show_count_sevenseg()
{
#ifdef SEVEN_SEG_PIO_BASE
    sevenseg_set_dec(count);
#endif
}

// ===========================================================================
// static void show_count_all()
// 
// Displays the current count on both led strip and seven segment displays
// ===========================================================================
static void show_count_all()
{
    show_count_led();
    show_count_sevenseg();
    printf("%d,  ", count);
}

// ===========================================================================
// static void handle_button_press()
// 
// Selects appropriate display output based on switch selection
// ===========================================================================
static void handle_button_press()
{
    switch (edge_capture) 
    {
    case 0x1:                                                                   // Button 1 is switched
        show_count_led();                                                       // Show on LED strip
        break;
    case 0x2:                                                                   // Button 2 is switched
        show_count_sevenseg();                                                  // Show on seven segment display
        break;
    case 0x4:                                                                   // Button 3 is switched
        show_count_all();                                                       // Show on both
        break;
    default:                                                                    // Other (shouldn't occur)
        show_count_all();                                                       // Show on both
        break;
    }
}

// ===========================================================================
// int calculate_count_max(int num1, int num2)
// 
// Returns the sum of all digits in num1 and num2
// ===========================================================================
int calculate_count_max(int num1, int num2)
{
    int sum = 0;

    while(num1 != 0 || num2 != 0){                                              // While there are still digits left
        sum += (num1 % 10) + (num2 % 10);                                       // Get the digits and add to sum
        num1 /= 10;                                                             // Move to next digit
        num2 /= 10;                                                             // Move to next digit
    }
    return sum;
}

// ===========================================================================
// void calculate_usleep(int window_sec, int countMax, float prescaler, unsigned int *sec, unsigned int *usec)
// 
// Calculates the delay needed to reach countMax in window_sec seconds based on
// a calibration prescaler. Returns as two parts: a full-second delay and a
// fractional second delay in microseconds. This is to overcome the 1s max on
// usleep()
// ===========================================================================
void calculate_usleep(int window_sec, int countMax, float prescaler, unsigned int *sec, unsigned int *usec)
{
    unsigned int utime = window_sec*1000000;                                    // Convert to microseconds
    unsigned int usleepInterval = utime/countMax/prescaler;                     // Calculate target interval
    while(usleepInterval >= 1000000){                                           // Determine full second part
        *sec += 1;
        usleepInterval -= 1000000;
    }
    *usec = usleepInterval;                                                     // Remainder in microseconds
}

// ===========================================================================
// void do_sleep(unsigned int sec, unsigned usec)
// 
// Wrapper function for usleep() that allows for delays greater than 1 second
// ===========================================================================
void do_sleep(unsigned int sec, unsigned usec)
{
    for(int i = 0; i < sec; i++) {                                              // Delay for the full second portion
        usleep(1000000);
    }
    usleep(usec);                                                               // Delay for the remaining microseconds
}

// ===========================================================================
// int main(void)
// 
// Continuously counts from 0 to an arbitrary maximum. Selectable output based
// on DE1_SoC switches. Count is displayed on STDOUT at all times.
//
// SW0 - LED strip shows count in binary
// SW1 - First two seven segment displays show count in decimal
// SW2 - Both devices display count
// ===========================================================================
int main(void)
{ 
    int i;                                                                      // Iterator (I assume declared here for optimization)
    int  __attribute__ ((unused))  wait_time;                                   // Attribute suppresses "var set but not used" warning.
    int maxCount = calculate_count_max(STUDENT_ID_1, STUDENT_ID_2);             // Target number for counting
    unsigned int sec = 0;                                                       // Full second portion of needed delay interval
    unsigned int usec = 0;                                                      // Microsecond portion of needed delay interval
    
    #ifdef BUTTON_PIO_BASE                                                      
        init_button_pio();                                                      // If the board has button PIO, initialize
    #endif
 
    calculate_usleep(LOOP_TIME_SEC, maxCount, DELAY_PRESCALER, &sec, &usec);    // Calculate delays
    initial_message(maxCount, sec, usec);                                       // Print debug info to the console

    count = 0;                                                                  // Initialize the count at 0
    while( 1 )                                                                  // Loop counting from 0 to maxCount forever
    {
        do_sleep(sec, usec);                                                    // Wait based on calculated interval

        // If there is a button press handle it, otherwise attempt to show all
        if (edge_capture != 0){
            handle_button_press();
        } else {
            show_count_all();
        }

        // If we have reached the final number
        if( count >= maxCount )
        {
            edge_capture = 0;                                                   // Clear the capture value
            initial_message(maxCount, sec, usec);                               // Print debug info to the console
            count = 0;                                                          // Reset the count
        }
        count++;                                                                // Increment the count
    }

    return 0;                                                                   // Should never reach here
}