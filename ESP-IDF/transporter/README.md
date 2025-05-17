# 1-UNIVERSE DMX Signal Transporter

- using template : [example/protocols/sockets/udp_server](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_servert) from [esp-idf](https://github.com/espressif/esp-idf) by [espressif](https://github.com/espressif).

## Description

Devices powered by this code provide an AP and send 1-Universe DMX signals by UDP data to four connected Receivers.

## Destination IP address list & Max connection

Max connection : 5 (Receiver \* 4 + PC \* 1)

> [!WARNING]  
> If more than two PCs are connected, the receiver will not be able to connect.  
> Please limit the number of PCs for radio and signal checks to one.

Destination IP address List
| Receiver1 | Receiver2 | Receiver3 | Receiver4 |
| ----------- | ----------- | ----------- | ----------- |
| 192.168.4.2 | 192.168.4.3 | 192.168.4.4 | 192.168.4.5 |

> [!TIP]  
> To change the maximum number of connections, rewrite `wifi_config.ap.max_connection` in [udp_dmx_transporter.c](/main/udp_dmx_transporter.c).
