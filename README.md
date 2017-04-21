# Cyberfalls

## Introduction

Well there's not really much to say about this... The cyberfalls wig implementation on the adafruit site didn't work for me. I like to use the Arduino Pro Mini because it's a nice tradeoff between size and power. 

This implementation sets state correctly for interrupt handling and the like. You can check out some of the tutorials I used below.

 * [Timer Overflow Interrupts](http://www.keychainino.com/how-timer-overflow-interrupt-works/)
 * [Timer Overflow Interrupts 2](https://arduino-info.wikispaces.com/Timers-Arduino)
 * [Brushup on structs in C](https://en.wikipedia.org/wiki/Struct_(C_programming_language))
 * [Brushup on arrays in C](https://www.tutorialspoint.com/cprogramming/c_arrays.htm)
 * [Arduino Language Reference](https://www.arduino.cc/en/Reference/HomePage)
 * [NeoPixel Library Reference](https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library)
 * [Strandtest](https://learn.adafruit.com/neopixel-painter/test-neopixel-strip)
 * [The Original Code](https://learn.adafruit.com/neopixel-cyber-falls/the-code)

## Notes

I had issues with the original code where various registers were named differently and it didn't work as expected when I attempted to just rename them. 

There are a few gotchas I learned about over the course of this... Namely the Adafruit\_Neopixel API only requires _one_ instantiation. You can access every strip with one instance of that class. There is no reason to use many of them, and in fact the code simply will not work. 

When using structs it's better to keep them as simple as possible. IMHO if you can keep them pared down to just primitives it's simpler to manage things.
