#include "appGlobals.h"
#include "Base64.h"

// Google Drive Script ID 
static char scriptID[70] = "";
static bool gdrive_enabled = false;

bool configureGDrive(const char* script_id) {
  if (strlen(script_id) > 0) {
    strncpy(scriptID, script_id, sizeof(scriptID) - 1);
    gdrive_enabled = true;
    LOG_INF("Google Drive upload configured with script ID: %s", scriptID);
    return true;
  }
  return false;
}

bool uploadToGoogleDrive(const char* fileName) {
  if (!gdrive_enabled || strlen(scriptID) == 0) {
    LOG_WRN("Google Drive not configured");
    return false;
  }

  LOG_INF("Preparing to upload %s to Google Drive", fileName);
  
  // Check if file exists
  File fileToUpload = STORAGE.open(fileName, FILE_READ);
  if (!fileToUpload) {
    LOG_WRN("Failed to open file %s for upload", fileName);
    return false;
  }
  
  // Extract just the filename part
  const char* baseFileName = strrchr(fileName, '/');
  if (baseFileName != NULL) baseFileName++;
  else baseFileName = fileName;
  
  // Create secure client
  NetworkClientSecure client;
  client.setInsecure(); // For simplicity, but less secure
  
  const char* host = "script.google.com";
  const int port = 443;
  
  LOG_INF("Connecting to Google Script");
  if (!client.connect(host, port)) {
    LOG_WRN("Connection to Google Script failed");
    fileToUpload.close();
    return false;
  }
  
  // Prepare file upload in chunks
  size_t fileSize = fileToUpload.size();
  LOG_INF("File size: %s", fmtSize(fileSize));
  
  // Prepare HTTP POST request
  String url = "/macros/s/" + String(scriptID) + "/exec";
  
  String head = "--ESP32_XIAO\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + String(baseFileName) + "\"\r\nContent-Type: application/octet-stream\r\n\r\n";
  String tail = "\r\n--ESP32_XIAO--\r\n";
  
  client.print("POST " + url + " HTTP/1.1\r\n");
  client.print("Host: " + String(host) + "\r\n");
  client.print("Content-Length: " + String(fileSize + head.length() + tail.length()) + "\r\n");
  client.print("Content-Type: multipart/form-data; boundary=ESP32_XIAO\r\n");
  client.print("Connection: close\r\n\r\n");
  
  client.print(head);
  
  // Read and send file in chunks
  uint8_t buffer[1024];
  size_t bytesRead = 0;
  unsigned long uploadStart = millis();
  uint8_t uploadProgress = 0;
  
  while (fileToUpload.available()) {
    size_t readBytes = fileToUpload.read(buffer, sizeof(buffer));
    if (readBytes > 0) {
      client.write(buffer, readBytes);
      bytesRead += readBytes;
      
      // Show progress
      if (calcProgress(bytesRead, fileSize, 10, uploadProgress)) {
        LOG_INF("Upload progress: %u%%", uploadProgress);
      }
      
      // Give some time to process
      yield();
    }
  }
  
  client.print(tail);
  fileToUpload.close();
  
  // Wait for response
  unsigned long timeout = millis() + 10000;
  while (client.connected() && millis() < timeout) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.indexOf("HTTP/1.1 302") >= 0 || line.indexOf("HTTP/1.1 200") >= 0) {
        LOG_INF("Upload successful");
        client.stop();
        return true;
      }
    }
    delay(10);
  }
  
  client.stop();
  LOG_WRN("Upload failed or timed out");
  return false;
}