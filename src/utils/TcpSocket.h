//-----------------------------------------------------------------------------
// TcpSocket.h
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_TCP_SOCKET_H
#define SUBSIM_TCP_SOCKET_H

#include "Platform.h"
#include "Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class TcpSocket : public Printable {
//-----------------------------------------------------------------------------
private: // variables
  std::string label;
  std::string address;
  int port = -1;
  int handle = -1;
  enum Mode { Unknown, Client, Server, Remote } mode = Unknown;

//-----------------------------------------------------------------------------
public: // constructors
  TcpSocket() = default;
  TcpSocket(TcpSocket&& other) noexcept;
  TcpSocket(const TcpSocket&) = delete;
  TcpSocket& operator=(TcpSocket&& other) noexcept;
  TcpSocket& operator=(const TcpSocket&) = delete;

//-----------------------------------------------------------------------------
private: // constructors
  explicit TcpSocket(const std::string& address,
                     const int port,
                     const int handle) noexcept
    : address(address),
      port(port),
      handle(handle),
      mode(Remote)
  { }

//-----------------------------------------------------------------------------
public: // destructor
  virtual ~TcpSocket() noexcept { close(); }

//-----------------------------------------------------------------------------
public: // static methods
  static bool isValidPort(const int port) noexcept {
    return ((port > 0) && (port <= 0x7FFF));
  }

//-----------------------------------------------------------------------------
public: // Printable implementation
  std::string toString() const override;

//-----------------------------------------------------------------------------
public: // methods
  bool isOpen() const noexcept { return (handle >= 0); }
  bool send(const Printable& p) const { return send(p.toString()); }
  int getHandle() const noexcept { return handle; }
  int getPort() const noexcept { return port; }
  Mode getMode() const noexcept { return mode; }
  std::string getAddress() const { return address; }
  std::string getLabel() const { return label; }
  void setLabel(const std::string& value) { label = value; }

  bool send(const std::string&) const;
  void close() noexcept;
  TcpSocket accept() const;
  TcpSocket& connect(const std::string& hostAddress, const int port);
  TcpSocket& listen(const std::string& bindAddress,
                    const int port,
                    const int backlog = 10);

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return isOpen(); }

  bool operator==(const TcpSocket& other) const noexcept {
    return ((handle >= 0) && (handle == other.handle));
  }

//-----------------------------------------------------------------------------
private: // methods
  void openSocket();
};

} // namespace subsim

#endif // SUBSIM_TCP_SOCKET_H
