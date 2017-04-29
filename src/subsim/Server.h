//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SERVER_H
#define SUBSIM_SERVER_H

#include "Platform.h"
#include "Board.h"
#include "Configuration.h"
#include "Game.h"
#include "Input.h"
#include "TcpSocket.h"
#include "Version.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Server {
//-----------------------------------------------------------------------------
public: // enums
  enum {
    DEFAULT_PORT = 7948
  };

//-----------------------------------------------------------------------------
private: // variables
  bool quietMode = false;
  bool autoStart = false;
  bool repeat = false;
  Game game;
  Input input;
  TcpSocket socket;
  std::set<std::string> blackList;
  std::map<int, BoardPtr> newBoards;

//-----------------------------------------------------------------------------
public: // constructors
  Server() { input.addHandle(STDIN_FILENO); }
  Server(Server&&) = delete;
  Server(const Server&) = delete;
  Server& operator=(Server&&) = delete;
  Server& operator=(const Server&) = delete;

//-----------------------------------------------------------------------------
public: // destructor
  ~Server() { close(); }

//-----------------------------------------------------------------------------
public: // static methods
  static Version getVersion();

//-----------------------------------------------------------------------------
public: // methods
  void showHelp();
  bool init();
  bool run();
  bool isAutoStart() const { return autoStart; }
  bool isRepeatOn() const { return repeat; }

//-----------------------------------------------------------------------------
private: // methods
  void sendToAll(const Printable& p) {
    sendToAll(p.toString());
  }

  bool send(Board& recipient, const Printable& p) {
    return send(recipient, p.toString());
  }

  std::string prompt(Coordinate,
                     const std::string& question,
                     const char fieldDelimeter = 0);

  Configuration newGameConfig();

  bool getGameTitle(std::string&);
  bool handleUserInput(Coordinate);
  bool isServerHandle(const int) const;
  bool isUserHandle(const int) const;
  bool isValidPlayerName(const std::string&) const;
  bool quitGame(Coordinate);
  bool sendBoard(Board& recipient, const Board&);
  bool sendGameInfo(Board&);
  bool sendYourBoard(Board&);
  bool waitForInput(const int timeout = -1);
  bool send(Board& recipient, const std::string& msg,
            const bool removeOnFailure = true);

  void addPlayerHandle();
  void blacklistAddress(Coordinate);
  void blacklistPlayer(Coordinate);
  void bootPlayer(Coordinate);
  void clearBlacklist(Coordinate);
  void clearScreen();
  void close();
  void handlePlayerInput(const int handle);
  void joinGame(BoardPtr&);
  void leaveGame(Board&);
  void nextTurn();
  void ping(Board&);
  void printGameInfo(Coordinate&);
  void printOptions(Coordinate&);
  void printPlayers(Coordinate&);
  void rejoinGame(Board&);
  void removeNewBoard(const int);
  void removePlayer(Board&, const std::string& msg = "");
  void saveResult();
  void sendBoardToAll(const Board&);
  void sendGameResults();
  void sendMessage(Board&);
  void sendMessage(Coordinate);
  void sendStart();
  void sendToAll(const std::string& msg);
  void setTaunt(Board&);
  void shoot(Board&);
  void skipBoard(Coordinate);
  void skipTurn(Board&);
  void startGame(Coordinate);
  void startListening(const int backlog);
};

} // namespace subsim

#endif // SUBSIM_SERVER_H
