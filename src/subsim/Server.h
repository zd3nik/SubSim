//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SERVER_H
#define SUBSIM_SERVER_H

#include "utils/Platform.h"
#include "utils/Input.h"
#include "utils/Socket.h"
#include "utils/Version.h"
#include "GameConfig.h"
#include "Game.h"
#include "Player.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Server {
//-----------------------------------------------------------------------------
public: // enums
  enum {
    DEFAULT_PORT = 9555
  };

//-----------------------------------------------------------------------------
private: // variables
  bool autoStart = false;
  bool repeat = false;
  Game game;
  Input input;
  Socket socket;
  std::set<std::string> blackList;
  std::map<int, PlayerPtr> stagedPlayers;

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
  std::string prompt(Coordinate,
                     const std::string& question,
                     const char fieldDelimeter = 0);

  GameConfig newGameConfig();

  bool getGameTitle(std::string&);
  bool handleUserInput(Coordinate);
  bool isServerHandle(const int) const;
  bool isUserHandle(const int) const;
  bool isValidPlayerName(const std::string&) const;
  bool quitGame(Coordinate);
  bool sendGameInfo(Player&);
  bool waitForInput(const int timeout = -1);
  bool send(Player& recipient, const std::string& msg,
            const bool removeOnFailure = true);

  void addPlayerHandle();
  void beginGame();
  void blacklistAddress(Coordinate);
  void blacklistPlayer(Coordinate);
  void bootPlayer(Coordinate);
  void clearBlacklist(Coordinate);
  void clearScreen();
  void close();
  void handlePlayerInput(const int handle);
  void joinGame(PlayerPtr&);
  void printGameInfo(Coordinate&);
  void printOptions(Coordinate&);
  void printPlayers(Coordinate&);
  void removePlayer(Player&, const std::string& msg = "");
  void removeStagedPlayer(const int);
  void saveResult();
  void sendGameResults();
  void sendToAll(const std::string& msg);
  void startGame(Coordinate);
  void startListening(const int backlog);
  void stopListening();
};

} // namespace subsim

#endif // SUBSIM_SERVER_H
