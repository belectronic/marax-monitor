#include "mqtt.h"
#include <map>
#include "MaraData.h"

Mqtt::Mqtt(DisplayData &displayData) : displayData(displayData)
{
    connect();
}

void Mqtt::connect(void)
{
    if (!connectionActive)
    {
        Serial.println("Start connecting to mqtt server");
        client.setServer(server, port);
        client.setClient(espClient);
        while (!client.connected())
        {
            Serial.println("Connecting...");
            if (client.connect("MaraxClient", username, password))
            {
                Serial.println("connected!");
                connectionActive = true;
                break;
            }
            delay(500U);
        }
    }
}

void Mqtt::send(const char *topic, const char *payload)
{
    client.publish(topic, payload);
}

void Mqtt::send_int_val(const char *topic, const uint8_t &value)
{
    char msg[MaraData::MARADATA_MAX_ELEMENT_SIZE];
    itoa(value, msg, 10);
    send(topic, msg);
}

void Mqtt::sendMaraData(void)
{
    std::map<const char *, uint8_t> data_topic_map{
        {TOPIC_CURRENT_STEAM_TEMP, displayData.current_steam_temp},
        {TOPIC_TARGET_STEAM_TEMP, displayData.target_steam_temp},
        {TOPIC_CURRENT_HX_TEMP, displayData.current_hx_temp},
        {TOPIC_HEATING_STATE, displayData.heating_state},
        {TOPIC_PUMP_STATE, displayData.pump_state}};

    std::map<const char *, uint8_t>::iterator it;
    for (it = data_topic_map.begin(); it != data_topic_map.end(); it++)
    {
        send_int_val(it->first, it->second);
    }
}

void Mqtt::stop(void)
{
    espClient.stop();
    connectionActive = false;
}
