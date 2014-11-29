
#define LED_STRIP_PIN 3
#define NB_LEDS 3
#define LEDS_DEFAULT_BRIGHTNESS 50


#define GLOBAL_STATE_LED     0
#define SPECIFIC_STATE_LED   1
#define BUFF_FULL_LED        2

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NB_LEDS, LED_STRIP_PIN);

void ledSetup()
{
  strip.begin();
  strip.setBrightness(LEDS_DEFAULT_BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'
}


void ledSetColor(uint32_t color)
{
    strip.setPixelColor(0, color);
    strip.show();
}

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
    

    ledSetColor(cur_color);
}