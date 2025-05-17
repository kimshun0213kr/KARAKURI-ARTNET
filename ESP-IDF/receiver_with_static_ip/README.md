# Static IP 1-UNIVERSE DMX Signal Receiver

- using template : [example/protocols/static_ip](https://github.com/espressif/esp-idf/tree/master/examples/protocols/static_ip) & [example/protocols/sockets/udp_client](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client) from [esp-idf](https://github.com/espressif/esp-idf) by [espressif](https://github.com/espressif).

## IP LIST & Max connection

Max connection : 4

IP List
| Receiver1 | Receiver2 | Receiver3 | Receiver4 |
| ----------- | ----------- | ----------- | ----------- |
| 192.168.4.2 | 192.168.4.3 | 192.168.4.4 | 192.168.4.5 |

> [!TIP]
> To change the maximum number of connections, rewrite `wifi_config.ap.max_connection` in [udp_dmx_transporter.c](../transporter/main/udp_dmx_transporter.c).
