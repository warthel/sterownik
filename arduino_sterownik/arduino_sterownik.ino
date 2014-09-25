/*
 * Copyright (C) 2014 jt
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "DHT.h"

DHT dht;

// Hardware configuration
RF24 radio(8,7);

byte addresses[][6] = {"1Node","2Node"};


// Set up roles to simplify testing 
boolean role;                                    // The main role variable, holds the current role identifier
boolean role_ping_out = 1, role_pong_back = 0;   // The two different roles.

void setup() {
  Serial.begin(9600);
  printf_begin();
  
  dht.setup(2);
  
  // Setup and configure rf radio
  radio.begin();                          // Start up the radio
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  
  radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
}

void write(char[] msg) {
  radio.stopListening();
  
  printf("Now sending \n\r");

  unsigned long time = micros();                             // Take the time, and send it.  This will block until complete
  if (!radio.write( &time, sizeof(unsigned long) )){  printf("failed.\n\r");  }
      
  radio.startListening();                                    // Now, continue listening
  
  unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
  boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
  
  while ( ! radio.available() ){                             // While nothing is received
    if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
        timeout = true;
        break;
    }      
  }
      
  if (timeout) {                                             // Describe the results
      printf("Failed, response timed out.\n\r");
  } else{
      unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Sent %lu, Got response %lu, round-trip delay: %lu microseconds\n\r",time,got_time,micros()-got_time);
  }
}

void readTemp(char[] & out) {
  Serial.print(dht.getHumidity());
  Serial.print("\t");
  Serial.print(dht.getTemperature());
  Serial.print("\n");
}

void loop(void){
    char[255] msg;
    
    readTemp(msg);
    write(msg);
    
    delay(1000);
}

//  delay(dht.getMinimumSamplingPeriod());


