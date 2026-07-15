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

bool MQTTClient::connect(const std::string& username,
                         const std::string& password,
                         const std::string& caCertFile)
{
    try
    {
        mqtt::ssl_options ssl;

        ssl.set_trust_store(caCertFile);
        ssl.set_enable_server_cert_auth(true);

        connOpts_.set_ssl(ssl);
        connOpts_.set_user_name(username);
        connOpts_.set_password(password);
        connOpts_.set_clean_session(true);
        connOpts_.set_automatic_reconnect(true);

        client_.connect(connOpts_)->wait();

        std::cout << "MQTT connected\n";

        return true;
    }
    catch (const mqtt::exception& e)
    {
        std::cerr << e.what() << std::endl;
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