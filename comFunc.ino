//#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>

//
unsigned long comFunc_conv_time2num(char *src){
	unsigned long ret=0;
	char cHH[2+1];
	char cMM[2+1];
	for(int i=0 ;i < 2; i++){
		cHH[i]=src[i];
	}
	cHH[2]=0x00;
	for(int i=0 ;i < 2; i++){
		cMM[i]=src[i+2];
	}
	cMM[2]= 0x00;
	
//	printf( "cHH=%s\n",cHH);
	printf( "cMM=%s\n",cMM);
	unsigned long nHH=atoi(cHH);
	nHH= nHH * (60 *60 );
	unsigned long nMM=atoi(cMM);
	nMM = nMM * 60;
	ret= nHH + nMM;
//    ret= atoi(cHH);
	return ret;
}
//
String comFunc_conv_num2time(unsigned long src ){
  String sRet="";
	char cHHMM[4+1];
	int iHH = src / (60 *60);
	printf("iHH=%d\n",iHH );
	int iDiv = src % (60 *60);
	printf("iDiv=%d\n",iDiv );
	int iMM  = iDiv / 60;
printf("iMM=%d\n",iMM );
	sprintf(cHHMM, "%02ld%02ld\n", iHH , iMM );
	printf("cHHMM=%s\n",cHHMM );
  sRet = String(cHHMM );
  return sRet;

}
//
int get_timeNow(){
  int ret=0;
  time_t n = now();
  time_t t;
  char s[20];
  const char* format_hhmm = "%02d%02d";
  t= n+ (9 * SECS_PER_HOUR);
  // HHMM
  sprintf(s, format_hhmm , hour(t), minute(t) );
  Serial.println( "HHMM=" + String( s) );
  int iHHMM= atoi(s);
  Serial.println( "iHHMM=" + String( iHHMM ) );
  ret= iHHMM;
  return ret;
}
//
String get_timeNowStr(){
  String sRet="";
  // int ret=0;
  time_t n = now();
  time_t t;
  char s[20];
  const char* format_hhmm = "%02d%02d";
  t= n+ (9 * SECS_PER_HOUR);
  // HHMM
  sprintf(s, format_hhmm , hour(t), minute(t) );
  Serial.println( "HHMM=" + String( s) );
  sRet=String(s );
  return sRet;
}
//
void time_display(){
  time_t n = now();
  time_t t;
  char s[20];
  const char* format      = "%04d-%02d-%02d %02d:%02d:%02d";
  // const char* format_hhmm = "%02d%02d";
  // JST
  t= n+ (9 * SECS_PER_HOUR);
  sprintf(s, format, year(t), month(t), day(t), hour(t), minute(t), second(t));
  Serial.print("JST : ");
  Serial.println(s);
}
//
int Is_valid_active(int iNum){
  String sSt  = String(mActiveStart ) + "00";
  String sEnd = String(mActiveEnd ) + "00";

  int iStart= sSt.toInt();
  int iEnd =sEnd.toInt();
  int ret=mNG_CODE;
  if(iNum >= iStart  ){
      if( iNum <  iEnd ){
          ret = mOK_CODE;
      }
  }
  return ret;
}
//
//unsigned  long get_nowTime(unsigned long nTime, int addMin ){
unsigned  long get_nowTime(unsigned long nTime, int addMin ,int nCt ){
  unsigned long ret=0;
  ret =nTime + (nCt * addMin * 60);
  return ret;
}

//
void proc_http(){
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/update?key="+ mAPI_KEY + "&field1=1"  ;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }  
  Serial.println();
  Serial.println("closing connection");  
}


