#include <SPI.h>

// Pins for ESP8266
const int FSYNC = 15; // D8 on NodeMCU

// Constants for AD9833
#define WAVE_SINE     0x2000
#define WAVE_TRIANGLE 0x2002
#define WAVE_SQUARE   0x2028
#define B28_BIT       0x2000 // Control bit for 28-bit frequency update

void setup() {
  pinMode(FSYNC, OUTPUT);
  digitalWrite(FSYNC, HIGH);

  // Initialize Hardware SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE2); // AD9833 uses SPI Mode 2 (CPOL=1, CPHA=0)
  
  // Set an initial frequency
  setFrequency(1000, WAVE_SINE); 
}

void loop() {
  // Example: Change frequency every 2 seconds
  delay(2000);
  setFrequency(5000, WAVE_SINE);
  delay(2000);
  setFrequency(1000, WAVE_SINE);
}

void AD9833_Write(uint16_t data) {
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE2));
  digitalWrite(FSYNC, LOW);    // Select the chip
  SPI.transfer16(data);        // Send 16-bit word
  digitalWrite(FSYNC, HIGH);   // Deselect the chip
  SPI.endTransaction();
}

void setFrequency(float freq, uint16_t waveType) {
  // Formula: (Freq * 2^28) / 25MHz
  // 2^28 / 25,000,000 = 10.73741824
  uint32_t freqWord = (uint32_t)(freq * 10.73741824);

  // Split into two 14-bit chunks
  uint16_t LSB = (uint16_t)(freqWord & 0x3FFF);
  uint16_t MSB = (uint16_t)((freqWord >> 14) & 0x3FFF);

  // Set the register address bits (01 for FREQ0 register)
  LSB |= 0x4000;
  MSB |= 0x4000;

  // Write sequence
  AD9833_Write(0x2100);   // Control Word: Reset + B28 bit
  AD9833_Write(LSB);      // Write Lower 14 bits
  AD9833_Write(MSB);      // Write Upper 14 bits
  AD9833_Write(waveType); // Set waveform and exit Reset
}