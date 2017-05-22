Dudly Junior
============

DudlyJr is a simplified version of DudlyJr.  DudlyJr is the same as DudlyJr with the following exceptions:

 * Custom game settings not supported.
 * Only messages related to Move, Fire, and Ping are supported.

How to compile
--------------

Run this command in the BotExamples/java/DudlyJr directory (the directory that contains this README.md file)

    javac -d . src/DudlyJr.java

How to run with defaults
------------------------

DudlyJr expects the SubSim game server to be running on localhost and port 9555 by default.  So first make sure there is a SubSim server running on localhost, then simply run DudlyJr:

    java DudlyJr

Run with custom parameters
--------------------------

DudlyJr accepts a custom username, game server address, and game server port.

Here is an example that runs DudlyJr with the username `The Dude`, server address `172.17.0.1`, and port `7777`:

    java DudlyJr "The Dude" 172.17.0.1 7777

Run in DEBUG mode
-----------------

Set an environment variable named DEBUG to 1 to run DudlyJr in debug mode:

    DEBUG=1 java DudlyJr

In this mode DudlyJr will output all client/server protocol messages and wait for you to press enter at the beginning of every turn.

