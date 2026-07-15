#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <syslog.h>
#include <cstdarg>
#include "mqtt/client.h"

static std::atomic<bool> shutdownRequested(false);
static bool useSyslog = false;

void logMessage(int priority, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    if (useSyslog) {
        vsyslog(priority, format, args);
    } else {
        vfprintf(stdout, format, args);
        fprintf(stdout, "\n");
    }
    
    va_end(args);
}

void signalHandler(int signal) {
    logMessage(LOG_NOTICE, "Shutdown signal received (%d)", signal);
    shutdownRequested = true;
}

int main() {
    // Initialize syslog for system logging (Debian/Linux standard)
    openlog("ae_bridge_app", LOG_PID | LOG_CONS, LOG_LOCAL0);
    useSyslog = true;
    
    logMessage(LOG_INFO, "=========================================");
    logMessage(LOG_INFO, " Hello Raspberry Pi!");
    logMessage(LOG_INFO, " Compiled instantly on Ubuntu 24.04 WSL!");
    logMessage(LOG_INFO, "=========================================");

    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    MQTTClient mqtt(
    "ssl://mqtt.example.com:8883",
    "gateway001");

    mqtt.setMessageCallback(
        [](const std::string& topic,
           const std::string& payload)
        {
            std::cout << topic << std::endl;
            std::cout << payload << std::endl;

            // Handle command here
        });

    if (!mqtt.connect(
            "username",
            "password",
            "/etc/ssl/certs/ca.pem"))
    {
        logMessage(LOG_ERR, "Failed to connect to MQTT broker");
        closelog();
        return -1;
    }
    
    logMessage(LOG_INFO, "Connected to MQTT broker successfully");

    mqtt.subscribe("gateway/001/cmd");
    logMessage(LOG_INFO, "Subscribed to topic: gateway/001/cmd");

    mqtt.publish(
        "gateway/001/status",
        R"({"status":"online"})");
    logMessage(LOG_INFO, "Published online status");

    logMessage(LOG_INFO, "Gateway running. Press Ctrl+C to shutdown...");

    while (!shutdownRequested)
    {
        std::this_thread::sleep_for(
            std::chrono::seconds(1));
    }

    logMessage(LOG_INFO, "Disconnecting from MQTT broker...");
    mqtt.disconnect();
    logMessage(LOG_INFO, "Shutdown complete.");
    
    closelog();
    return 0;
}

