/*
 * Copyright (c) 2017 Shawn Chidester, All rights reserved
 */
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.*;

/**
 * Single source file example of a SubSim bot
 * <p>
 * The internal classes in this example do not need to be internal.  They were made internal in order to keep
 * this example to a single source file.
 */
public class Dudly {

    enum Direction {
        North,
        East,
        South,
        West
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

        boolean isValid(int width, int height) {
            return ((x > 0) && (x <= width) && (y > 0) && (y <= height));
        }

        int getX() {
            return x;
        }

        int getY() {
            return y;
        }
    }

    enum Equipment { // TODO string conversion
        None,
        Sonar,
        Torpedo,
        Sprint
    }

    class Submarine {
        private int subId;
        private Coordinate location = new Coordinate();
        // TODO customizable options

        Submarine(int subId) {
            this->subId = subId;
        }

        int getSubId() {
            return subId;
        }

        int getX() {
            return location.getX();
        }

        int getY() {
            return location.getY();
        }

        Coordinate getLocation() {
            return location;
        }

        void setLocation(Coordinate location) {
            this->location = location;
        }

        MoveCommand move(int turnNumber, Direction direction, Equipment charge) {
            location.move(direction);
            return new MoveCommand(turnNumber, subId, direction, charge);
        }

        // TODO add command methods
    }

    class SubmarineCommand {
        private int turnNumber;
        private int subId;
        
        SubmarineCommand(int turnNumber, int subId) {
            this->turnNumber = turnNumber;
            this->subId = subId;
        }

        int getTurnNumber() {
            return turnNumber;
        }

        int getSubId() {
            return subId;
        }
    }

    class MoveCommand extends SubmarineCommand {
        private Direction direction;
        private Equipment charge;

        MoveCommand(int turnNumber, int subId, Direction direction, Equipment charge) {
            super(turnNumber, subId);
            this->direction = direction;
            this->charge = charge;
        } 

        Direction getDirection() {
            return direction;
        }

        Equipement getCharge() {
            return charge;
        }
    }

    class ServerMessage {
        private String[] parts;

        ServerMessage(String prefix, String messageType, int minPartCount, String message)
            throws IllegalArgumentException
        {
            parts = (message == null ? new String[0] : message.split("\\|"));
            if (!message.startsWith(prefix) || (parts.length < minPartCount)) {
                throw new IllegalArgumentException(String.format("invalid %s message: %s", messageType, message));
            }
        }

        int partCount() {
            return (parts != null) ? parts.length : 0;
        }

        String getPart(int index) {
            return ((parts != null) && (index >= 0) && (index < parts.length)) ? parts[index] : null;
        }

        int getInt(int index) {
            return Integer.parse(getPart(index));
        }
    }

    class GameSettingMessage extends ServerMessage {
        private String settingName;

        GameSettingMessage(String message) throws IllegalArgumentException {
            super("V|", "game setting", 3, message);
            settingName = getPart(1);
        }

        String getSettingName() {
            return settingName;
        }
    }

    class GameConfigMessage extends ServerMessage {
        private String serverVersion;
        private String gameTitle;
        private int mapWidth;
        private int mapHeight;
        List<GameSettingMessage> customSettings;

        GameConfigMessage(String message) throws IllegalArgumentException {
            super("C|", "game config", 6, message);
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

    class TrunMessage extends ServerMessage {
        private int turnNumber;

        TurnMessage(String prefix, Strng messageType, int minPartCount, String message)
            throws IllegalArgumentException
        {
            super(prefix, messageType, minPartCount, message);
            turnNumber = getInt(1);
        }

        int getTurnNumber() {
            return turnNumber;
        }
    }

    class BeginTurnMessage extends TurnMessage {
        BeginTurnMessage(String message) throws IllegalArgumentException {
            super("B|", "begin turn", 2, message);
        }
    }

    class ActivationMessage extends TurnMessage {
        private int activations;

        ActivationsMessage(String prefix, String messageType, String message) throws IllegalArgumentException {
            super(prefix, messageType, 3, message);
            activations = getInt(2);
        }

        int getActivations() {
            return activations;
        }
    }

    class SonarActivationsMessage extends ActivationMessage {
        SonarActivationsMessage(String message) throws IllegalArgumentException {
            super("S|", "sonar activation", message);
        }
    }

    class SprintActivationsMessage extends ActivationMessage {
        SprintActivationsMessage(String message) throws IllegalArgumentException {
            super("R|", "sprint activation", message);
        }
    }

    class DetonationMessage extends TurnMessage {
        private Coordinate location;
        private int radius;

        DetonationMessage(String message) throws IllegalArgumentException {
            super("D|", "detonation", 5, message);
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

        DiscoveredObjectMessage(String message) throws IllegalArgumentException {
            super("O|", "discovered object", 5, message);
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

        HitMessage(String prefix, String messageType, String message) throws IllegalArgumentException {
            super(prefix, messageType, 5, message);
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
        TorpedoHitMessage(String message) throws IllegalArgumentException {
            super("T|", "torpedo hit", message);
        }
    }

    class MineHitMessage extends HitMessage {
        MineHitMessage(String message) throws IllegalArgumentException {
            super("M|", "mine hit", message);
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
        private int torpedoRange = 0;
        private boolean mineReady = false;
        private int surfaceTurnsRemaining = 0;
        private int reactorDamage = 0;
        private boolean dead = false;

        SubmarineInfoMessage(String messageType, String message) throws IllegalArgumentException {
            super("I|", messageType, 6, message);
            subId = getInt(2);
            int x = getInt(3);
            int y = getInt(4);
            active = (getInt(5) == 1);
            location = new Coordinate(x, y);

            for (int i = 6; i < getPartCount(); ++i) {
                String[] var = getPart(i).split('=');
                if (var.length != 2) {
                    throw new IllegalArgumentException(String.format("invalid %s message: %s",
                                messageType, message));
                }
                if ("shields".equals(var[0])) {
                    shieldCount = getInt(var[1]);
                } else if ("torpedos".equals(var[0])) {
                    torpedoCount = getInt(var[1]);
                } else if ("mines".equals(var[0])) {
                    mineCount = getInt(var[1]);
                } else if ("sonar_range".equals(var[0])) {
                    sonarRange = getInt(var[1]);
                } else if ("sprint_range".equals(var[0])) {
                    sprintRange = getInt(var[1]);
                } else if ("torpedo_range".equals(var[0])) {
                    torpedoRange = getInt(var[1]);
                } else if ("mine_ready".equals(var[0])) {
                    mineReady = (getInt(var[1]) == 1);
                } else if ("surface_remain".equals(var[0])) {
                    surfaceTurnsRemaining = getInt(var[1]);
                } else if ("reactor_damage".equals(var[0])) {
                    reactorDamage = getInt(var[1]);
                } else if ("dead".equals(var[0])) {
                    dead = (getInt(var[1]) == 1);
                }
            }
        }

        private int getSubId() {
            return subId;
        }

        Coordinate getLocation() {
            return location;
        }

        boolean isActive() {
            return active;
        }

        int getShields() {
            return shields;
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

        getSonarRange() {
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
    }

    class PlayerScoreMessage extends TurnMessage {
        private int score;

        PlayerScoreMessage(String message) throws IllegalArgumentException {
            super("H|", "player score", 3, message);
            score = getInt(2);
        }

        int getScore() {
            return score;
        }
    }

    class GameFinishedMessage extends ServerMessage {
        private int playerCount;
        private int turnCount;
        private String state;
        private List<PlayerResultMessage> playerResults;

        GameFinishedMessage(String message) throws IllegalArgumentException {
            super("F", "game finished", 4, message);
            playerCount = getInt(1);
            turnCount = getInt(2);
            gameState = getPart(3);
            for (int i = 0; i < playerCount; ++i) {
                customSettings.add(new PlayerResultMessage(receiveMessage());
            }
        }

        int getPlayerCount() {
            return playerCount;
        }

        int getTurnCount() {
            return turnCount;
        }

        String getState() {
            return state;
        }

        List<PlayerResultMessage> getPlayerResults() {
            return playerResults;
        }
    }

    class PlayerResultMessage extends ServerMessage {
        private String playerName;
        private int playerScore;

        PlayerResultMessage(String message) throws IllegalArgumentException {
            super("P|", "player result", 3, message);
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
    private Submarine mySub = new Submarine(); // only 1 sub supported in this bot
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

    public void login() throws IOException {
        // open socket reader/writer to the game server
        connect();

        // immediately after connecting the server sends a game config message
        String message = receiveMessage();
        configure(new GameConfigMessage(message)); // NOTE: this sets mySub location

        // register with the game server by sending a join message: J|username|subX|subY
        // NOTE: this assumes game configured for 1 sub per player and no pre-set sub location
        message = String.format("J|%s|%d|%d", username, sub.getX(), sub.getY());
        sendMessage(joinMessage);

        // if registration is successful we get a join message back (without the board): J|username
        // this is the only time we are in challenge/response mode with the server
        message = receiveMessage();
        if (!response.equals(String.format("J|%s", username))) {
            throw new IOException("Failed to join. Server response = " + response);
        }
    }

    public void play() throws IOException {
        // here we simply handle server messages in the order they are received
        String message;
        while ((message = receiveMessage()) != null) {
            if (message.startsWith("B|")) {
                mySub.handleMessage(new BeginTurnMessage(message));
            } else if (message.startsWith("S|")) {
                mySub.handleMessage(new SonarActivationsMessage(message));
            } else if (message.startsWith("R|")) {
                mySub.handleMessage(new SprintActivationsMessage(message));
            } else if (message.startsWith("D|")) {
                mySub.handleMessage(new DetonationMessage(message));
            } else if (message.startsWith("O|")) {
                mySub.handleMessage(new DiscoveredObjectMessage(message));
            } else if (message.startsWith("T|")) {
                mySub.handleMessage(new TorpedoHitMessage(message));
            } else if (message.startsWith("M|")) {
                mySub.handleMessage(new MineHitMessage(message));
            } else if (message.startsWith("I|")) {
                mySub.handleMessage(new SubmarineInfoMessage(message));
            } else if (message.startsWith("H|")) {
                mySub.handleMessage(new PlayerScoreMessage(message));
            } else if (message.startsWith("F|")) {
                outputResults(new GameFinishedMessage(message));
                break; // exit the loop
            } else {
                System.err.println("Error Message: " + message);
            }
        }
    }

    private void outputResults(GameFinishedMessage message) throws IOException {
        System.out.println("game finished");
        // after the game finished message the server will send a player result message for each player in the game
        for (PlayerResultMessage result : message.getPlayerResults()) {
            // in this example we just print each player result message to the console
            System.out.println("  " + playerResult.getPlayerName() + " score = " + result.getPlayerScore());
        }
    }

    private boolean isConnected() {
        return ((socket != null) && (socketReader != null) && (socketWriter != null));
    }

    private void connect() throws UnknownHostException, IOException, IllegalStateException {
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

    private void sendMessage(String message) throws IOException, IllegalArgumentException {
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

    private String receiveMessage() throws IOException {
        if (isConnected()) {
            return socketReader.readLine();
        } else {
            throw new IOException("not connected");
        }
    }

}

