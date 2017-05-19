Dudly
=====

Dudly is a minimal SubSim bot example.  It provides a simple foundation on which to build a more advanced bot.  It handles the game join process and message handling required for a functional bot.  Its game play is hopefully enough to provide the basic mechanics needed to build a more intelligent bot.

How to compile
--------------

Run this command in the BotExamples/java/Dudly directory (the directory that contains this README.md file)

    javac -d . src/Dudly.java

How to run with defaults
------------------------

Dudly expects the SubSim game server to be running on localhost and port 9555 by default.  So first make sure there is a SubSim server running on localhost, then simply run Dudly:

    java Dudly

Run with custom parameters
--------------------------

Dudly accepts a custom username, game server address, and game server port.

You can run `java Dudly -h` or `java Dudly --help` to see the command-line syntax.

Here is an example that runs Dudly with the username `The Dude`, server address `172.17.0.1`, and port `7777`:

    java Dudly "The Dude" 172.17.0.1 7777

