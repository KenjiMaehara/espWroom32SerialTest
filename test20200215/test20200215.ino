String sendstr = "arduino000";
String sendstr01 = "arduino001";
String sendstr02 = "arduino002";

void setup() {
  // initialize both serial ports:

  #if 1
  Serial.begin(9600);
  Serial1.begin(9600,SERIAL_8N1,4,0);
  Serial2.begin(9600);
  #endif

  pinMode(2,OUTPUT);
}

void loop() {

  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
  }
  

    #if 0
    for (int i = 0; i < sendstr.length(); i++){
       Serial.write(sendstr.charAt(i)); 
    }

    for (int i = 0; i < sendstr01.length(); i++){
       Serial1.write(sendstr01.charAt(i)); 
    }

    for (int i = 0; i < sendstr02.length(); i++){
       Serial2.write(sendstr02.charAt(i)); 
    }
    #endif
  



  #if 0
  digitalWrite(2,HIGH);
  delay(500);
  digitalWrite(2,LOW); 

  delay(1000);
  #endif
}
