#include <Arduboy2.h>

#include "Size.h"

Arduboy2 arduboy;

const char exampleA[8] PROGMEM {};
const int exampleB[4] PROGMEM {};
const uint8_t exampleC[10] PROGMEM {};

void setup()
{
	arduboy.begin();
}

void loop()
{
	if(!arduboy.nextFrame())
		return;
	
	arduboy.clear();
	
	arduboy.print(F("exampleA: "));
	arduboy.println(getSize(exampleA));
	
	arduboy.print(F("exampleB: "));
	arduboy.println(getSize(exampleB));
	
	arduboy.print(F("exampleC: "));
	arduboy.println(getSize(exampleC));
	
	arduboy.display();
}
