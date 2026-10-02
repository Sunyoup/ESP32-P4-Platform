/*
 * SPDX-FileCopyrightText: 2015-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_check.h"
#include "esp_err.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"

#include "product_support.h"

static const char *TAG = "product_ethernet";

/* On-board IP101 PHY, the RMII pins are the ESP32-P4 EMAC defaults */
#define PRODUCT_ETH_MDC_GPIO        (31)
#define PRODUCT_ETH_MDIO_GPIO       (52)
#define PRODUCT_ETH_PHY_RST_GPIO    (51)
#define PRODUCT_ETH_PHY_ADDR        (1)

static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Ethernet link up");
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet link down");
        break;
    default:
        break;
    }
}

static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)event_data;

    ESP_LOGI(TAG, "Ethernet got IP " IPSTR ", mask " IPSTR ", gateway " IPSTR,
             IP2STR(&event->ip_info.ip), IP2STR(&event->ip_info.netmask), IP2STR(&event->ip_info.gw));
}

esp_err_t product_ethernet_start(void)
{
    static bool started = false;
    if (started) {
        return ESP_OK;
    }

    /* The Wi-Fi setup of the Settings app shares the network stack and the default event loop */
    esp_err_t ret = esp_netif_init();
    ESP_RETURN_ON_FALSE(ret == ESP_OK || ret == ESP_ERR_INVALID_STATE, ret, TAG, "esp_netif_init failed");
    ret = esp_event_loop_create_default();
    ESP_RETURN_ON_FALSE(ret == ESP_OK || ret == ESP_ERR_INVALID_STATE, ret, TAG, "Create event loop failed");

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = PRODUCT_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = PRODUCT_ETH_PHY_RST_GPIO;
    eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    emac_config.smi_gpio.mdc_num = PRODUCT_ETH_MDC_GPIO;
    emac_config.smi_gpio.mdio_num = PRODUCT_ETH_MDIO_GPIO;

    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&emac_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
    esp_eth_handle_t eth_handle = NULL;
    esp_netif_t *netif = NULL;
    esp_eth_netif_glue_handle_t glue = NULL;
    ESP_GOTO_ON_FALSE(mac && phy, ESP_ERR_NO_MEM, err, TAG, "Create MAC or PHY failed");

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_GOTO_ON_ERROR(esp_eth_driver_install(&eth_config, &eth_handle), err, TAG, "Ethernet driver install failed");

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH();
    netif = esp_netif_new(&netif_config);
    ESP_GOTO_ON_FALSE(netif, ESP_ERR_NO_MEM, err, TAG, "Create Ethernet netif failed");
    glue = esp_eth_new_netif_glue(eth_handle);
    ESP_GOTO_ON_FALSE(glue, ESP_ERR_NO_MEM, err, TAG, "Create Ethernet netif glue failed");
    ESP_GOTO_ON_ERROR(esp_netif_attach(netif, glue), err, TAG, "Attach Ethernet netif failed");

    ESP_GOTO_ON_ERROR(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler, NULL), err, TAG,
                      "Register Ethernet event handler failed");
    ESP_GOTO_ON_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, got_ip_event_handler, NULL), err, TAG,
                      "Register IP event handler failed");
    ESP_GOTO_ON_ERROR(esp_eth_start(eth_handle), err, TAG, "Ethernet start failed");

    started = true;

    return ESP_OK;

err:
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, got_ip_event_handler);
    esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler);
    if (glue) {
        esp_eth_del_netif_glue(glue);
    }
    if (netif) {
        esp_netif_destroy(netif);
    }
    if (eth_handle) {
        esp_eth_driver_uninstall(eth_handle);
    }
    if (mac) {
        mac->del(mac);
    }
    if (phy) {
        phy->del(phy);
    }
    return ret;
}
