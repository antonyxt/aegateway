#pragma once

#include <mqtt/async_client.h>

#include <functional>
#include <memory>
#include <string>

class MQTTClient : public virtual mqtt::callback
{
public:
    using MessageCallback =
        std::function<void(const std::string& topic,
                           const std::string& payload)>;

    MQTTClient(const std::string& serverURI,
               const std::string& clientId);

    ~MQTTClient();

    bool connect(const std::string& caCertFile,
                 const std::string& clientBundleFile);

    void disconnect();

    bool publish(const std::string& topic,
                 const std::string& payload,
                 int qos = 1,
                 bool retained = false);

    bool subscribe(const std::string& topic,
                   int qos = 1);

    void setMessageCallback(MessageCallback cb);

private:
    // mqtt::callback
    void connected(const std::string& cause) override;
    void connection_lost(const std::string& cause) override;
    void message_arrived(mqtt::const_message_ptr msg) override;
    void delivery_complete(mqtt::delivery_token_ptr tok) override;

private:
    mqtt::async_client client_;
    mqtt::connect_options connOpts_;

    MessageCallback messageCallback_;
};