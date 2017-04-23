XtermBoatSinker Bots!
=====================

An XtermBoatSinker bot is any process that can independentantly play a complete XtermBoatSinker game.

A stand-alone bot is one that can connect to `xbs-server` via TCP socket and send/receive message via that socket.

A shell-bot is a bot that uses stdin and stdout for sending/receiving messages.  A shell-bot cannot connect to `xbs-server` directly, it requires an intermediary like `xbs-client` to handle the TCP socket connection to the game server.

A shell-bot uses the same [Communication Protocol](protocol.md) as a stand-alone bot with one exception:

    It must send a bot info message before any other messages.

A bot info message has the form: `I|botName|botVersion|playerName`

    Field       |  Description
    ============|=========================================================
    botName     |  The name of the bot
    botVersion  |  The bot version
    playerName  |  The name to use to join the game

Provided Bots
-------------

The bots provided in the [src/bots](src/bots) directory are built by default when you run `make`.

These bots can run stand-alone (just run the bot) or as a shell-bot (run the bot with `xbs-client --bot`).  Running as a shell-bot with `xbs-client --bot` provides you with a view of the game boards and user messages as the game progresses.  In this mode you can also post messages and setup automatic taunts.

NOTE: The only difference between running `xbs-client` with and without the `--bot` option is when the `--bot` option is used the specified shell-bot will make all the shots for you when your turn comes around.

When the bots in [src/bots](src/bots) are run stand-alone there is no board display and player messages are not shown.  The bot simply fires shots when its turn comes around and exits when the game ends.

For example, to run the Random Rufus bot in `stand-alone` mode and a game server running on localhost:

    ./xbs-rufus --host localhost

To run Random Rufus as a `shell-bot` with `xbs-client --bot` and a game server running on localhost:

    ./xbs-client --host localhost --bot ./xbs-rufus

To run Random Rufus as a `shell-bot` with a custom player name (notice the use of quote `"` marks to encapsulate the `xbs-rufus` command line):

    ./xbs-client --host localhost --bot "./xbs-rufus --name fred"

Run any of the provided bots with the `--help` option to see what command-line options they support.

    ./xbs-rufus --help

Testing
-------

The bots provided in the [src/bots](src/bots) directory all have a `--test` mode.  This gives you a general idea of the strength and speed of the bot.  The test generates a number of boards with random ship placement and lets the bot take shots at each board until it has achieved the point goal on each board.  Bots that consistently reach the point goal with fewer shots are generally stronger.  The number of test boards used can be set with the `--postitions` command-line parameter.

### Testing Shell-Bots

If you write a shell-bot you can use the `--test` option that is built-in to `xbs-client` to perform the same kind of testing described above on your bot.  Just be aware that testing this way will likely be much slower than a test mode built directly into your bot.

For example, to use the testing mode built-in to Random Rufus:

    ./xbs-rufus --test

To test Random Rufus as a `shell-bot` with `xbs-client --bot`:

    ./xbs-client --test --bot ./xbs-rufus

### Why test?

You should test your bots for at least 2 reasons:

1. To make sure it works
2. To gauge its playing strength

There are 2 primary aspects of a player's skill that determine their playing strength:

* Search method: How they search for hits when there are no *open* hits on any opponent board.
* Destroy method: How they target around *open* hits.

An *open* hit is any hit that has at least one adjacent square that has not already been shot at.  In other words an *open* hit is any hit that is not surrounded on all sides by misses, the edge of the board, or other hits.

### Stressing your bot's search method

In `--test` mode the standard number and size of ships is always used, regardless of board size.  However, you can increase the board size to better judge a bot's search method.  The size of the board can effect a bot's destroy method, but usually to a lesser degree than it effects the search method.

Use the `--width` and `--height` command-line options to change the `--test` board size.

Testing with Skipper
--------------------

The `xbs-skipper` bot provided in this project can be used to test bots in much the same way the `--test` mode described above does.

Skipper does nothing but skip its turn, every time.  You can setup a match between your bot and Skipper to see how many shots your bot needs to reach the point goal because you don't need to worry about Skipper reaching the point goal first.  This is essentially the same thing the `--test` mode described above does: it determines the average number of shots your bot needs to attain the point goal on randomly generated boards.

To run multiple matches between your bot and Skipper you can do the following (assuming the terminal shell you're using is `bash` or something similar):

 * Run xbs-server in repeat mode in one terminal (replace "Game Title" with whatever title you prefer):

    `./xbs-server --max 2 --auto-start --repeat --title "Game Title"`

 * Run Skipper in another terminal:

    `while ./xbs-skipper --host localhost; do sleep 0.1; done`

 * Run your bot in another terminal.  To run your bot 200 times in succession you can use:

    `for i in {1..200}; do (command to run your bot); sleep 0.1; done`

 * When done check the `db/game.Game Title.ini` file for stats (replace *Game Title* with the actual game title used).  Use *total turns taken by your bot* divided by *game count* to calculate the average number of turns your bot needed to attain the point goal.  The lower the average the stronger your bot is!

NOTE: This testing approach will run much slower than a built in test mode or shell-bot testing with xbs-client --bot.

Writing Your Own Bot
--------------------

Writing your own bot is easy.  The [Communication Protocol](protocol.md) is simple and **you only need to handle a sub-set of the full protocol to write a bot**.  For example, your bot doesn't need to handle user messages, or set taunt configurations, etc.  Your bot only needs to know how to:

1. Connect
2. Join the game
3. Shoot when its turn comes around

If you're writing a shell-bot you read messages from stdin and write message to stdout.  Otherwise you open a TCP socket connection to the game server and receive/send messages over that socket.

### Step 1. Connect

In a shell bot there is no connection.  Instead you write an `I` (bot info) message immediately after starting up - before reading/writing any other messages.  The bot info message structure is defined in the top section of this document.

In a stand-alone bot you simply open a TCP socket to the game server.  How you get the host address and port of the game server is up to you. Command-line parameters are a good way to get such information.

Once you've connected you will receive a `G` (game info) message.  This tells you whether the game you are attempting to join is already started, the dimensions of the game board, the size and number of ships, the game server version, etc.  With this information your bot can decide to continue to the next step (join the game) or disconnect.

### Step 2. Join the game

You attempt to join a game by sending a `J` (join game) message that provides your player name and board layout.  In response you will get a `J|playerName` message with the player name you specified if the join attempt was successful.  If you get an `E` message instead that means there's a problem with the player name you supplied.  The message will contain an English description of the problem.  At this point your bot should try a different player name or disconnect.  You may also get an error message in response, in which case you will be disconnected immediately.

### Step 3. Shoot when it's your turn

After successfully joining a game your bot should go into a loop that simply consumes all messages.  It only needs to process the following types of messages to function properly:

1. Next turn messages
2. Board info messages
3. (Optional) Game finished message and following Player Result messages

Whenever a next turn message (`N|playerName`) is received, check the player name in the message.  If the player name matches the name your bot gave during the game join process then it's your bot's turn and it should send a shoot message (`S|targetPlayer|X|Y`) as soon as possible.

In order to send *legal* shot messages when it's your bot's turn, not to mention making *good* shots, your bot must also watch for `B` (board info) messages.  Every board info message that is sent provides you with the most recent board state for the player name in the board info message.  Your bot should store this information so it can decide which board to shoot at and which square to shoot at on the selected board when its turn to shoot comes around.

### Making your bot more advanced

Your bot will function properly with no more logic than what's described above.  But if you want to track your bot's success/failure rate you should also make it watch for `G` (game finished) messages.  Whenever it receives a message of this kind it will be followed by one `R` (player result) message for each player in the game.  From these messages you can obtain the final score each player attained (including your bot's score) and do what you will with it (print it to the console, update a database, etc).

If you really want to get fancy you can also have your bot send periodic messages to taunt, encourage, annoy, or amuse its opponents.  It can also watch for user messages and respond to specific texts or you could even try to build advanced message interaction skills into your bot.

Examples
--------

See the [BotExamples](BotExamples) directory for some example stand-alone bots.  [TurkeyBot.java](BotExamples/java/TurkeyBot/TurkeyBot.java) and [TurkeyBotMinimal.java](BotExamples/java/TurkeyBot/TurkeyBotMinimal.java) are provided as part of this project.  There are also some sub-modules in the BotExamples directory that link to external github projects graciously provided by some friends.  Use the `--recursive` option when cloning this project to pull down those external bot projects.  If you've already cloned XtermBoatSinker without the `--recursive` option you can pull down the external bot projects by running the following git command in the XtermBoatSinker project directory:

    git submodule update --init --recursive
