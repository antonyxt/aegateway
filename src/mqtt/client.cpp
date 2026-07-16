#include "client.h"

#include <iostream>

MQTTClient::MQTTClient(const std::string& serverURI,
                       const std::string& clientId)
    : client_(serverURI, clientId)
{
    client_.set_callback(*this);
}

MQTTClient::~MQTTClient()
{
    try
    {
        disconnect();
    }
    catch (...)
    {
    }
}

bool MQTTClient::connect(const std::string& caCertFile, 
                         const std::string& clientBundleFile)
{
    try
    {
        mqtt::ssl_options ssl;

        // 1. Trust store verifies the broker
        ssl.set_trust_store(caCertFile);
        ssl.set_enable_server_cert_auth(true);

        // 2. Client bundle sends both client certificate and private key
        ssl.set_key_store(clientBundleFile);
        ssl.set_private_key(clientBundleFile);

        // 3. Attach SSL to connection options
        connOpts_.set_ssl(ssl); // Note: modern Paho uses set_ssl_opts()
        connOpts_.set_clean_session(true);
        connOpts_.set_automatic_reconnect(true);

        // Note: Removed set_user_name and set_password since 
        // Mosquitto uses the client certificate identity for authentication instead.

        client_.connect(connOpts_)->wait();

        std::cout << "MQTT connected securely via mTLS\n";
        return true;
    }
    catch (const mqtt::exception& exc)
    {
        std::cerr << "MQTT Connection error: " << exc.what() << "\n";
        return false;
    }
}


void MQTTClient::disconnect()
{
    if (client_.is_connected())
        client_.disconnect()->wait();
}

bool MQTTClient::publish(const std::string& topic,
                         const std::string& payload,
                         int qos,
                         bool retained)
{
    try
    {
        auto msg = mqtt::make_message(topic, payload);

        msg->set_qos(qos);
        msg->set_retained(retained);

        client_.publish(msg)->wait();

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool MQTTClient::subscribe(const std::string& topic,
                           int qos)
{
    try
    {
        client_.subscribe(topic, qos)->wait();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void MQTTClient::setMessageCallback(MessageCallback cb)
{
    messageCallback_ = std::move(cb);
}

void MQTTClient::connected(const std::string&)
{
    std::cout << "Connected callback\n";
}

void MQTTClient::connection_lost(const std::string& cause)
{
    std::cout << "Connection lost: " << cause << std::endl;
}

void MQTTClient::message_arrived(mqtt::const_message_ptr msg)
{
    if (messageCallback_)
    {
        messageCallback_(
            msg->get_topic(),
            msg->to_string());
    }
}

void MQTTClient::delivery_complete(mqtt::delivery_token_ptr)
{
}