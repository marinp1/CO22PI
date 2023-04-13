#define INIT_LED            \
	{                       \
		pinMode(2, OUTPUT); \
	}
#define LED_ON                \
	{                         \
		digitalWrite(2, LOW); \
	}
#define LED_OFF                \
	{                          \
		digitalWrite(2, HIGH); \
	}

// ------------------------
void setup()
{
	Serial.begin(9600);

	LED_ON;

	// Wait for USB Serial
	while (!Serial)
	{
		yield();
	}
}

// ------------------------
void loop()
{
	//
}

void blink()
{
	LED_ON;
	delay(50);
	LED_OFF;
	delay(50);
}
