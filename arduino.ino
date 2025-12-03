#include <WiFi.h>
#include <Firebase_ESP_Client.h>

const char* ssid = "52TK";
const char* password = "Jhonson1000";

#define API_KEY "AIzaSyB5bIbl15Jk5Qp7YaN0AQM3sXPPsUf9xTQ"
#define DATABASE_URL "https://internetofthingsmikro-default-rtdb.firebaseio.com/";
#define USER_EMAIL "cynthia@gmail.com"
#define USER_PASSWORD "admin123"

#define dht 23
#define ldr 19
#define soil 18

void setup () {
  Serial.begin (115200);
  delay(100);
  Serial.println("\n=== SMART PLANT GREENHOUSE ===");
  Serial.println ("Inisialisasi sistem...\n");

  pinMode (LDR_PIN, INPUT);
  pinMode (SOIL_PIN, INPUT);
  pinMode (PIR_PIN, INPUT);
  pinMode (FLAME_PIN, INPUT);
  pinMode (OBJECT_PIN, INPUT;

  connectWifi ();
  configTime (gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println ("Sinkronisasi waktu dengan NTP..");
  delay (2000);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Serial.println("Menghubungkan ke firebase..");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWifi(true);
  unsigned long fbStart = millis ();
  while (!Firebase.ready() && millis () - fbStart < 10000) {
    Serial.print (".");
    delay (500);
  }
  if (Firebase.ready ()) {
    Serial.println ("\n Firebase terhubung !");
    Serial.println ("Sistem siap monitoring! \n");
  } else {
    Serial.println ("\n Firebase gagal terhubung, sistem tetap berjalan...\n");
  }
}

void loop () {
  if (Wifi.status () != WL_CONNECTED) {
    Serial.println("Wifi terputus! Mencoba reconnect..");
    connectWifi();
  }

  unsigned log now = millis ();
  if (now - lastSensorUpdate > sensorInterval) {
    lastSensorUpdate = now;
    bacaDanKirimData ();
  }
}

void connectWifi () {
  Wifi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print ("Menghubungkan ke Wifi");
  unsigned long start = millis ();
  while (Wifi.status () != WL_CONNECT) {
    Serial.println (".");
    delay (500);
    if (millis () - start > 20000) {
      Serial.println ("\n Gagal terhubung Wifi - restart..");
      ESP.restart();
    }
  }
  Serial.println();
  Serial.println ("Wifi terhubung!");
  Serial.print("IP Address : ");
  Serial.pritnln (Wifi.localIp());
}
unsigned long getTimestamp () {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime (&timeinfo)) {
    Serial.println ("Gagal mendapat waktu NTP, gunakan millis()");
    return millis ();
  }
  time (&now);
  return (unsigned long)now * 1000;
}

void bacaDanKirimData () {
  Serial.println ("Pembacaan Sensor Greenhouse");

  int rawLdr = analogRead (LDR_PIN);
  lightLevel = map (rawLdr, 4095, 0, 0, 100);
  lightLevel = constrain (lightLevel, 0, 100);

  Serial.println ("Cahaya : %d %% (ADC=%d) \n", lightLevel, rawLdr);

  int rawSoil = analogRead (SOIL_PIN);

  soilPercent = map (rawSoil, 4095,0,0,100);
  SoilPercent = constrain (soilPercennt, 0, 100);

  Serial.println ("Kelembaban tanah : %d %% (ADC = %d)\n", soilPercent, rawSoil);
  if (soilPercent < 40) {
    Serial.println ("Status : Kering - Perlu penyiraman");
  } else {
    Serial.println ("Status : Kelembaban cukup");
  }
  motionDetected = digitalRead(PIR_PIN) == HIGH;
  flameDetected = digitalRead(FLAME_PIN) == HIGH;
  objectDetected = digitalRead(OBJECT_PIN) == HIGH;

  Serial.printf("Gerakan (PIR) : %s\n", motionDetected ? "Terdeteksi" : "Tidak ada");
  Serial.printf("Api: %s\n", flameDetected ? "Terdeteksi" : "Aman");
  Serial.printf("Objek : %s\n", objectDetected ? "Terdeteksi" : "Tidak ada");

  if (Firebase.ready()) {
    Serial.println ("\n Mengirim data ke firebase...");

    String basePath = "/greenhouse/sensors";
    bool allSuccess = true;

    if (Firebase.RTDB.setInt(&fbdo, basePath + "/lightLevel", lightLevel)) {
      Serial.println ("soilMouisture terkirim");
    } else {
      Serial.printf("soilMoisture gagal: %s\n", fbdo.errorReason().c_str());
      allSuccess = false
    }
    if(Firebase.RTDB.setBool(&fbdo, basePath + "/motion", motionDetected)) {
      Serial.println ("Motion terkirim");
    } else {
      Serial.println ("Motion gagal: %s\n", fbdo.errorReason.c_str());
      allSuccess = false;
    }
    if(Firebase.RTDB.setBool(&fbdo, basePath + "/flame", flameDetected)) {
      Serial.println ("Flame terkirim");
    } else {
      Serial.println ("Flame gagal: %s\n", fbdo.errorReason.c_str());
      allSuccess = false;
    }
    if(Firebase.RTDB.setBool(&fbdo, basePath + "/object", objectDetected)) {
      Serial.println ("Object terkirim");
    } else {
      Serial.println ("Object gagal: %s\n", fbdo.errorReason.c_str());
      allSuccess = false;
    }

    unsigned long timestamp = getTimestamp ();
    if(Firebase.RTDB.setDouble(&fbdo, basePath + "/timestamp", timestamp)) {
      Serial.println ("timestamp terkirim (%lu)\n", timestamp);
    } else {
      Serial.println ("timestamp gagal: %s\n", fbdo.errorReason.c_str());
      allSuccess = false;
    }

    if (allSuccess) {
      Serial.println ("\n Semua data berhasil dikirim!");
    } else {
      Serial.println ("\n Beberapa data gagal dikirim");
    }
  } else {
    Serial.println ("\n Firebase belum siap, skip pengiriman");
  }
  Serial.println ("------------------------\n");

  delay (100);
}