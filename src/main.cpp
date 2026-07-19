#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <random>
#include <iomanip>
#include <ctime>
#include <sstream>
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

static std::mt19937& getRng()
{
    static std::random_device rd;
    static std::mt19937 rng(rd());
    return rng;
}

std::string randomDeviceId()
{
    std::uniform_int_distribution<int> dist(100, 150);
    return "device-" + std::to_string(dist(getRng()));
}

std::string randomTenantId()
{
    std::uniform_int_distribution<int> dist('A', 'Z');
    char letter = static_cast<char>(dist(getRng()));
    return std::string("tenant-") + letter;
}

std::string randomSiteId()
{
    std::uniform_int_distribution<int> dist(0, 50);
    std::ostringstream oss;
    oss << "site-" << std::setw(2) << std::setfill('0') << dist(getRng());
    return oss.str();
}

double randomTemperature()
{
    std::uniform_int_distribution<int> dist(0, 100);
    int steps = dist(getRng());
    return 26.0 + steps * 0.02;
}

double randomHumidity()
{
    std::uniform_int_distribution<int> dist(0, 6);
    int steps = dist(getRng());
    return 60.0 + steps * 0.5;
}

double randomBattery()
{
    std::uniform_int_distribution<int> dist(70, 100);
    return static_cast<double>(dist(getRng()));
}

std::string currentTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    gmtime_r(&now_t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}

bool publishTelemetryMessage(MQTTClient& mqtt, const std::string& topic)
{
    const std::string deviceId = randomDeviceId();
    const std::string tenantId = randomTenantId();
    const std::string siteId = randomSiteId();
    const double temperature = randomTemperature();
    const double humidity = randomHumidity();
    const double battery = randomBattery();

    const std::string timestamp = currentTimestamp();

    std::ostringstream payload;
    payload << "{\n"
            << "  \"messageId\": \"msg-10001\",\n"
            << "  \"deviceId\": \"" << deviceId << "\",\n"
            << "  \"tenantId\": \"" << tenantId << "\",\n"
            << "  \"siteId\": \"" << siteId << "\",\n"
            << "  \"timestamp\": \"" << timestamp << "\",\n"
            << "  \"protocol\": \"MQTT\",\n"
            << "  \"messageType\": \"TELEMETRY\",\n"
            << "  \"correlationId\": \"corr-xyz-001\",\n"
            << "  \"payload\": {\n"
            << "    \"temperature\": " << std::fixed << std::setprecision(2) << temperature << ",\n"
            << "    \"humidity\": " << std::fixed << std::setprecision(1) << humidity << ",\n"
            << "    \"battery\": " << static_cast<int>(battery) << "\n"
            << "  },\n"
            << "  \"metadata\": {\n"
            << "    \"gatewayId\": \"gw-01\",\n"
            << "    \"firmware\": \"1.2.3\",\n"
            << "    \"signalStrength\": -70\n"
            << "  }\n"
            << "}";

    return mqtt.publish(topic, payload.str(), 1, false);
}

int main() {
    // Initialize syslog for system logging (Debian/Linux standard)
    openlog("ae_bridge_app", LOG_PID | LOG_CONS, LOG_LOCAL0);
    useSyslog = false;
    
    logMessage(LOG_INFO, "=========================================");
    logMessage(LOG_INFO, " Hello Raspberry Pi!");
    logMessage(LOG_INFO, " Compiled instantly on Ubuntu 24.04 WSL!");
    logMessage(LOG_INFO, "=========================================");

    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    MQTTClient mqtt(
    "ssl://localhost:8883",
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
            "/home/pi/.mqtt_cert/ca.crt",
            "/home/pi/.mqtt_cert/client_bundle.pem"))
    {
        logMessage(LOG_ERR, "Failed to connect to MQTT broker");
        closelog();
        return -1;
    }
    
    logMessage(LOG_INFO, "Connected to MQTT broker successfully");

    mqtt.subscribe("gateway/001/cmd");
    logMessage(LOG_INFO, "Subscribed to topic: gateway/001/cmd");

    mqtt.publish(
        "ubuntu/tls",
        R"({"status":"online"})");
    logMessage(LOG_INFO, "Published online status");



    logMessage(LOG_INFO, "Gateway running. Press Ctrl+C to shutdown...");

    while (!shutdownRequested)
    {
        for (int i = 0; i < 50 && !shutdownRequested; ++i)
        {
            if (publishTelemetryMessage(mqtt, "telemetry/device-123")) {
                logMessage(LOG_INFO, "Published telemetry packet %d/50", i + 1);
            } else {
                logMessage(LOG_ERR, "Failed to publish telemetry packet %d/50", i + 1);
            }
        }

        std::this_thread::sleep_for(
            std::chrono::seconds(1));
    }

    logMessage(LOG_INFO, "Disconnecting from MQTT broker...");
    mqtt.disconnect();
    logMessage(LOG_INFO, "Shutdown complete.");
    
    closelog();
    return 0;
}

