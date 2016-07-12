#include "Websocket.h"
#include <string>

//#define DEBUG

Websocket::Websocket(char * url, Wifly * wifi) {
    this->wifi = wifi;
    netif = WIF;
    fillFields(url);
}


#ifdef TARGET_LPC1768
Websocket::Websocket(char * url) {
    server_ip = NULL;
    netif = ETH;
    eth_writeable = false;
    eth_readable = false;
    eth_connected = false;
    response_server_eth = false;
    new_msg = false;
    fillFields(url);

    eth = new EthernetNetIf();
    sock = new TCPSocket();

    EthernetErr ethErr = eth->setup();
#ifdef DEBUG
    if (ethErr) {
        printf("\r\nERROR %d in setup.\r\n", ethErr);
    }
#endif

    //we must use dnsresolver to find the ip address
    if (server_ip == NULL) {
        DNSResolver dr;
        server_ip = new IpAddr();
        *server_ip = dr.resolveName(ip_domain.c_str());
#ifdef DEBUG
        printf("\r\nserver with dns=%d.%d.%d.%d\r\n", (*server_ip)[0], (*server_ip)[1], (*server_ip)[2], (*server_ip)[3]);
#endif

    }

    IpAddr ipt = eth->getIp();
#ifdef DEBUG
    printf("\r\nmbed IP Address is %d.%d.%d.%d\r\n", ipt[0], ipt[1], ipt[2], ipt[3]);
#endif

    sock->setOnEvent(this, &Websocket::onTCPSocketEvent);
}
#endif //target


void Websocket::fillFields(char * url) {
    char *res = NULL;
    char *res1 = NULL;

    char buf[50];
    strcpy(buf, url);

    res = strtok(buf, ":");
    if (strcmp(res, "ws")) {
#ifdef DEBUG
        printf("\r\nFormat error: please use: \"ws://ip-or-domain[:port]/path\"\r\n\r\n");
#endif
    } else {
        //ip_domain and port
        res = strtok(NULL, "/");

        //path
        res1 = strtok(NULL, " ");
        if (res1 != NULL) {
            path = res1;
        }

        //ip_domain
        res = strtok(res, ":");

        //port
        res1 = strtok(NULL, " ");
        //port
        if (res1 != NULL) {
            port = res1;
        } else {
            port = "80";
        }

        if (res != NULL) {
            ip_domain = res;

            //if we use ethernet, we must decode ip address or use dnsresolver
#ifdef TARGET_LPC1768
            if (netif == ETH) {
                strcpy(buf, res);

                //we try to decode the ip address
                if (buf[0] >= '0' && buf[0] <= '9') {
                    res = strtok(buf, ".");
                    int i = 0;
                    int ip[4];
                    while (res != NULL) {
                        ip[i] = atoi(res);
                        res = strtok(NULL, ".");
                        i++;
                    }
                    server_ip = new IpAddr(ip[0], ip[1], ip[2], ip[3]);
#ifdef DEBUG
                    printf("server without dns=%i.%i.%i.%i\n",(*server_ip)[0],(*server_ip)[1],(*server_ip)[2],(*server_ip)[3]);
#endif
                }
            }
#endif //target
        }
    }
}


bool Websocket::connect() {
    char cmd[50];
    if (netif == WIF) {
        //enter in cmd mode
        wifi->exit();
        while (!wifi->cmdMode()) {
#ifdef DEBUG
            printf("cannot enter in CMD mode\r\n");
#endif
            wifi->send("a\r\n");
            wait(0.2);
            wifi->exit();
            if (!wifi->cmdMode())
                return false;
        }

        if (!wifi->send("set comm remote 0\r\n", "AOK")) {
#ifdef DEBUG
            printf("Websocket::connect(): cannot set empty remote string\r\n");
#endif
            return false;
        }

        //open the connection
        sprintf(cmd, "open %s %s\r\n", ip_domain.c_str(), port.c_str());
        if (!wifi->send(cmd, "OPEN")) {
            if (wifi->send(cmd, "nected")) {
#ifdef DEBUG
                printf("will try to close the conn\r\n");
#endif
                if (!wifi->send("close\r", "CLOS")) {
                    return false;
                }
                if (!wifi->send(cmd, "OPEN"))
                    return false;
            } else {
                return false;
            }
        }

        //send websocket HTTP header
        sprintf(cmd, "GET /%s HTTP/1.1\r\n", path.c_str());
        wifi->send(cmd);
        wifi->send("Upgrade: websocket\r\n");
        wifi->send("Connection: Upgrade\r\n");
        sprintf(cmd, "Host: %s:%s\r\n", ip_domain.c_str(), port.c_str());
        wifi->send(cmd);
        wifi->send("Origin: null\r\n");
        wifi->send("Sec-WebSocket-Key: L159VM0TWUzyDxwJEIEzjw==\r\n");
        if (!wifi->send("Sec-WebSocket-Version: 13\r\n\r\n", "DdLWT/1JcX+nQFHebYP+rqEx5xI="))
            return false;

        wifi->flush();

        //printf("\r\nip_domain: %s\r\npath: /%s\r\nport: %s\r\n\r\n",this->ip_domain.c_str(), this->path.c_str(), this->port.c_str());
        return true;
    }
#ifdef TARGET_LPC1768
    else if (netif == ETH) {
        Host server (*server_ip, atoi(port.c_str()));
        sock->close();
        TCPSocketErr bindErr = sock->connect(server);
        if (bindErr) {
#ifdef DEBUG
            printf("\r\nERROR binderr: %d\r\n", bindErr);
#endif
            return false;
        }

        Timer tmr;
        tmr.start();

        Timer stop;
        stop.start();

        int i = 0;
        while (true) {
            Net::poll();
            if (stop.read() > 3)
                return false;
            if (tmr.read() > 0.05) {
                tmr.reset();
                if (eth_connected) {
                    switch (i) {
                        case 0:
                            sprintf(cmd, "GET /%s HTTP/1.1\r\n", path.c_str());
                            sock->send(cmd, strlen(cmd));
                            i++;
                            break;
                        case 1:
                            sprintf(cmd, "Host: %s:%s\r\n", ip_domain.c_str(), port.c_str());
                            sock->send(cmd, strlen(cmd));
                            i++;
                            break;
                        case 2:
                            sprintf(cmd, "Upgrade: WebSocket\r\n");
                            sock->send(cmd, strlen(cmd));
                            i++;
                            break;
                        case 3:
                            sprintf(cmd, "Origin: null\r\n");
                            sock->send(cmd, strlen(cmd));
                            i++;
                            break;
                        case 4:
                            sprintf(cmd, "Connection: Upgrade\r\n");
                            sock->send(cmd, strlen(cmd));
                            i++;
                            break;
                        case 5:
                            sprintf(cmd, "Sec-WebSocket-Key: L159VM0TWUzyDxwJEIEzjw==\r\n");
                            sock->send(cmd, strlen(cmd));
                            i++;
                            break;
                        case 6:
                            sprintf(cmd, "Sec-WebSocket-Version: 13\r\n\r\n");
                            sock->send(cmd, strlen(cmd));
                            i++;
                            break;
                        case 7:
                            if (response_server_eth)
                                i++;
                            else
                                break;

                        default:
                            break;
                    }
                }
                if (i==8) {
#ifdef DEBUG
                    printf("\r\nip_domain: %s\r\npath: /%s\r\nport: %s\r\n\r\n",this->ip_domain.c_str(), this->path.c_str(), this->port.c_str());
#endif
                    return true;
                }
            }
        }
    }
#endif //target
    //the program shouldn't be here
    return false;
}

void Websocket::sendLength(uint32_t len) {
    if (len < 126) {
        sendChar(len | (1<<7));
    } else if (len < 65535) {
        sendChar(126 | (1<<7));
        sendChar((len >> 8) & 0xff);
        sendChar(len & 0xff);
    } else {
        sendChar(127 | (1<<7));
        for (int i = 0; i < 8; i++) {
            sendChar((len >> i*8) & 0xff);
        }
    }
}

void Websocket::sendChar(uint8_t c) {
    if (netif == WIF) {
        wifi->putc(c);
    }
#ifdef TARGET_LPC1768
    else if (netif == ETH) {
        Net::poll();
        sock->send((const char *)&c, 1);
    }
#endif
}

void Websocket::sendOpcode(uint8_t opcode) {
    sendChar(0x80 | (opcode & 0x0f));
}

void Websocket::sendMask() {
    for (int i = 0; i < 4; i++) {
        sendChar(0);
    }
}

void Websocket::send(char * str) {
    sendOpcode(0x01);
    sendLength(strlen(str));
    sendMask();
    if (netif == WIF) {
        wifi->send(str, NULL);
    }
#ifdef TARGET_LPC1768
    else if (netif == ETH) {
        Net::poll();
        sock->send(str, strlen(str));
    }
#endif //target
}



bool Websocket::read(char * message) {
    int i = 0;
    int length_buffer = 0;
    uint64_t len_msg;
    char opcode = 0;
    char mask[4] = {0, 0, 0, 0};
    Timer tmr;

    if (netif == WIF) {
        length_buffer = wifi->readable();

        if (length_buffer > 1) {
            // read the opcode
            tmr.start();
            while (true) {
                if (tmr.read() > 3) {
                    return false;
                }
                if (wifi->readable()) {
                    opcode = wifi->getc();
                }
                if (opcode == 0x81) {
                    break;
                }
            }
#ifdef DEBUG
            printf("opcode: 0x%X\r\n", opcode);
#endif
            len_msg = wifi->getc() & 0x7f;
            if (len_msg == 126) {
                len_msg = (wifi->getc() << 8);
                len_msg += wifi->getc();
            } else if (len_msg == 127) {
                len_msg = 0;
                for (int i = 0; i < 8; i++) {
                    len_msg += (wifi->getc() << (7-i)*8);
                }
            }
            if (len_msg == 0) {
                return false;
            }
#ifdef DEBUG
            printf("length: %lld\r\n", len_msg);
#endif
            if ((len_msg & 0x80)) {
#ifdef DEBUG
                printf("will read mask\r\n");
#endif
                for (int i = 0; i < 4; i++) {
                    mask[i] = wifi->getc();
#ifdef DEBUG
                    printf("mask[%d]: %d\r\n", i, mask[i]);
#endif
                }
            } else {
#ifdef DEBUG
                printf("len not 0x80\r\n");
#endif
            }
        } else {
            return false;
        }


        for (i = 0; i < len_msg; i++) {
            message[i] = wifi->getc() ^ mask[i % 4];
        }

        message[len_msg] = 0;
        return true;
    }
#ifdef TARGET_LPC1768
    else if (netif == ETH) {

        uint32_t index = 0;
        Net::poll();

        if (new_msg) {
            tmr.start();
            // read the opcode
            while (true) {
                if (tmr.read() > 3) {
                    return false;
                }
                opcode = eth_rx[index++];
                if (opcode == 0x81) {
                    break;
                }
            }
#ifdef DEBUG
            printf("opcode: 0x%X\r\n", opcode);
#endif

            len_msg = eth_rx[index++] & 0x7f;
            if (len_msg == 126) {
                len_msg = (eth_rx[index++] << 8);
                len_msg += eth_rx[index++];
            } else if (len_msg == 127) {
                len_msg = 0;
                for (int i = 0; i < 8; i++) {
                    len_msg += eth_rx[index++] << (7-i)*8;
                }
            }
            if (len_msg == 0) {
                return false;
            }
#ifdef DEBUG
            printf("length: %lld\r\n", len_msg);
#endif
            if ((len_msg & 0x80)) {
                for (int i = 0; i < 4; i++)
                    mask[i] = eth_rx[index++];
            }

            for (i = 0; i < len_msg; i++) {
                message[i] = eth_rx[index++] ^ mask[i % 4];
            }

            message[len_msg] = 0;
            new_msg = false;
            return true;
        }
        return false;
    }
#endif //target
//the program shouldn't be here
    return false;
}

bool Websocket::close() {
    sendOpcode(0x08);
    sendLength(0);
    sendMask();
    if (netif == WIF) {

        wait(0.25);
        if (!wifi->cmdMode()) {
#ifdef DEBUG
            printf("Websocket::close: cannot enter in cmd mode\r\n");
#endif
            return false;
        }
        wait(0.25);

        wifi->send("close\r", NULL);

        if (!wifi->exit())
            return false;
    }
#ifdef TARGET_LPC1768
    else if (netif == ETH) {
#ifdef TARGET_LPC1768
        Net::poll();
#endif //target

        if (sock->close())
            return false;
        return true;
    }
#endif //target
    //the program shouldn't be here
    return false;
}



bool Websocket::connected() {
    if (netif == WIF) {
        char str[10];

        // we have to wait at least 0.25s to enter in cmd mode whan we was sending tcp packets
        wait(0.25);
        if (!wifi->cmdMode()) {
#ifdef DEBUG
            printf("Websocket::connected: cannot enter in cmd mode\r\n");
#endif
            return false;
        }
        wait(0.25);

        wifi->send("show c\r\n", NULL, str);
#ifdef DEBUG
        printf("Websocket::connected: str: %s\r\n", str);
#endif

        if (str[3] == '1') {
            if (!wifi->exit()) {
#ifdef DEBUG
                printf("Websocket::connected: cannot exit\r\n");
#endif
                return false;
            }
            return true;
        }
        if (!wifi->exit()) {
#ifdef DEBUG
            printf("Websocket::connected: cannot exit\r\n");
#endif
        }
        return false;
    }
#ifdef TARGET_LPC1768
    else if (netif == ETH) {
        return eth_connected;
    }
#endif //target
    //the program shouldn't be here
    return false;
}


std::string Websocket::getPath() {
    return path;
}




#ifdef TARGET_LPC1768
void Websocket::onTCPSocketEvent(TCPSocketEvent e) {
    if (e == TCPSOCKET_CONNECTED) {
        eth_connected = true;
#ifdef DEBUG
        printf("TCP Socket Connected\r\n");
#endif
    } else if (e == TCPSOCKET_WRITEABLE) {
    } else if (e == TCPSOCKET_READABLE) {
        int len = sock->recv(eth_rx, 512);
        eth_rx[len] = 0;
        new_msg = true;
        if (!response_server_eth) {
            string checking;
            size_t found = string::npos;
            checking = eth_rx;
            found = checking.find("DdLWT/1JcX+nQFHebYP+rqEx5xI=");
            if (found != string::npos)
                response_server_eth = true;
        }
    } else {
#ifdef DEBUG
        printf("TCP Socket Fail\r\n");
#endif
        eth_connected = false;
    }
}
#endif //target


