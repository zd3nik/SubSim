/*
 * Copyright (c) 2017 Shawn Chidester, All rights reserved
 */

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Single source file example of a SubSim bot
 * <p>
 * The internal classes in this example do not need to be internal.  They were made internal in order to keep
 * this example to a single source file.
 */
public class Dudly {

    private static final String DEFAULT_USERNAME = "Dudly";
    private static final String DEFAULT_SERVER_ADDRESS = "localhost";
    private static final int DEFAULT_SERVER_PORT = 9555;
    private static final Random random = new Random();
    private String username;
    private String serverAddress;
    private int serverPort;
    private Socket socket = null;
    private BufferedReader socketReader = null;
    private PrintWriter socketWriter = null;
    private List<Coordinate> gameMap = new ArrayList<>();
    private Submarine mySub = new Submarine(0); // only 1 sub supported in this bot
    private int turnNumber = 0;

    public Dudly(String username, String serverAddress, int serverPort) {
        this.username = username;
        this.serverAddress = serverAddress;
        this.serverPort = serverPort;
    }

    public static void main(String[] args) {
        if (helpRequested(args)) {
            System.out.println("usage: java Dudly [username [server_address [server_port]]]");
            return;
        }

        String usernameArg = getStringArg(args, 0, DEFAULT_USERNAME);
        String serverAddressArg = getStringArg(args, 1, DEFAULT_SERVER_ADDRESS);
        int serverPortArg = getIntArg(args, 2, DEFAULT_SERVER_PORT);
        Dudly bot = new Dudly(usernameArg, serverAddressArg, serverPortArg);

        try {
            bot.login();
            bot.play();
        } catch (Exception e) {
            System.err.println("ERROR: " + e.getMessage());
            e.printStackTrace();
        } finally {
            bot.disconnect();
        }
    }

    private static boolean helpRequested(String[] args) {
        for (String arg : args) {
            if ("-h".equals(arg) || "--help".equals(arg)) {
                return true;
            }
        }
        return false;
    }

    private static String getStringArg(String[] args, int index, String defaultValue) {
        if ((index >= 0) && (index < args.length)) {
            return args[index];
        } else {
            return defaultValue;
        }
    }

    private static int getIntArg(String[] args, int index, int defaultValue) {
        if ((index >= 0) && (index < args.length)) {
            return Integer.parseInt(args[index]);
        } else {
            return defaultValue;
        }
    }

    private void login() throws Exception {
        // open socket reader/writer to the game server
        connect();

        // immediately after connecting the server sends a game config message
        String message = receiveMessage();
        GameConfigMessage configMessage = new GameConfigMessage(message);
        // TODO configure - this sets mySub location

        // register with the game server by sending a join message: J|username|subX|subY
        // NOTE: this assumes game configured for 1 sub per player and no pre-set sub location
        message = String.format("J|%s|%d|%d", username, mySub.getX(), mySub.getY());
        sendMessage(message);

        // if registration is successful we get a join message back (without the board): J|username
        // this is the only time we are in challenge/response mode with the server
        message = receiveMessage();
        if (!message.equals(String.format("J|%s", username))) {
            throw new IOException("Failed to join. Server response = " + message);
        }
    }

    private void play() throws Exception {
        // here we simply handle server messages in the order they are received
        String message;
        while ((message = receiveMessage()) != null) {
            if (message.startsWith("B|")) {
                handleMessage(new BeginTurnMessage(message));
            } else if (message.startsWith("S|")) {
                handleMessage(new SonarActivationsMessage(message));
            } else if (message.startsWith("R|")) {
                handleMessage(new SprintActivationsMessage(message));
            } else if (message.startsWith("D|")) {
                handleMessage(new DetonationMessage(message));
            } else if (message.startsWith("O|")) {
                handleMessage(new DiscoveredObjectMessage(message));
            } else if (message.startsWith("T|")) {
                handleMessage(new TorpedoHitMessage(message));
            } else if (message.startsWith("M|")) {
                handleMessage(new MineHitMessage(message));
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

    private void handleMessage(SubmarineInfoMessage message) {

    }

    private void handleMessage(PlayerScoreMessage message) {

    }

    private void handleMessage(MineHitMessage message) {

    }

    private void handleMessage(TorpedoHitMessage message) {

    }

    private void handleMessage(DiscoveredObjectMessage message) {

    }

    private void handleMessage(DetonationMessage message) {

    }

    private void handleMessage(SprintActivationsMessage message) {

    }

    private void handleMessage(SonarActivationsMessage message) {

    }

    private void handleMessage(BeginTurnMessage message) {

    }

    private void handleMessage(GameFinishedMessage message) {
        System.out.println("game finished");
        // after the game finished message the server will send a player result message for each player in the game
        for (PlayerResultMessage result : message.getPlayerResults()) {
            // in this example we just print each player result message to the console
            System.out.println("  " + result.getPlayerName() + " score = " + result.getPlayerScore());
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
            socketWriter.println(message);
            socketWriter.flush();
        } else {
            throw new IOException("not connected");
        }
    }

    private String receiveMessage() throws Exception {
        if (isConnected()) {
            return socketReader.readLine();
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
        Torpedo("Torpedo"),
        Sprint("Sprint");

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
        private int x;
        private int y;

        Coordinate(int x, int y) {
            this.x = x;
            this.y = y;
        }

        Coordinate(Coordinate other) {
            this.x = other.x;
            this.y = other.y;
        }

        Coordinate move(Direction direction) {
            switch (direction) {
                case North:
                    --y;
                    break;
                case East:
                    ++x;
                    break;
                case South:
                    ++y;
                    break;
                case West:
                    --x;
                    break;
            }
            return this;
        }

        int getX() {
            return x;
        }

        int getY() {
            return y;
        }

        boolean isValid(int width, int height) {
            return ((x > 0) && (x <= width) && (y > 0) && (y <= height));
        }
    }

    class Submarine {
        private final int subId;
        private Coordinate location = new Coordinate(0, 0);
        private boolean active = true;
        private int shieldCount = 3;
        private int torpedoCount = -1; // -1 = unlimited
        private int mineCount = -1;    // -1 = unlimited
        private int sonarRange = 0;
        private int sprintRange = 0;
        private int torpedoRange = 0;
        private boolean mineReady = false;
        private int surfaceTurnsRemaining = 0;
        private int reactorDamage = 0;
        private boolean dead = false;

        Submarine(int subId) {
            this.subId = subId;
        }

        void update(SubmarineInfoMessage info) throws Exception {
            if (subId != info.getSubId()) {
                throw new Exception("Submarine ID doesn't match");
            }
            location = info.getLocation();
            active = info.isActive();
            shieldCount = info.getShieldCount();
            torpedoCount = info.getTorpedoCount();
            mineCount = info.getMineCount();
            sonarRange = info.getSonarRange();
            sprintRange = info.getSprintRange();
            torpedoRange = info.getTorpedoRange();
            mineReady = info.isMineReady();
            surfaceTurnsRemaining = info.getSurfaceTurnsRemaining();
            reactorDamage = info.getReactorDamage();
            dead = info.isDead();
        }

        int getSubId() {
            return subId;
        }

        Coordinate getLocation() {
            return location;
        }

        int getX() {
            return location.getX();
        }

        int getY() {
            return location.getY();
        }

        int getShieldCount() {
            return shieldCount;
        }

        boolean unlimitedTorpedos() {
            return (torpedoCount < 0);
        }

        boolean unlimitedMines() {
            return (mineCount < 0);
        }

        int getTorpedoCount() {
            return torpedoCount;
        }

        int getMineCount() {
            return mineCount;
        }

        int getSonarRange() {
            return sonarRange;
        }

        int getSprintRange() {
            return sprintRange;
        }

        int getTorpedoRange() {
            return torpedoRange;
        }

        boolean isMineReady() {
            return mineReady;
        }

        boolean isSurfaced() {
            return (surfaceTurnsRemaining > 0);
        }

        int getSurfaceTurnsRemaining() {
            return surfaceTurnsRemaining;
        }

        int getReactorDamage() {
            return reactorDamage;
        }

        boolean isDead() {
            return dead;
        }

        PingCommand ping(int turnNumber) {
            return new PingCommand(turnNumber, subId);
        }

        SleepCommand sleep(int turnNumber, Equipment equip1, Equipment equip2) {
            return new SleepCommand(turnNumber, subId, equip1, equip2);
        }

        MoveCommand move(int turnNumber, Direction direction, Equipment equip) {
            return new MoveCommand(turnNumber, subId, direction, equip);
        }

        SprintCommand sprint(int turnNumber, Direction direction, int distance) {
            return new SprintCommand(turnNumber, subId, direction, distance);
        }

        FireCommand fireTorpedo(int turnNumber, Coordinate destination) {
            return new FireCommand(turnNumber, subId, destination);
        }

        MineCommand deployMine(int turnNumber, Direction direction) {
            return new MineCommand(turnNumber, subId, direction);
        }

        SurfaceCommand surface(int turnNumber) {
            return new SurfaceCommand(turnNumber, subId);
        }
    }

    class SubmarineCommand {
        private int turnNumber;
        private int subId;

        SubmarineCommand(int turnNumber, int subId) {
            this.turnNumber = turnNumber;
            this.subId = subId;
        }

        int getTurnNumber() {
            return turnNumber;
        }

        int getSubId() {
            return subId;
        }
    }

    class PingCommand extends SubmarineCommand {
        PingCommand(int turnNumber, int subId) {
            super(turnNumber, subId);
        }
    }

    class SleepCommand extends SubmarineCommand {
        private Equipment equip1;
        private Equipment equip2;

        SleepCommand(int turnNumber, int subId, Equipment equip1, Equipment equip2) {
            super(turnNumber, subId);
            this.equip1 = equip1;
            this.equip2 = equip2;
        }

        public Equipment getEquip1() {
            return equip1;
        }

        public Equipment getEquip2() {
            return equip2;
        }
    }

    class SprintCommand extends SubmarineCommand {
        private Direction direction;
        private int distance;

        SprintCommand(int turnNumber, int subId, Direction direction, int distance) {
            super(turnNumber, subId);
            this.direction = direction;
            this.distance = distance;
        }

        public Direction getDirection() {
            return direction;
        }

        public int getDistance() {
            return distance;
        }
    }

    class FireCommand extends SubmarineCommand {
        private Coordinate destination;

        FireCommand(int turnNumber, int subId, Coordinate destination) {
            super(turnNumber, subId);
            this.destination = destination;
        }

        public Coordinate getDestination() {
            return destination;
        }
    }

    class MineCommand extends SubmarineCommand {
        private Direction direction;

        MineCommand(int turnNumber, int subId, Direction direction) {
            super(turnNumber, subId);
            this.direction = direction;
        }

        public Direction getDirection() {
            return direction;
        }
    }

    class SurfaceCommand extends SubmarineCommand {
        SurfaceCommand(int turnNumber, int subId) {
            super(turnNumber, subId);
        }
    }

    class MoveCommand extends SubmarineCommand {
        private Direction direction;
        private Equipment equip;

        MoveCommand(int turnNumber, int subId, Direction direction, Equipment equip) {
            super(turnNumber, subId);
            this.direction = direction;
            this.equip = equip;
        }

        Direction getDirection() {
            return direction;
        }

        Equipment getEquip() {
            return equip;
        }
    }

    class ServerMessage {
        private String[] parts;

        ServerMessage(String messageType, int minPartCount, String message) {
            this(null, messageType, minPartCount, message);
        }

        ServerMessage(String prefix, String messageType, int minPartCount, String message) {
            parts = (message == null ? new String[0] : message.split("\\|"));
            if (((prefix != null) && ((parts.length < 1) || !parts[0].equals(prefix))) ||
                    (parts.length < minPartCount)) {
                throw new IllegalArgumentException(String.format("invalid %s message: %s", messageType, message));
            }
        }

        int getPartCount() {
            return (parts != null) ? parts.length : 0;
        }

        String getPart(int index) {
            return ((parts != null) && (index >= 0) && (index < parts.length)) ? parts[index] : null;
        }

        int getInt(int index) {
            return Integer.parseInt(getPart(index));
        }
    }

    class GameSettingMessage extends ServerMessage {
        private String settingName;
        private List<String> values = new ArrayList<>();

        GameSettingMessage(String message) throws Exception {
            super("V", "game setting", 3, message);
            settingName = getPart(1);
            for (int i = 2; i < getPartCount(); ++i) {
                values.add(getPart(i));
            }
        }

        String getSettingName() {
            return settingName;
        }

        String getValue() {
            return values.get(0);
        }

        List<String> getValues() {
            return values;
        }
    }

    class GameConfigMessage extends ServerMessage {
        private String serverVersion;
        private String gameTitle;
        private int mapWidth;
        private int mapHeight;
        List<GameSettingMessage> customSettings = new ArrayList<>();

        GameConfigMessage(String message) throws Exception {
            super("game config", 5, message);
            serverVersion = getPart(1);
            gameTitle = getPart(2);
            mapWidth = getInt(3);
            mapHeight = getInt(4);

            int settings = getInt(5);
            for (int i = 0; i < settings; ++i) {
                customSettings.add(new GameSettingMessage(receiveMessage()));
            }
        }

        String getServerVersion() {
            return serverVersion;
        }

        String getGameTitle() {
            return gameTitle;
        }

        int getMapWidth() {
            return mapWidth;
        }

        int getMapHeight() {
            return mapHeight;
        }

        List<GameSettingMessage> getCustomSettings() {
            return customSettings;
        }
    }

    class TurnMessage extends ServerMessage {
        private int turnNumber;

        TurnMessage(String messageType, int minPartCount, String message) throws Exception {
            super(messageType, minPartCount, message);
            turnNumber = getInt(1);
        }

        int getTurnNumber() {
            return turnNumber;
        }
    }

    class BeginTurnMessage extends TurnMessage {
        BeginTurnMessage(String message) throws Exception {
            super("begin turn", 2, message);
        }
    }

    class ActivationsMessage extends TurnMessage {
        private int activations;

        ActivationsMessage(String messageType, String message) throws Exception {
            super(messageType, 3, message);
            activations = getInt(2);
        }

        int getActivations() {
            return activations;
        }
    }

    class SonarActivationsMessage extends ActivationsMessage {
        SonarActivationsMessage(String message) throws Exception {
            super("sonar activation", message);
        }
    }

    class SprintActivationsMessage extends ActivationsMessage {
        SprintActivationsMessage(String message) throws Exception {
            super("sprint activation", message);
        }
    }

    class DetonationMessage extends TurnMessage {
        private Coordinate location;
        private int radius;

        DetonationMessage(String message) throws Exception {
            super("detonation", 5, message);
            int x = getInt(2);
            int y = getInt(3);
            radius = getInt(4);
            location = new Coordinate(x, y);
        }

        Coordinate getLocation() {
            return location;
        }

        int getRadius() {
            return radius;
        }
    }

    class DiscoveredObjectMessage extends TurnMessage {
        private Coordinate location;
        private int size;

        DiscoveredObjectMessage(String message) throws Exception {
            super("discovered object", 5, message);
            int x = getInt(2);
            int y = getInt(3);
            size = getInt(4);
            location = new Coordinate(x, y);
        }

        Coordinate getLocation() {
            return location;
        }

        int getSize() {
            return size;
        }
    }

    class HitMessage extends TurnMessage {
        private Coordinate location;
        private int damage;

        HitMessage(String messageType, String message) throws Exception {
            super(messageType, 5, message);
            int x = getInt(2);
            int y = getInt(3);
            damage = getInt(4);
            location = new Coordinate(x, y);
        }

        Coordinate getLocation() {
            return location;
        }

        int getDamage() {
            return damage;
        }
    }

    class TorpedoHitMessage extends HitMessage {
        TorpedoHitMessage(String message) throws Exception {
            super("torpedo hit", message);
        }
    }

    class MineHitMessage extends HitMessage {
        MineHitMessage(String message) throws Exception {
            super("mine hit", message);
        }
    }

    class SubmarineInfoMessage extends TurnMessage {
        private int subId;
        private Coordinate location;
        private boolean active;
        private int shieldCount = 0;
        private int torpedoCount = -1; // -1 == unlimited
        private int mineCount = -1;    // -1 == unlimited
        private int sonarRange = 0;
        private int sprintRange = 0;
        private int torpedoRange = 0;
        private boolean mineReady = false;
        private int surfaceTurnsRemaining = 0;
        private int reactorDamage = 0;
        private boolean dead = false;

        SubmarineInfoMessage(String message) throws Exception {
            super("submarine info", 6, message);
            subId = getInt(2);
            int x = getInt(3);
            int y = getInt(4);
            active = (getInt(5) == 1);
            location = new Coordinate(x, y);

            for (int i = 6; i < getPartCount(); ++i) {
                String[] var = getPart(i).split("=");
                if (var.length != 2) {
                    throw new IllegalArgumentException(String.format("invalid submarine info message: %s", message));
                } else if ("shields".equals(var[0])) {
                    shieldCount = Integer.parseInt(var[1]);
                } else if ("torpedos".equals(var[0])) {
                    torpedoCount = Integer.parseInt(var[1]);
                } else if ("mines".equals(var[0])) {
                    mineCount = Integer.parseInt(var[1]);
                } else if ("sonar_range".equals(var[0])) {
                    sonarRange = Integer.parseInt(var[1]);
                } else if ("sprint_range".equals(var[0])) {
                    sprintRange = Integer.parseInt(var[1]);
                } else if ("torpedo_range".equals(var[0])) {
                    torpedoRange = Integer.parseInt(var[1]);
                } else if ("mine_ready".equals(var[0])) {
                    mineReady = (Integer.parseInt(var[1]) == 1);
                } else if ("surface_remain".equals(var[0])) {
                    surfaceTurnsRemaining = Integer.parseInt(var[1]);
                } else if ("reactor_damage".equals(var[0])) {
                    reactorDamage = Integer.parseInt(var[1]);
                } else if ("dead".equals(var[0])) {
                    dead = (Integer.parseInt(var[1]) == 1);
                }
            }
        }

        int getSubId() {
            return subId;
        }

        int getSonarRange() {
            return sonarRange;
        }

        Coordinate getLocation() {
            return location;
        }

        boolean isActive() {
            return active;
        }

        int getShieldCount() {
            return shieldCount;
        }

        boolean unlimitedTorpedos() {
            return (torpedoCount < 0);
        }

        boolean unlimitedMines() {
            return (mineCount < 0);
        }

        int getTorpedoCount() {
            return torpedoCount;
        }

        int getMineCount() {
            return mineCount;
        }

        int getSprintRange() {
            return sprintRange;
        }

        int getTorpedoRange() {
            return torpedoRange;
        }

        boolean isMineReady() {
            return mineReady;
        }

        boolean isSurfaced() {
            return (surfaceTurnsRemaining > 0);
        }

        int getSurfaceTurnsRemaining() {
            return surfaceTurnsRemaining;
        }

        int getReactorDamage() {
            return reactorDamage;
        }

        boolean isDead() {
            return dead;
        }
    }

    class PlayerScoreMessage extends TurnMessage {
        private int score;

        PlayerScoreMessage(String message) throws Exception {
            super("player score", 3, message);
            score = getInt(2);
        }

        int getScore() {
            return score;
        }
    }

    class GameFinishedMessage extends ServerMessage {
        private int playerCount;
        private int turnCount;
        private String gameState;
        private List<PlayerResultMessage> playerResults;

        GameFinishedMessage(String message) throws Exception {
            super("game finished", 4, message);
            playerCount = getInt(1);
            turnCount = getInt(2);
            gameState = getPart(3);
            for (int i = 0; i < playerCount; ++i) {
                playerResults.add(new PlayerResultMessage(receiveMessage()));
            }
        }

        int getPlayerCount() {
            return playerCount;
        }

        int getTurnCount() {
            return turnCount;
        }

        String getGameState() {
            return gameState;
        }

        List<PlayerResultMessage> getPlayerResults() {
            return playerResults;
        }
    }

    class PlayerResultMessage extends ServerMessage {
        private String playerName;
        private int playerScore;

        PlayerResultMessage(String message) throws Exception {
            super("player result", 3, message);
            playerName = getPart(1);
            playerScore = getInt(2);
        }

        String getPlayerName() {
            return playerName;
        }

        int getPlayerScore() {
            return playerScore;
        }
    }

}

