/**
* @author Samuel Mokrani
*
* @section LICENSE
*
* Copyright (c) 2011 mbed
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @section DESCRIPTION
*    Simple websocket client
*
*/ 

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "mbed.h"
#include "Wifly.h"
#include <string>


#ifdef TARGET_LPC1768
#include "EthernetNetIf.h"
#include "TCPSocket.h"
#include "dnsresolve.h"
#endif //target


/** Websocket client Class. 
 *
 * Warning: you must use a wifi module (Wifly RN131-C) or an ethernet network to use this class
 *
 * Example (wifi network):
 * @code
 * #include "mbed.h"
 * #include "Wifly.h"
 * #include "Websocket.h"
 * 
 * DigitalOut l1(LED1);
 * 
 * //Here, we create an instance, with pins 9 and 10 connecting to the
 * //WiFly's TX and RX pins, and pin 21 to RESET. We are connecting to the
 * //"mbed" network, password "password", and we are using WPA.
 * Wifly wifly(p9, p10, p21, "mbed", "password", true);
 * 
 * //Here, we create a Websocket instance in 'wo' (write-only) mode
 * //on the 'samux' channel
 * Websocket ws("ws://sockets.mbed.org/ws/samux/wo", &wifly);
 * 
 * 
 * int main() {
 *     while (1) {
 * 
 *         //we connect the network
 *         while (!wifly.join()) {
 *             wifly.reset();
 *         }
 * 
 *         //we connect to the websocket server
 *         while (!ws.connect());
 * 
 *         while (1) {
 *             wait(0.5);
 * 
 *             //Send Hello world
 *             ws.send("Hello World! over Wifi");
 *             
 *             // show that we are alive
 *             l1 = !l1;
 *         }
 *     }
 * }
 * @endcode
 *
 *
 *
 * Example (ethernet network):
 * @code
 * #include "mbed.h"
 * #include "Websocket.h"
 * 
 * Timer tmr;
 * 
 * //Here, we create a Websocket instance in 'wo' (write-only) mode
 * //on the 'samux' channel
 * Websocket ws("ws://sockets.mbed.org/ws/samux/wo");
 * 
 * int main() {
 *     while (1) {
 * 
 *         while (!ws.connect());
 * 
 *         tmr.start();
 *         while (1) {
 *             if (tmr.read() > 0.5) {
 *                 ws.send("Hello World! over Ethernet");
 *                 tmr.start();
 *             }
 *             Net::poll();
 *         }
 *     }
 * }
 * @endcode
 */
class Websocket
{
    public:
        /**
        * Constructor
        *
        * @param url The Websocket url in the form "ws://ip_domain[:port]/path"  (by default: port = 80)
        * @param wifi pointer to a wifi module (the communication will be establish by this module)
        */
        Websocket(char * url, Wifly * wifi);
        
#ifdef TARGET_LPC1768
        /**
        * Constructor for an ethernet communication
        *
        * @param url The Websocket url in the form "ws://ip_domain[:port]/path" (by default: port = 80)
        */
        Websocket(char * url);
#endif //target
        
        /**
        * Connect to the websocket url
        *
        *@return true if the connection is established, false otherwise
        */
        bool connect();
        
        /**
        * Send a string according to the websocket format
        *
        * @param str string to be sent
        */
        void send(char * str);
        
        /**
        * Read a websocket message
        *
        * @param message pointer to the string to be read (null if drop frame)
        *
        * @return true if a string has been read, false otherwise
        */
        bool read(char * message);
        
        /**
        * To see if there is a websocket connection active
        *
        * @return true if there is a connection active
        */
        bool connected();
        
        /**
        * Close the websocket connection
        *
        * @return true if the connection has been closed, false otherwise
        */
        bool close();
        
        /**
        * Accessor: get path from the websocket url
        *
        * @return path
        */
        std::string getPath();
        
        enum Type {
            WIF,
            ETH
        };
        
    
    private:
    
        
        void fillFields(char * url);
        void sendOpcode(uint8_t opcode);
        void sendLength(uint32_t len);
        void sendMask();
        void sendChar(uint8_t c);
        
        std::string ip_domain;
        std::string path;
        std::string port;
        
        Wifly * wifi;
        
#ifdef TARGET_LPC1768
        void onTCPSocketEvent(TCPSocketEvent e);
        bool eth_connected;
        bool eth_readable;
        bool eth_writeable;
        char eth_rx[512];
        bool response_server_eth;
        bool new_msg;
        
        EthernetNetIf * eth;
        TCPSocket * sock;
        IpAddr * server_ip;
#endif //target
        
        Type netif;

};

#endif