
void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome , " + from_name + "\n";
      welcome += "Use the following commands to interact with the ESP32-CAM \n";
      welcome += "/photo : takes a new photo\n";
      welcome += "/flash : toggles flash LED \n";
      bot.sendMessage(CHAT_ID, welcome, "");
      statusCamera = true;
      sendPhoto = true;    }
    if(statusCamera){
    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      Serial.println("Change flash LED state");
    }
    if (text == "/photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
    if(!statusBersih)
      if (text == "dibersihkan") {
//        sendPhoto = true;
//        Serial.println("New photo request");
        
      String welcome = "Ketik 'sudah' Jika telah di bersihkan ,,,\n";
      bot.sendMessage(CHAT_ID, welcome, "");
      }
      if (text == "sudah") {
        sendPhoto = true;
        Serial.println("New photo request");
        
//      String welcome = "Ketik 'Sudah' Jika telah di bersihkan ,,,\n";
//      bot.sendMessage(CHAT_ID, welcome, "");
      }
    }
    if (text == "/stop") {
      statusCamera = false;
      Serial.println("stop camera");
    }
  }
}
