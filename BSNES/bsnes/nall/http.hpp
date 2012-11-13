#ifndef NALL_HTTP_HPP
#define NALL_HTTP_HPP

#if !defined(_WIN32)
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
#else
  #include <windows.h>
  #include <winsock2.h>
  #include <ws2tcpip.h>
#endif

#include <nall/platform.hpp>
#include <nall/string.hpp>

namespace nall {

struct http {
  string hostname;
  addrinfo *serverinfo;
  int serversocket;
  string header;

  inline void download(const string &path, uint8_t *&data, unsigned &size) {
    data = 0;
    size = 0;

    send({
      "GET ", path, " HTTP/1.1\r\n"
      "Host: ", hostname, "\r\n"
      "Connection: close\r\n"
      "\r\n"
    });

    header = downloadHeader();
    downloadContent(data, size);
  }

  inline bool connect(string host, unsigned port) {
    hostname = host;

    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(hostname, string(port), &hints, &serverinfo);
    if(status != 0) return false;

    serversocket = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if(serversocket == -1) return false;

    int result = ::connect(serversocket, serverinfo->ai_addr, serverinfo->ai_addrlen);
    if(result == -1) return false;

    return true;
  }

  inline bool send(const string &data) {
    return send((const uint8_t*)(const char*)data, data.length());
  }

  inline bool send(const uint8_t *data, unsigned size) {
    while(size) {
      int length = ::send(serversocket, (const char*)data, size, 0);
      if(length == -1) return false;
      data += length;
      size -= length;
    }
    return true;
  }

  inline string downloadHeader() {
    string output;
    do {
      char buffer[2];
      int length = recv(serversocket, buffer, 1, 0);
      if(length <= 0) return output;
      buffer[1] = 0;
      output.append(buffer);
    } while(output.endswith("\r\n\r\n") == false);
    return output;
  }

  inline string downloadChunkLength() {
    string output;
    do {
      char buffer[2];
      int length = recv(serversocket, buffer, 1, 0);
      if(length <= 0) return output;
      buffer[1] = 0;
      output.append(buffer);
    } while(output.endswith("\r\n") == false);
    return output;
  }

  inline void downloadContent(uint8_t *&data, unsigned &size) {
    unsigned capacity = 0;

    if(header.iposition("\r\nTransfer-Encoding: chunked\r\n")) {
      while(true) {
        unsigned length = hex(downloadChunkLength());
        if(length == 0) break;
        capacity += length;
        data = (uint8_t*)realloc(data, capacity);

        char buffer[length];
        while(length) {
          int packetlength = recv(serversocket, buffer, length, 0);
          if(packetlength <= 0) break;
          memcpy(data + size, buffer, packetlength);
          size += packetlength;
          length -= packetlength;
        }
      }
    } else if(auto position = header.iposition("\r\nContent-Length: ")) {
      unsigned length = decimal((const char*)header + position() + 18);
      while(length) {
        char buffer[256];
        int packetlength = recv(serversocket, buffer, min(256, length), 0);
        if(packetlength <= 0) break;
        capacity += packetlength;
        data = (uint8_t*)realloc(data, capacity);
        memcpy(data + size, buffer, packetlength);
        size += packetlength;
        length -= packetlength;
      }
    } else {
      while(true) {
        char buffer[256];
        int packetlength = recv(serversocket, buffer, 256, 0);
        if(packetlength <= 0) break;
        capacity += packetlength;
        data = (uint8_t*)realloc(data, capacity);
        memcpy(data + size, buffer, packetlength);
        size += packetlength;
      }
    }

    data = (uint8_t*)realloc(data, capacity + 1);
    data[capacity] = 0;
  }

  inline void disconnect() {
    close(serversocket);
    freeaddrinfo(serverinfo);
    serverinfo = 0;
    serversocket = -1;
  }

  #ifdef _WIN32
  inline int close(int sock) {
    return closesocket(sock);
  }

  inline http() {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED) {
      WSADATA wsaData;
      if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        WSACleanup();
        return;
      }
    } else {
      close(sock);
    }
  }
  #endif
};

}

#endif
