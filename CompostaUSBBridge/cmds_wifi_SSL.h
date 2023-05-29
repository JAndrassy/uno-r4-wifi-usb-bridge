#ifndef CMDS_WIFI_SSL_H
#define CMDS_WIFI_SSL_H


#include "at_handler.h"

void CAtHandler::add_cmds_wifi_SSL() {
   /* ....................................................................... */
   command_table[_SSLBEGINCLIENT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            if (sslclients_num < MAX_CLIENT_AVAILABLE) {
               for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                  if (sslclients[i] == nullptr) {
                     sslclients[i] = new WiFiClientSecure();
                     if(sslclients[i] == nullptr) {
                        return chAT::CommandStatus::ERROR;
                     }
                     sslclients_num++;
                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i).c_str());
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                  }
               }
            }
            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
   

   /* ....................................................................... */
   command_table[_SSLSETINSERCURE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               srv.write_response_prompt();
               srv.write_str("sock num empty");
               srv.write_line_end();
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if (sslclients[sock] == nullptr) {
               srv.write_response_prompt();
               srv.write_str("null pointer");
               srv.write_line_end();
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            tmp_clt->setInsecure();
            srv.write_response_prompt();
            srv.write_str("setted insecure");
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SETCAROOT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() < 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            bool ca_root_custom = false;
            int ca_root_size = 0;
            if (parser.args.size() >= 2){
               auto &ca_root_size_str = parser.args[1];
               if (ca_root_size_str.empty()) {
                  return chAT::CommandStatus::ERROR;
               }
               ca_root_size = atoi(ca_root_size_str.c_str());
               ca_root_custom = true;
            }

            if(ca_root_custom) {

               std::vector<uint8_t> data_received;
               data_received = srv.inhibit_read(ca_root_size);
               size_t offset = data_received.size();
               
               if(offset < ca_root_size) {

                  data_received.resize(ca_root_size);
                  do {
                     offset += serial->read(data_received.data() + offset, ca_root_size - offset);
                  } while (offset < ca_root_size);
               }
               WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
               tmp_clt->setCACert((const char *)data_received.data());
               srv.continue_read();
            } else {
               SPIFFS.begin(true); //useless here
               if(!SPIFFS.begin(true)){ //useless here call it on top because a singletone
                  return chAT::CommandStatus::ERROR;
               }

               File file = SPIFFS.open("/root_ca_Test.txt", FILE_READ);
               if(!file){
                  return chAT::CommandStatus::ERROR;
               }

               std::vector<uint8_t> buf;
               int length = file.available();
               buf.resize(length);

               file.read(buf.data(), length);
               file.close();

               WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
               tmp_clt->setCACert((const char *)buf.data());
            }

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
   /* ....................................................................... */
   command_table[_SSLCLIENTSTATE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            if (tmp_clt->connected()) {
               String client_status = tmp_clt->remoteIP().toString() + "," + String(tmp_clt->remotePort()) + "," + String(tmp_clt->localPort()) + "\r\n";
               srv.write_response_prompt();
               srv.write_str((const char *)(client_status.c_str()));
               srv.write_line_end();
            }
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTCONNECTNAME] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &host = parser.args[1];
            if (host.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            auto &port = parser.args[2];
            if (port.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            if (!tmp_clt->connect(host.c_str(), atoi(port.c_str()))) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
 
   /* ....................................................................... */
   command_table[_SSLCLIENTCONNECTIP] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());

            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }
            
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &hostip = parser.args[1];
            if (hostip.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            auto &hostport = parser.args[2];
            if (hostport.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            IPAddress address;
            if(!address.fromString(hostip.c_str())) {
               return chAT::CommandStatus::ERROR;
            }

            if(sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            if (!tmp_clt->connect(address, atoi(hostport.c_str()))) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTSEND] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            /* the command receive 2 data: the socket and the length of data 
               to be transmitted */

            if (parser.args.size() != 2) {
               
               return chAT::CommandStatus::ERROR;
            }

            /* socket */
            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               
               return chAT::CommandStatus::ERROR;
            }
            if (sslclients[sock] == nullptr) {
               
               return chAT::CommandStatus::ERROR;
            }

            /* data len */
            auto &size_p = parser.args[1];
            if (size_p.empty()) {
               
               return chAT::CommandStatus::ERROR;
            }

            int data_size = atoi(size_p.c_str());

            if(data_size <= 0) {
               srv.write_str("F");
               return chAT::CommandStatus::ERROR;
            }

            /* socket and data received 
               answer back _CLIENTDATA: DATA\r\n 
               so that data transmission can begin */
            //srv.write_response_prompt();
            //srv.write_str(" DATA");
            //srv.write_line_end();
            //return chAT::CommandStatus::OK;
            
            /* -----------------------------------
             * BEGIN TRANSPARENT DATA TRANSMISSION
             * ----------------------------------- */
            std::vector<uint8_t> data_received;
            data_received = srv.inhibit_read(data_size);
            size_t offset = data_received.size();
            
            if(offset < data_size) {

               data_received.resize(data_size);
               do {
                  offset += serial->read(data_received.data() + offset, data_size - offset);
               } while (offset < data_size);
            }

            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            auto ok = tmp_clt->write(data_received.data(), data_received.size());

            srv.continue_read();
            if (!ok) {
              return chAT::CommandStatus::ERROR;
            }
            
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTCLOSE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(sock_num.c_str());

            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }
            

            if (sslclients[sock] == nullptr) {
               
            }
            else {
               WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
               tmp_clt->stop();
               sslclients_num--;

               delete sslclients[sock];
               sslclients[sock] = nullptr;
            }
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLIPCLIENT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(sock_num.c_str());
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            if (tmp_clt != nullptr && tmp_clt->connected()) {
              String client_status = tmp_clt->localIP().toString() + "\r\n";
              srv.write_response_prompt();
              srv.write_str((const char *)(client_status.c_str()));
            }
            return chAT::CommandStatus::OK;
         }
         default:
           return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTCONNECTED] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {// write to do the read of a specific list
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }

            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(socket_num.c_str());
            
            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if (sslclients[sock] == nullptr) {
              return chAT::CommandStatus::ERROR;
            }

            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            srv.write_response_prompt();
            String con(tmp_clt->connected());
            srv.write_str((const char *)(con.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;

         }
         default:
           return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTRECEIVE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 2) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &size = parser.args[1];
            if (size.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int data_wanted = atoi(size.c_str());
            if(data_wanted <= 0) {
               return chAT::CommandStatus::ERROR;
            } 
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            int data_available = tmp_clt->available();
            data_wanted = (data_wanted < data_available) ? data_wanted : data_available;

            std::vector<uint8_t> data_received;
            data_received.resize(data_wanted);

            int res = tmp_clt->read(data_received.data(), data_wanted);
            String results = String(data_received.size()) + "|";
            
            srv.write_response_prompt();
            srv.write_str((const char *)(results.c_str()));
            srv.write_vec8(data_received);
            srv.write_line_end();

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLAVAILABLE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
        
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            srv.write_response_prompt();
            String av(tmp_clt->available());
            srv.write_str((const char *)(av.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTSTATUS] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            
            srv.write_response_prompt();
            //String st(sslclients[sock]->status());
            //srv.write_str((const char *)(st.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTFLUSH] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
        
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            tmp_clt->flush();
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLREMOTEIP] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
        
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            IPAddress ip = tmp_clt->remoteIP();
            srv.write_response_prompt();
            srv.write_str((const char *)(ip.toString().c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLREMOTEPORT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
        
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            String port(tmp_clt->remotePort());
            srv.write_response_prompt();
            srv.write_str((const char *)(port.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };


   /* ....................................................................... */
   command_table[_SSLPEEK] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (sslclients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) sslclients[sock];
            srv.write_response_prompt();
            String p(tmp_clt->peek());
            srv.write_str((const char *)(p.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
}

#endif
