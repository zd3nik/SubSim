/*
 * Copyright (c) 2017 Shawn Chidester, All rights reserved
 */

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.*;

/**
 * Single source file example of a SubSim bot
 * <p>
 * The internal classes in this example do not need to be internal.  They were made internal in order to keep
 * this example to a single source file.
 */
public class DudlyJr {

    private static final String DEFAULT_USERNAME = "DudlyJr";
    private static final String DEFAULT_SERVER_ADDRESS = "localhost";
    private static final int DEFAULT_SERVER_PORT = 9555;
    private static final Random random = new Random();
    private static final Direction[] ALL_DIRECTIONS = new Direction[]{
            Direction.North,
            Direction.East,
            Direction.South,
            Direction.West
    };

    private String username;
    private String serverAddress;
    private int serverPort;
    private boolean debugMode = false;
    private Socket socket = null;
    private BufferedReader socketReader = null;
    private PrintWriter socketWriter = null;
    private Map<Coordinate, MapSquare> gameMap = new HashMap<>();
    private List<SonarDetectionMessage> spotted = new ArrayList<>();
    private List<DetonationMessage> detonations = new ArrayList<>();
    private List<TorpedoHitMessage> torpedoHits = new ArrayList<>();
    private Submarine mySub = new Submarine(0); // only 1 sub supported in this bot
    private Coordinate randomDestination = null;
    private int mapWidth = 0;
    private int mapHeight = 0;
    private int turnNumber = 0;

    private DudlyJr(String username, String serverAddress, int serverPort) {
        this.username = username;
        this.serverAddress = serverAddress;
        this.serverPort = serverPort;
        this.debugMode = "1".equals(System.getenv("DEBUG"));
    }

    public static void main(String[] args) {
        String usernameArg      = getArg(args, 0, DEFAULT_USERNAME);
        String serverAddressArg = getArg(args, 1, DEFAULT_SERVER_ADDRESS);
        int serverPortArg       = getArg(args, 2, DEFAULT_SERVER_PORT);

        DudlyJr bot = new DudlyJr(usernameArg, serverAddressArg, serverPortArg);

        try {
            bot.login();
            bot.play();
        } catch (Exception e) {
            System.err.println("ERROR: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        } finally {
            bot.disconnect();
        }
    }

    private static String getArg(String[] args, int index, String defaultValue) {
        if ((index >= 0) && (index < args.length)) {
            return args[index];
        } else {
            return defaultValue;
        }
    }

    private static int getArg(String[] args, int index, int defaultValue) {
        if ((index >= 0) && (index < args.length)) {
            return Integer.parseInt(args[index]);
        } else {
            return defaultValue;
        }
    }

    private Coordinate randomSquare() {
        return randomSquare(null);
    }

    private Coordinate randomSquare(Coordinate from) {
        while (true) {
            int x = (1 + random.nextInt(mapWidth));
            int y = (1 + random.nextInt(mapHeight));
            Coordinate location = new Coordinate(x, y);
            MapSquare square = gameMap.get(location);
            if ((square != null) && !square.blocked && !location.equals(from)) {
                return location;
            }
        }
    }

    private void login() throws Exception {
        // open socket reader/writer to the game server
        connect();

        // immediately after connecting the server sends a game config message
        configure(new GameConfigMessage(receiveMessage()));

        // send our player name and sub location to join
        mySub.location = randomSquare();
        sendMessage(String.format("J|%s|%d|%d", username, mySub.getX(), mySub.getY()));

        // if join was successful we get a "J|username" message back, otherwise the join failed
        String response = receiveMessage();
        if (!String.format("J|%s", username).equals(response)) {
            throw new IOException("Failed to join. Server response = " + response);
        }
    }

    private void configure(GameConfigMessage message) throws Exception {
        turnNumber = 0;
        mapWidth = message.mapWidth;
        mapHeight = message.mapHeight;
        gameMap = new HashMap<>(mapWidth * mapHeight);
        mySub = new Submarine(0);

        for (int y = 1; y <= mapHeight; ++y) {
            for (int x = 1; x <= mapWidth; ++x) {
                MapSquare square = new MapSquare(x, y);
                gameMap.put(square, square);
            }
        }

        System.out.println("Joining as Player  = " + username);
        System.out.println("Server Host:Port   = " + serverAddress + ":" + serverPort);
        System.out.println("Server Version     = " + message.serverVersion);
        System.out.println("Game Title         = " + message.gameTitle);
        System.out.println("Game Map Size      = " + mapWidth + " x " + mapHeight);
    }

    private void play() throws Exception {
        // here we simply handle server messages in the order they are received
        String message;
        while ((message = receiveMessage()) != null) {
            if (message.startsWith("B|")) {
                handleMessage(new BeginTurnMessage(message));
            } else if (message.startsWith("S|")) {
                handleMessage(new SonarDetectionMessage(message));
            } else if (message.startsWith("D|")) {
                handleMessage(new DetonationMessage(message));
            } else if (message.startsWith("T|")) {
                handleMessage(new TorpedoHitMessage(message));
            } else if (message.startsWith("O|")) {
                handleMessage(new DiscoveredObjectMessage(message));
            } else if (message.startsWith("I|")) {
                handleMessage(new SubmarineInfoMessage(message));
            } else if (message.startsWith("H|")) {
                handleMessage(new PlayerScoreMessage(message));
            } else if (message.startsWith("F|")) {
                handleMessage(new GameFinishedMessage(message));
                break; // exit the loop
            } else {
                System.err.println("Error Message: " + message);
            }
        }
    }

    private void checkTurnNumber(TurnRelatedMessage message) {
        if (message.turnNumber != turnNumber) {
            throw new IllegalStateException("expected turn number " + turnNumber +
                    " got turn number " + message.turnNumber + " on message: " + message);
        }
    }

    private void handleMessage(BeginTurnMessage message) throws Exception {
        // update turn number
        turnNumber = message.getIntPart(1);
        if (debugMode) {
            System.out.println("Press ENTER to continue turn " + turnNumber);
            byte[] buf = new byte[1];
            while (System.in.read(buf) == 1) {
                if (buf[0] == '\n') {
                    break;
                }
            }
        }

        // this is where all the A.I. is
        issueCommand(mySub);

        // clear information - it will be re-populated by turn results messages from the server
        for (Map.Entry<Coordinate, MapSquare> entry : gameMap.entrySet()) {
            entry.getValue().reset();
        }

        spotted.clear();
        detonations.clear();
        torpedoHits.clear();
    }

    private Map<Coordinate, Integer> squaresInRangeOf(Coordinate from, int range) throws Exception {
        if (!gameMap.containsKey(from)) {
            throw new IllegalAccessException("squaresInRangeOf() invalid coordinate: " + from.x + "|" + from.y);
        }
        Map<Coordinate, Integer> destinations = new HashMap<>();
        destinations.put(from, 0);
        if (range > 0) {
            // do a breadth-first search to find all squares within range of from square
            addDestinations(destinations, from, 1, range);
        }
        return destinations;
    }

    private void addDestinations(Map<Coordinate, Integer> dests, final Coordinate coord, int distance, int range) {
        List<Coordinate> next = new ArrayList<>();
        for (Direction dir : ALL_DIRECTIONS) {
            Coordinate dest = coord.shifted(dir);
            if (gameMap.containsKey(dest) && !gameMap.get(dest).blocked) {
                Integer previousDist = dests.get(dest);
                if ((previousDist == null) || (distance < previousDist)) {
                    dests.put(dest, distance);
                    if (gameMap.get(dest).isEmpty()) {
                        next.add(dest);
                    }
                }
            }
        }

        if (distance < range) {
            for (Coordinate square : next) {
                addDestinations(dests, square, (distance + 1), range);
            }
        }
    }

    private int getBlastDistance(Coordinate from, Coordinate to) throws Exception {
        if (!gameMap.containsKey(from) || !gameMap.containsKey(to)) {
            throw new IllegalAccessException("getBlastDistance() invalid coordinates");
        }
        int xDiff = Math.abs(from.x - to.x);
        int yDiff = Math.abs(from.y - to.y);
        return Math.max(xDiff, yDiff);
    }

    private Coordinate getTorpedoTarget(Submarine sub) throws Exception {
        if (sub.torpedoRange < 2) {
            return null;
        }

        int largestSize = 1;
        List<Coordinate> targets = new ArrayList<>();
        Map<Coordinate, Integer> dests = squaresInRangeOf(sub.location, sub.torpedoRange);
        for (Map.Entry<Coordinate, Integer> entry : dests.entrySet()) {
            Coordinate location = entry.getKey();
            if ((getBlastDistance(sub.location, location) > 1)) {
                MapSquare square = gameMap.get(location);
                if ((square.foreignObjectSize >= largestSize) && (square.foreignObjectSize == square.objectSize)) {
                    targets.add(location);
                }
            }
        }

        if (targets.isEmpty()) {
            return null;
        }

        return targets.get(random.nextInt(targets.size()));
    }

    /*
     * For simplicity this bot uses a very dumb first-square direction guess in this routine.
     * This can result in the bot getting stuck if the game map has obstacles!
     * To mitigate this, issueCommand() randomly chooses a different destination square every once in a while.
     * This method provides an opportunity to do some research on path-finding algorithms.
     * For example, the squaresInRangeOf routine can be adapted to an A* search relatively easily; look it up! :-)
     */
    private Direction getDirectionToward(Coordinate from, Coordinate to) throws Exception {
        if (from.equals(to) || !gameMap.containsKey(from) || !gameMap.containsKey(to)) {
            throw new IllegalArgumentException("getDirectionToward() invalid coordinates: " + from + " - " + to);
        }

        // get intended direction(s) - using very naive approach
        List<Direction> directions = new ArrayList<>();
        if (to.x == from.x) {
            if (to.y < from.y) {
                directions.add(Direction.North);
            } else {
                directions.add(Direction.South);
            }
        } else if (to.y == from.y) {
            if (to.x < from.x) {
                directions.add(Direction.West);
            } else {
                directions.add(Direction.East);
            }
        } else {
            if (to.x < from.x) {
                directions.add(Direction.West);
            } else {
                directions.add(Direction.East);
            }
            if (to.y < from.y) {
                directions.add(Direction.North);
            } else {
                directions.add(Direction.South);
            }
        }

        // pick randomly from intended directions
        Collections.shuffle(directions);
        for (Direction direction : directions) {
            MapSquare square = gameMap.get(from.shifted(direction));
            if ((square != null) && !square.blocked) {
                return direction;
            }
        }

        // none of the intended directions can be moved to, pick a random direction instead
        directions.addAll(Arrays.asList(ALL_DIRECTIONS));
        Collections.shuffle(directions);
        for (Direction direction : directions) {
            MapSquare square = gameMap.get(from.shifted(direction));
            if ((square != null) && !square.blocked) {
                return direction;
            }
        }

        // it should not be possible to get here!
        throw new Exception("No legal direction to move from square " + from.x + "|" + from.y);
    }

    private void issueCommand(Submarine sub) throws Exception {
        // always shoot at stuff when we can!
        Coordinate target = getTorpedoTarget(sub);
        if (target != null) {
            sendMessage(sub.fireTorpedo(turnNumber, target).toString());
            randomDestination = null;
            return;
        }

        // do sonar ping?
        if (sub.maxSonarCharge || (spotted.isEmpty() && (sub.torpedoRange >= sub.sonarRange) &&
                (sub.sonarRange > (1 + random.nextInt(6))))) {
            sendMessage(sub.ping(turnNumber).toString());
            randomDestination = null;
            return;
        }

        // pick a new random destination square?
        if ((randomDestination == null) || (randomDestination.equals(sub.location))) {
            randomDestination = randomSquare(sub.location);
            if (debugMode) {
                System.out.println("New destination square = " + randomDestination);
            }
        }

        // pick an item to charge
        Equipment charge = (sub.maxTorpedoCharge || (sub.torpedoRange >= sub.sonarRange) || (random.nextInt(100) < 33))
                ? Equipment.Sonar
                : Equipment.Torpedo;

        // move toward randomDestination
        Direction direction = getDirectionToward(sub.location, randomDestination);
        sendMessage(sub.move(turnNumber, direction, charge).toString());
    }

    private void handleMessage(SonarDetectionMessage message) {
        checkTurnNumber(message);
        spotted.add(message);
    }

    private void handleMessage(DetonationMessage message) {
        checkTurnNumber(message);
        detonations.add(message);
    }

    private void handleMessage(TorpedoHitMessage message) {
        checkTurnNumber(message);
        torpedoHits.add(message);
    }

    private void handleMessage(DiscoveredObjectMessage message) {
        checkTurnNumber(message);
        MapSquare square = gameMap.get(message.location);
        if (!square.blocked) {
            square.objectSize += message.size;
            square.foreignObjectSize += message.size;
        }
    }

    private void handleMessage(SubmarineInfoMessage message) {
        checkTurnNumber(message);
        mySub.update(message);
        gameMap.get(mySub.location).foreignObjectSize -= mySub.size;
    }

    private void handleMessage(PlayerScoreMessage message) {
        checkTurnNumber(message);
        // this bot doesn't do anything with this information
    }

    private void handleMessage(GameFinishedMessage message) {
        System.out.println("game finished");
        for (PlayerResultMessage result : message.playerResults) {
            System.out.println("  " + result.playerName + " score = " + result.playerScore);
        }
    }

    private boolean isConnected() {
        return ((socket != null) && (socketReader != null) && (socketWriter != null));
    }

    private void connect() throws Exception {
        if (isConnected()) {
            throw new IOException("already connected");
        }
        if (serverAddress.trim().isEmpty()) {
            throw new IllegalStateException("empty server address");
        }
        if (serverPort < 1) {
            throw new IllegalStateException("invalid server port: " + Integer.toString(serverPort));
        }

        socket = new Socket(serverAddress, serverPort);
        socketReader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        socketWriter = new PrintWriter(socket.getOutputStream());
    }

    private void disconnect() {
        if (socketReader != null) {
            try {
                socketReader.close();
            } catch (IOException e) {
                System.err.println("Error closing socket reader: " + e.getMessage());
                e.printStackTrace();
            } finally {
                socketReader = null;
            }
        }
        if (socketWriter != null) {
            socketWriter.close();
            socketReader = null;
        }
        if (socket != null) {
            try {
                socket.close();
            } catch (IOException e) {
                System.err.println("Error closing socket: " + e.getMessage());
                e.printStackTrace();
            } finally {
                socket = null;
            }
        }
    }

    private void sendMessage(String message) throws Exception {
        if ((message == null) || message.trim().isEmpty()) {
            throw new IllegalArgumentException("cannot send null or empty message to game server");
        }
        if (message.contains("\n")) {
            throw new IllegalArgumentException("cannot send multi-line message to game server");
        }
        if (isConnected()) {
            if (debugMode) {
                System.out.println("SEND: " + message);
            }
            socketWriter.println(message);
            socketWriter.flush();
        } else {
            throw new IOException("not connected");
        }
    }

    private String receiveMessage() throws Exception {
        if (isConnected()) {
            String message = socketReader.readLine();
            if (debugMode) {
                System.out.println("RECV: " + message);
            }
            return message;
        } else {
            throw new IOException("not connected");
        }
    }

    enum Direction {
        North("N"),
        East("E"),
        South("S"),
        West("W");

        private final String name;

        Direction(String name) {
            this.name = name;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    enum Equipment {
        None("None"),
        Sonar("Sonar"),
        Torpedo("Torpedo");

        private final String name;

        Equipment(String name) {
            this.name = name;
        }

        @Override
        public String toString() {
            return name;
        }
    }

    class Coordinate {
        final int x;
        final int y;

        Coordinate(int x, int y) {
            this.x = x;
            this.y = y;
        }

        @Override
        public String toString() {
            return (x + "|" + y);
        }

        @Override
        public boolean equals(Object other) {
            if (this == other) {
                return true;
            } else if (other instanceof Coordinate) {
                return (hashCode() == ((Coordinate) other).hashCode());
            }
            return false;
        }

        @Override
        public int hashCode() {
            return ((100000 * y) + x); // this assumes map width <= 100000 in all cases
        }

        boolean isValid() {
            return ((x > 0) && (y > 0));
        }

        Coordinate shifted(Direction direction) {
            switch (direction) {
                case North:
                    return new Coordinate(x, (y - 1));
                case South:
                    return new Coordinate(x, (y + 1));
                case East:
                    return new Coordinate((x + 1), y);
                case West:
                    return new Coordinate((x - 1), y);
            }
            return this;
        }
    }

    class MapSquare extends Coordinate {
        boolean blocked = false;
        int objectSize = 0;
        int foreignObjectSize = 0; // objectSize minus mySub - if mySub not on this square then = objectSize

        MapSquare(int x, int y) {
            super(x, y);
        }

        boolean isEmpty() {
            return (!blocked && (objectSize == 0));
        }

        void reset() {
            // do not reset blocked flag, that never changes
            objectSize = 0;
            foreignObjectSize = 0;
        }
    }

    class Submarine {
        final int subId;
        Coordinate location = new Coordinate(0, 0);
        boolean dead = false;
        boolean active = true;
        boolean maxSonarCharge = false;
        boolean maxTorpedoCharge = false;
        int size = 100;
        int shieldCount = 3;
        int sonarRange = 0;
        int torpedoRange = 0;

        Submarine(int subId) {
            this.subId = subId;
        }

        void update(SubmarineInfoMessage info) {
            if (subId != info.subId) {
                throw new RuntimeException("subId (" + info.subId + ") in sub info message doesn't match mySub id");
            }
            location = info.location;
            dead = info.dead;
            active = info.active;
            maxSonarCharge = info.maxSonarCharge;
            maxTorpedoCharge = info.maxTorpedoCharge;
            shieldCount = info.shieldCount;
            sonarRange = info.sonarRange;
            torpedoRange = info.torpedoRange;
        }

        int getX() {
            return location.x;
        }

        int getY() {
            return location.y;
        }

        PingCommand ping(int turnNumber) {
            return new PingCommand(turnNumber, subId);
        }

        MoveCommand move(int turnNumber, Direction direction, Equipment equip) {
            return new MoveCommand(turnNumber, subId, direction, equip);
        }

        FireCommand fireTorpedo(int turnNumber, Coordinate destination) {
            return new FireCommand(turnNumber, subId, destination);
        }
    }

    class SubmarineCommand {
        final int turnNumber;
        final int subId;

        SubmarineCommand(int turnNumber, int subId) {
            this.turnNumber = turnNumber;
            this.subId = subId;
        }
    }

    class PingCommand extends SubmarineCommand {
        PingCommand(int turnNumber, int subId) {
            super(turnNumber, subId);
        }

        @Override
        public String toString() {
            return String.format("P|%d|%d", turnNumber, subId);
        }
    }

    class MoveCommand extends SubmarineCommand {
        final Direction direction;
        final Equipment equip;

        MoveCommand(int turnNumber, int subId, Direction direction, Equipment equip) {
            super(turnNumber, subId);
            this.direction = direction;
            this.equip = equip;
        }

        @Override
        public String toString() {
            return String.format("M|%d|%d|%s|%s", turnNumber, subId, direction, equip);
        }
    }

    class FireCommand extends SubmarineCommand {
        final Coordinate destination;

        FireCommand(int turnNumber, int subId, Coordinate destination) {
            super(turnNumber, subId);
            this.destination = destination;
        }

        @Override
        public String toString() {
            return String.format("F|%d|%d|%d|%d", turnNumber, subId, destination.x, destination.y);
        }
    }

    class ServerMessage {
        final String message;
        final String[] parts;

        ServerMessage(String prefix, String messageType, int minPartCount, String message) {
            this.message = message;
            parts = (message == null ? new String[0] : message.split("\\|"));
            if (((prefix != null) && ((parts.length < 1) || !parts[0].equals(prefix))) ||
                    (parts.length < minPartCount)) {
                throw new IllegalArgumentException(String.format("invalid %s message: %s", messageType, message));
            }
        }

        @Override
        public String toString() {
            return message;
        }

        int getPartCount() {
            return (parts != null) ? parts.length : 0;
        }

        String getPart(int index) {
            return ((parts != null) && (index >= 0) && (index < parts.length)) ? parts[index] : null;
        }

        int getIntPart(int index) {
            return Integer.parseInt(getPart(index));
        }
    }

    class GameConfigMessage extends ServerMessage {
        final String serverVersion;
        final String gameTitle;
        final int mapWidth;
        final int mapHeight;

        GameConfigMessage(String message) throws Exception {
            super("C", "game config", 5, message);
            serverVersion = getPart(1);
            gameTitle = getPart(2);
            mapWidth = getIntPart(3);
            mapHeight = getIntPart(4);

            int settings = getIntPart(5);
            for (int i = 0; i < settings; ++i) {
                receiveMessage(); // consume and ignore custom settings
            }
        }
    }

    class TurnRelatedMessage extends ServerMessage {
        final int turnNumber;

        TurnRelatedMessage(String prefix, String messageType, int minPartCount, String message) throws Exception {
            super(prefix, messageType, minPartCount, message);
            turnNumber = getIntPart(1);
        }
    }

    class BeginTurnMessage extends TurnRelatedMessage {
        BeginTurnMessage(String message) throws Exception {
            super("B", "begin turn", 2, message);
        }
    }

    class SonarDetectionMessage extends TurnRelatedMessage {
        final int subId;
        final int distance;

        SonarDetectionMessage(String message) throws Exception {
            super("S", "sonar detection", 4, message);
            subId = getIntPart(2);
            distance = getIntPart(3);
        }
    }

    class DetonationMessage extends TurnRelatedMessage {
        final Coordinate location;
        final int radius;

        DetonationMessage(String message) throws Exception {
            super("D", "detonation", 5, message);
            int x = getIntPart(2);
            int y = getIntPart(3);
            radius = getIntPart(4);
            location = new Coordinate(x, y);
        }
    }

    class DiscoveredObjectMessage extends TurnRelatedMessage {
        final Coordinate location;
        final int size;

        DiscoveredObjectMessage(String message) throws Exception {
            super("O", "discovered object", 5, message);
            int x = getIntPart(2);
            int y = getIntPart(3);
            location = new Coordinate(x, y);
            if (gameMap.get(location).blocked) {
                size = 0;
            } else {
                size = getIntPart(4);
            }
        }
    }

    class TorpedoHitMessage extends TurnRelatedMessage {
        final Coordinate location;
        final int damage;

        TorpedoHitMessage(String message) throws Exception {
            super("T", "torpedo hit", 5, message);
            int x = getIntPart(2);
            int y = getIntPart(3);
            damage = getIntPart(4);
            location = new Coordinate(x, y);
        }
    }

    class SubmarineInfoMessage extends TurnRelatedMessage {
        final int subId;
        final Coordinate location;
        final boolean active;
        boolean dead = false;
        int shieldCount = 0;
        int sonarRange = 0;
        int torpedoRange = 0;
        boolean maxSonarCharge = false;
        boolean maxTorpedoCharge = false;

        SubmarineInfoMessage(String message) throws Exception {
            super("I", "submarine info", 6, message);
            subId = getIntPart(2);
            int x = getIntPart(3);
            int y = getIntPart(4);
            active = (getIntPart(5) == 1);
            location = new Coordinate(x, y);

            for (int i = 6; i < getPartCount(); ++i) {
                String[] var = getPart(i).split("=");
                if (var.length != 2) {
                    throw new IllegalArgumentException(String.format("invalid submarine info message: %s", message));
                } else if ("shields".equals(var[0])) {
                    shieldCount = Integer.parseInt(var[1]);
                } else if ("sonar_range".equals(var[0])) {
                    sonarRange = Integer.parseInt(var[1]);
                } else if ("max_sonar".equals(var[0])) {
                    maxSonarCharge = "1".equals(var[1]);
                } else if ("torpedo_range".equals(var[0])) {
                    torpedoRange = Integer.parseInt(var[1]);
                } else if ("max_torpedo".equals(var[0])) {
                    maxTorpedoCharge = "1".equals(var[1]);
                } else if ("dead".equals(var[0])) {
                    dead = (Integer.parseInt(var[1]) == 1);
                }
            }
        }
    }

    class PlayerScoreMessage extends TurnRelatedMessage {
        final int score;

        PlayerScoreMessage(String message) throws Exception {
            super("H", "player score", 3, message);
            score = getIntPart(2);
        }
    }

    class PlayerResultMessage extends ServerMessage {
        final String playerName;
        final int playerScore;

        PlayerResultMessage(String message) throws Exception {
            super("P", "player result", 3, message);
            playerName = getPart(1);
            playerScore = getIntPart(2);
        }
    }

    class GameFinishedMessage extends ServerMessage {
        final int playerCount;
        final int turnCount;
        final String gameState;
        final List<PlayerResultMessage> playerResults = new ArrayList<>();

        GameFinishedMessage(String message) throws Exception {
            super("F", "game finished", 4, message);
            playerCount = getIntPart(1);
            turnCount = getIntPart(2);
            gameState = getPart(3);
            for (int i = 0; i < playerCount; ++i) {
                playerResults.add(new PlayerResultMessage(receiveMessage()));
            }
        }
    }

}

