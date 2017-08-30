
//
int rpcMem_isValid_wakeup(){
  int ret= mNG_CODE;
  String resetReason = ESP.getResetReason();
  Serial.println(resetReason);
  if ( resetReason == "Deep-Sleep Wake" ) {
    Serial.println("## wake=DS") ;
    ret= mOK_CODE;
  }  
  return ret;
}





