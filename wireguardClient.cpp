// wireguardClient.cpp
#include "appGlobals.h"
#include <WiFi.h>
#include <esp_wireguard.h>

// WireGuard configuration
static const char* wg_private_key = "YOUR_PRIVATE_KEY_HERE"; // Will be generated in setup
static const char* tailscale_endpoint = "YOUR_TAILSCALE_ENDPOINT"; // Will be set in code
static const int tailscale_port = 41641; // Standard Tailscale port

// Create a WireGuard interface
esp_wireguard_ctx_t wg_ctx;

bool setupWireGuard() {
  LOG_INF("Initializing WireGuard connection");
  
  // Initialize WireGuard
  esp_wireguard_init(&wg_ctx);
  
  // Set up the interface
  esp_wireguard_set_private_key(&wg_ctx, wg_private_key);
  esp_wireguard_set_endpoint(&wg_ctx, tailscale_endpoint, tailscale_port);
  
  // Connect
  esp_err_t result = esp_wireguard_connect(&wg_ctx);
  
  if (result == ESP_OK) {
    LOG_INF("WireGuard connected successfully");
    return true;
  } else {
    LOG_WRN("WireGuard connection failed: %s", esp_err_to_name(result));
    return false;
  }
}

void maintainWireGuard() {
  // Call this periodically to maintain the WireGuard connection
  esp_wireguard_periodic(&wg_ctx);
}