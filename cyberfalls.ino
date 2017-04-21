#include <Adafruit_NeoPixel.h>

#define N_LEDS 12         // Per strip
#define N_STRIPS 2        // How many strips total?
#define DT 35             // Milliseconds
#define MIN_PIN 2         // The starting pin number on the board. Add the strips consecutively from this one.
#define FALL_LENGTH 100.0 // Percent of the strip.


volatile struct {
  float x;        // The x-position of the drop.
  uint8_t pin;    // I can't find 0 on my board.
  bool falling;   // Whether or not a drop is currently falling.
} drops[N_STRIPS];

// It's possible for an interrupt to fire when we're handling another already. Just return.
int locked = false;

// NeoPixels!
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, 0);

void setup() {
  // Initialize NeoPixel Strip API
  strip.begin();

  // We use random numbers to choose strands to drop
  randomSeed(analogRead(1));
  
  // initialize Timer1
  cli();         // disable global interrupts
  TCCR1A = 0;    // set entire TCCR1A register to 0
  TCCR1B = 0;    // set entire TCCR1A register to 0
  
  // enable Timer1 overflow interrupt:
  bitSet(TIMSK1, TOIE1);
  /*
    16000000 / 1024 = 15625.0 Hz
    15625 / 1 = 15625
    65536 - 15625 = 49911

    where:
    - 16000000 is the main clock frequency in Hz
    - 1024 is the prescaler that we have set
    - 1 is the frequency that we want in Hz
    - 65536 is the maximum timer value
  */
  TCNT1 = 49911;

  // set 1024 prescaler
  bitSet(TCCR1B, CS12);
  bitSet(TCCR1B, CS10);

  sei(); // enable all interrupts

  for (uint16_t drop_no=0; drop_no < N_STRIPS; drop_no++){
    drops[drop_no].pin = drop_no + MIN_PIN;
    drops[drop_no].falling = false;
    drops[drop_no].x = -random(0,50);
  }
}

void select_active_drops() {
  /*
   * Chooses to drop something or return. Then decides which strips to drop,
   * Then initializes the fall by setting "falling" to true on the state of 
   * the drop.
   */
  if (random(0, 10) > 3) return;
  for (uint16_t drop_no=0; drop_no<N_STRIPS; drop_no++) {
    if (random(0, 10) > 2) {
      uint16_t active_drop_no = random(0, N_STRIPS);
      drops[active_drop_no].falling = true;
    }
  }
}

// Timer 1 overflow Interrupt Service Routine.
ISR(TIMER1_OVF_vect) {
  TCNT1 = 49911; // This is required to properly time the next signal.

  // We may already be dropping something. 
  if (locked) return;
  
  locked = true;         // Lock
  select_active_drops(); // Choose what to drop. 
  fall();                // Drop it!
  locked = false;        // Unlock
}

float distance_to_intensity(float distance) {
  /*
   * We model a drop that has some "glow" which is displayed by the
   * pixels in the strand. This determines how fast the intensity of 
   * the glow drops off. 
   * 
   * Boundary conditions are important here. When the distance is 0
   * the intensity must be 255. When the distance is infinite the 
   * intensity should be zero.
   */
  return 255.0 / pow(1.25, distance);
}

void illuminate(uint16_t drop_no) {
  /*
   * This method illuminates a drop in it's current state. It computes
   * the distance from the drop to each LED on the strand, sets the 
   * pixel intensity of every pixel on the strand to the correct value,
   * then displays the pixels at that intensity.
   * 
   * This method is also responsible for setting the pin to the correct 
   * pin for the strand.
   * 
   * :param uint15_t drop_no: The drop to illuminate.
   */
  strip.setPin(drops[drop_no].pin);
  for (uint16_t i = 0; i < N_LEDS; i++) {
    float x_LED = float(FALL_LENGTH) * float(i) / float(N_LEDS);
    float distance = abs(x_LED - float(drops[drop_no].x));
    int intensity = int(distance_to_intensity(distance));
    strip.setPixelColor(i, intensity, intensity, intensity);
  }
  strip.show();
}


void extinguish(uint16_t drop_no) {
  /*
   * This method "puts out" the strand after a drop has fallen. It selects
   * the correct pin for the drop, sets all the color values to "black", 
   * then indicates the drop is no longer falling. It also re-initializes
   * the drop for the next fall by setting the x-value of the drop to 0
   * 
   * :param uint15_t drop_no: The drop to extinguish.
   */
  strip.setPin(drops[drop_no].pin);
  for (uint16_t i = 0; i < N_LEDS; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
  drops[drop_no].falling = false;
  drops[drop_no].x = -random(0, 50);
}

bool still_falling() {
  /*
   * This method indicates whether or not ANY drop is currently falling.
   */
  for (uint16_t drop_no=0; drop_no<N_STRIPS; drop_no++) {
    if (drops[drop_no].falling && drops[drop_no].x < FALL_LENGTH) return true;
  }
  return false;
}

static void fall() {
  /*
   * This method handles a quadratic fall. There are many parameters to tune,
   * they're chosen to look appealing. Some of the choices are:
   * 
   *  1. t=0.5 causes the drop to already be falling rather than starting off 
   *     at a stop, which appears choppy.
   *  2. DT=35ms is a nice frame rate. Efficient and looks clean.
   *  3. t+=0.05 makes the drop fall pretty fast, but changes incrementally 
   *     enough that it looks smooth.
   *  
   * To find the falling drops we cycle through all strands and check to see
   * if their state is set to falling=true. They fall until the total FALL_LENGTH
   * is less than the drop's x-position. At that point the drop is extinguished.
   */
  float t = 0.5;
  while (still_falling()) {
    for (uint16_t drop_no=0; drop_no<N_STRIPS; drop_no++) {
      if (drops[drop_no].falling) {
        drops[drop_no].x = drops[drop_no].x + 5. * pow(t, 2.);
        illuminate(drop_no);
        if (drops[drop_no].x > FALL_LENGTH) extinguish(drop_no);
      }
    }
    t += 0.05;
    delay(DT);
  }
}

void loop() {} // We're using interrupts instead of looping.
