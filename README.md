Submarine Battle Simulator
==========================

This is a multi-player submarine battle simulator that supports 2 or more players per game.

It is intended mainly for `bot` (e.g. robot) competition.  The game mechanics are not suitable for (fun) human play.  The game mechanics are meant to be simple enough that writing a bot to play the game is easy, but writing a bot that can play the game consistently better than other bots is difficult.  See the [BotExamples](BotExamples) directory for some example bots.

Each player has one or more submarines that begin at a position of their choice on the game map.  The game map consists of multiple squares arranged in a grid of rows and columns.  All squares of the game map begin empty or containing a permanent obstacle.  During the game empty squares may become occupied by one or more mobile objects - such as submarines.  But squares containing obstacles cannot become occupied by mobile objects.

How to Build
------------

Run these commands starting at the root SubSim directory (the directory that contains this README.md file):

    mkdir build
    cd build
    cmake ..
    make

This should produce a subsim-server binary file.  Run `./subsum-server -h` for usage help.

Game Objective
--------------

The objective of this game is to score the most points.  A destroyed submarine cannot score hits on enemy submarines.  So by necessity, keeping your submarines alive is also a game objective.

Points are awarded when you inflict damage on enemy submarines with torpedos and mines.  Direct hits are worth 2 points, indirect hits are worth 1 point.

A game ends when only one player has any remaining submarines or the maximum turn count has been met.  Being the only player with any remaining submarines at the end of a game is worth an extra point - more incentive to keep your submarines alive (e.g. to avoid kamikaze attacks).

Every submarine begins with some number of shields.  One hit point destroys one shield.  Any hit points a submarine takes after all of its shields have been destroyed will destroy the submarine.  For example, if a submarine has 3 shields, it requires 3 hit points to destroy those shields and one more hit point to destroy the submarine.  So it requires 4 hits to destroy a submarine with 3 shields.

Game Configuration
------------------

Every game is configured with a title, a map of a fixed size, and a list of customized game settings.  A message with the server version, game title, map size, and the number of customized game settings is sent to each player when they connect to the game server.  If the number of customized game settings is greater than zero an additional message will be sent for each customized setting immediately after the first message.

When a game is created the game server first waits for players to join.  When a player connects they are provided the game configuration as described above.  To join the game they must provide their player name and submarine starting positions.  **NOTE:** Games may be configured so that submarine starting positions are pre-set, in which case the starting coordinates you provide are ignored.

See the [Communication Protocol Reference](protocol.md) for complete details about the message structure and message types.

See the [Game Settings Reference](settings.md) for a list of available game settings and their descriptions.

### Default Game Settings

The default game settings are as follows:

 * MinPlayers    = 2
 * MaxPlayers    = UNLIMITED
 * MaxTurns      = UNLIMITED
 * TurnTimeout   = NO LIMIT
 * MapSize       = 20 x 20
 * SubsPerPlayer = 1
 * Obstancles    = NONE

The following values can be customized per submarine.  The default values are:

 * SubStartLocation    = NONE
 * SubSize             = 100
 * SubSurfaceTurnCount = 3
 * SubMaxShields       = 3
 * SubMaxReactorDamage = 9
 * SubMaxSonarCharge   = 100
 * SubMaxTorpedoCharge = 100
 * SubMaxMineCharge    = 3
 * SubMaxSprintCharge  = 9
 * SubTorpedoCount     = UNLIMITED
 * SubMineCount        = UNLIMITED

The default value of a setting is assumed when no custom setting message is provided by the server to override it.

Game Map
--------

The game map is a rectangular grid of squares arranged as `width` columns and `height` rows.  Column and row numbers begin at 1.  In the small map below, square 1|1 is the north-west (top-left) corner and square 5|5 is the south-east (bottom-right) corder.  And starting from square 3|3, one square north is 3|2, one square south is 3|4, one square west is 2|3, and one square east is 4|3.

      1 2 3 4 5
    1 . . . . .
    2 . . N . .
    3 . W * E .
    4 . . S . .
    5 . . . . .

Game Play
---------

Once enough players have joined to satisfy the game's minimum player count the game may begin.  The game server can be configured to start the game automatically when the maximum player count has been reached.  Otherwise the server administrator must start the game manually.

When the game is started play proceeds as a series of "turns".  Each turn has 4 phases:

 * Phase 1: The server sends info messages to each player
 * Phase 2: The server sends a `B` (begin turn) message to all players
 * Phase 3: Players submit their submarine commands (1 command per submarine) to the server
 * Phase 4: The server executes the commands

Phase 1 provides information such as submarine state, objects that were discovered by sonar activations from the previous turn, detonations that occurred in the previous turn, etc.

**NOTE: The starting location you provided for your submarines in your "Join Game" message may be overridden if the game is configured with fixed starting locations.  The true location of your submarine(s) is provided in phase 1. So regardless of what positions you specified in your "Join Game" message the submarine info message(s) you receive in phase 1 is the source of truth about your submarine location(s).**

Phase 4 begins when all players have submitted one command for each of their active submarines.  A submarine is active if it hasn't been destroyed and is not surfaced for repairs.  Any player that does not submit one command for each of their active submarines in the alloted time is removed from the game and given a score of 0.

After the last turn of the game is completed the server sends a type `F` (game finished) message and one type `P` (player result) message for each player to all the players and disconnects them.  Otherwise a new turn is begun by going back to phase 1.

Commands
--------

Players must submit exactly one command per active submarine (owned by them) per turn.  Available commands are:

### Commands that leave power available for charging:

    Command  |  Description
    =========|=====================================================================================
    Move     |  The submarine is moved one square north, east, south, or west.
             |  Allows 1 equipment charge.
    ---------|-------------------------------------------------------------------------------------
    Sleep    |  The submarine does not move from its current square.
             |  Allows 2 equipment charges.

### Commands that do not leave power available for charging:

    Command  |  Description
    =========|=====================================================================================
    Ping     |  Activates sonar to discover the exact location and approximate size of all objects
             |  within sonar range.
             |
             |  The range of the sonar ping is determined by the amount of charge it has when it
             |  is activated.
             |
             |  Any submarines within range of your ping will hear it!  They will know how far
             |  away the ping was, but they won't know the exact location of the ping source.
    ---------|-------------------------------------------------------------------------------------
    Fire     |  Fires a single torpedo to specified coordinates.  The torpedo will detonate when
             |  it reaches the specified coordinates.
             |
             |  The range of the torpedo is determined by the amount of charge it has when it is
             |  fired.  If the specified coordinates are out of the torpedo range the torpedo does
             |  not detonate.
    ---------|-------------------------------------------------------------------------------------
    Mine     |  Deploy a single stationary mine in the square immediately north, east, south, or
             |  west of the submarine.
             |
             |  The mine must be fully charged for a successful deploy.  If the mine is not fully
             |  charged it will sink to the bottom of the ocean (in essence, it disappears and
             |  will never detonate).
             |
             |  NOTE: If you deploy a mine to a square that is occupied by one or more objects it
             |        will detonate immediately (in the destination square).
    ---------|-------------------------------------------------------------------------------------
    Sprint   |  The submarine is moved 2 or more squares north, east, south, or west in a straight
             |  line.
             |
             |  The maximum number of squares the submarine can move is determined by the amount
             |  of charge the sprint engine has when it is activated.
             |
             |  The sprint is stopped short if you run into a mine.
             |
             |  All submarines within a range of 4 of your starting or ending position will hear
             |  your sprint engine!  But they won't know the starting position, direction, or end
             |  position of the sprint.
             |
             |  Sprinting puts temporary strain on your nuclear reactor.  If the number of squares
             |  you attempt to sprint plus the number of damage points your reactor has taken is
             |  greater or equal to the maximum damage your reactor can take, the reactor will
             |  detonate (See the "Nuclear Reactor Detonation" section below).
    ---------|-------------------------------------------------------------------------------------
    Surface  |  Surface the submarine at its current square and remain there for 3 turns to repair
             |  one shield.  The shield is repaired at the end of the 3rd turn.
             |
             |  This will not increase the submarine shield count beyond its maximum.  So it does
             |  nothing if the submarine shield count is already at the maximum.
             |
             |  The submarine is not visible to sonar while it is surfaced.  But you cannot move,
             |  charge equipement, or submit any other commands for this submarine while it is
             |  surfaced.

Charging Equipment
------------------

Each submarine produces a finite amount of power per turn.  This power can be used to `charge` or `activate` equipment.  Sonar, torpedos, mines, sprints, and surfacing all require power to activate and they must be charged before they are activated, otherwise they do nothing when activated.

The maximum units of charge and the effectiveness of the charge per equipment item is as follows:

    Equipment  |  Maximum Charge  |  Effectiveness of Charge Units
    ===========|==================|================================================================
    Sonar      |  100 units       |  range = charge units + 1
    -----------|------------------|----------------------------------------------------------------
    Torpedo    |  100 units       |  range = charge units - 1
    -----------|------------------|----------------------------------------------------------------
    Mine       |  3 units         |  active if charge units = maximum charge
    -----------|------------------|----------------------------------------------------------------
    Sprint     |  9 units         |  range = (charge units / 3) + 1

The `Move` and `Sleep` commands leave extra power available for charging equipment.  When you issue these commands you must designate which equipment to charge with that extra power.  For example, you could issue the command to move north and charge sonar.

### Range vs Blast Radius

Sonar range and torpedo range are measured as a `path` from the source square.  This path must be traversed along north, east, south, or west directions and cannot go diagonally.  So it requires a range of 2 to reach a square that is north-east of the source square because the shortest path to get there must be north-then-east or east-then-north.

The blast radius of a detonation allows traversal along diagonals.

      Range of 2     |  Blast Radius of 2
      ===============|===================
      . . . . . . .  |  . . . . . . .
      . . . 2 . . .  |  . 2 2 2 2 2 .
      . . 2 1 2 . .  |  . 2 1 1 1 2 .
      . 2 1 0 1 2 .  |  . 2 1 0 1 2 .
      . . 2 1 2 . .  |  . 2 1 1 1 2 .
      . . . 2 . . .  |  . 2 2 2 2 2 .
      . . . . . . .  |  . . . . . . .

Any objects occupying the detonation square of a torpedo or mine suffer a direct hit, which inflicts 2 points of damage.  Any additional mines occupying the detonation square are destroyed and have no effect on the amount of damage inflicted or the blast radius of the initial detonation.

Torpedo and mine detonations have a blast radius of 1.  Any objects occupying a square within the blast radius suffer an indirect hit, which inflicts 1 point of damage.  Mines occupying squares within the blast radius are detonated, which starts a new set of damage infliction, which can set off other mines, and so on.  Detonation chain reactions can be very devastating!

### Nuclear Reactor Detonation

If you do not charge or activate equipment in a turn, or you try to charge equipment that has already been charged to its maximum amount, the excess power causes damage to the nuclear reactor of the submarine.  If the amount of damage becomes greater or equal to the maximum damage the reactor can take it causes a nuclear detonation.  This instantly destroys your submarine and any other objects occupying the same square at the time of detonation.

NOTE: Damage inflicted on enemy submarines via nuclear detonation is *not* added to your score.  There may be strategic reasons to use nuclear detonation, but as a game mechanic it is primarily intended to discourage inactivity.  If you do not make use of your submarine, or use it very ineffectively (such as overcharging a single equipment item), it will eventually self destruct.

Nuclear detonations have a blast radius of 2.  Any objects within a blast radius of 1 from a nuclear detonation suffer the same damage as a direct hit from a torpedo or mine.  Any objects within a blast radius of 2 from a nuclear detonation suffer the same damage as an indirect hit from a torpedo or mine.

The default amount of damage a nuclear reactor can take is 9 points.  This can be customized in the game configuration.

Execution Sequence
------------------

Commands of the same *type* from all players are executed simultaneously in a turn (once all commands have been received for that turn).  For example, all `Move` commands from all players are executed at the same time.  And all `Ping` commands from all players are executed at the same time.  But `Move` commands and `Ping` commands are *not* executed at the same time.

Events are executed in this sequence:

 1. Sleep
 2. Move
 3. Sprint
 4. Mine
 5. Fire
 6. Nuclear detonations
 7. Repairs
 8. Ping

Information Messages
--------------------

At the beginning of each turn the server sends information messages to each player.  This always includes information about the state of each of your submarines.  It may also include the result of events that occurred in the previous turn, such as torpedo detonations and objects discovered by sonar.

### Global Result Data

These results are sent to all players.

    Data                 |  Description
    =====================|=========================================================================
    Detonation           |  There will be one of these messages for each detonation that occurred
                         |  in the previous turn.
                         |
                         |  The exact location and size (blast radius) of the detonation is
                         |  provided.
                         |
                         |  This does not include information about damage caused by the
                         |  detonation or the source of the detonation.

### Private Result Data

These results are sent only to the player the information is intended for.

    Data                 |  Description
    =====================|=========================================================================
    Sonar Detection      |  Each of your submarines that is detected by enemy sonar will receive
                         |  one of these messages.
                         |
                         |  The range (distance) to the enemy submarine that detected your
                         |  submarine is provided.
                         |
                         |  NOTE: You will get more that one of these messages for a given
                         |        submarine if that submarine was detected by more than one
                         |        enemy submarine.
    ---------------------|-------------------------------------------------------------------------
    Sprint Activations   |  Each of your submarines will receive a message of this type if it was
                         |  close enough to hear any enemy sprint engine activations.
                         |
                         |  The number of sprint engines this submarine could hear is provided.
                         |
                         |  The start location, distance, direction, and end location of the
                         |  sprints is not provided.
    ---------------------|-------------------------------------------------------------------------
    Discovered Object    |  There will be one of these messages for each square within range of
                         |  your sonar pings that contains one or more objects.
                         |
                         |  The exact location of the square containing the object(s) and total
                         |  size of the object(s) within the square is provided.
    ---------------------|-------------------------------------------------------------------------
    Torpedo Hit          |  There will be one of these messages for each torpedo detonation you
                         |  fired this turn that inflicts one or more points of damage on enemies.
                         |
                         |  It provides the location of the torpedo detonation and the total amount
                         |  of damage inflicted.
                         |
                         |  It does not include damage inflicted from chain reaction detonations.
    ---------------------|-------------------------------------------------------------------------
    Mine Hit             |  There will be one of these messages for each mine you have deployed
                         |  that was detonated this turn and inflicted one or more points of
                         |  damage on enemies.
                         |
                         |  It provides the location of the mine that detonated and the total
                         |  amount of damage inflicted.
                         |
                         |  It does not include damage inflicted from chain reaction detonations.
    ---------------------|-------------------------------------------------------------------------
    Submarine Info       |  There will be one of these messages for each of your submarines.
                         |
                         |  It includes the location, shields remaining, nuclear reactor damage,
                         |  whether the submarine is surfaced for repairs, whether the
                         |  submarine has been destroyed, and other submarine specific data.
                         |
                         |  See the protocol.md reference for more specific information.
    ---------------------|-------------------------------------------------------------------------
    Player Score         |  Includes your current score (the total number of damage points you
                         |  have inflicted on enemy submarines throughout the game so far).

Ineffective, Illegal, and Invalid Commands
------------------------------------------

An ineffective command is a legal command that doesn't do anything.  All the consequences of issuing an ineffective command take effect.  For example, if you submit a command to fire a torpedo that is not charged or insufficiently charged to reach its destination the torpedo is still fired but it will not detonate, so you just wasted a torpedo (in some game configurations you may have a limited number of torpedos).  The same thing applies to mines; if they are not fully charged you can still deploy them, but they are inert and you've wasted a mine.  You will not be disqualified for ineffective commands, but you will not be able to score points by submitting ineffective commands.

An illegal command is one that is recognized and correctly formed but cannot be executed.  Such as attempting to move to an illegal square (a square beyond the edge of the game map or a square that contains a permanent obstacle), attempting to fire a torpedo or deploy a mine to an illegal square, attempting to charge an unknown or un-chargeable equipment item, moving in an unknown direction, etc.  You will be removed from the game and given a score of 0 if you issue any illegal commands.

Invalid commands are commands that are unrecognized or malformed.  You will be removed from the game and given a score of 0 if you issue any illegal commands.

To summarise: You will be disqualified for issuing any illegal or invalid commands.  You will *not* be disqualified for issuing ineffective commands.

Communication Protocol
----------------------

See the [Communication Protocol Reference](protocol.md) for details about how players (clients) and the game server communicate.

Bots!
-----

See the [Bot Guide](bots.md) for information about writing bots.

