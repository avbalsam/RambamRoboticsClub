int getInfrared() {
  int index = 0;
  Wire.requestFrom(0x50, 2);
  for (int i = 0; i <= 4; i++) { 
    if (Wire.available()) {
      index = Wire.read();
      ave.push(index);
    }
  }
  index = ave.mode();
  ave.clear();
//  if (index > 3) {
//    index = 4;
//  } else if (index < 3) {
//    index = 2;
//  }

  return index;
}

boolean getPixy() {
  uint16_t blocks;
  int total = 0;
  for(int i = 0; i <= 4; i++) {
    total += pixy.getBlocks();
    delay(10);
  }
  return(total > 0 ? true : false);
//  if(blocks) {
//    return true;
//  } else {
//    return false;
//  }
//  char buf[32]; 
//  sprintf(buf, "Detected %d:\n", blocks);
//  Serial.print(buf);
}

