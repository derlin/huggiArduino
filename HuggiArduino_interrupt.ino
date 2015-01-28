/**
 * @file    HuggiArduino_interrupt.ino
 * @author  Lucy Linder (lucy.derlin@gmail.com)
 * @date    September, 2014
 * @brief   Allow the Arduino to be put to sleep and woken up using interrupts.
 *
 * The interruptions that wake up the device can be either from the hardware serial 
 * interface (receiving a char from bluetooth), or from a pressure (if the wiring
 * if properly done).
 */
#include <avr/power.h>
#include <avr/sleep.h>

volatile bool triggered = false;


ISR (ANALOG_COMP_vect)
{
  triggered = true;
}

/**
 * Allow the device to be woken up by a pressure on the sensor.
 * Should be called before entering sleep.
 * 
 * Note: The Atmega328 chip has a built-in analog comparator. 
 * That is, it can trigger an interrupt when an incoming voltage passes a threshold.
 * To allow the pressure interrupt, the pin 7 should be
 * wired to the reference voltage, while the pin 6 should be
 * wired to one of the pressure analog input.
 * 
 */
void enablePressureInterrupt()
{
    pinMode(6, INPUT_PULLUP);
    ADCSRB = 0;         // (Disable) ACME: Analog Comparator Multiplexer Enable
    ACSR =  _BV (ACI)   // (Clear) Analog Comparator Interrupt Flag
        | _BV (ACIE)    // Analog Comparator Interrupt Enable
        | _BV (ACIS1);  // ACIS1, ACIS0: Analog Comparator Interrupt Mode Select (trigger on falling edge)
    //ACSR &= ~3; // for interrupt on output toggle
}

/**
 * Disable the pressure interrupt, i.e. turn off the analog comparator.
 * This should be called right after waking up to save power.
 */
void disablePressureInterrupt()
{
    ACSR &= ~ _BV(ACIE);
}

/**
 * Put the Arduino on SLEEP_MODE_IDLE, i.e. a sleep mode
 * where only the necessary stuff is kept on.
 *
 * The wake up will happen either by a char received on the
 * hardware serial interface or by a pressure on the sensor,
 * if the latter is properly wired.
 */
void enterSleep()
{
  delay(100); // let the serial finish its job
  set_sleep_mode(SLEEP_MODE_IDLE); // set the kind of sleep mode
  sleep_enable();                  // enable the sleep bit in the mcucr register
                                   // so sleep is possible (just a safety pin)  
  enablePressureInterrupt();       // enable the analog comparator
  
  // power off everything we can
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();
  

  sleep_mode();      // actually put the device to sleep
                     // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  sleep_disable();   // first thing after waking up: disable sleep...

  power_all_enable();         // turn everything on
  disablePressureInterrupt(); // disable the analog comparator

}