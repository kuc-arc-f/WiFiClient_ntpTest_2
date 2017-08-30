/* esp8266 SNTP Udp Client, 
  active time zonve ,version
*/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>
WiFiUDP udp;
const char* ssid = " ";
const char* password = " ";
const char* host = "api.thingspeak.com";
String mAPI_KEY="your-KEY";
//
unsigned int localPort = 8888;       // local port to listen for UDP packets
const char* timeServer = "ntp.nict.jp";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
const int mSleepMin= 5; //sleep- timer
const int mMaxNTtpMin=60;
unsigned long mNowTime=0;

const int mOK_CODE=1;
const int mNG_CODE=0;
const int mActiveStart=8;
const int mActiveEnd  =16;
// rtcMem
struct {
  unsigned long timeCt;
  int wakeCt;  
}memData;

// send an NTP request to the time server at the given address
void sendNTPpacket(const char* address)
{
  Serial.print("sendNTPpacket : ");
  Serial.println(address);

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0]  = 0b11100011;   // LI, Version, Mode
  packetBuffer[1]  = 0;     // Stratum, or type of clock
  packetBuffer[2]  = 6;     // Polling Interval
  packetBuffer[3]  = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

time_t readNTPpacket() {
  Serial.println("Receive NTP Response");
  udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
  unsigned long secsSince1900 = 0;
  // convert four bytes starting at location 40 to a long integer
  secsSince1900 |= (unsigned long)packetBuffer[40] << 24;
  secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
  secsSince1900 |= (unsigned long)packetBuffer[42] <<  8;
  secsSince1900 |= (unsigned long)packetBuffer[43] <<  0;
  return secsSince1900 - 2208988800UL;
}

time_t getNtpTime()
{
  while (udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);

  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      return readNTPpacket();
    }
   }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

//
void setup() {
  Serial.begin( 115200 );
  Serial.println("#Start-setup-ssEsp");
  // wifi-start
  int iWake = rpcMem_isValid_wakeup();
  Serial.println("iWake=" + String(iWake ));
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  int iStartNTP= mNG_CODE;
  int iWakeCt=0;
  if(iWake ==mOK_CODE){
     ESP.rtcUserMemoryRead(0, (uint32_t*) &memData, sizeof(memData));
     iWakeCt = memData.wakeCt;
     Serial.println("iWakeCt="+ String(iWakeCt  )+ ",tm=" +String(memData.timeCt ) );
     // timeCt
     iWakeCt=iWakeCt +1;
     mNowTime = get_nowTime(memData.timeCt , mSleepMin , iWakeCt  ) ;
     Serial.println( "mNowTime="+ String(mNowTime ) );
     String sTime =comFunc_conv_num2time(mNowTime);
     Serial.println( "sTime="+ sTime );
     //max-count
     int iMax= mMaxNTtpMin / mSleepMin;
     Serial.println("iWakeCt="+ String(iWakeCt  )+ ",iMax=" +String(iMax ) );
     if( iWakeCt > iMax ){
        iStartNTP= mOK_CODE;
     }
     memData.wakeCt= iWakeCt;
  }else{
    iStartNTP= mOK_CODE;
  }  
  //NTP
  if(iStartNTP== mOK_CODE){
      udp.begin(localPort);
      delay(2000);
      setSyncProvider(getNtpTime);
      delay(100 );  
      String sNow= get_timeNowStr();
      mNowTime =comFunc_conv_time2num( (char *)sNow.c_str() );
      memData.timeCt = mNowTime;
      memData.wakeCt=  0;
  }
  //write
//  int iNow =get_timeNow();
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &memData, sizeof(memData));
  Serial.println("#End-Setup");
}

//
void loop() {
 int iSleep= mSleepMin * 60; 
  delay(100 );
  if (millis() > 30000 ){
       ESP.deepSleep( iSleep * 1000 * 1000);
  }
  String sNow = comFunc_conv_num2time(mNowTime );
  int iNow =sNow.toInt();
  int iNum  = Is_valid_active( iNow );
  Serial.println( "iNum=" + String(iNum  ) );
  if(iNum== mNG_CODE){
      iSleep= 3600;  //  60 min
  }else{
      proc_http();
  }
  Serial.println();
  Serial.println("#deep-Sleep Start.iSleep =" + String(iSleep ));
//  delay(1000);
  ESP.deepSleep( iSleep * 1000 * 1000);
}




