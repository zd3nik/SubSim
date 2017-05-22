Communication Protocol Reference
================================

The SubSim game server (`subsim-server`) and clients communicate with one-line ASCII text messages over a TCP socket.  Each line is terminated by a single new-line character: `\n`

All messages are a pipe delimited list of fields of this form:

    TYPE|VALUE|VALUE|...\n

The number of `VALUE` fields in a message is determined by the `TYPE`.

The `TYPE` field is always a single ASCII character.  This is so you can easily check the message type by examining the first two characters of the message, which will always be the type character followed by a pipe `|` character. **Only checking the first character to determine the message type could lead to errors.**  For example, if you receive `Protocol Error` the message type is _not_ `P`, it is an error message or an errant/invalid message and should be treated as an error message.

Any message that does not follow the prescribed format should be treated as an error message.

Leading and trailing whitespace characters should be stripped from each field.

An example message type is `M` (move submarine).  An `M` message has 4 value fields: `turn_number|sub_id|direction|equipment_name`

 * `turn_number` = which turn the command was submitted in
 * `sub_id` = the integer ID of the submarine to move (your first submarine is ID 0, second is ID 1, etc)
 * `direction` = the direction to move the submarine: N, E, S, or W
 * `equipment_name` = the name of the equipment to charge: Sonar, Torpedo, Mine, Sprint, None

So an `M` message that moves submarine 0 north and charges sonar in the 5th turn looks like this:

    M|5|0|N|Sonar\n

### Limitations

No form of character escaping is supported in the messaging protocol.  So message `fields` may not contain pipe `|` or new-line `\n` characters.

The maximum length (including the new-line) of a single message is 4095 bytes.  Messages that exceed this length are invalid (sorry hackers, it will not cause a buffer overflow).

-------------------------------------------------------------------------------

Protocol Reference
------------------

### Client to server messages:

    Description    |  Message format
    ===============|=================================
    Join game      |  J|name|X0|Y0|X1|Y1|...|Xn|Yn
    Move           |  M|tn|id|direction|equip_name
    Sleep          |  S|tn|id|equip_name|equip_name
    Ping           |  P|tn|id
    Fire Torpedo   |  F|tn|id|X|Y
    Deploy Mine    |  D|tn|id|direction
    Sprint (Run)   |  R|tn|id|direction|distance
    Surface (Up)   |  U|tn|id

More info:

    Client Message    |  Details
    ==================|===========================================================================
    J|name|X|Y|X|Y    |  Join the current game using the specified player name.
                      |
                      |  Send this immediately after receiving the game configuration and game
                      |  setting messages (See the "Server to client messages" section below).
                      |
                      |  Fields:
                      |
                      |    name  = The player name you would like to use.  If the name is invalid
                      |            or in use you will receive an error message and be disconnected.
                      |    X     = The column number to place a submarine at.
                      |            Column numbers start at 1.
                      |    Y     = The row number to place a submarine at.
                      |            Row numbers start at 1.
                      |
                      |  You must provide one X|Y field pair for each of your submarines.  The
                      |  first pair is for submarine ID 0, the next pair is for submarine ID 1, etc.
                      |
                      |  If the game configuration specifies 1 submarine per player then you must
                      |  send 1 X|Y pair.  If the game configuration specifies 2 submarines per
                      |  player you send 2 X|Y pairs.  Etc.
                      |
                      |  NOTE: The starting positions you provide in this message will be ignored
                      |        for any submarines that have a pre-set starting location per the
                      |        game configuration.
                      |
                      |  NOTE: You may place one or more submarines on the same starting square.
                      |        Conceptually, each square represents a large section of ocean so
                      |        multiple objects can fit in one square.
    ------------------|---------------------------------------------------------------------------
    M|tn|id|          |  Move the specified submarine one square in the specified direction and
      dir|            |  charge the specified equipment item.
      equip           |
                      |  Fields:
                      |
                      |    tn    = The turn number.
                      |    id    = The ID of the submarine to move.
                      |            Your first submarine is ID 0, the second is ID 1, etc.
                      |    dir   = The direction to move.
                      |            N = north, E = east, S = south, W = west
                      |    equip = The name of the equipment item to charge.
                      |
                      |  Valid equipment names are: Sonar, Torpedo, Mine, Sprint, or None
                      |
                      |  If `equip` is set to `None` the reactor core of the submarine being moved
                      |  takes 1 point of damage.
    ------------------|---------------------------------------------------------------------------
    S|tn|id|          |  Make the specified submarine sleep this turn and charge 2 equipment items.
      equip|          |
      equip           |  Fields:
                      |
                      |    tn    = The turn number.
                      |    id    = The ID of the submarine that will sleep.
                      |            Your first submarine is ID 0, the second is ID 1, etc.
                      |    equip = The name of the first equipment item to charge.
                      |    equip = The name of the second equipment item to charge.
                      |
                      |  Valid equipment names are: Sonar, Torpedo, Mine, Sprint, or None
                      |
                      |  You may specify the same equipment item name in both `equip` fields.
                      |  Doing so will result in the specified equipment item receiving 2 units
                      |  of charge.
                      |
                      |  The reactor core of the submarine takes 1 point of damage for each
                      |  `equip` field set to `None`.
    ------------------|---------------------------------------------------------------------------
    P|tn|id           |  Perform a sonar ping from the specified submarine.
                      |
                      |  Fields:
                      |
                      |    tn    = The turn number.
                      |    id    = The ID of the submarine that will perform the sonar ping.
                      |            Your first submarine is ID 0, the second is ID 1, etc.
    ------------------|---------------------------------------------------------------------------
    F|tn|id|X|Y       |  Fire a torpedo from the specified submarine to the specified coordinates.
                      |
                      |  Fields:
                      |
                      |    tn    = The turn number.
                      |    id    = The ID of the submarine that will fire the torpedo.
                      |            Your first submarine is ID 0, the second is ID 1, etc.
                      |    X     = The column number of the torpedo destination.
                      |            Column numbers start at 1.
                      |    Y     = The row number of the torpedo destination.
                      |            Row numbers start at 1.
                      |
                      |  NOTE: The torpedo destination may not be a square that contains a
                      |        permanent obstacle.
    ------------------|---------------------------------------------------------------------------
    D|tn|id|dir       |  Deploy a mine from the specified submarine.
                      |
                      |  The mine is deployed to the square adjacent to submarine in the specified
                      |  direction.
                      |
                      |  Fields:
                      |
                      |    tn    = The turn number.
                      |    id    = The ID of the submarine that will deploy the mine.
                      |            Your first submarine is ID 0, the second is ID 1, etc.
                      |    dir   = The direction of the adjacent square to deploy to.
                      |            N = north, E = east, S = south, W = west
                      |
                      |  NOTE: The mine destination may not be a square that contains a
                      |        permanent obstacle.
                      |
                      |  NOTE: If you deploy a charged mine to a square that is occupied by one or
                      |        more objects the mine will detonate (in the destination square).
    ------------------|---------------------------------------------------------------------------
    R|tn|id|dir|dist  |  Move the specified submarine 2 or more squares in the specified direction
                      |  using its sprint engine.
                      |
                      |  Fields:
                      |
                      |    tn    = The turn number.
                      |    id    = The ID of the submarine to move.
                      |            Your first submarine is ID 0, the second is ID 1, etc.
                      |    dir   = The direction to move.
                      |            N = north, E = east, S = south, W = west
                      |    dist  = The number of squares to move.
                      |            Must be greater or equal to 2.
    ------------------|---------------------------------------------------------------------------
    U|tn|id           |  Surface the specified submarine to perform a shield repair.
                      |
                      |  Fields:
                      |
                      |    tn    = The turn number.
                      |    id    = The ID of the submarine to surface.
                      |            Your first submarine is ID 0, the second is ID 1, etc.

### Server to client messages:

    Description          |  Message format
    =====================|=====================================================
    Game Configuration   |  C|version|title|width|height|setting_count
    Game Setting         |  V|name|value|...
    Begin Turn           |  B|tn
    Sonar Discovery      |  S|tn|range
    Sprint Activations   |  R|tn|count
    Detonation           |  D|tn|X|Y|radius
    Discovered Object    |  O|tn|X|Y|size
    Torpedo Hit          |  T|tn|X|Y|damage
    Mine Hit             |  M|tn|X|Y|damage
    Submarine Info       |  I|tn|id|X|Y|active|var=value|...
    Player Score (Hits)  |  H|tn|score
    Game Finished        |  F|player_count|turn_count|status
    Player Result        |  P|name|score

More info:

    Server Message    |  Details
    ==================|===========================================================================
    C|version|title|  |  Sent to each player that connects while the game server is accepting new
      W|H|count       |  players for a game.
                      |
                      |  Fields:
                      |
                      |    version = The game server version.
                      |    title   = The game title.
                      |    W       = The map width (number of columns).
                      |    H       = The map height (number of rows).
                      |    count   = The number of customized game settings.
                      |
                      |  If `count` is greater than 0 then `count` game setting messages will be
                      |  sent immediately after this message.
    ------------------|---------------------------------------------------------------------------
    V|name|value      |  One of these will be sent to each player immediately after the game
                      |  configuration message for each game setting that has been customized.
                      |
                      |  Fields:
                      |
                      |    name    = The setting name.
                      |    value   = The setting value.
                      |
                      |  NOTE: Some settings have multiple value fields.  For example the
                      |        MapSize setting has a width and a height value:
                      |
                      |            V|MapSize|60|40
                      |
                      |  See "settings.md" for a list of available game setting names and their
                      |  legal values.
    ------------------|---------------------------------------------------------------------------
    B|tn              |  Sent to all players at the beginning of each new turn.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |              Turn numbers start at 1.
    ------------------|---------------------------------------------------------------------------
    D|tn|X|Y|radius   |  All payers will receive one of these messages for each detonation that
                      |  occurred the specified turn.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the detonation.
                      |              Column numbers start at 1.
                      |    Y       = The row of the detonation.
                      |              Row numbers start at 1.
                      |    radius  = The blast radius of the detonation.
    ------------------|---------------------------------------------------------------------------
    S|tn|id|range     |  Players will receive one of these messages when one of their submarines
                      |  is discovered by enemy sonar.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    id      = The ID of the submarine that was detected.
                      |              Submarine IDs start at 0 (per player).
                      |    range   = The distance to the submarine that activated the sonar which
                      |              discovered your submarine.
    ------------------|---------------------------------------------------------------------------
    R|tn|id|count     |  Players will receive one of these messages when one of their submarines
                      |  is within hearing range of any activated enemy sprint engines.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    id      = The ID of the submarine that heard the sprint engines.
                      |              Submarine IDs start at 0 (per player).
                      |    count   = The number of sprint engine activations within hearing range
                      |              of your submarine.
    ------------------|---------------------------------------------------------------------------
    O|tn|X|Y|size     |  Players that activated sonar will receive one of these messages for each
                      |  square that contains one or more objects within range of their sonar
                      |  activations.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the square containing one or more objects.
                      |              Column numbers start at 1.
                      |    Y       = The row of the square containing one or more objects.
                      |              Row numbers start at 1.
                      |    size    = The total size of the objects within the square.
    ------------------|---------------------------------------------------------------------------
    T|tn|X|Y|damage   |  Players that fired torpedos will receive one of these messages for each
                      |  torpedo that inflicted damage on one or more enemy submarines.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the torpedo detonation.
                      |              Column numbers start at 1.
                      |    Y       = The row of the torpedo detonation.
                      |              Row numbers start at 1.
                      |    damage  = The total number of damage points inflicted by the torpedo.
                      |
                      |  NOTE: If the damage value is 2 that could mean the torpedo got a
                      |        direct hit on 1 enemy or it could mean the torpedo got an indirect
                      |        hit on 2 enemies.  3 could be a direct hit and an indirect hit or
                      |        3 indirect hits.  Etc..
                      |
                      |  NOTE: This message is not sent for torpedos that detonate without
                      |        inflicting damage.
    ------------------|---------------------------------------------------------------------------
    M|tn|X|Y|damage   |  Players that have deployed mines will receive one of these messages for
                      |  each mine that detonates and inflicts damage on one or more enemy
                      |  submarines.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the mine that detonated.
                      |              Column numbers start at 1.
                      |    Y       = The row of the mine that detonated.
                      |              Row numbers start at 1.
                      |    damage  = The total number of damage points inflicted by the mine.
                      |
                      |  NOTE: If the damage value is 2 that could mean the mine got a
                      |        direct hit on 1 enemy or it could mean the mine got an indirect
                      |        hit on 2 enemies.  3 could be a direct hit and an indirect hit or
                      |        3 indirect hits.  Etc..
                      |
                      |  NOTE: This message is not sent for mines that detonate without
                      |        inflicting damage.
    ------------------|---------------------------------------------------------------------------
    I|tn|id|X|Y|      |  Players will receive one of these messages for each of their submarines
      active|         |  between each turn.
      var=val|        |
      ...             |  Fields:
                      |
                      |    tn      = The turn number.
                      |    id      = The ID of the submarine.
                      |              Submarine IDs start at 0 (per player).
                      |    X       = The column of the submarine.
                      |              Column numbers start at 1;
                      |    Y       = The row of the submarine.
                      |              Row numbers start at 1;
                      |    active  = Is this submarine active?
                      |              1 = active, 0 = not active
                      |    var=val = An individual submarine variable and its value.
                      |              There will be one of these for each submarine variable.
                      |              Order of variables is not guaranteed.
                      |              Legal submarine variables are:
                      |
                      |              Variable Name   |  Value Description
                      |              ----------------|------------------------------------------
                      |              shields         |  Number of shields remaining.
                      |              ----------------|------------------------------------------
                      |              size            |  The size of this submarine.
                      |                              |  This variable will not be present if
                      |                              |  the submarine is the standard size.
                      |              ----------------|------------------------------------------
                      |              torpedos        |  Number of torpedos remaining.
                      |                              |  This variable will not be present if
                      |                              |  the submarine has unlimited torpedos.
                      |              ----------------|------------------------------------------
                      |              mines           |  Number of mines remaining.
                      |                              |  This variable will not be present if
                      |                              |  the submarine has unlimited mines.
                      |              ----------------|------------------------------------------
                      |              sonar_range     |  Current sonar range.
                      |              ----------------|------------------------------------------
                      |              sprint_range    |  Current sprint range.
                      |              ----------------|------------------------------------------
                      |              torpedo_range   |  Current torpedo range.
                      |              ----------------|------------------------------------------
                      |              mine_ready      |  Can this submarine deploy a mine?
                      |                              |    0 = not ready, 1 = ready
                      |              ----------------|------------------------------------------
                      |              max_sonar       |  Sonar charged to maximum?
                      |                              |    0 = no, 1 = yes
                      |              ----------------|------------------------------------------
                      |              max_sprint      |  Sprint engine charged to maximum?
                      |                              |    0 = no, 1 = yes
                      |              ----------------|------------------------------------------
                      |              max_torpedo     |  Torpedo charged to maximum?
                      |                              |    0 = no, 1 = yes
                      |              ----------------|------------------------------------------
                      |              surface_remain  |  The number of turns this submarine will
                      |                              |  remain surfaced.
                      |              ----------------|------------------------------------------
                      |              reactor_damage  |  Amount of reactor core damage sustained.
                      |              ----------------|------------------------------------------
                      |              dead            |  Has this submarine been destroyed?
                      |                              |    0 = not dead, 1 = dead
                      |
                      |              With the exception of size, torpedos, and mines
                      |              assume a value of 0 for any variable not present.
                      |
                      |  NOTE: Do not submit commands for inactive (surfaced or dead) submarines!
    ------------------|---------------------------------------------------------------------------
    H|tn|score        |  Sent to each player between turns to provide the player with their
                      |  current score.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    score   = The total points of damage you have inflicted on enemy
                      |              submarines so far in the current game.
    ------------------|---------------------------------------------------------------------------
    F|players|        |  This message is sent to all players when the game is finished.
      turns|          |
      status          |  Fields:
                      |
                      |    players = The number of players in the game.
                      |    turns   = The number of turns executed in the game.
                      |    status  = An English message describing how the game ended.
    ------------------|---------------------------------------------------------------------------
    P|name|score      |  For each player that finished the game one of these messages is sent to
                      |  all remaining players after the `F` (game finished) message.
                      |
                      |  Fields:
                      |
                      |    name    = The name of the player.
                      |    score   = The number of hits this player scored during the game.
                      |
                      |  Once all messages of this type have been sent to each player the game
                      |  is complete and all players are disconnected.

-------------------------------------------------------------------------------

Examples
--------

### Game Start

Before a game can start the minimum number of players required by the game configuration must join.  During this time the server accepts new TCP socket connections.

When a player (client) connects during this time the game server immediately sends the player a game configuration message:

    Message                       |  Details
    ==============================|================================
    C|1.0.x|Example Game|50|30|2  |  Server Version = 1.0.x
                                  |  Game Title     = Example Game
                                  |  Map Width      = 50
                                  |  Map Height     = 30
                                  |  Settings Count = 2

Since the settings count field is set to 2, the server will send 2 game setting messages immediately after the game configuration message.

    Message               |  Details
    ======================|=====================================
    V|TurnTimeout|60000   |  Setting Name  = TurnTimeout
                          |  Setting Value = 60000 milliseconds
    ----------------------|-------------------------------------
    V|SubsPerPlayer|3     |  Setting Name  = SubsPerPlayer
                          |  Setting Value = 3

To reiterate: the 3 messages shown above would be sent to each player that connected to the game server.  This is what each player would receive from the server immediately after connecting (note the inclusion of a new-line `\n` character at the end of each message):

    C|1.0.x|Example Game|50|30|2\n
    V|TurnTimeout|60000\n
    V|SubsPerPlayer|3\n

Then each player would attempt to join the game by sending their player name and submarine coordinates to the server.  Here is an example of the `J` (join game) message one of those players might send:

    Message                          |  Details
    =================================|===================================
    J|Captain Nimo|13|3|39|10|27|15  |  Player Name       = Captain Nimo
                                     |  Sub 0 coordinates = 13|3
                                     |  Sub 1 coordinates = 39|10
                                     |  Sub 2 coordinates = 27|15

NOTE: Coordinates for 3 submarines must be sent in the `J` (join game) message because `SubsPerPlayer` is set to 3.

### Turn Sequence

After enough players have joined to meet the minimum player count setting the game may begin.  At that point the server stops listening for new connections, sends pertinent messages to each player (such as submarine info messages), and then sends a `B` (begin turn) message to each player.

Here is an example of what one player might see:

    Message                  |  Details
    =========================|============================================================
    I|0|0|13|3|1|shields=3   |  Submarine 0 is at square 39|10, active, and has 3 shields.
    -------------------------|------------------------------------------------------------
    I|0|1|39|10|1|shields=3  |  Submarine 1 is at square 39|10, active, and has 3 shields.
    -------------------------|------------------------------------------------------------
    I|0|2|27|15|1|shields=3  |  Submarine 2 is at square 27|15, active, and has 3 shields.
    -------------------------|------------------------------------------------------------
    B|1                      |  Begin turn 1.

NOTE: Turn number 0 is used on any info messages sent before the first turn.

Summary of messages from server:

    I|0|0|13|3|1|shields=3\n
    I|0|1|39|10|1|shields=3\n
    I|0|2|27|15|1|shields=3\n
    B|1\n

During a turn each player must send exactly one command message for each of their `active` submarines.  Here is an example of what the messages might look like from player 1:

    Message        |  Details
    ===============|==========================================================
    M|1|0|E|Sonar  |  Move submarine 0 east and charge Sonar in turn 1.
    ---------------|----------------------------------------------------------
    P|1|1          |  Activate sonar ping from submarine 1 in turn 1.
    ---------------|----------------------------------------------------------
    F|1|2|31|15    |  Fire torpedo at square 31|15 from submarine 2 in turn 1.

Summary of messages from player 1:

    M|1|0|E|Sonar\n
    P|1|1\n
    F|1|2|31|15\n

That player would then consume any informational message while waiting for the next `B` (begin turn) message.  Here are some examples:

    Message      |  Details
    =============|================================================================================
    D|1|31|15|1  |  A detonation with a blast radius of 1 was detected at square 31|15 in turn 1.
                 |  Note that player 1 fired a torpedo at this square this turn.
    -------------|--------------------------------------------------------------------------------
    D|1|28|16|1  |  A detonation with a blast radius of 1 was detected at square 28|16 in turn 1.

The messages above are sent to all players.

The messages below are only sent to player 1 (you).

    Message                  |  Details
    =========================|====================================================================
    S|1|0|6                  |  In turn 1 submarine ID 0 was detected by an enemy submarine 6
                             |  squares away.
    -------------------------|--------------------------------------------------------------------
    O|1|33|19|20             |  Square 33|19 has one or more objects with a total size of 20.
                             |  This was detected by one or more of your sonar pings in turn 1.
    -------------------------|--------------------------------------------------------------------
    O|1|34|17|100            |  Square 34|17 has one or more objects with a total size of 100.
                             |  This was detected by one or more of your sonar pings in turn 1.
    -------------------------|--------------------------------------------------------------------
    T|1|37|15|1              |  In turn 1 a torpedo fired by you detonated at square 37|15
                             |  and inflicted 1 point of damage on an enemy submarine.
    -------------------------|--------------------------------------------------------------------
    I|1|0|13|4|1|shields=3|  |  In turn 1 submarine 0 is active and ended up at square 13|4.
      sonar_range=2          |  This sub has 3 shields and sonar is charged to a range of 2.
    -------------------------|--------------------------------------------------------------------
    I|1|1|39|10|1|shields=3  |  In turn 1 submarine 1 is active and ended up at square 39|10.
                             |  This sub has 3 shields.
    -------------------------|--------------------------------------------------------------------
    I|1|2|27|15|1|shields=2  |  In turn 1 submarine 2 ended up at square 27|15.
                             |  This sub has 2 shields (took an indirect hit from D|1|28|16|1).
    -------------------------|--------------------------------------------------------------------
    H|1|1                    |  After turn 1 your score is 1

Then the server sends a `B` (begin turn) message to all players to begin turn 2:

    Message  |  Details
    =========|================
    B|2      |  Begin turn 2.

Summary of messages from player 1 perspective:

    Server Message                          |  Client Message
    ========================================|=================
    I|0|0|13|3|1|shields=3\n                |
    I|0|1|39|10|1|shields=3\n               |
    I|0|2|27|15|1|shields=3\n               |
    B|1\n                                   |
                                            |  M|1|0|E|Sonar\n
                                            |  P|1|1\n
                                            |  F|1|2|31|15\n
    D|1|31|15|1\n                           |
    D|1|28|16|1\n                           |
    S|1|0|6\n                               |
    O|1|33|19|20\n                          |
    O|1|34|17|100\n                         |
    T|1|37|15|1\n                           |
    I|1|0|13|4|1|shields=3|sonar_range=2\n  |
    I|1|1|39|10|1|shields=3\n               |
    I|1|2|27|15|1|shields=2\n               |
    H|1|1                                   |

NOTE: The turn number must be the same in all messages sent during a single turn.

After all info messages for a turn have been sent out the server will send a new `B` (begin turn) message or send an `F` (game finished) message.

NOTE: Info messages are not guaranteed to arrive in any particular order.  So, for example, do not assume that the arrival of the `H` message signals the last info message for a turn. Only the arrival of a `B` (begin turn) or `F` (game finished) message indicates that all info messages for the current turn have been sent.

### Game Finished

When the server sends an `F` (game finished) message.

    Message          |  Details
    =================|========================================================
    F|2|87|finished  |  Game ended with 2 players and finished after 87 turns

NOTE: Currently the only status values that may be returned from this message are `finished` or `aborted`.

The `F` message will be followed by a number of `P` (player result) messages equal to the player count provided in the `F` message (2 in this example).

    Message          |  Details
    =================|===================================
    P|fred|10        |  Player `fred` scored 10 points.
    -----------------|-----------------------------------
    P|alice|14       |  Player `alice` scored 14 points.

NOTE: Player results are only provided for players that finish the game.  Players that disconnected or got disqualified are removed from the game.

Summary of final messages:

    F|2|87|finished\n
    P|fred|10\n
    P|alice|14\n

After these messages are sent all players are disconnected.

