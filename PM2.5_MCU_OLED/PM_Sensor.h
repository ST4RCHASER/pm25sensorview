
// Choose program options.
//#define PRINT_RAW_DATA
#define USE_AVG
 
// For averaging last N raw voltage readings.
#ifdef USE_AVG
#define N 100
static unsigned long VoRawTotal = 0;
static int VoRawCount = 0;
#endif // USE_AVG
 
// Set the typical output voltage in Volts when there is zero dust. 
static float Voc = 0.6;
 
// Use the typical sensitivity in units of V per 100ug/m3.
const float K = 0.5;

 // Helper functions to print a data value to the serial monitor.
void printValue(String text, unsigned int value, bool isLast = false) {
Serial.print(text);
Serial.print("=");
Serial.print(value);
if (!isLast) {
Serial.print(", ");
}
}
void printFValue(String text, float value, String units, bool isLast = false) {
Serial.print(text);
Serial.print("=");
Serial.print(value);
Serial.print(units);
if (!isLast) {
Serial.print(", ");
}
}
