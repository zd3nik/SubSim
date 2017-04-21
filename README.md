Submarine Battle Simulator
==========================

This is a multi-player submarine battle simulator that supports 2 or more players per game.

Each player has one or more submarines that begin at a position of their choice on the game map.  The game map consists of multiple squares arranged in a grid of rows and columns.  All squares of the game map begin empty or containing a permanent obstacle.  During the game empty squares may become occupied by one or more mobile objects - such as submarines.  But squares containing permanent obstacles cannot become occupied by mobile objects.

Game Setup
----------

Every game configuration is defined by a game descriptor.  A game descriptor consists of the game map size (width and height), the location of any permanent obstacles within the game map, and game options such as minimum player count, maximum player count, maximum turn count, number of submarines per player, hit points per submarine, maximum equipment charge capacities, timeout values, etc..

The game descriptor is as a single line ASCII message containing multiple pipe-delimited fields.  The first field is the message type (always `G` which is the message type ID for a game info message). The second field is the width of the game map.  The third field is the height.  And so on.

See the [Communication Protocol Reference](protocol.md) for complete details about the game descriptor message and other message types.

See the [Game Options Reference](options.md) for a list of available game options and their descriptions.

Game Play
---------

Once a game is started the game server waits for players to join.  Each player that joins is provided the game descriptor and in return they each provide their player name and chosen starting position.  **NOTE:** Games may also be configured so that starting positions are preset, in which case the players do not provide their own starting positions.

Once enough players have joined to satisfy the game's minimum player count the game may begin.  The game server can be configured to start games automatically when the desired player count has been reached.

When the game is started play proceeds as a series of "turns".  Each turn has 4 phases:

 * Phase 1: The server sends a `B` (begin turn) message to all players
 * Phase 2: Players submit their submarine orders (1 order per submarine) to the server
 * Phase 3: The server executes the orders
 * Phase 4: The server sends turn results to each player

Phase 3 begins when all players have submitted one order for each of their active submarines.  A submarine is active if it hasn't been destroyed and is not surfaced for repairs.  Any player that does not submit one order for each of their active submarines in the alloted time is removed from the game and given a score of 0.

Orders
------

Only one order may be given per submarine per turn.  Available orders are:

### Orders that leave power available for charging:

    Order    |  Description
    ===============================================================================================
    Move     |  The submarine is moved one square north, east, south, or west.
             |  Allows 1 equipment charge.
    -----------------------------------------------------------------------------------------------
    Sleep    |  The submarine does not move from its current square.
             |  Allows 2 equipment charges.

### Orders that do not leave power available for charging:

    Order    |  Description
    ===============================================================================================
    Ping     |  Activates sonar to discover the exact location and approximate size of all objects
             |  within sonar range.
             |  The range of the sonar ping is determined by the amount of charge the sonar has.
    -----------------------------------------------------------------------------------------------
    Fire     |  Fires a single torpedo to a specified coordinate.
             |  The range of the torpedo is determined by the amount of charge the torpedo has.
             |  If the target coordinate is out of the torpedo range it is unsuccessful.
    -----------------------------------------------------------------------------------------------
    Mine     |  Deploy a single stationary mine in a square north, east, south, or west of the
             |  submarine.  The mine must be fully charged for a successful deploy.
    -----------------------------------------------------------------------------------------------
    Sprint   |  The submarine is moved 2 or more squares north, east, south, or west in a straight
             |  line.  The maximum number of squares the submarine can move is determined by the
             |  amount of charge the sprint engine has.
             |  If you sprint when your nuclear reactor is nearly overloaded you trigger an
             |  immediate nuclear detonation - see Build-Up to Detination below.
    -----------------------------------------------------------------------------------------------
    Surface  |  Remain at the same square for 4 successive turns to repair up to 2 damage points.
             |  You cannot submit any orders during those turns and no equipment is charged during
             |  those turns.  At the end of the 2nd turn one damage point is repaired.  At the end
             |  of the 4th turn another damage point is repaired.  You will remain surfaced for
             |  all 4 turns no matter how much damage your submarine has taken, even if it has
             |  taken no damage.
             |  NOTE: The submarine is not visible to sonar while it is surfaced.
             |  NOTE: You are a sitting duck for the duration of this maneuver, so use it wisely!

Charging Equipment
------------------

Each submarine produces a finite amount of power per turn.  This power can be used to `charge` or `activate` equipment per turn.  Sonar, torpedos, mines, sprints, and surfacing all require power to activate.  And they must be charged before they are activated (with the exception of surfacing), otherwise they do nothing.

The maximum units of charge and the effectiveness of the charge per equipment item is as follows:

    Equipment  |  Maximum Charge  |  Effectiveness of Charge Units
    ===============================================================================================
    Sonar      |  no limit        |  range = charge units + 1
    -----------------------------------------------------------------------------------------------
    Torpedo    |  no limit        |  range = charge units - 1
    -----------------------------------------------------------------------------------------------
    Mine       |  3 units         |  active if charge units = maximum charge
    -----------------------------------------------------------------------------------------------
    Sprint     |  6 units         |  range = (charge units / 3) + 1

The `Move` and `Sleep` orders leave extra power available for charging equipment.  When you issue these orders you may designate which equipment to charge with that extra power.  For example, you could issue the order to move north and charge sonar.

### Range vs Blast Radius

Sonar range and torpedo range are measured as a `path` from the source square `S`.  This path must be traversed along north, east, south, or west directions and cannot go diagonally.  So it requires a range of 2 to reach a square that is north-east of the source square because the shortest path to get there must be north-then-east or east-then-north.

The blast radius of an explosion allows traversal along diagonals.

      Range of 2     |  Blast Radius of 2
      ===================================
      . . . . . . .  |  . . . . . . .
      . . . 2 . . .  |  . 2 2 2 2 2 .
      . . 2 1 2 . .  |  . 2 1 1 1 2 .
      . 2 1 S 1 2 .  |  . 2 1 S 1 2 .
      . . 2 1 2 . .  |  . 2 1 1 1 2 .
      . . . 2 . . .  |  . 2 2 2 2 2 .
      . . . . . . .  |  . . . . . . .

When a torpedo is fired at a coordinate it detonates when it reaches that coordinate.  Any objects occupying the detonation square of a torpedo (or mine) suffer a direct hit, which inflicts 2 points of damage.  Any mines occupying the detonation square are destroyed and have no effect on the amount of damage inflicted or the blast radius of the detonation.

Torpedo and mine detonations have a blast radius of 1.  Any objects occupying a square within the blast radius suffer an indirect hit, which inflicts 1 point of damage.  Mines occupying squares within the blast radius are detonated, which starts a new set of damage infliction, which can set off other mines, and so on.  Detonation chain reactions can be very devastating!

### Build-Up to Detonation

If you do not charge or activate equipment in a turn, or you try to charge equipment that has already been charged to its maximum amount, the excess power accumulates in the nuclear reactor of the submarine.  If the amount of excess power units accumulated becomes more than the reactor core can contain it causes a nuclear detonation.  This instantly destroys your submarine and any other objects occupying the same square at the time of detonation.

Nuclear detonations have a blast radius of 2.  Any objects within a blast radius of 1 from a nuclear detonation suffer the same damage as a direct hit from a torpedo or mine.  Any objects within a blast radius of 2 from a nuclear detonation suffer the same damage as an indirect hit from a torpedo or mine.  

The default amount of excess power units required to trigger a nuclear detonation is 9.  This can be customized in the game descriptor.

NOTE: Excess power *cannot* be extracted from a reactor core.

Execution Sequence
------------------

Orders of the same *type* from all players are executed simultaneously in a turn (once all orders have been received for that turn).  For example, all `Move` orders from all players are executed at the same time.  And all `Ping` orders from all players are executed at the same time.  But `Move` orders and `Ping` orders are *not* executed at the same time.

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

After orders have been submitted for all active submarines they are executed by the server and the results are sent to each player.

Turn result data:

    Data                 |  Description
    ===============================================================================================
    Sonar Activations    |  The number of enemy sonar activations that occurred in this turn.
                         |  This is only a count.  It does not include the location of the
                         |  submarines that activated their sonar.
    -----------------------------------------------------------------------------------------------
    Detonations          |  The exact location and size (blast radius) of all detonations that
                         |  occurred this turn.
                         |  This does not include information about damage caused by those
                         |  detonations or the source of the detonations.
    -----------------------------------------------------------------------------------------------
    Discovered Objects   |  If you activated any sonar scans in this turn you will recieve a list
                         |  of the exact coordinates and approximate size of all objects within
                         |  range of those sonar scans.
    -----------------------------------------------------------------------------------------------
    Torpedo Hits         |  If you fired torpedos which scored one or more hits on enemy subs
                         |  you will receive a list of damage points inflicted on each enemy and
                         |  which submarines the torpedos were fired from.
                         |  This will not contain information about damage inflicted via chain
                         |  reactions.
                         |  You will not received feedback of this type for torpedos that did not
                         |  inflict any damage.
    -----------------------------------------------------------------------------------------------
    Mine Hits            |  If you have deployed any mines on this or previous turns you will
                         |  receive a list of damage points inflicted on enemy subs by those mines
                         |  if they were detonated this turn.
                         |  NOTE: Once a mine has been detinated it no longer exists.
    -----------------------------------------------------------------------------------------------
    Submarine Status     |  This is a list of info about each of your submarines.  For each of your
                         |  submarines this list includes the location, damage inflicted, damage
                         |  taken, amount of excess power accumluated, whether the submarine is
                         |  surfaced for repairs, and whether the submarine has been destroyed.
    -----------------------------------------------------------------------------------------------
    Error Message        |  If you submitted an unrecognized order or an order that could not be
                         |  carried out you will receive an error message.  If you submit two
                         |  consequtive invalid orders you will be removed from the game, and
                         |  given a score of 0.

Invalid Orders
--------------

All the consequences of issuing an order take effect even if the order does not make sense.  For example, if you submit an order to fire a torpedo but you have not sufficiently charged the torpedo, it is fired but it is inert and you start with a new, uncharged torpedo.  In short you just wasted a torpedo (in some game configurations you may have a limited number of torpedos).  Another example: If you fire a torpedo at a coordinate that is beyond the range of the torpedo (which is based on how much the torpedo has been charged) it will not reach its destination and you've wasted a torpedo.  The same thing applies to mines; if they are not fully charged you can still deploy them, but they are inert and you've wasted a mine.

If you submit an order that is unrecognized or malformed, you waste a turn and get an error message in the turn results phase.  If you submit an order that is correctly formed but cannot be fulfilled, such as attempting to move beyond the edge of the game map, you waste a turn and get an error message in the turn results stage.  Generate two consecutive error messages and you will be removed from the game and given a score of 0.

Communication Protocol
----------------------

See the [Communication Protocol Guide](protocol.md) for details about how players (clients) and the game server communicate.

Bots!
-----

See the [Bot Guide](bots.md) for information about writing bots.

