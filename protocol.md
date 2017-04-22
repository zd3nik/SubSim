Communication Protocol Guide
============================

The SubSim game server (`subsim-server`) and clients communicate with one-line ASCII text messages over a TCP socket.  Each line is terminated by a single new-line character: `\n`

All messages (except error messages) are a pipe delimited list of fields of this form:

    TYPE|VALUE|VALUE|...\n

The `TYPE` field is always a single ASCII character.  The number of `VALUE` fields that follow (if any) is determined by the `TYPE`.

Any message that does not follow this format should be treated as an `ERROR MESSAGE`.

Leading and trailing whitespace characters are stripped from each field.

An example message type is `M` - move submarine.  An `M` message has 3 value fields: `sub_id|direction|equipment_id`

 * `sub_id` = the integer ID of the submarine to move (your first submarine is ID 0, second is ID 1, etc)
 * `direction` = the direction to move the submarine: N, E, S, or W
 * `equipment_id` = the name of the equipment to charge: Sonar, Torpedo, Mine, Sprint, None

So an `M` message that moves submarine 0 north and charges sonar looks like this:

    M|0|N|Sonar\n

No form of character escaping is supported in the messaging protocol.  So message fields may not contain pipe `|` or new-line `\n` characters.

The maximum length (including the new-line) of a single message is 4095 bytes.  Messages that exceed this length will get split, resulting in errors.

-------------------------------------------------------------------------------

Protocol Reference
------------------

### Client to server messages:

    Description    |  Message format
    ===============|=================================
    Join game      |  J|name|X0|Y0|X1|Y1|...|Xn|Yn
    Move           |  M|sub_id|direction|equip_name
    Sleep          |  S|sub_id|equip_name|equip_name
    Ping           |  P|sub_id
    Fire Torpedo   |  F|sub_id|X|Y
    Deploy Mine    |  D|sub_id|direction
    Sprint (Run)   |  R|sub_id|direction|distance
    Surface (Up)   |  U|sub_id

More info:

    Client Message    |  Details
    ==================|===========================================================================
    J|name|X|Y|X|Y    |  Join the current game using the specified player `name`.
                      |  Send this immediately after receiving a game descriptor message from the
                      |  server.
                      |  The number of `X|Y` coordinates provided must match the number of
                      |  submarines per player defined in the game descriptor.  Unless the
                      |  PRESET_START_POSITIONS option is enabled in the game descriptor, in which
                      |  case the J message should only include the `name` field and no `X|Y`
                      |  fields.
    ------------------|---------------------------------------------------------------------------
    M|id|dir|equip    |  Move submarine `id` one square in the specified `dir` (direction) and
                      |  charge the specified `equip` (equipment item).
                      |  Valid `equip` values are: Sonar, Torpedo, Mine, Sprint, or None
                      |  If `equip` is None then one unit of excess power is added to the
                      |  submarine `id` reactor core.
    ------------------|---------------------------------------------------------------------------
    S|id|equip|equip  |  Make submarine `id` sleep this turn and charge 2 `equip` items.
                      |  Valid `equip` values are: Sonar, Torpedo, Mine, Sprint, or None
                      |  For each `equip` that is set to None one unit of excess power is added to
                      |  the submarine `id` reactor core.
    ------------------|---------------------------------------------------------------------------
    P|id              |  Perform a sonar ping from submarine `id`.
    ------------------|---------------------------------------------------------------------------
    F|id|X|Y          |  Fire torpedo from submarine `id` to coordinate `X|Y`.
    ------------------|---------------------------------------------------------------------------
    D|id|dir          |  Deploy mine from submarine `id`.
                      |  The mine is deployed to the square adjacent to submarine `id` in the
                      |  specified `dir` (direction).
    ------------------|---------------------------------------------------------------------------
    R|id|dir|dist     |  Move submarine `id` `dist` (distance) squares in the specified `dir`
                      |  (direction).
    ------------------|---------------------------------------------------------------------------
    U|id              |  Surface submarine `id` for repairs.

### Server to client messages:

    Description        |  Message format
    ===================|============================================
    Game Descriptor    |  G|--see "Game Descriptor Message" below--
    Begin Turn         |  B
    Sonar Activation   |  S|count
    Detonation         |  D|X|Y|radius
    Discovered Object  |  O|X|Y|size
    Torpedo Hit        |  T|X|Y|damage
    Mine Hit           |  M|X|Y|damage
    Submarine Info     |  I|id|X|Y|score|damage|excess|surf|dead
    Error Message      |  E|message

More info:

    Server Message    |  Details
    ==================|===========================================================================
    G|...             |  See "Game Descriptor Message" below for the game info message structure.
                      |  One of these will be sent to every player that connects while the
                      |  server is accepting new players for a game.
    ------------------|---------------------------------------------------------------------------
    B                 |  Sent to each player at the beginning of each new turn once a game has
                      |  started.
    ------------------|---------------------------------------------------------------------------
    S|count           |  Sent to each player at the end of each turn.  Provides the `count` of
                      |  sonar activations that occurred that turn.  Not sent if there were no
                      |  sonar activations.
    ------------------|---------------------------------------------------------------------------
    D|X|Y|radius      |  Sent to each player at the end of each turn.  Provides the location
                      |  and blast radius of each detonation that occurred that turn.  Not sent
                      |  if there were no detonations that turn.
    ------------------|---------------------------------------------------------------------------
    O|X|Y|size.       |  At the end of each turn zero or more of this message type is sent
                      |  privately to each player that activated sonar that turn.  Provides the
                      |  location and size of each object discovered by the sonar from that
                      |  player on that turn.
    ------------------|---------------------------------------------------------------------------
    T|X|Y|damage      |  At the end of each turn zero or more of this message type is sent
                      |  privately to each player that inflicted damage via torpedos that turn.
                      |  Provides the location of and the damage inflicted by each torpedo
                      |  detonation.
    ------------------|---------------------------------------------------------------------------
    M|X|Y|damage      |  At the end of each turn zero or more of this message type is sent
                      |  privately to each player that inflicted damage via mines that turn.
                      |  Provides the location of and the damage inflicted by each mine
                      |  detonation.
    ------------------|---------------------------------------------------------------------------
    I|id|X|Y|score|   |  At the end of each turn one of these messages is sent privately to each
      damage|excess|  |  player for each of their submarines.  Provides the id, location, damage
      surf|dead       |  inflicted (score), damage taken (damage), amount of excess power units
                      |  accumulated in the reactor core, whether the submarine is surfaced, and
                      |  whether the submarine has been destroyed. 
    ------------------|---------------------------------------------------------------------------
    E|message         |  For each unrecognized or invalid order received from a player in a turn
                      |  one of these will be sent privately to the player that issues the invalid
                      |  order(s).

-------------------------------------------------------------------------------

### Game Descriptor Message

The game info message is sent from the server to clients the moment they connect or upon request via the `G` message.

The game info message has the following values:

     #  |  Value
    ====|==========================================================
     1  |  Server version
     2  |  Game title
     3  |  Game started (Y or N)
     4  |  Min players required to start the game
     5  |  Max players allowed to join the game
     6  |  Number of players currently joined
     7  |  Point goal
     8  |  Board width
     9  |  Board height
    10  |  Number of ships (determines number of remaining values)
    11  |  Ship 1 descriptor (A5 = ship 'A', length 5)
    12  |  Ship 2 descriptor
    13  |  etc..

Example game info message:

    G|1.0.0|Example Game|N|2|4|3|17|10|10|5|A5|B4|C3|D3|E2

    Server Version = 1.0.0
    Game Title     = Example Game
    Game Started   = N  (game hasn't started yet)
    Min Players    = 2
    Max Players    = 4
    Players Joined = 3  (game can be started at any time)
    Point Goal     = 17 (game will end when 1 or more players get 17 hits)
    Board Width    = 10
    Board Height   = 10
    Ship Count     = 5
    Ship 1         = Ship A, length 5
    Ship 2         = Ship B, length 4
    Ship 3         = Ship C, length 3
    Ship 4         = Ship D, length 3
    Ship 5         = Ship E, length 2

NOTE: Ship letters do not have to be sequential, they must only be unique and in the range of 'A' through 'W'

-------------------------------------------------------------------------------

### Board Info Message

A board info message is sent to all players whenever the state of a board changes (hits/misses added, player connects/disconnects, etc).

The board info message has the following values:

    #  |  Value          |  Description
    ===|==============================================================
    1  |  Player name    |
    2  |  Player status  |  "disconnected", etc..  Usually blank
    3  |  Board value    |  See "Board Value" below
    4  |  Player score   |  Number of hits this player has scored
       |                 |    *not* the number of hits made on this board
    5  |  Player skips   |  Times this player has skipped their turn

Example board info message:

    B|turkey||--see Board Value below--|1|0

    Player name    = turkey
    Status         = (none)
    Board          = see "Board Value" below for an example
    Score          = 1 (turkey has scored 1 hit on some other player)
    Skips          = 0 (turkey has skipped 0 turns)

#### Board Value

The state of a board is sent as a single line of square values.  Row 1 is first, row 2 is second, etc.

A board value may represent a `complete` board or a `masked` board.  A `complete` board value contains all available information about the board, including ship locations.  A `masked` board value only contains hits, misses, and untouched square information.

When a player joins a game they must send a `complete` board value to the game server.  And when a player re-joins a game in progress the server will send their `complete` board value back to them.

When your board value is sent to other players it is always sent `masked`.

Legal `masked` board values are:

    Character  |  Description
    ===========|===================
            .  |  Un-touched square
            X  |  Hit
            0  |  Miss

Legal `complete` board values are:

    Character  |  Description
    ===========|=============================
            .  |  Empty square
            0  |  Empty square has been shot
        A - W  |  Unhit ship segment
        a - w  |  Hit ship segment

Example board value:

    .........................B..EE.....B....CCC..B.........B....................DDD............AAAAA....

Example viewed one row at a time with spaces between each square:

    . . . . . . . . . .
    . . . . . . . . . .
    . . . . . B . . E E
    . . . . . B . . . .
    C C C . . B . . . .
    . . . . . B . . . .
    . . . . . . . . . .
    . . . . . . D D D .
    . . . . . . . . . .
    . A A A A A . . . .

-------------------------------------------------------------------------------

### Game Finished Message

When the game has finished the server sends a type `F` message followed by one type `R` message for every player.

Type `F` message has the following values:

    #  |  Value
    ===|=======================================================================
    1  |  Game state ("finished" or "aborted")
    2  |  Number of turns taken
    3  |  Number of players (this is the number of type `R` messages to expect)

Example type `F` message:

    F|finished|35|4

    Game state = finished
    Turns      = 35
    Players    = 4

Type `R` messages have the following values:

    #  |  Value
    ===|=======================================================================
    1  |  Player name
    2  |  Player score
    3  |  Number of times this player skipped their turn
    4  |  Number of turns this player had (including skipped turns)
    5  |  Status (disconnected, etc) usually blank

Example type `R` message:

    R|turkey|13|0|35|

    Player name = turkey
    Score       = 13
    Skips       = 0
    Turns       = 35
    Status      = (none)

-------------------------------------------------------------------------------

Example Protocol Exchanges
--------------------------

Messages going from client (you) to server prefixed with `-->`

Messages going from server to client (you) prefixed with `<--`

### Standard Example

    --> (establish connection to server, send no message)

        (server sends game info message)
        <-- G|ver|title|N|2|9|0|17|10|10|5|A5|B4|C3|D3|E2

        (join game)
    --> J|turkey|<board value>

        (confirmation)
        <-- J|turkey

        (another player joins)
        <-- J|shooter

        (say hello to shooter)
    --> M|shooter|hello

        (shooter responds)
        <-- M|shooter|prepare to be cooked

        (another player joins)
        <-- J|edgar

        (shooter sends text to ALL)
        <-- M|shooter|it's hunting season!|All

        (you send text to ALL)
    --> M||i'm coming for you shooter, and hell's coming with me!

        (server sends text to ALL)
        <-- M||i'm going to start the game now, any objections?|All

        (you send text to ALL, NOTE: cannot send private text to server)
    --> M||no objection

        (other players respond with text to ALL)
        <-- M|shooter|nope|All
        <-- M|edgar|no objects|All

        (server sends boards, start message, next player message)
        <-- B|shooter||<board value>|0|0
        <-- B|turkey||<board value>|0|0
        <-- B|edgar||<board value>|0|0
        <-- S|shooter|turkey|edgar
        <-- N|shooter

        (shooter takes shot at you and misses)
        <-- B|turkey||<board value>|0|0
        <-- N|turkey

        (you shoot at shooter, coordinate D2)
    --> S|shooter|4|2

        (you got a hit, server sends board, hit, next turn messages)
        <-- H|turkey|shooter|d2
        <-- B|turkey|<board value>|1|0
        <-- B|shooter|<board value>|0|0
        <-- N|edgar

        (shooter sends taunt)
        <-- M|shooter|you hit like a goldfish!

        (edgar gets hit, server sends board, hit, next turn messages)
        <-- H|edgar|shooter|e2
        <-- B|edgar|<board value>|1|0
        <-- B|shooter|<board value>|0|0
        <-- N|shooter

        (game proceeds until edgar wins, server sends game finished message)
        <-- F|finished|41|3
        <-- R|edgar|17|0|41|
        <-- R|turkey|13|0|41|
        <-- R|shooter|6|0|38|disconnected

-------------------------------------------------------------------------------

### Invalid Player Name

    --> (establish connection to server, send no message)

        (server sends game info message)
        <-- G|ver|title|N|2|9|5|17|10|10|5|A5|B4|C3|D3|E2

        (join game attempt 1)
    --> J|I love really long usernames|<board value>

        (join request rejected, retry permitted)
        <-- E|name too long

        (join game attempt 2)
    --> J|turkey|<board value>

        (join request rejected, retry permitted)
        <-- E|name in use

        (join game attempt 3)
    --> J|me|<board value>

        (join request rejected, retry permitted)
        <-- E|invalid name

        (join game attempt 4)
    --> J|4 attempts|<board value>

        (join request rejected, retry permitted)
        <-- E|invalid name

        (join game attempt 5)
    --> J|dead meat|<board value>

        (confirmation, and list of other players sent)
        <-- J|dead meat
        <-- J|alice
        <-- J|turkey
        <-- J|aakbar
        <-- J|captain nimo
        <-- J|your mom

-------------------------------------------------------------------------------

### Rejoin a Game In Progress

Attempt #1

    --> (establish connection to server, send no message)

        (server sends game info message)
        <-- G|ver|title|Y|2|9|5|17|10|10|5|A5|B4|C3|D3|E2

        (join game attempt using a new player name)
    --> J|some new name

        (error message sent in reply and you are disconnected)
        <-- game is already started

Attempt #2

    --> (establish connection to server, send no message)

        (server sends game info message)
        <-- G|ver|title|Y|2|9|5|17|10|10|5|A5|B4|C3|D3|E2

        (join game attempt using same player name as original join)
    --> J|dead meat

        (confirmation, your board, and list of other players sent)
        <-- J|dead meat
        <-- Y|<board value>
        <-- J|alice
        <-- J|turkey
        <-- J|aakbar
        <-- J|captain nimo
        <-- J|your mom

NOTE: You do not send a board value in the join message when re-joining a game in progress.

NOTE: Your board (type `Y` message) is only sent on rejoin attempts.
