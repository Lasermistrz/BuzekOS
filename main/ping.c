#include "interface.h"

static const char *TAG = "ping";
extern uint8_t chosenOption;
extern uint8_t connectedFlag;
extern uint8_t pos;
extern uint8_t actPos;
extern TaskHandle_t statusTaskHandle;

void ping_on_success(esp_ping_handle_t hdl, void *args)
{
    /*uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    ESP_LOGI(TAG, "%ld bytes from %s icmp_seq=%d ttl=%d time=%ld ms\n", recv_len, inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);*/
}

void ping_on_timeout(esp_ping_handle_t hdl, void *args)
{
    /*uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    ESP_LOGI(TAG, "From %s icmp_seq=%d timeout\n", inet_ntoa(target_addr.u_addr.ip4), seqno);*/
}

void ping_on_end(esp_ping_handle_t hdl, void *args)
{
    ip_addr_t target_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);

    lcd_ping_result(transmitted, received, loss, total_time_ms, target_addr);
    vTaskSuspend(statusTaskHandle);
    led_set_color(0,31,0);
    usleep(8000000); // display results for 8 seconds
    chosenOption = 0;
    lcd_print_menu(pos, actPos, connectedFlag);
    
     esp_ping_delete_session(hdl);
}

void ping_send(uint8_t *host)
{
    ip_addr_t target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    struct addrinfo *res = NULL;
    int err = getaddrinfo((char *)host, NULL, &hint, &res);
    if (err != 0 || res == NULL)
    {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        pos=0;
        actPos=0;
        chosenOption=0;
        lcd_dns_error(res, err);
        led_set_color(31,0,0);
        usleep(8000000);
        lcd_print_menu(pos, actPos, connectedFlag);
        return;
    }
    else
    {
        ESP_LOGI(TAG, "DNS lookup success");
    }

    if (res->ai_family == AF_INET)
    {
        struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
        inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    }
    else
    {
        struct in6_addr addr6 = ((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr;
        inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
    }
    freeaddrinfo(res);

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;
    ping_config.count = PING_COUNT;

    esp_ping_callbacks_t cbs = {
        .on_ping_success = ping_on_success,
        .on_ping_timeout = ping_on_timeout,
        .on_ping_end = ping_on_end,
        .cb_args = NULL};

    vTaskResume(statusTaskHandle);

    esp_ping_handle_t ping;
    esp_ping_new_session(&ping_config, &cbs, &ping);
    esp_ping_start(ping);
}