Communication Protocol Guide
============================

The SubSim game server (`subsim-server`) and clients communicate with one-line ASCII text messages over a TCP socket.  Each line is terminated by a single new-line character: `\n`

All messages are a pipe delimited list of fields of this form:

    TYPE|VALUE|VALUE|...\n

The number of `VALUE` fields in a message is determined by the `TYPE`.

The `TYPE` field is always a single ASCII character.  This is so you can easily check the message type by examining the first two characters of the message, which will always be the type character followed by a pipe `|` character. **Only checking the first character to determine the message type could lead to errors.**  For example, if you receive `No space left on device` the message type is *not* `N`, it is an errant/invalid message and should be treated as an error message.

Any message that does not follow the prescribed format should be treated as an `ERROR MESSAGE`.

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

The maximum length (including the new-line) of a single message is 4095 bytes.  Messages that exceed this length will cause errors (sorry hackers, it will not cause a buffer overflow).

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
                      |  Send this immediately after receiving the game configuration and game
                      |  setting messages (See the "Server to client messages" section below).
                      |
                      |  Fields:
                      |
                      |    name  = The player name you would like to use.  If the name is invalid
                      |            or in use you will receive an error message and be disconnected.
                      |    X     = The column number to place a submarine at.
                      |            Column numbers start a 0.
                      |    Y     = The row number to place a submarine at.
                      |            Row numbers start at 0.
                      |  
                      |  If the game configuration contains pre-set starting positions for your
                      |  submarines you must not provide any X|Y fields.
                      |
                      |  If the game configuration does *not* contain pre-set starting positions
                      |  for your submarines you must provide one X|Y field pair for each of your
                      |  submarines.  The first pair is for submarine ID 0, the next pair is for
                      |  submarine ID 1, etc.
                      |
                      |  You may place one or more submarines on the same starting square.
                      |  Conceptually, each square represents a large section of ocean so
                      |  multiple objects can fit in one square.
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
                      |            Column numbers start a 0.
                      |    Y     = The row number of the torpedo destination.
                      |            Row numbers start at 0.
    ------------------|---------------------------------------------------------------------------
    D|tn|id|dir       |  Deploy a mine from the specified submarine.
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
                      |  NOTE: If you deploy a mine to a square that is occupied by one or more
                      |        objects the mine will detonate (in the destination square).
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
    Game Setting         |  V|setting_name|setting_value
    Begin Turn           |  B|tn
    Sonar Activations    |  S|tn|count
    Sprint Activations   |  R|tn|count
    Detonation           |  D|tn|X|Y|radius
    Discovered Object    |  O|tn|X|Y|size
    Torpedo Hit          |  T|tn|X|Y|damage
    Mine Hit             |  M|tn|X|Y|damage
    Submarine Info       |  I|tn|id|X|Y|shield_count|core_damage|surfaced|dead
    Player Score (Hits)  |  H|tn|score
    Error Message        |  E|tn|message
    Game Finished        |  F|turn_count
    Player Result        |  P|name|score|turn_count|message

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
                      |  See "settings.md" for a list of available game setting names and their
                      |  legal values.
    ------------------|---------------------------------------------------------------------------
    B|tn              |  Sent to each player at the beginning of each new turn.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |              Turn numbers start at 1.
    ------------------|---------------------------------------------------------------------------
    S|tn|count        |  Sent to each player at the end of each turn that included one or more
                      |  sonar activations.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    count   = The number of sonar activations that occurred.
    ------------------|---------------------------------------------------------------------------
    R|tn|count        |  Sent to each player at the end of each turn that included one or more
                      |  sprint engine activations.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    count   = The number of sprint engine activations that occurred.
    ------------------|---------------------------------------------------------------------------
    D|tn|X|Y|radius   |  One of these messages will be sent, at the end of a turn, for each
                      |  detonation that occurred during that turn.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the detonation.
                      |              Column numbers start at 0.
                      |    Y       = The row of the detonation.
                      |              Row numbers start at 0.
                      |    radius  = The blast radius of the detonation.
    ------------------|---------------------------------------------------------------------------
    O|tn|X|Y|size     |  One of these messages will be sent, at the end of a turn, for each object
                      |  discovered by sonar during that turn.
                      |  Only sent to the player that owns the submarine(s) that discovered the
                      |  object via sonar.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the object.
                      |              Column numbers start at 0.
                      |    Y       = The row of the object.
                      |              Row numbers start at 0.
                      |    size    = The approximate size of the object.
                      |
                      |  Exactly one of these messages is sent per object (per player that
                      |  discovered the object), even if the player activated multiple sonar
                      |  scans within range of the object.
                      |
                      |  NOTE: If multiple objects are in the same square they will appear as
                      |        one object to sonar.  And the approximate size will be the sum
                      |        of all the objects.  For example, if 2 objects of size 20 are
                      |        in the same square sonar will see them as 1 object of size 40.
    ------------------|---------------------------------------------------------------------------
    T|tn|X|Y|damage   |  One of these messages will be sent, at the end of a turn, for each
                      |  torpedo detonation that inflicted damage on one or more enemies during
                      |  that turn.
                      |  Only sent to the player that owns the submarine that fired the torpedo.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the torpedo detonation.
                      |              Column numbers start at 0.
                      |    Y       = The row of the torpedo detonation.
                      |              Row numbers start at 0.
                      |    damage  = The total number of damage points inflicted by the torpedo.
    ------------------|---------------------------------------------------------------------------
    M|tn|X|Y|damage   |  One of these messages will be sent, at the end of a turn, for each
                      |  mine detonation that inflicted damage on one or more enemies during
                      |  that turn.
                      |  Only sent to the player that owns the submarine that deployed the mine.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    X       = The column of the mine that detonated.
                      |              Column numbers start at 0.
                      |    Y       = The row of the mine that detonated.
                      |              Row numbers start at 0.
                      |    damage  = The total number of damage points inflicted by the mine.
    ------------------|---------------------------------------------------------------------------
    I|tn|id|X|Y|      |  One of these messages will be sent, at the end of a turn, for each
      shields|        |  submarine owned by the player the message is sent to.
      damage|         |
      surface|        |  Fields:
      dead            |
                      |    tn      = The turn number.
                      |    id      = The ID of the submarine.
                      |              Submarine IDs start at 0 (per player).
                      |    X       = The column of the submarine.
                      |              Column numbers start at 0;
                      |    Y       = The row of the submarine.
                      |              Row numbers start at 0;
                      |    shields = The number of shields this submarine has remaining.
                      |    damage  = The amount of reactor core damage this submarine has.
                      |    surface = Is this submarine currently surfaced?
                      |              1 = surfaced, 0 = not surfaced
                      |    dead    = Has this submarine been destroyed?
                      |              1 = destroyed, 0 = not destroyed
    ------------------|---------------------------------------------------------------------------
    H|tn|score        |  Sent to each player at the end of each turn to provide the player with
                      |  their current score.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    score   = The total points of damage you have inflicted on enemy
                      |              submarines so far in the current game.
    ------------------|---------------------------------------------------------------------------
    E|tn|message      |  For each unrecognized or invalid command received from a player in a
                      |  turn one of these will be sent to the player that issues the invalid
                      |  command.
                      |
                      |  Fields:
                      |
                      |    tn      = The turn number.
                      |    message = The error message
    ------------------|---------------------------------------------------------------------------
    F|turns           |  This message is sent to all players when the game is finished.
                      |
                      |  Fields:
                      |
                      |    turns   = The number of turns executed in the game.
    ------------------|---------------------------------------------------------------------------
    P|name|score|     |  For each player that joined the game one of these messages is sent to
      turns|message   |  all players at the end of the game.
                      |
                      |  Fields:
                      |
                      |    name    = The name of the player.
                      |    score   = The number of hits this player scored during the game.
                      |    turns   = The number of turns the player participated in.
                      |    message = An optional message to provide exceptional details.
                      |              Will usually not be present unless the player disconnected
                      |              or was disqualified.
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

In this example the customized game settings are Turn Timeout and the Submarine Count each player will start with:

    Message               |  Details
    ======================|=====================================
    V|Turn Timeout|60000  |  Setting Name  = Turn Timeout
                          |  Setting Value = 60000 milliseconds
    ----------------------|-------------------------------------
    V|Submarine Count|3   |  Setting Name  = Submarine Count
                          |  Setting Value = 3

To reiterate: the 3 messages shown above would be sent to each player that connected to the game server.  This is what each player would receive from the server immediately after connecting (note the inclusion of a new-line `\n` character at the end of each message):

    C|1.0.x|Example Game|50|30|2\n
    V|Turn Timeout|60000\n
    V|Submarine Count|3\n

Then each player would attempt to join the game by sending their player name and submarine coordinates to the server.  Here is an example of the `J` (join game) message one of those players might send:

    Message                          |  Details
    =================================|===================================
    J|Captain Nimo|13|3|39|10|27|15  |  Player Name       = Captina Nimo
                                     |  Sub 0 coordinates = 13|3
                                     |  Sub 1 coordinates = 39|10
                                     |  Sub 2 coordinates = 27|15

### Turn Sequence

After enough players have joined to meet the minimum player count setting the game may begin.  Once the game begins the server stops listening for new connections and sends a `B` (begin turn) message to all joined players.

    Message  |  Details
    =========|==================
    B|1      |  Turn Number = 1

During a turn each player must send exactly one command message for each of their `active` submarines.  Here is an example of what the messages might look like from player 1 in a game where each player has 3 submarines:

    Message        |  Details
    ===============|==========================================================
    M|1|0|E|Sonar  |  Move submarine 0 east and charge Sonar in turn 1.
    ---------------|----------------------------------------------------------
    P|1|1          |  Activate sonar ping from submarine 1 in turn 1.
    ---------------|----------------------------------------------------------
    F|1|2|31|15    |  Fire torpedo at square 31|15 from submarine 2 in turn 1.

This is what the messages would look like (note the inclusion of a new-line `\n` character at the end of each message):

    M|0|E|Sonar\n
    P|1\n
    F|2|31|15\n
 
That player would then wait for turn result messages.  Here are some examples:

    Message      |  Details
    =============|================================================================================
    S|1|3        |  3 sonar pings occurred in turn 1.
                 |  This includes the ping done by player 1 this turn, so the other 2 pings
                 |  came from submarines owned by other players.
    -------------|--------------------------------------------------------------------------------
    D|1|31|15|1  |  A detonation with a blast radius of 1 was detected at square 31|15 in turn 1.
                 |  Note that player 1 fired a torpedo at this square this turn.
    -------------|--------------------------------------------------------------------------------
    D|1|28|16|1  |  A detonation with a blast radius of 1 was detected at square 28|16 in turn 1.

The messages above are sent to all players.

The messages below are only sent to player 1.

    Message                |  Details
    =======================|======================================================================
    O|1|33|19|20           |  Something with an approximate size of 20 was detected at square
                           |  33|19 in turn 1.
    -----------------------|----------------------------------------------------------------------
    O|1|34|17|100          |  Something with an approximate size of 100 was detected at square
                           |  34|17 in turn 1.
    -----------------------|----------------------------------------------------------------------
    T|1|37|15|1            |  In turn 1 a torpedo fired by player 1 detonated at square 37|15 and
                           |  inflicted 1 point of damage on an enemy submarine.
    -----------------------|----------------------------------------------------------------------
    I|1|0|13|4|0|3|0|0|0   |  In turn 1 submarine 0 ended up at square 13|4.
                           |  This sub has scored 0 hits on enemy subs so far.
                           |  This sub has 3 shields.
                           |  This sub has 0 points of reactor core damage.
                           |  This sub is not surfaced and not dead.
    -----------------------|----------------------------------------------------------------------
    I|1|1|39|10|0|3|0|0|0  |  In turn 1 submarine 1 ended up at square 39|10.
                           |  This sub has scored 0 hits on enemy subs so far.
                           |  This sub has 3 shields.
                           |  This sub has 0 points of reactor core damage.
                           |  This sub is not surfaced and not dead.
    -----------------------|----------------------------------------------------------------------
    I|1|2|27|15|1|2|0|0|0  |  In turn 1 submarine 2 ended up at square 27|15.
                           |  This sub has scored 1 hit on enemy subs so far.
                           |  This sub has 2 shields (took an indirect hit from D|28|16|1).
                           |  This sub has 0 points of reactor core damage.
                           |  This sub is not surfaced and not dead.

To reiterate, a single turn within the game is started when the server sends a `B` (begin turn) message to all players.  Then all players must submit one command for every active submarine they own.  Then the server sends the appropriate turn result messages to each player.  The above example message sequence (which only shows messages from player 1 perspective) would look like this (note the new-line `\n` character at the end of each message):

    Server Message           |  Client Message
    =========================|=================
    B|1\n                    |
                             |  M|1|0|E|Sonar\n
                             |  P|1|1\n
                             |  F|1|2|31|15\n
    S|1|3\n                  |
    D|1|31|15|1\n            |
    D|1|28|16|1\n            |
    O|1|33|19|20\n           |
    O|1|34|17|100\n          |
    T|1|37|15|1\n            |
    I|1|0|13|4|0|3|0|0|0\n   |
    I|1|1|39|10|0|3|0|0|0\n  |
    I|1|2|27|15|1|2|0|0|0\n  |

NOTE: The turn number must be the same in all message sent during a single turn.  In this example the turn number is 1.  If it was turn number 2, then all messages would have 2 as the turn number.

After all turn results messages have been sent out the server will either start a new turn (send out a new `B` message) or send out an `F` (game finished) message.

### Game Finished

When the server sends an `F` (game finished) message it will also send out one `P` (player result) message for each player that joined the game.  These messages are sent to all players.

    Message  |  Details
    =========|============================
    F|87     |  Game finished in 87 turns

Followed by one `P` (player result) message for each player that joined the game.

    Message                |  Details
    =======================|======================================================================
    P|0|10|87              |  Player 0 scored 10 points and played 87 turns.
    -----------------------|----------------------------------------------------------------------
    P|1|14|87              |  Player 1 scored 14 points and played 87 turns.
    -----------------------|----------------------------------------------------------------------
    P|2|0|19|disqualified  |  Player 2 was disqualified after 19 turns and received a score of 0.

These final messages look like this (note the new-line `\n` character at the end of each message):

    F|87\n
    P|0|10|87\n
    P|1|14|87|\n
    P|2|0|19|disqualified\n

After these messages are sent all players are disconnected.

