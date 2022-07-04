#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <TMRpcm.h>

LiquidCrystal_I2C ecran(0x27, 20, 4);

//   button +   button -   button enter  button stop
byte plus = A0, moin = A1, enter = A2, stopB = A3, jack_out = 9;
//                                                 jack out 3.5mm

// sd pin
const byte CS_PIN = 3;

uint8_t eAigu[8] = {130, 132, 142, 145, 159, 144, 142, 128};

int channel = 1;
int nbChannel = 0;

boolean isPlayingBefore = false;

boolean isLoop = false;

TMRpcm tmrpcm;

void setup() {
  pinMode(plus, INPUT);
  pinMode(moin, INPUT);
  pinMode(enter, INPUT);
  pinMode(stopB, INPUT);
  pinMode(CS_PIN, OUTPUT);

  tmrpcm.speakerPin = jack_out;
  pinMode(10, OUTPUT);
  Serial.begin(9600);
  ecran.init();
  ecran.backlight();
  lcdprint("Initialisation...", 0, 0, 230);
  ecran.createChar(0, eAigu);
  delay(50);
  if (!SD.begin(CS_PIN)) {
    ecran.clear();
    lcdprint("Erreur de carte SD", 0, 0, 100);
    for (;;);
  }
  else {
    ecran.clear();
    lcdprint("Initialisation", 0, 0, 25);
    lcdprint("termin", 1, 0, 25);
    ecran.write(0);
    lcdprint("e!", 1, 7, 25);
  }
  tmrpcm.setVolume(6);
  File directory = SD.open("/");
  File entry = directory.openNextFile();
  while (entry) {
    boolean isWav = false;
    String beforeCaract = "";
    boolean isW = false;
    boolean isA = false;
    boolean isV = false;
    String entryName = entry.name();
    for (int i = 0; i < entryName.length(); i++) {
      beforeCaract = entryName.substring(i, i + 1);
      if (beforeCaract.equalsIgnoreCase("w") && !isA && !isV && !isW) {
        isW = true;
      }
      if (beforeCaract.equalsIgnoreCase("a") && !isA && !isV && isW) {
        isA = true;
      }
      if (beforeCaract.equalsIgnoreCase("v") && isA && !isV && isW) {
        isV = true;
      }
      if (isV && isA && isW) {
        isWav = true;
        break;
      }
    }
    if (isWav) {
      nbChannel++;
    }
    entry.close();
    entry = directory.openNextFile();
  }
  directory.rewindDirectory();
  directory.close();
  if (nbChannel == 0) {
    ecran.clear();
    lcdprint("Aucun fichier WAV trouv", 0, 0, 25);
    ecran.write(0);
    for (;;);
  }
  initLCDplus();
}

void loop() {
  if (!tmrpcm.isPlaying()) {
    if (analogTest(plus)) {
      if (nbChannel != channel) {
        channel++;
        initLCDplus();
      }
      else {
        return;
      }
    }
    if (analogTest(moin)) {
      if (channel != 1) {
        channel--;
        initLCDmoin();
      }
      else {
        return;
      }
    }
    if (analogTest(enter)) {
      initLCDstartSound();
    }
  }
  if ((!tmrpcm.isPlaying()) && isPlayingBefore) {
    isPlayingBefore = false;
    initLCDplus();
  }
  if (analogTest(stopB)) {
    if (tmrpcm.isPlaying()) {
      tmrpcm.stopPlayback();
      File directory = SD.open("/");
      File entry = directory.openNextFile();
      int i = 0;
      while (entry) {
        String entryName = entry.name();
        boolean isWav = false;
        String beforeCaract = "";
        boolean isW = false;
        boolean isA = false;
        boolean isV = false;
        for (int a = 0; a < entryName.length(); a++) {
          beforeCaract = entryName.substring(a, a + 1);
          if (beforeCaract.equalsIgnoreCase("w") && !isA && !isV && !isW) {
            isW = true;
          }
          if (beforeCaract.equalsIgnoreCase("a") && !isA && !isV && isW) {
            isA = true;
          }
          if (beforeCaract.equalsIgnoreCase("v") && isA && !isV && isW) {
            isV = true;
          }
          if (isV && isA && isW) {
            isWav = true;
            break;
          }
        }
        if (isWav) {
          i++;
          if (i == channel) {
            ecran.clear();
            lcdprint((String)channel + ": " + entryName.substring(0, entryName.length() - 4), 0, 0, 15);
            lcdprint((String)channel + "/" + (String)nbChannel, 2, 0, 15);
            break;
          }
        }
        entry.close();
        entry = directory.openNextFile();
      }
      i -= 1;
      directory.rewindDirectory();
      directory.close();

    }
  }
}

boolean analogTest(int pin) {
  if (analogRead(pin) <= 1000) {
    return false;
  }
  else {
    return true;
  }
}

void initLCDplus() {
  File directory = SD.open("/");
  File entry = directory.openNextFile();
  int i = 0;
  while (entry) {
    String entryName = entry.name();
    boolean isWav = false;
    String beforeCaract = "";
    boolean isW = false;
    boolean isA = false;
    boolean isV = false;
    for (int a = 0; a < entryName.length(); a++) {
      beforeCaract = entryName.substring(a, a + 1);
      if (beforeCaract.equalsIgnoreCase("w") && !isA && !isV && !isW) {
        isW = true;
      }
      if (beforeCaract.equalsIgnoreCase("a") && !isA && !isV && isW) {
        isA = true;
      }
      if (beforeCaract.equalsIgnoreCase("v") && isA && !isV && isW) {
        isV = true;
      }
      if (isV && isA && isW) {
        isWav = true;
        break;
      }
    }
    if (isWav) {
      i++;
      if (i == channel) {
        ecran.clear();
        lcdprint((String)channel + ": " + entryName.substring(0, entryName.length() - 4), 0, 0, 15);
        lcdprint((String)channel + "/" + (String)nbChannel, 2, 0, 15);
        break;
      }
    }
    entry.close();
    entry = directory.openNextFile();
  }
  i -= 1;
  directory.rewindDirectory();
  directory.close();
}

void initLCDmoin() {
  File directory = SD.open("/");
  File entry = directory.openNextFile();
  int i = 0;
  while (entry) {
    String entryName = entry.name();
    boolean isWav = false;
    String beforeCaract = "";
    boolean isW = false;
    boolean isA = false;
    boolean isV = false;
    for (int a = 0; a < entryName.length(); a++) {
      beforeCaract = entryName.substring(a, a + 1);
      if (beforeCaract.equalsIgnoreCase("w") && !isA && !isV && !isW) {
        isW = true;
      }
      if (beforeCaract.equalsIgnoreCase("a") && !isA && !isV && isW) {
        isA = true;
      }
      if (beforeCaract.equalsIgnoreCase("v") && isA && !isV && isW) {
        isV = true;
      }
      if (isV && isA && isW) {
        isWav = true;
        break;
      }
    }
    if (isWav) {
      i++;
      if (i == channel) {
        ecran.clear();
        lcdprint((String)channel + ": " + entryName.substring(0, entryName.length() - 4), 0, 0, 15);
        lcdprint((String)channel + "/" + (String)nbChannel, 2, 0, 15);
        break;
      }
    }
    entry.close();
    entry = directory.openNextFile();
  }
  i -= 1;
  directory.rewindDirectory();
  directory.close();
}

void initLCDstartSound() {
  File directory = SD.open("/");
  File entry = directory.openNextFile();
  int i = 0;
  File fileName;
  while (entry) {
    String entryName = entry.name();
    boolean isWav = false;
    String beforeCaract = "";
    boolean isW = false;
    boolean isA = false;
    boolean isV = false;
    for (int a = 0; a < entryName.length(); a++) {
      beforeCaract = entryName.substring(a, a + 1);
      if (beforeCaract.equalsIgnoreCase("w") && !isA && !isV && !isW) {
        isW = true;
      }
      if (beforeCaract.equalsIgnoreCase("a") && !isA && !isV && isW) {
        isA = true;
      }
      if (beforeCaract.equalsIgnoreCase("v") && isA && !isV && isW) {
        isV = true;
      }
      if (isV && isA && isW) {
        isWav = true;
        break;
      }
    }
    if (isWav) {
      i++;
      if (i == channel) {
        fileName = entry;
        ecran.clear();
        lcdprint((String)channel + ": " + entryName.substring(0, entryName.length() - 4), 0, 0, 15);
        lcdprint("Musique lanc", 3, 0, 15);
        ecran.write(0);
        lcdprint("e!", 3, 13, 15);
        lcdprint((String)channel + "/" + (String)nbChannel, 2, 0, 15);
        fileName = entry;
        break;
      }
    }
    entry.close();
    entry = directory.openNextFile();
  }
  i -= 1;
  directory.rewindDirectory();
  directory.close();
  sound(fileName);
}

void lcdprint(String s, int cases, int ligne, int animation) {
  ecran.setCursor(ligne, cases);
  for (int i = 0; i < s.length(); i++) {
    ecran.print(s.substring(i, i + 1));
    delay(animation);
  }
}

void sound(File fileName) {
  if (!tmrpcm.isPlaying()) {
    isPlayingBefore = true;
    tmrpcm.play(fileName.name());
  }
}
