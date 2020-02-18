 void task0(void* arg)
 {
     while (1)
     {
         static int count = 0;
         Serial.print("task 0 : ");
         Serial.println(count++);
         delay(1000);
     }
 }

 void task1(void* arg)
 {
     while (1)
     {
         static int count = 0;
         Serial.print("task 1 : ");
         Serial.println(count++);
         delay(2000);
     }
 }

 void setup()
 {
     Serial.begin(115200);

     // create tasks
     xTaskCreatePinnedToCore(task0, "Task0", 4096, NULL, 1, NULL, 0);
     xTaskCreatePinnedToCore(task1, "Task1", 4096, NULL, 1, NULL, 1);
 }

 void loop()
 {
     static int count = 0;
     Serial.print("main   : ");
     Serial.println(count++);
     delay(500);
 }
