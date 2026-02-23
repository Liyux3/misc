import java.rmi.*;
import java.rmi.registry.*;
import java.rmi.server.*;
import java.sql.*;
import java.util.*;
import java.util.concurrent.*;
import javax.jms.*;
import javax.naming.Context;
import javax.naming.InitialContext;

public class JPoker24GameServer extends UnicastRemoteObject implements GameService, MessageListener {

    // --- Config ---
    private static final String DB_HOST = "localhost";
    private static final String DB_USER = "c3358";
    private static final String DB_PASS = "c3358PASS";
    private static final String DB_NAME = "c3358";

    private static final String CF_JNDI = "jms/JPoker24GameConnectionFactory";
    private static final String QUEUE_JNDI = "jms/JPoker24GameQueue";
    private static final String TOPIC_JNDI = "jms/JPoker24GameTopic";

    private static final long JOIN_WAIT_MS =
        Long.getLong("JPoker24.joinWaitMs", 10_000L);
    private static final long GAME_TIMEOUT_MS =
        Long.getLong("JPoker24.gameTimeoutMs", 0L);
    private static final int MAX_PLAYERS = 4;
    private static final int MIN_PLAYERS = 2;

    // --- DB ---
    private final java.sql.Connection conn;

    // --- JMS ---
    private final javax.jms.Connection jmsConn;
    private final Session jmsSession;
    private final MessageProducer topicProducer;

    // --- Game state ---
    private enum State { IDLE, JOINING, PLAYING }
    private final Object lock = new Object();
    private State state = State.IDLE;
    private final LinkedHashSet<String> joined = new LinkedHashSet<>();
    private long firstJoinTime;
    private String currentGameId;
    private int[] currentCards;
    private long gameStartTime;
    private boolean gameEnded;
    private final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
    private ScheduledFuture<?> joinTickFuture;
    private ScheduledFuture<?> gameTimeoutFuture;
    private final Random random = new Random();

    public JPoker24GameServer(String jmsHost) throws Exception {
        super();
        Class.forName("com.mysql.cj.jdbc.Driver");
        conn = DriverManager.getConnection(
            "jdbc:mysql://" + DB_HOST + "/" + DB_NAME, DB_USER, DB_PASS);

        // JMS setup (via GlassFish JNDI)
        System.setProperty("org.omg.CORBA.ORBInitialHost", jmsHost);
        System.setProperty("org.omg.CORBA.ORBInitialPort", "3700");
        Context ctx = new InitialContext();
        ConnectionFactory factory = (ConnectionFactory) ctx.lookup(CF_JNDI);
        javax.jms.Queue queue = (javax.jms.Queue) ctx.lookup(QUEUE_JNDI);
        Topic topic = (Topic) ctx.lookup(TOPIC_JNDI);

        jmsConn = factory.createConnection();
        jmsSession = jmsConn.createSession(false, Session.AUTO_ACKNOWLEDGE);
        topicProducer = jmsSession.createProducer(topic);
        MessageConsumer queueConsumer = jmsSession.createConsumer(queue);
        queueConsumer.setMessageListener(this);
        jmsConn.start();

        System.out.println("[Server] JMS connected, listening on queue " + QUEUE_JNDI);
    }

    // ================= RMI methods =================

    public synchronized String login(String username, String password) throws RemoteException {
        if (!validateUser(username, password)) return "Invalid username or password";
        if (isOnline(username)) return "User already logged in";
        try { addOnlineUser(username); } catch (SQLException e) {
            System.err.println("login: " + e); return "Login failed";
        }
        System.out.println("[RMI] login " + username);
        return "success";
    }

    public synchronized String register(String username, String password) throws RemoteException {
        if (username.isEmpty() || password.isEmpty()) return "Username and password cannot be empty";
        if (userExists(username)) return "Username already exists";
        try {
            conn.setAutoCommit(false);
            insertUser(username, password);
            addOnlineUser(username);
            conn.commit();
            System.out.println("[RMI] register " + username);
            return "success";
        } catch (SQLException e) {
            try { conn.rollback(); } catch (SQLException ex) {}
            System.err.println("register: " + e);
            return "Registration failed";
        } finally {
            try { conn.setAutoCommit(true); } catch (SQLException e) {}
        }
    }

    public synchronized String logout(String username) throws RemoteException {
        synchronized (lock) {
            joined.remove(username);
        }
        try { removeOnlineUser(username); } catch (SQLException e) {
            System.err.println("logout: " + e); return "Logout failed";
        }
        System.out.println("[RMI] logout " + username);
        return "success";
    }

    public synchronized PlayerStats getStats(String username) throws RemoteException {
        try (PreparedStatement ps = conn.prepareStatement(
                "SELECT wins, played, total_win_time_ms FROM GameStats WHERE username = ?")) {
            ps.setString(1, username);
            ResultSet rs = ps.executeQuery();
            int wins = 0, played = 0; long total = 0;
            if (rs.next()) {
                wins = rs.getInt(1);
                played = rs.getInt(2);
                total = rs.getLong(3);
            }
            long avg = wins > 0 ? total / wins : 0L;
            int rank = 0;
            if (wins > 0) {
                try (PreparedStatement ps2 = conn.prepareStatement(
                        "SELECT COUNT(*)+1 FROM GameStats WHERE wins > ? " +
                        "OR (wins = ? AND wins > 0 AND total_win_time_ms/wins < ?)")) {
                    ps2.setInt(1, wins);
                    ps2.setInt(2, wins);
                    ps2.setLong(3, avg);
                    ResultSet rs2 = ps2.executeQuery();
                    if (rs2.next()) rank = rs2.getInt(1);
                }
            }
            return new PlayerStats(username, wins, played, avg, rank);
        } catch (SQLException e) {
            throw new RemoteException("getStats failed", e);
        }
    }

    public synchronized List<PlayerStats> getLeaderboard() throws RemoteException {
        List<PlayerStats> out = new ArrayList<>();
        try (PreparedStatement ps = conn.prepareStatement(
                "SELECT username, wins, played, total_win_time_ms FROM GameStats " +
                "WHERE played > 0 ORDER BY wins DESC, " +
                "CASE WHEN wins>0 THEN total_win_time_ms/wins ELSE 9223372036854775807 END ASC, " +
                "username ASC")) {
            ResultSet rs = ps.executeQuery();
            int rank = 0;
            while (rs.next()) {
                rank++;
                String u = rs.getString(1);
                int w = rs.getInt(2);
                int p = rs.getInt(3);
                long t = rs.getLong(4);
                long avg = w > 0 ? t / w : 0L;
                out.add(new PlayerStats(u, w, p, avg, w > 0 ? rank : 0));
            }
        } catch (SQLException e) {
            throw new RemoteException("getLeaderboard failed", e);
        }
        return out;
    }

    // ================= JMS listener =================

    public void onMessage(Message m) {
        try {
            if (!(m instanceof ObjectMessage)) return;
            Object body = ((ObjectMessage) m).getObject();
            if (!(body instanceof GameMessage)) return;
            GameMessage gm = (GameMessage) body;
            switch (gm.type) {
                case JOIN: handleJoin(gm.username); break;
                case ANSWER: handleAnswer(gm); break;
                default: break;
            }
        } catch (Exception e) {
            System.err.println("onMessage: " + e);
            e.printStackTrace();
        }
    }

    private void handleJoin(String username) {
        if (username == null || username.isEmpty()) {
            System.out.println("[Join] rejected null/empty username");
            return;
        }
        synchronized (lock) {
            if (state == State.PLAYING) {
                // queue for next game
                joined.add(username);
                System.out.println("[Join] " + username + " queued (game in progress)");
                return;
            }
            boolean first = joined.isEmpty();
            if (!joined.add(username)) return; // duplicate
            System.out.println("[Join] " + username + " (total " + joined.size() + ")");
            publishSafely(GameMessage.playerJoined(new ArrayList<>(joined)));

            if (state == State.IDLE) {
                state = State.JOINING;
            }
            if (first) {
                firstJoinTime = System.currentTimeMillis();
                schedulePolling();
            }
            if (joined.size() >= MAX_PLAYERS) {
                startGame();
            }
        }
    }

    private void schedulePolling() {
        joinTickFuture = scheduler.scheduleAtFixedRate(() -> {
            synchronized (lock) {
                if (state != State.JOINING) return;
                long elapsed = System.currentTimeMillis() - firstJoinTime;
                if (joined.size() >= MAX_PLAYERS ||
                    (joined.size() >= MIN_PLAYERS && elapsed >= JOIN_WAIT_MS)) {
                    startGame();
                }
            }
        }, 1, 1, TimeUnit.SECONDS);
    }

    private void startGame() {
        if (joinTickFuture != null) { joinTickFuture.cancel(false); joinTickFuture = null; }
        currentGameId = UUID.randomUUID().toString();
        currentCards = drawSolvableCards();
        List<String> players = new ArrayList<>(joined);
        joined.clear();
        currentPlayersSnapshot = players;
        gameStartTime = System.currentTimeMillis();
        gameEnded = false;
        state = State.PLAYING;
        System.out.println("[Game] start " + currentGameId + " players=" + players +
                           " cards=" + Arrays.toString(currentCards));
        publishSafely(GameMessage.gameStart(currentGameId, players, currentCards, gameStartTime));

        if (GAME_TIMEOUT_MS > 0) {
            final List<String> playersFinal = players;
            gameTimeoutFuture = scheduler.schedule(() -> {
                synchronized (lock) {
                    if (state == State.PLAYING && !gameEnded && currentGameId != null) {
                        gameEnded = true;
                        System.out.println("[Game] " + currentGameId + " timed out, no winner");
                        updateStats(playersFinal, null, 0);
                        publishSafely(GameMessage.gameResult(currentGameId, null, null, 0));
                        endGame();
                    }
                }
            }, GAME_TIMEOUT_MS, TimeUnit.MILLISECONDS);
        }
    }

    private void handleAnswer(GameMessage gm) {
        int[] cardsSnapshot;
        String expectedGameId;
        synchronized (lock) {
            if (state != State.PLAYING || gameEnded) return;
            if (gm.gameId == null || !gm.gameId.equals(currentGameId)) return;
            if (gm.username == null || !currentPlayersSnapshot.contains(gm.username)) {
                System.out.println("[Answer] rejected non-player: " + gm.username);
                return;
            }
            cardsSnapshot = currentCards.clone();
            expectedGameId = currentGameId;
        }
        // Validate outside lock
        Expr24.Result r = Expr24.validate(cardsSnapshot, gm.expression);
        synchronized (lock) {
            if (state != State.PLAYING || gameEnded) return;
            if (!expectedGameId.equals(currentGameId)) return;
            if (!r.ok) {
                System.out.println("[Answer] " + gm.username + " wrong: " + gm.expression +
                                   " (" + r.error + ")");
                return;
            }
            gameEnded = true;
            long winMs = System.currentTimeMillis() - gameStartTime;
            List<String> players = currentPlayersSnapshot;
            System.out.println("[Answer] WINNER " + gm.username + " " + gm.expression +
                               " time=" + winMs + "ms");
            updateStats(players, gm.username, winMs);
            publishSafely(GameMessage.gameResult(currentGameId, gm.username, gm.expression, winMs));
            endGame();
        }
    }

    private int[] drawSolvableCards() {
        List<Integer> deck = new ArrayList<>();
        for (int i = 1; i <= 13; i++) deck.add(i);
        for (int attempt = 0; attempt < 200; attempt++) {
            Collections.shuffle(deck, random);
            int[] cards = {deck.get(0), deck.get(1), deck.get(2), deck.get(3)};
            if (Expr24.hasSolution(cards)) return cards;
        }
        return new int[]{1, 2, 3, 4};
    }

    // track players in current game
    private List<String> currentPlayersSnapshot = Collections.emptyList();

    private void endGame() {
        if (gameTimeoutFuture != null) { gameTimeoutFuture.cancel(false); gameTimeoutFuture = null; }
        currentGameId = null;
        currentCards = null;
        state = joined.isEmpty() ? State.IDLE : State.JOINING;
        if (state == State.JOINING) {
            firstJoinTime = System.currentTimeMillis();
            publishSafely(GameMessage.playerJoined(new ArrayList<>(joined)));
            schedulePolling();
            if (joined.size() >= MAX_PLAYERS) startGame();
        }
    }

    // Patch: startGame should also set currentPlayersSnapshot
    private void publishSafely(GameMessage m) {
        try {
            ObjectMessage om = jmsSession.createObjectMessage(m);
            topicProducer.send(om);
        } catch (JMSException e) {
            System.err.println("publish: " + e);
        }
    }

    // ================= JDBC helpers =================

    private boolean validateUser(String u, String p) {
        try (PreparedStatement ps = conn.prepareStatement(
                "SELECT 1 FROM UserInfo WHERE username=? AND password=?")) {
            ps.setString(1, u); ps.setString(2, p);
            return ps.executeQuery().next();
        } catch (SQLException e) { return false; }
    }
    private boolean userExists(String u) {
        try (PreparedStatement ps = conn.prepareStatement(
                "SELECT 1 FROM UserInfo WHERE username=?")) {
            ps.setString(1, u);
            return ps.executeQuery().next();
        } catch (SQLException e) { return false; }
    }
    private boolean isOnline(String u) {
        try (PreparedStatement ps = conn.prepareStatement(
                "SELECT 1 FROM OnlineUser WHERE username=?")) {
            ps.setString(1, u);
            return ps.executeQuery().next();
        } catch (SQLException e) { return false; }
    }
    private void insertUser(String u, String p) throws SQLException {
        try (PreparedStatement ps = conn.prepareStatement(
                "INSERT INTO UserInfo (username,password) VALUES (?,?)")) {
            ps.setString(1, u); ps.setString(2, p); ps.execute();
        }
    }
    private void addOnlineUser(String u) throws SQLException {
        try (PreparedStatement ps = conn.prepareStatement(
                "INSERT INTO OnlineUser (username) VALUES (?)")) {
            ps.setString(1, u); ps.execute();
        }
    }
    private void removeOnlineUser(String u) throws SQLException {
        try (PreparedStatement ps = conn.prepareStatement(
                "DELETE FROM OnlineUser WHERE username=?")) {
            ps.setString(1, u); ps.executeUpdate();
        }
    }
    private void clearOnlineUsers() {
        try (Statement st = conn.createStatement()) {
            st.executeUpdate("DELETE FROM OnlineUser");
        } catch (SQLException e) {}
    }

    private void updateStats(List<String> players, String winner, long winTimeMs) {
        if (players == null || players.isEmpty()) return;
        try {
            conn.setAutoCommit(false);
            try (PreparedStatement ps = conn.prepareStatement(
                    "INSERT INTO GameStats (username, wins, played, total_win_time_ms) " +
                    "VALUES (?, ?, 1, ?) " +
                    "ON DUPLICATE KEY UPDATE played = played + 1, " +
                    "wins = wins + VALUES(wins), " +
                    "total_win_time_ms = total_win_time_ms + VALUES(total_win_time_ms)")) {
                for (String p : players) {
                    boolean isWinner = p.equals(winner);
                    ps.setString(1, p);
                    ps.setInt(2, isWinner ? 1 : 0);
                    ps.setLong(3, isWinner ? winTimeMs : 0L);
                    ps.addBatch();
                }
                ps.executeBatch();
            }
            conn.commit();
        } catch (SQLException e) {
            try { conn.rollback(); } catch (SQLException ex) {}
            System.err.println("updateStats: " + e);
        } finally {
            try { conn.setAutoCommit(true); } catch (SQLException e) {}
        }
    }

    // ================= Main =================
    public static void main(String[] args) {
        String jmsHost = args.length > 0 ? args[0] : "localhost";
        try {
            JPoker24GameServer server = new JPoker24GameServer(jmsHost);
            server.clearOnlineUsers();
            try { System.setSecurityManager(new SecurityManager()); }
            catch (UnsupportedOperationException e) {
                System.out.println("SecurityManager not supported, skipping");
            }
            try { LocateRegistry.createRegistry(1099); }
            catch (java.rmi.server.ExportException e) {}
            Naming.rebind("GameService", server);
            System.out.println("[Server] RMI ready at //localhost/GameService");
        } catch (Exception e) {
            System.err.println("Server error: " + e);
            e.printStackTrace();
        }
    }
}
