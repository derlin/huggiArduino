/**
 * @file    HuggiArduino_leds.ino
 * @author  Lucy Linder (lucy.derlin@gmail.com)
 * @date    September, 2014
 * @brief   utilities to deal with the NeoPixels
 */

#define LED_STRIP_PIN 3   //<! Digital pin used as output in the Arduino.
#define NB_LEDS       1   //<! Number of leds in the strip.
#define LEDS_DEFAULT_BRIGHTNESS 50  //<! Brightness is 50%.


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NB_LEDS, LED_STRIP_PIN); //<! The NeoPixel controller.

/**
 * Initialise the led strip. Should be called in the setup.
 */
void ledSetup()
{
  strip.begin();
  strip.setBrightness(LEDS_DEFAULT_BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'
}

/**
 * Set the color of the led. Use 0 to turn it off.
 * 
 * @param color The color, in hexadecimal
 */
void ledSetColor(uint32_t color)
{
    strip.setPixelColor(0, color);
    strip.show();
}

/**
 * Make the led blink at a frequency of 300ms,
 * 
 * @param color The color
 * @param times The number of blinks
 */
void ledBlink(uint32_t color, byte times)
{
    uint32_t cur_color = strip.getPixelColor(0);

    while(times-- > 0)
    {
        ledSetColor(color);
        delay(300);
        ledSetColor(0);
        delay(200);

    }
    
    // set back the previous color
    ledSetColor(cur_color);
}