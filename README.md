Submarine Battle Simulator
==========================

This is a multi-player submarine battle simulator that supports 2 or more players per game.

Each player has one or more submarines that begin at a position of their choice on the game map.  The game map consists of multiple squares arranged in a grid of rows and columns.  All squares of the game map begin empty or containing a permanent obstacle.  During the game empty squares may become occupied by one or more mobile objects - such as submarines.  But squares containing permanent obstacles cannot become occupied by mobile objects.

Game Objective
--------------

The goal of this game is to score hits on enemy submarines.  Every submarine begins with some number of shields.  One hit point destroys one shield.  Any hit points a submarine takes after all of its shields have been destroyed will destroy the submarine.  For example, if a submarine has 3 shields, it takes 3 hit points to destroy the shields and one more hit point to destroy the submarine.  In short, it requires 4 hits to destroy a submarine with 3 shields.

A destroyed submarine cannot score hits on enemy submarines.  So by necessity, keeping your submarines alive is also a game objective.

A game ends when only one player has any remaining submarines or the maximum turn count has been met.  The player that scored the most hits on enemy submarines wins.  The player with the second most hits is the runner up, etc.

Game Configuration
------------------

Every game is configured with a title, a map of a fixed size, and a list of game settings.  The title, map size, and the number of customized game settings are sent to each player as a single message when they connect to the game server.  If the number of customized game settings is greater that zero then that number of game setting messages will be sent immediately after the first message.

When a game is created the game server first waits for players to join.  Each player that joins is provided the game configuration (as described above) and in return they each provide their player name and submarine starting positions.  **NOTE:** Games may be configured so that submarine starting positions are pre-set, in which case the players do not provide their own starting positions.

See the [Communication Protocol Reference](protocol.md) for complete details about the message structure and message types.

See the [Game Settings Reference](settings.md) for a list of available game settings and their descriptions.

Game Map
--------

The game map is a rectangular grid of squares arranged as `width` columns and `height` rows.  Column and row numbers begin at 1.  In the small map below, square 1|1 is the north-west (top-left) corner and square 5|5 is the south-east (bottom-right) corder.  And starting from square 3|3, one square north is 3|2, one square south is 3|4, one square west is 232, and one square east is 4|3.

      1 2 3 4 5
    1 . . . . .
    2 . . N . .
    3 . W * E .
    4 . . S . .
    5 . . . . .

Game Play
---------

Once enough players have joined to satisfy the game's minimum player count the game may begin.  The game server can be configured to start the game automatically when the desired player count has been reached.  Otherwise the server administrator must start the game manually.

When the game is started play proceeds as a series of "turns".  Each turn has 4 phases:

 * Phase 1: The server sends a `B` (begin turn) message to all players
 * Phase 2: Players submit their submarine commands (1 command per submarine) to the server
 * Phase 3: The server executes the commands
 * Phase 4: The server sends turn results to each player

Phase 3 begins when all players have submitted one command for each of their active submarines.  A submarine is active if it hasn't been destroyed and is not surfaced for repairs.  Any player that does not submit one command for each of their active submarines in the alloted time is removed from the game and given a score of 0.

After the last turn of the game is completed the server sends a type `F` (game finished) message and one type `P` (player result) message for each player to all the players and disconnects them.  Otherwise a new turn is begun.

Commands
--------

Players must submit exactly one command per active submarine (owned by them) per turn.  Available commands are:

### Commands that leave power available for charging:

    Command  |  Description
    ===============================================================================================
    Move     |  The submarine is moved one square north, east, south, or west.
             |  Allows 1 equipment charge.
    -----------------------------------------------------------------------------------------------
    Sleep    |  The submarine does not move from its current square.
             |  Allows 2 equipment charges.

### Commands that do not leave power available for charging:

    Command  |  Description
    ===============================================================================================
    Ping     |  Activates sonar to discover the exact location and approximate size of all objects
             |  within sonar range.
             |  The range of the sonar ping is determined by the amount of charge it has when it
             |  is activated.
             |
             |  Pings are loud!  Your enemies will know when you activate them, but they do not
             |  know the source location of pings.
    -----------------------------------------------------------------------------------------------
    Fire     |  Fires a single torpedo to specified coordinates.  The torpedo will detonate when
             |  it reaches the specified coordinates.
             |  The range of the torpedo is determined by the amount of charge it has when it is
             |  fired.  If the specified coordinates are out of the torpedo range the torpedo does
             |  not detonate.
    -----------------------------------------------------------------------------------------------
    Mine     |  Deploy a single stationary mine in the square immediately north, east, south, or
             |  west of the submarine.
             |  The mine must be fully charged for a successful deploy.
             |  If the mine is not fully charged it will sink to the bottom of the ocean (in
             |  essence, it disappears and will never detonate).
             |
             |  NOTE: If you deploy a mine to a square that is occupied by one or more objects it
             |        will detonate (in the destination square).
    -----------------------------------------------------------------------------------------------
    Sprint   |  The submarine is moved 2 or more squares north, east, south, or west in a straight
             |  line.
             |  The maximum number of squares the submarine can move is determined by the amount
             |  of charge the sprint engine has when it is activated.
             |  The sprint is stopped short if you run into a mine.
             |
             |  The sprint engine makes a lot of noise!  Your enemies will know when you use it,
             |  but they do not know the starting point, direction, or the distance of the sprint.
             |
             |  Sprinting puts excess strain on your nuclear reactor.  If the number of squares
             |  you attempt to sprint plus the number of damage points your reactor has taken is
             |  greater or equal to the maximum damage your reactor can take, the reactor will
             |  detonate (See the "Nuclear Reactor Detonation" section below).
    -----------------------------------------------------------------------------------------------
    Surface  |  Remain at the same square for 3 successive turns to repair one shield.
             |  The shield is repaired at the end of the 3rd turn.
             |  You cannot submit any commands for the submarine while it is surfaced.
             |  No equipment on the submarine can be charged while it is surfaced.
             |
             |  You will remain surfaced for all 3 turns no matter how many shields your submarine
             |  had at the beginning of the maneuver, even if the submarine already had its
             |  maximum number of shields when you issued the command to surface.
             |
             |  This maneuver can only restore up to the maximum shield count for the submarine.
             |  The submarine is not visible to sonar while it is surfaced.
             |  You are a sitting duck for the duration of this maneuver, so use it wisely!

Charging Equipment
------------------

Each submarine produces a finite amount of power per turn.  This power can be used to `charge` or `activate` equipment.  Sonar, torpedos, mines, sprints, and surfacing all require power to activate.  And they must be charged before they are activated (with the exception of surfacing), otherwise they do nothing when activated.

The maximum units of charge and the effectiveness of the charge per equipment item is as follows:

    Equipment  |  Maximum Charge  |  Effectiveness of Charge Units
    ===============================================================================================
    Sonar      |  100 units       |  range = charge units + 1
    -----------------------------------------------------------------------------------------------
    Torpedo    |  100 units       |  range = charge units - 1
    -----------------------------------------------------------------------------------------------
    Mine       |  3 units         |  active if charge units = maximum charge
    -----------------------------------------------------------------------------------------------
    Sprint     |  9 units         |  range = (charge units / 3) + 1

The `Move` and `Sleep` commands leave extra power available for charging equipment.  When you issue these commands you must designate which equipment to charge with that extra power.  For example, you could issue the command to move north and charge sonar.

### Range vs Blast Radius

Sonar range and torpedo range are measured as a `path` from the source square.  This path must be traversed along north, east, south, or west directions and cannot go diagonally.  So it requires a range of 2 to reach a square that is north-east of the source square because the shortest path to get there must be north-then-east or east-then-north.

The blast radius of a detonation allows traversal along diagonals.

      Range of 2     |  Blast Radius of 2
      ===================================
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

If you do not charge or activate equipment in a turn, or you try to charge equipment that has already been charged to its maximum amount, the excess power causes damage to the nuclear reactor of the submarine.  If the amount of damage becomes greater or equal to the maximum damage the reactor core can take it causes a nuclear detonation.  This instantly destroys your submarine and any other objects occupying the same square at the time of detonation.

NOTE: Damage inflicted on enemy submarines via nuclear detonation is *not* added to your score.  There may be strategic reasons to use nuclear detonation, but as a game mechanic it is primarily intended to discourage inactivity.  If you do not make use of your submarine, or use it very ineffectively (such as overcharging a single equipment item), it will eventually self destruct.

Nuclear detonations have a blast radius of 2.  Any objects within a blast radius of 1 from a nuclear detonation suffer the same damage as a direct hit from a torpedo or mine.  Any objects within a blast radius of 2 from a nuclear detonation suffer the same damage as an indirect hit from a torpedo or mine.  

The default amount damage a nuclear reactor can take is 9 points.  This can be customized in the game configuration.

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

Turn Results
------------

After commands have been submitted for all active submarines they are executed by the server and the results are sent to each player.

### Global Result Data

These results are sent to all players.

    Data                 |  Description
    ===============================================================================================
    Sonar Activations    |  The number of sonar activations that occurred in this turn.
                         |  This is only a count.  It does not include the location of the
                         |  submarines that activated their sonar.
    -----------------------------------------------------------------------------------------------
    Sprint Activations   |  The number of sprint engines activated in this turn.
                         |  This is only a count.  It does not include the location of the
                         |  submarines that activated their sprint engine, the direction of the
                         |  sprints, or the distance of the sprints.
    -----------------------------------------------------------------------------------------------
    Detonation           |  There will be one of these messages for each detonation that occurred
                         |  this turn.  It provides the exact location and size (blast radius) of
                         |  each detonation.
                         |  This does not include information about damage caused by the
                         |  detonation or the source of the detonation.

### Private Result Data

These results are sent only to the player the information is intended for.

    Data                 |  Description
    ===============================================================================================
    Discovered Object    |  There will be one of these messages for each object discovered by
                         |  sonar pings you activated this turn.  It provides the exact location
                         |  and approximate size of each object within range of your sonar pings.
    -----------------------------------------------------------------------------------------------
    Torpedo Hit          |  There will be one of these messages for each torpedo detonation you
                         |  fired this turn that inflicts one or more points of damage on enemies.
                         |  It provides the location of the torpedo detonation and the total amount
                         |  of damage inflicted.
                         |  It does not include damage inflicted from chain reaction detonations.
    -----------------------------------------------------------------------------------------------
    Mine Hit             |  There will be one of these messages for each mine you have deployed
                         |  that was detonated this turn and inflicted one or more points of damage
                         |  on enemies.
                         |  It provides the location of the mine that detonated and the total
                         |  amount of damage inflicted.
                         |  It does not include damage inflicted from chain reaction detonations.
    -----------------------------------------------------------------------------------------------
    Submarine Info       |  There will be one of these messages for each of your submarines.
                         |  It includes the location, shields remaining, nuclear reactor damage,
                         |  whether the submarine is surfaced for repairs, and whether the
                         |  submarine has been destroyed.
    -----------------------------------------------------------------------------------------------
    Player Score         |  Includes your current score (the total number of damage points you have
                         |  inflicted on enemy submarines throughout the game so far).
    -----------------------------------------------------------------------------------------------
    Error Message        |  You will receive one error message for each illegal or invalid
                         |  command you submitted this turn.
                         |
                         |  NOTE: If you generate two or more error messages throughout the game
                         |        you will be removed from the game and given a score of 0.

Ineffective, Illegal, and Invalid Commands
------------------------------------------

An ineffective command is a legal command that doesn't do anything effective.  All the consequences of issuing an ineffective command take effect.  For example, if you submit a command to fire a torpedo that is not charged or insufficiently charged to reach its destination the torpedo is still fired but it will not detonate, so you just wasted a torpedo (in some game configurations you may have a limited number of torpedos).  The same thing applies to mines; if they are not fully charged you can still deploy them, but they are inert and you've wasted a mine.  You do not receive an error message for ineffective commands, but you will not be able to score points by submitting ineffective commands.

An illegal command is one that is recognized and correctly formed but cannot be executed.  Such as attempting to move beyond the edge of the game map, attempting to charge an unknown or un-chargeable equipment item, or moving in an unknown direction.  You will receive one error message for every illegal command you submit.

Invalid commands are commands that are unrecognized or malformed.  You will receive one error message for every invalid command you submit.

NOTE: If you generate two or more error messages throughout the course of the game you will be removed from the game and given a score of 0.

Communication Protocol
----------------------

See the [Communication Protocol Guide](protocol.md) for details about how players (clients) and the game server communicate.

Bots!
-----

See the [Bot Guide](bots.md) for information about writing bots.

