
void reportReadingsToServer(Adafruit_CC3000 cc3000, long reportTime, float insideTemp)
{
  ip = 0;
  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  } 
  
  cc3000.printIPdotsRev(ip);
  
  
  // Optional: Do a ping test on the website
  /*
  Serial.print(F("\n\rPinging ")); cc3000.printIPdotsRev(ip); Serial.print("...");  
  replies = cc3000.ping(ip, 5);
  Serial.print(replies); Serial.println(F(" replies"));
  */  

  /* Try connecting to the website.
     Note: HTTP/1.1 protocol is used to keep the server from closing the connection before all data is read.
  */
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);
  
  Serial.println(F("Int Report time"));
  Serial.println(reportTime);
  
  char reportTimeString[8*sizeof(long)+1];
  memset(reportTimeString, 0, 8*sizeof(long)+1);
  ultoa(reportTime, reportTimeString, 10);
  
  
  Serial.println(F("String Report time"));
  Serial.println(reportTimeString);
  

  // Generate string for the temperature reading.
  char tempString[8*sizeof(unsigned long)+5];
  memset(tempString, 0, 8*sizeof(unsigned long)+5);
  // Convert to fixed point string.  Using a proper float to string function
  // like dtostrf takes too much program memory (~1.5kb) to use in this sketch.
  ultoa((unsigned long)insideTemp, tempString, 10);
  int n = strlen(tempString);
  tempString[n] = '.';
  tempString[n+1] = '0' + ((unsigned long)(insideTemp*10)) % 10;
  tempString[n+2] = '0' + ((unsigned long)(insideTemp*100)) % 10;
  tempString[n+3] = '0' + ((unsigned long)(insideTemp*1000)) % 10;
  
  Serial.println(F("String Report temp"));
  Serial.println(tempString);
  
  
  if (www.connected()) {
    www.fastrprint(F("GET "));
    www.fastrprint(WEBPAGE);
    www.fastrprint(F("?reading_timestamp="));
    www.fastrprint(reportTimeString);
    www.fastrprint(F("&reading_temperature="));
    www.fastrprint(tempString);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }

  Serial.println(F("-------------------------------------"));
  
  /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  www.close();
  Serial.println(F("-------------------------------------"));
  
  return;
}
