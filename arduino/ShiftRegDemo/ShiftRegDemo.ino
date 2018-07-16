//#define OUTPUT_TRACE 1

const byte led_bus_clock_pin=12;
const byte led_bus_storage_pin=11;
const byte led_bus_data_pin=10;

const int frame_delay=100;
const byte led_count=8;


void setup() {
  Serial.begin(9600);
  pinMode(led_bus_clock_pin,OUTPUT);
  pinMode(led_bus_storage_pin,OUTPUT);
  pinMode(led_bus_data_pin,OUTPUT);
  Serial.println();
 #ifdef OUTPUT_TRACE
  Serial.println("--- setup complete ---");
  #endif
}

void loop() {
  byte pattern=B00000001;
  byte index=0;
  digitalWrite(led_bus_storage_pin,LOW);
  for (index=0;index<led_count;index++) {
      shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,pattern);
      digitalWrite(led_bus_storage_pin,HIGH);     
      digitalWrite(led_bus_storage_pin,LOW);  
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(frame_delay);
      pattern=B00000001|pattern<<1; 
  }
  for (index=0;index<=led_count;index++) {
      shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,pattern);
      digitalWrite(led_bus_storage_pin,HIGH);     
      digitalWrite(led_bus_storage_pin,LOW);  
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(frame_delay);
      pattern=pattern<<1; 
  }

}
