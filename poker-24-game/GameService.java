import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.List;

public interface GameService extends Remote {
    String login(String username, String password) throws RemoteException;
    String register(String username, String password) throws RemoteException;
    String logout(String username) throws RemoteException;
    PlayerStats getStats(String username) throws RemoteException;
    List<PlayerStats> getLeaderboard() throws RemoteException;
}
