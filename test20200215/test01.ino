#include "test01.h"
 
 SemaphoreHandle_t xMutex = NULL;
 int sharedResource = 0;

//OS_EVENT       *MultiWait;
int         multi_rx_rear= 0;
unsigned char  multi_rx_buffer[MULTI_BUF_SIZE]; 
unsigned word  multi_exist=0x00000000L;
//multi_dev_t   multi_dev[32];
unsigned char   g_old_multictl[32];





 static void vTask1(void *pvParameters)
 {
     BaseType_t xStatus;
     const TickType_t xTicksToWait = 1000UL; //
     xSemaphoreGive(xMutex);

     while(1)
     {
         xStatus = xSemaphoreTake(xMutex, xTicksToWait);

         Serial.println("check for mutex (task1)");

         if(xStatus == pdTRUE)
         {
             sharedResource = 100;
             Serial.print("shared resource change by task1 : ");
             Serial.println(sharedResource);
         }

         xSemaphoreGive(xMutex);
         delay(500);
     }
 }
