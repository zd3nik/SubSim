//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SOCKET_H
#define SUBSIM_SOCKET_H

#include "Platform.h"
#include "Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Socket : public Printable {
//-----------------------------------------------------------------------------
private: // variables
  std::string label;
  std::string address;
  int port = -1;
  int handle = -1;
  enum Mode { Unknown, Client, Server, Remote } mode = Unknown;

//-----------------------------------------------------------------------------
public: // constructors
  Socket() = default;
  Socket(Socket&& other) noexcept;
  Socket(const Socket&) = delete;
  Socket& operator=(Socket&& other) noexcept;
  Socket& operator=(const Socket&) = delete;

//-----------------------------------------------------------------------------
private: // constructors
  explicit Socket(const std::string& address,
                     const int port,
                     const int handle) noexcept
    : address(address),
      port(port),
      handle(handle),
      mode(Remote)
  { }

//-----------------------------------------------------------------------------
public: // destructor
  virtual ~Socket() noexcept { close(); }

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
  Socket accept() const;
  Socket& connect(const std::string& hostAddress, const int port);
  Socket& listen(const std::string& bindAddress,
                    const int port,
                    const int backlog = 10);

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return isOpen(); }

  bool operator==(const Socket& other) const noexcept {
    return ((handle >= 0) && (handle == other.handle));
  }

//-----------------------------------------------------------------------------
private: // methods
  void openSocket();
};

} // namespace subsim

#endif // SUBSIM_SOCKET_H
