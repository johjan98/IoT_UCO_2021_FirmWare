#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

String payload;
String datetime;

String apiRest(String url){
    HTTPClient http;
 
   if (http.begin(url)) //Start connection
   {
      Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();  // GET Request
      if (httpCode > 0) {
         Serial.printf("[HTTP] GET... code: %d\n", httpCode);
 
         if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();   // Get HTTP response            
            datetime = payload.substring(63, 79);         
         }
      }
      else {
         Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      } 
      http.end();
   }
   else {
      Serial.printf("[HTTP} Unable to connect\n");
   }    
   return datetime;
}