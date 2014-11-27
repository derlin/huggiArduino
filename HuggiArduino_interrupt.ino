
#include <avr/power.h>
#include <avr/sleep.h>

volatile bool triggered = false;


ISR (ANALOG_COMP_vect)
{
  triggered = true;
}


void enablePressureInterrupt()
{
    pinMode(6, INPUT_PULLUP);
    ADCSRB = 0;           // (Disable) ACME: Analog Comparator Multiplexer Enable
    ACSR =  _BV (ACI)     // (Clear) Analog Comparator Interrupt Flag
        | _BV (ACIE)    // Analog Comparator Interrupt Enable
        | _BV (ACIS1);  // ACIS1, ACIS0: Analog Comparator Interrupt Mode Select (trigger on falling edge)
    //ACSR &= ~3; // interrupt on output toggle
}


void disablePressureInterrupt()
{
    ACSR &= ~ _BV(ACIE);
}


void enterSleep()
{
  delay(100); // let the serial finish its job
  set_sleep_mode(SLEEP_MODE_IDLE);   // sleep mode is set here
  sleep_enable();          // enables the sleep bit in the mcucr register
                             // so sleep is possible. just a safety pin  
  enablePressureInterrupt();
  
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();
  

  sleep_mode();            // here the device is actually put to sleep!!
  
                             // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  sleep_disable();         // first thing after waking from sleep:
                            // disable sleep...

  power_all_enable();
  disablePressureInterrupt();

}