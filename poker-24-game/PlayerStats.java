import java.io.Serializable;

public class PlayerStats implements Serializable {
    private static final long serialVersionUID = 1L;

    public String username;
    public int wins;
    public int played;
    public long avgWinTimeMs;  // 0 if no wins
    public int rank;           // 0 if unranked

    public PlayerStats() {}
    public PlayerStats(String username, int wins, int played, long avgWinTimeMs, int rank) {
        this.username = username;
        this.wins = wins;
        this.played = played;
        this.avgWinTimeMs = avgWinTimeMs;
        this.rank = rank;
    }
}
