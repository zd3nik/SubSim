//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Game.h"
#include "utils/Error.h"
#include "utils/Logger.h"
#include "utils/Msg.h"
#include "utils/Screen.h"
#include "utils/StringUtils.h"
#include "db/DBRecord.h"
#include "Mine.h"
#include "Obstacle.h"
#include "Submarine.h"

namespace subsim
{

//-----------------------------------------------------------------------------
enum {
  TORPEDO,
  MINE,
};

//-----------------------------------------------------------------------------
bool
Game::addCommand(const int handle, Input& input, std::string& err) {
  PlayerPtr player = getPlayer(handle);
  if (!player) {
    throw Error(Msg() << "Game::addCommand() Invalid player handle: "
                << handle);
  }

  if (!isStarted()) {
    err = "Game has not started";
    return false;
  } else if (isFinished()) {
    err = "Game is finished";
    return false;
  }

  std::set<unsigned> subIDs;
  const unsigned playerID = static_cast<unsigned>(handle);
  for (const auto& command : commands) {
    if (command->getPlayerID() == playerID) {
      subIDs.insert(command->getSubID());
    }
  }

  if (input.getFieldCount() < 3) {
    err = "Command messages must begin with turn number and sub ID";
    return false;
  }

  const unsigned tn = input.getUInt(1, ~0U);
  if (tn != turnNumber) {
    err = ("Invalid turn number: " + input.getStr(1));
    return false;
  }

  const unsigned subID = input.getUInt(2, ~0U);
  if (subID >= config.getSubsPerPlayer()) {
    err = ("Invalid sub ID: " + input.getStr(2));
    return false;
  } else if (subIDs.count(subID)) {
    err = ("Multiple commands for sub ID " + toStr(subID));
    return false;
  }

  const std::string type = input.getStr();
  if (type.size() != 1) {
    err = ("Invalid command type: " + type);
    return false;
  }

  const Submarine& sub = player->getSubmarine(subID);
  if (sub.isDead()) {
    err = ("Command submitted for dead sub ID" + toStr(subID));
    return false;
  } else if (sub.isSurfaced()) {
    err = ("Command submitted for surfaced sub ID " + toStr(subID));
    return false;
  }

  try {
    switch (type[0]) {
    case MineCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new MineCommand(playerID, input)));
      break;
    case FireCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new FireCommand(playerID, input)));
      break;
    case MoveCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new MoveCommand(playerID, input)));
      break;
    case PingCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new PingCommand(playerID, input)));
      break;
    case SprintCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new SprintCommand(playerID, input)));
      break;
    case SleepCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new SleepCommand(playerID, input)));
      break;
    case SurfaceCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new SurfaceCommand(playerID, input)));
      break;
    default:
      err = ("Invalid command type: " + type);
      return false;
    }
  } catch (const std::exception& e) {
    err = e.what();
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
PlayerPtr
Game::getPlayer(const int handle) const {
  if (handle > 0) {
    for (auto it = players.begin(); it != players.end(); ++it) {
      const PlayerPtr& player = it->second;
      if (player->handle() == handle) {
        ASSERT(player->getPlayerID() == static_cast<unsigned>(handle));
        return player;
      }
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
PlayerPtr
Game::getPlayer(const std::string name) const {
  if (!isEmpty(name)) {
    for (auto it = players.begin(); it != players.end(); ++it) {
      const PlayerPtr& player = it->second;
      if (iEqual(player->getName(), name)) {
        return player;
      }
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
std::vector<PlayerPtr>
Game::getPlayers() const {
  std::vector<PlayerPtr> result;
  result.reserve(players.size());
  for (auto it = players.begin(); it != players.end(); ++it) {
    result.push_back(it->second);
  }
  return std::move(result);
}

//-----------------------------------------------------------------------------
std::vector<PlayerPtr>
Game::playersFromAddress(const std::string address) const {
  std::vector<PlayerPtr> result;
  if (!isEmpty(address)) {
    for (auto it = players.begin(); it != players.end(); ++it) {
      const PlayerPtr& player = it->second;
      if (player->getAddress() == address) {
        result.push_back(player);
      }
    }
  }
  return std::move(result);
}

//-----------------------------------------------------------------------------
void
Game::printSummary(Coordinate& coord) const {
  config.print(title, coord);
  gameMap.printSummary(coord.south().setX(1));
}

//-----------------------------------------------------------------------------
void
Game::reset(const GameConfig& gameConfig, const std::string& gameTitle) {
  title = gameTitle;
  config = gameConfig;
  gameMap.reset(config.getMapWidth(), config.getMapHeight());
  started = 0;
  aborted = 0;
  finished = 0;
  turnNumber = 0;

  for (const Coordinate& coord : config.getObstacles()) {
    gameMap.addObject(coord, std::make_unique<Obstacle>());
  }
}

//-----------------------------------------------------------------------------
void
Game::abort() noexcept {
  if (!aborted) {
    aborted = Timer::now();
  }
}

//-----------------------------------------------------------------------------
void
Game::finish() noexcept {
  if (!finished) {
    finished = Timer::now();
  }
}

//-----------------------------------------------------------------------------
std::map<unsigned, std::string>
Game::start() {
  config.validate();
  if (started) {
    throw Error("Game is already started");
  } else if (!canStart()) {
    throw Error("Game cannot start");
  } else if (turnNumber) {
    throw Error(Msg() << "Game turn number is " << turnNumber
                << " at game start!");
  }
  started = Timer::now();
  turnNumber = 1;
  errs.clear();

  sendToAll(Msg('B') << turnNumber);

  return errs;
}

//-----------------------------------------------------------------------------
std::string
Game::addPlayer(PlayerPtr player, Input& input) {
  if (!player) {
    throw Error("Game::addPlayer() null player");
  } else if (started) {
    throw Error("Game::addPlayer() game has already started");
  } else if (isEmpty(player->getName())) {
    throw Error("Game::addPlayer() empty player name");
  } else if (player->handle() <= 0) {
    throw Error(Msg() << "Game::addPlayer() invalid player handle: "
                << player->handle());
  } else if (getPlayer(player->getName())) {
    throw Error(Msg() << "Game::addPlayer() duplicate player name: "
                << player->getName());
  }

  unsigned i = 2; // input field index
  unsigned subID = 0;
  for (const Submarine& subConfig : config.getSubmarineConfigs()) {
    // double check sub ID
    if (subConfig.getObjectID() != subID) {
      throw Error(Msg() << "Invalid object ID (" << subConfig.getObjectID()
                  << " on sub config " << subID);
    }

    // verify sub has a valid starting location
    Coordinate coord(subConfig.getLocation());
    if (!coord) {
      const unsigned x = input.getUInt(i++);
      const unsigned y = input.getUInt(i++);
      if (!gameMap.contains(coord.set(x, y))) {
        return Msg() << "Missing or invalid coordinates for sub ID " << subID;
      }
    }
    if (gameMap.getSquare(coord).isBlocked()) {
      return Msg() << "Coordinate " << coord << " is blocked";
    }

    SubmarinePtr sub = std::make_shared<Submarine>(player->handle(), subID,
                                                   subConfig);
    sub->setLocation(coord);

    // add submarine to player
    player->addSubmarine(sub);
    subID++;
  }
  if (i != input.getFieldCount()) {
    return "Incorrect number of submarine coordinate values";
  }

  // add player to player map
  players[player->handle()] = player;

  // add player submarines to game map
  for (unsigned subID = 0; subID < player->getSubmarineCount(); ++subID) {
    SubmarinePtr sub = player->getSubmarinePtr(subID);
    if (sub->getLocation()) {
      gameMap.addObject(sub->getLocation(), sub);
    }
  }

  return "";
}

//-----------------------------------------------------------------------------
void
Game::removePlayer(const int handle) {
  for (auto it = commands.begin(); it != commands.end(); ) {
    const UniqueCommand& command = (*it);
    if (command->getPlayerID() == static_cast<unsigned>(handle)) {
      it = commands.erase(it);
    } else {
      it++;
    }
  }
  for (auto it = players.begin(); it != players.end(); ++it) {
    PlayerPtr player = it->second;
    if (player->handle() == handle) {
      for (unsigned subID = 0; subID < player->getSubmarineCount(); ++subID) {
        SubmarinePtr sub = player->getSubmarinePtr(subID);
        if (sub->getLocation()) {
          gameMap.removeObject(sub->getLocation(), sub);
        }
      }
      players.erase(it);
      return;
    }
  }
}

//-----------------------------------------------------------------------------
void
Game::saveResults(Database& db) const {
  if (isEmpty(title)) {
    throw Error(Msg() << "Can't same results of a game with no title");
  }

  config.validate();

  unsigned hits = 0;
  unsigned highScore = 0;
  unsigned lowScore = ~0U;
  for (auto it = players.begin(); it != players.end(); ++it) {
    const PlayerPtr& player = it->second;
    hits += player->getScore();
    highScore = std::max<unsigned>(highScore, player->getScore());
    lowScore = std::min<unsigned>(lowScore, player->getScore());
  }

  unsigned ties = 0;
  for (auto it = players.begin(); it != players.end(); ++it) {
    const PlayerPtr& player = it->second;
    ties += (player->getScore() == highScore);
  }
  if (ties > 0) {
    ties--;
  } else {
    Logger::error() << "Error calculating ties for game '" << getTitle() << "'";
  }

  std::shared_ptr<DBRecord> stats = db.get(("game." + getTitle()), true);
  if (!stats) {
    throw Error(Msg() << "Failed to get stats record for game title '"
                << getTitle() << "' from '" << db << "'");
  }

  config.saveTo(*stats);

  Milliseconds count = stats->incUInt("gameCount");
  Milliseconds elapsed = elapsedTime();
  Milliseconds totalMS = (stats->getUInt64("total.timeMS") + elapsed);
  Milliseconds avgMS = count ? ((totalMS + count - 1) / count) : 0;

  stats->setString("averageTime", Timer(Timer::now() - avgMS).toString());
  stats->setString("total.time", Timer(Timer::now() - totalMS).toString());
  stats->incUInt64("total.timeMS", elapsed);
  stats->incUInt("total.aborted", (aborted ? 1 : 0));
  stats->incUInt("total.turnNumber", turnNumber);
  stats->incUInt("total.playerCount", players.size());
  stats->incUInt("total.hits", hits);
  stats->incUInt("total.ties", ties);

  stats->setString("last.time", Timer(Timer::now() - elapsed).toString());
  stats->setUInt64("last.timeMS", elapsed);
  stats->setBool("last.aborted", aborted);
  stats->setUInt("last.turnNumber", turnNumber);
  stats->setUInt("last.playerCount", players.size());
  stats->setUInt("last.hits", hits);
  stats->setUInt("last.ties", ties);

  for (auto it = players.begin(); it != players.end(); ++it) {
    const PlayerPtr& player = it->second;
    const bool first = (player->getScore() == highScore);
    const bool last = (player->getScore() == lowScore);
    player->addStatsTo(*stats, first, last);

    std::string recordID = ("player." + player->getName());
    auto record = db.get(recordID, true);
    if (!record) {
      throw Error(Msg() << "Failed to get record for player '"
                  << player->getName() << "' from '" << db << "'");
    }
    player->saveTo((*record), (players.size() - 1), first, last);
  }
}

//-----------------------------------------------------------------------------
bool
Game::canStart() const noexcept {
  if (isEmpty(title)) {
    return false;
  }
  if (players.size() < config.getMinPlayers()) {
    return false;
  }
  if (config.getMaxPlayers() && (players.size() > config.getMaxPlayers())) {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Game::allCommandsReceived() const noexcept {
  if (!started || isFinished() || commands.empty()) {
    return false;
  }

  std::map<unsigned, std::set<unsigned>> ids;
  for (auto it = players.begin(); it != players.end(); ++it) {
    const PlayerPtr& player = it->second;
    if (!player) {
      throw Error("Null player in game.players map");
    }
    for (unsigned subID = 0; subID < player->getSubmarineCount(); ++subID) {
      const Submarine& sub = player->getSubmarine(subID);
      if (!sub.isDead() && !sub.isSurfaced()) {
        ids[player->getPlayerID()].insert(subID);
      }
    }
  }

  if (ids.empty()) {
    return false;
  }

  for (auto it = commands.begin(); it != commands.end(); ++it) {
    const UniqueCommand& commandPtr = (*it);
    if (!commandPtr) {
      throw Error("Null command in command list");
    }

    const Command& command = (*commandPtr);
    if (command.getType() == Command::Invalid) {
      throw Error("Invalid command in command list");
    } else if (command.getTurnNumber() != turnNumber) {
      throw Error(Msg() << "Incorrect turn number (" << command.getTurnNumber()
                  << " in command from command list");
    }

    auto player = ids.find(command.getPlayerID());
    if (player == ids.end()) {
      throw Error(Msg() << "Command for dead player ID ("
                  << command.getPlayerID() << ") in command list");
    }

    auto sub = player->second.find(command.getSubID());
    if (sub == player->second.end()) {
      throw Error(Msg() << "Command for dead sub ID (" << command.getSubID()
                  << ") player ID (" << command.getPlayerID()
                  << ") in command list");
    }

    player->second.erase(sub);
    if (player->second.empty()) {
      ids.erase(player);
    }
  }

  return ids.empty();
}

//-----------------------------------------------------------------------------
void
Game::sendToAll(const std::string& message) {
  for (auto it = players.begin(); it != players.end(); ++it) {
    PlayerPtr& player = it->second;
    if (player) {
      sendTo((*player), message);
    }
  }
}

//-----------------------------------------------------------------------------
bool
Game::sendTo(Player& player, const std::string& message) {
  if (player.send(message)) {
    return true;
  }
  if (!errs.count(player.getPlayerID())) {
    errs[player.getPlayerID()] = "I/O error";
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
Game::sendDiscoveredObjects(Player& player) {
  for (auto pair : discovered[player.getPlayerID()]) {
    const Coordinate coord(pair.first);
    const unsigned size = pair.second;
    if (!sendTo(player, Msg('O') << turnNumber << coord << size)) {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Game::sendTorpedoHits(Player& player) {
  for (auto pair : torpedoHits[player.getPlayerID()]) {
    const Coordinate coord(pair.first);
    const unsigned damage = pair.second;
    if (!sendTo(player, Msg('T') << turnNumber << coord << damage)) {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Game::sendMineHits(Player& player) {
  for (auto pair : mineHits[player.getPlayerID()]) {
    const Coordinate coord(pair.first);
    const unsigned damage = pair.second;
    if (!sendTo(player, Msg('M') << turnNumber << coord << damage)) {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Game::sendSubInfo(Player& player) {
  for (unsigned subID = 0; subID < player.getSubmarineCount(); ++subID) {
    const Submarine& sub = player.getSubmarine(subID);
    if (!sendTo(player, Msg('I') << turnNumber << subID << sub.getLocation()
                << sub.getShieldCount() << sub.getReactorDamage()
                << (sub.isSurfaced() ? 1 : 0)
                << (sub.isDead() ? 1 : 0)))
    {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Game::sendScore(Player& player) {
  return sendTo(player, Msg('H') << turnNumber << player.getScore());
}

//-----------------------------------------------------------------------------
std::map<unsigned, std::string>
Game::executeTurn() {
  if (!started) {
    throw Error("Game::executeTurn() game has not been started");
  } else if (!turnNumber) {
    throw Error("Game::executeTurn() turn number has not been initialized!");
  }

  sonarActivations = 0;
  sprintActivations = 0;
  nuclearDetonations.clear();
  detonations.clear();
  discovered.clear();
  torpedoHits.clear();
  mineHits.clear();
  errs.clear();

  exec(Command::Sleep);
  exec(Command::Move);
  exec(Command::Sprint);
  exec(Command::DeployMine);
  exec(Command::FireTorpedo);
  executeNuclearDetonations();
  exec(Command::Surface);
  executeRepairs();
  exec(Command::Ping);

  commands.clear();

  if (sonarActivations) {
    sendToAll(Msg('S') << turnNumber << sonarActivations);
  }
  if (sprintActivations) {
    sendToAll(Msg('R') << turnNumber << sprintActivations);
  }
  for (auto pair : detonations) {
    sendToAll(Msg('D') << turnNumber << pair.first << pair.second);
  }

  for (auto it = players.begin(); it != players.end(); ) {
    PlayerPtr& player = it->second;
    if (!player) {
      throw Error("Null player in game.players map");
    }
    if (!sendDiscoveredObjects(*player) ||
        !sendTorpedoHits(*player) ||
        !sendMineHits(*player) ||
        !sendSubInfo(*player) ||
        !sendScore(*player))
    {
      for (unsigned subID = 0; subID < player->getSubmarineCount(); ++subID) {
        SubmarinePtr sub = player->getSubmarinePtr(subID);
        if (sub->getLocation()) {
          gameMap.removeObject(sub->getLocation(), sub);
        }
      }
      it = players.erase(it);
    } else {
      it++;
    }
  }

  if ((players.size() < 2) ||
      (config.getMaxTurns() && (turnNumber >= config.getMaxTurns())))
  {
    finish();
  } else {
    sendToAll(Msg('B') << ++turnNumber);
  }

  return errs;
}

//-----------------------------------------------------------------------------
void
Game::exec(const Command::CommandType type) {
  for (const UniqueCommand& command : commands) {
    if (command->getType() == type) {
      PlayerPtr player = getPlayer(static_cast<int>(command->getPlayerID()));
      SubmarinePtr sub = player->getSubmarinePtr(command->getSubID());

      if (sub->isDead()) {
        continue;
      } else if (sub->isSurfaced()) {
        throw Error("Game::exec() command queued for surfaced submarine!");
      } else if (sub->hasDetonated()) {
        throw Error("Game::exec() nuclear detonations out of sync!");
      }

      switch (type) {
      case Command::Invalid:
        throw Error("Invalid command in command queue");
      case Command::Sleep:
        exec(sub, static_cast<const SleepCommand&>(*command));
        break;
      case Command::Move:
        exec(sub, static_cast<const MoveCommand&>(*command));
        break;
      case Command::Sprint:
        exec(sub, static_cast<const SprintCommand&>(*command));
        break;
      case Command::DeployMine:
        exec(sub, static_cast<const MineCommand&>(*command));
        break;
      case Command::FireTorpedo:
        exec(sub, static_cast<const FireCommand&>(*command));
        break;
      case Command::Surface:
        exec(sub, static_cast<const SurfaceCommand&>(*command));
        break;
      case Command::Ping:
        exec(sub, static_cast<const PingCommand&>(*command));
        break;
      }

      if (sub->hasDetonated()) {
        nuclearDetonations.push_back(sub);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
Game::exec(SubmarinePtr& sub, const SleepCommand& command) {
  if (!sub->charge(command.getEquip1()) || !sub->charge(command.getEquip2())) {
    errs[sub->getPlayerID()] = "Illegal sleep command";
  }
}

//-----------------------------------------------------------------------------
void
Game::exec(SubmarinePtr& sub, const MoveCommand& command) {
  const Coordinate from = sub->getLocation();
  const Coordinate to = (from + command.getDirection());

  if (gameMap.contains(to) && !gameMap.getSquare(to).isBlocked() &&
      sub->charge(command.getEquip()))
  {
    gameMap.moveObject(from, to, sub);
    detonateMines(gameMap.getSquare(to));
  } else {
    errs[sub->getPlayerID()] = "Illegal move command";
  }
}

//-----------------------------------------------------------------------------
void
Game::exec(SubmarinePtr& sub, const SprintCommand& command) {
  const Coordinate from = sub->getLocation();
  Coordinate to(from + command.getDirection());

  if (sub->sprint(command.getDistance())) {
    unsigned dist = 0;
    for (unsigned i = 0; i < command.getDistance(); ++i) {
      to.shift(command.getDirection());
      if (gameMap.contains(to) && !gameMap.getSquare(to).isBlocked()) {
        dist++;
        gameMap.moveObject(from, to, sub);
        if (detonateMines(gameMap.getSquare(to))) {
          break;
        }
      } else {
        errs[sub->getPlayerID()] = "Illegal sprint command";
        break;
      }
    }
    if (dist) {
      sprintActivations++;
    }
  }
}

//-----------------------------------------------------------------------------
void
Game::exec(SubmarinePtr& sub, const MineCommand& command) {
  const Coordinate to = (sub->getLocation() + command.getDirection());
  if (gameMap.contains(to) && !gameMap.getSquare(to).isBlocked()) {
    if (sub->mine()) {
      Square& dest = gameMap.getSquare(to);
      const bool occupied = dest.isOccupied();
      gameMap.addObject(to, std::make_shared<Mine>(sub->getPlayerID()));
      if (occupied) {
        detonateMines(gameMap.getSquare(to));
      }
    }
  } else {
    errs[sub->getPlayerID()] = "Illegal mine command";
  }
}

//-----------------------------------------------------------------------------
void
Game::exec(SubmarinePtr& sub, const FireCommand& command) {
  const Coordinate to = command.getDestination();
  if (gameMap.contains(to) && !gameMap.getSquare(to).isBlocked()) {
    const auto dests = gameMap.squaresInRangeOf(sub->getLocation(),
                                                sub->getTorpedoRange());
    const auto it = dests.find(to);
    const unsigned distance = (it == dests.end()) ? ~0U : it->second;
    if (sub->fire(distance)) {
      PlayerPtr player = getPlayer(static_cast<int>(sub->getPlayerID()));
      if (player) {
        detonationFrom(player, to, TORPEDO, gameMap.getSquare(to));
      }
    }
  } else {
    errs[sub->getPlayerID()] = "Illegal fire command";
  }
}

//-----------------------------------------------------------------------------
void
Game::exec(SubmarinePtr& sub, const SurfaceCommand&) {
  if (!sub->surface()) {
    errs[sub->getPlayerID()] = "Illegal surface command";
  }
}

//-----------------------------------------------------------------------------
void
Game::exec(SubmarinePtr& sub, const PingCommand&) {
  const unsigned range = sub->ping();
  if (range) {
    sonarActivations++;
    auto dests = gameMap.squaresInRangeOf(sub->getLocation(), range);
    for (auto it = dests.begin(); it != dests.end(); ++it) {
      if (it->second > 0) {
        const Square& square = gameMap.getSquare(it->first);
        if (square.isOccupied()) {
          discovered[sub->getPlayerID()].push_back(
                std::make_pair(it->first, square.getSizeOfObjects()));
        }
      } else {
        ASSERT(it->first == sub->getLocation());
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
Game::executeNuclearDetonations() {
  for (SubmarinePtr sub : nuclearDetonations) {
    detonations.push_back(std::make_pair(sub->getLocation(), 2U));
    Square& square = gameMap.getSquare(sub->getLocation());
    for (auto it = square.begin(); it != square.end(); ) {
      Object* object = it->get();
      if (!object->isPermanent()) {
        Submarine* sub = dynamic_cast<Submarine*>(object);
        if (sub) {
          sub->kill();
        }
        it = square.erase(it);
      } else {
        it++;
      }
    }

    auto coords = getBlastCoordinates(square, 2);
    for (const Coordinate& coord : coords) {
      detonateMines(gameMap.getSquare(coord));
    }

    for (const Coordinate& coord : coords) {
      Square& adjacentSquare = gameMap.getSquare(coord);
      for (ObjectPtr& object : adjacentSquare) {
        Submarine* sub = dynamic_cast<Submarine*>(object.get());
        if (sub) {
          switch (blastDistance(square, adjacentSquare)) {
          case 1:
            sub->takeHits(2);
            break;
          case 2:
            sub->takeHits(1);
            break;
          }
        }
      }
    }
  }
  nuclearDetonations.clear();
}

//-----------------------------------------------------------------------------
void
Game::executeRepairs() {
  for (auto it = players.begin(); it != players.end(); ++it) {
    PlayerPtr& player = it->second;
    for (unsigned subID = 0; subID < player->getSubmarineCount(); ++subID) {
      Submarine& sub = player->getSubmarine(subID);
      if (!sub.isDead() && sub.isSurfaced()) {
        sub.repair();
      }
    }
  }
}

//-----------------------------------------------------------------------------
bool
Game::detonateMines(Square& square) {
  PlayerPtr player;
  Coordinate coord;

  for (auto it = square.begin(); it != square.end(); ) {
    Mine* mine = dynamic_cast<Mine*>(it->get());
    if (mine) {
      // only detonate first mine (it destroys all other mines on this square)
      if (!player) {
        player = getPlayer(static_cast<int>(mine->getPlayerID()));
        coord.set(square);
      }
      it = square.erase(it);
    } else {
      it++;
    }
  }

  if (coord) {
    detonationFrom(player, square, MINE, gameMap.getSquare(coord));
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
void
Game::detonationFrom(PlayerPtr& player, const Coordinate& sourceSquare,
                     const unsigned type, Square& damagedSquare)
{
  detonations.push_back(std::make_pair(Coordinate(damagedSquare), 1U));

  // destroy mines on this square
  for (auto it = damagedSquare.begin(); it != damagedSquare.end(); ) {
    Mine* mine = dynamic_cast<Mine*>(it->get());
    if (mine) {
      it = damagedSquare.erase(it);
    } else {
      it++;
    }
  }

  // direct hit on submarines in this square
  inflictDamageFrom(player, sourceSquare, type, damagedSquare, 2);

  // indirect hit on submarines in adjacent square
  // and detonate mines in surrounding squares
  auto coords = getBlastCoordinates(damagedSquare, 1);
  for (const Coordinate& coord : coords) {
    Square& adjacentSquare = gameMap.getSquare(coord);
    inflictDamageFrom(player, sourceSquare, type, adjacentSquare, 1);
    detonateMines(adjacentSquare);
  }
}

//-----------------------------------------------------------------------------
void
Game::inflictDamageFrom(PlayerPtr& player, const Coordinate& sourceSquare,
                        const unsigned type, Square& damagedSquare,
                        const unsigned damage)
{
  for (ObjectPtr& object : damagedSquare) {
    Submarine* sub = dynamic_cast<Submarine*>(object.get());
    if (sub) {
      sub->takeHits(damage);
      if (sub->isDead()) {
        for (auto it = nuclearDetonations.begin();
             it != nuclearDetonations.end(); )
        {
          if (it->get() == sub) {
            it = nuclearDetonations.erase(it);
          } else {
            it++;
          }
        }
      }
      if (player && (sub->getPlayerID() != player->getPlayerID())) {
        player->incScore(damage);
        switch (type) {
        case TORPEDO:
          torpedoHits[player->getPlayerID()][sourceSquare] += damage;
          break;
        case MINE:
          mineHits[player->getPlayerID()][sourceSquare] += damage;
          break;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
unsigned
Game::blastDistance(const Coordinate& from, const Coordinate& to) const {
  const unsigned xDiff = (from.getX() > to.getX())
      ? (from.getX() - to.getX())
      : (to.getX() - from.getX());

  const unsigned yDiff = (from.getY() > to.getY())
      ? (from.getY() - to.getY())
      : (to.getY() - from.getY());

  return std::max<unsigned>(xDiff, yDiff);
}

//-----------------------------------------------------------------------------
std::vector<Coordinate>
Game::getBlastCoordinates(const Coordinate& src, const unsigned range) const {
  const unsigned minX = (src.getX() > range) ? (src.getX() - range) : 0;
  const unsigned maxX = ((src.getX() + range) < gameMap.getWidth())
      ? (src.getX() + range) : gameMap.getWidth();

  const unsigned minY = (src.getY() > range) ? (src.getY() - range) : 0;
  const unsigned maxY = ((src.getY() + range) < gameMap.getHeight())
      ? (src.getY() + range) : gameMap.getHeight();

  std::vector<Coordinate> coords;
  for (unsigned x = minX; x <= maxX; ++x) {
    for (unsigned y = minY; y <= maxY; ++y) {
      const Coordinate coord(x, y);
      if (coord != src) {
        coords.push_back(coord);
      }
    }
  }

  std::random_shuffle(coords.begin(), coords.end());
  return std::move(coords);
}

} // namespace subsim
