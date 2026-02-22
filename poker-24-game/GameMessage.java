import java.io.Serializable;
import java.util.List;

public class GameMessage implements Serializable {
    private static final long serialVersionUID = 1L;

    public enum Type {
        JOIN,            // client -> queue
        ANSWER,          // client -> queue
        PLAYER_JOINED,   // server -> topic (roster update)
        GAME_START,      // server -> topic
        GAME_RESULT      // server -> topic
    }

    public Type type;
    public String gameId;
    public String username;
    public List<String> players;
    public int[] cards;
    public String expression;
    public long timestamp;     // client send time (ANSWER) or start time (GAME_START)
    public String winner;      // null if no winner
    public String winningExpr;
    public long winTimeMs;

    public static GameMessage join(String username) {
        GameMessage m = new GameMessage();
        m.type = Type.JOIN;
        m.username = username;
        m.timestamp = System.currentTimeMillis();
        return m;
    }

    public static GameMessage answer(String gameId, String username, String expression) {
        GameMessage m = new GameMessage();
        m.type = Type.ANSWER;
        m.gameId = gameId;
        m.username = username;
        m.expression = expression;
        m.timestamp = System.currentTimeMillis();
        return m;
    }

    public static GameMessage playerJoined(List<String> players) {
        GameMessage m = new GameMessage();
        m.type = Type.PLAYER_JOINED;
        m.players = players;
        return m;
    }

    public static GameMessage gameStart(String gameId, List<String> players, int[] cards, long startTime) {
        GameMessage m = new GameMessage();
        m.type = Type.GAME_START;
        m.gameId = gameId;
        m.players = players;
        m.cards = cards;
        m.timestamp = startTime;
        return m;
    }

    public static GameMessage gameResult(String gameId, String winner, String expression, long winTimeMs) {
        GameMessage m = new GameMessage();
        m.type = Type.GAME_RESULT;
        m.gameId = gameId;
        m.winner = winner;
        m.winningExpr = expression;
        m.winTimeMs = winTimeMs;
        return m;
    }
}
