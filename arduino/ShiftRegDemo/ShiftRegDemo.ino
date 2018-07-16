#define OUTPUT_TRACE 1

const byte led_bus_clock_pin=12;
const byte led_bus_storage_pin=11;
const byte led_bus_data_pin=10;

const int cycle_duration=16000;  /* complete cylce duration in ms */
const byte led_count=8;

const int frame_delay=cycle_duration/led_count/2;  


void setup() {
 #ifdef OUTPUT_TRACE 
 Serial.begin(9600);
  #endif  
  pinMode(led_bus_clock_pin,OUTPUT);
  pinMode(led_bus_storage_pin,OUTPUT);
  pinMode(led_bus_data_pin,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);

  /* remove all data from shift reg */ 
  digitalWrite(led_bus_storage_pin,LOW); 
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,0);
  digitalWrite(led_bus_storage_pin,HIGH);     
  digitalWrite(led_bus_storage_pin,LOW); 
      
 #ifdef OUTPUT_TRACE
  Serial.println();
  Serial.println("--- setup complete ---");
  #endif
}

void loop() {
  byte pattern=B00000000;
  byte index=0;
  digitalWrite(led_bus_storage_pin,LOW);
  digitalWrite(LED_BUILTIN,HIGH);
  for (index=0;index<led_count;index++) {
      pattern=B00000001|pattern<<1; 
      shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,pattern);
      digitalWrite(led_bus_storage_pin,HIGH);     
      digitalWrite(led_bus_storage_pin,LOW);  
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(frame_delay);

  }
  digitalWrite(LED_BUILTIN,LOW);
  for (index=0;index<led_count;index++) {
      pattern=pattern<<1; 
      shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,pattern);
      digitalWrite(led_bus_storage_pin,HIGH);     
      digitalWrite(led_bus_storage_pin,LOW);  
      #ifdef OUTPUT_TRACE
          Serial.print("<<");Serial.println(pattern,BIN);
      #endif
      delay(frame_delay);
  }

}
