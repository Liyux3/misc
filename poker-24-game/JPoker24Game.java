import java.awt.*;
import java.awt.event.*;
import java.rmi.registry.*;
import java.util.ArrayList;
import java.util.List;
import javax.jms.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.swing.*;
import javax.swing.table.DefaultTableModel;

public class JPoker24Game extends JFrame implements MessageListener {

    private static final String CF_JNDI = "jms/JPoker24GameConnectionFactory";
    private static final String QUEUE_JNDI = "jms/JPoker24GameQueue";
    private static final String TOPIC_JNDI = "jms/JPoker24GameTopic";

    private GameService service;
    private String currentUser;

    // JMS
    private javax.jms.Connection jmsConn;
    private Session jmsSession;
    private MessageProducer queueProducer;

    // Game UI state
    private String currentGameId;
    private long gameStartTime;

    // UI
    private CardLayout cardLayout;
    private JPanel mainPanel;
    private JTextField loginUsername;
    private JPasswordField loginPassword;
    private JTextField regUsername;
    private JPasswordField regPassword;
    private JPasswordField regConfirmPassword;
    private JLabel profileName, profileWins, profilePlayed, profileAvg, profileRank;
    private DefaultTableModel leaderboardModel;
    private JTable leaderboardTable;
    private JTabbedPane tabs;

    // Play tab panels (cardlayout inside tab)
    private CardLayout playCardLayout;
    private JPanel playPanel;
    private JLabel waitingLabel;
    private JTextField answerField;
    private JButton submitBtn;
    private JLabel resultLabel;
    private JLabel statusLabel;
    private JButton newGameBtn;
    private JButton nextGameBtn;

    public JPoker24Game(String host) {
        super("Poker 24-Game (JMS)");
        try {
            Registry registry = LocateRegistry.getRegistry(host);
            service = (GameService) registry.lookup("GameService");
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Cannot connect to RMI server: " + e.getMessage());
            System.exit(1);
        }
        try {
            setupJMS(host);
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Cannot connect to JMS server: " + e.getMessage());
            System.exit(1);
        }

        cardLayout = new CardLayout();
        mainPanel = new JPanel(cardLayout);
        mainPanel.add(createLoginPanel(), "login");
        mainPanel.add(createRegisterPanel(), "register");
        mainPanel.add(createGamePanel(), "game");
        add(mainPanel);
        cardLayout.show(mainPanel, "login");

        setSize(700, 500);
        setLocationRelativeTo(null);
        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                if (currentUser != null) {
                    try { service.logout(currentUser); } catch (Exception ex) {}
                }
                try { jmsConn.close(); } catch (Exception ex) {}
                System.exit(0);
            }
        });
        setVisible(true);
    }

    private void setupJMS(String host) throws Exception {
        System.setProperty("org.omg.CORBA.ORBInitialHost", host);
        System.setProperty("org.omg.CORBA.ORBInitialPort", "3700");
        Context ctx = new InitialContext();
        ConnectionFactory factory = (ConnectionFactory) ctx.lookup(CF_JNDI);
        javax.jms.Queue queue = (javax.jms.Queue) ctx.lookup(QUEUE_JNDI);
        Topic topic = (Topic) ctx.lookup(TOPIC_JNDI);
        jmsConn = factory.createConnection();
        jmsSession = jmsConn.createSession(false, Session.AUTO_ACKNOWLEDGE);
        queueProducer = jmsSession.createProducer(queue);
        MessageConsumer topicConsumer = jmsSession.createConsumer(topic);
        topicConsumer.setMessageListener(this);
        jmsConn.start();
    }

    // ================= Login / Register panels =================

    private JPanel createLoginPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(5, 5, 5, 5);
        gbc.fill = GridBagConstraints.HORIZONTAL;

        JLabel title = new JLabel("Login", SwingConstants.CENTER);
        title.setFont(new Font("SansSerif", Font.BOLD, 18));
        gbc.gridx = 0; gbc.gridy = 0; gbc.gridwidth = 2;
        panel.add(title, gbc);

        gbc.gridwidth = 1;
        gbc.gridx = 0; gbc.gridy = 1; panel.add(new JLabel("Login Name:"), gbc);
        loginUsername = new JTextField(15);
        gbc.gridx = 1; panel.add(loginUsername, gbc);
        gbc.gridx = 0; gbc.gridy = 2; panel.add(new JLabel("Password:"), gbc);
        loginPassword = new JPasswordField(15);
        gbc.gridx = 1; panel.add(loginPassword, gbc);

        JPanel buttons = new JPanel(new FlowLayout());
        JButton loginBtn = new JButton("Login");
        JButton toRegBtn = new JButton("Register");
        buttons.add(loginBtn); buttons.add(toRegBtn);
        gbc.gridx = 0; gbc.gridy = 3; gbc.gridwidth = 2;
        panel.add(buttons, gbc);

        loginBtn.addActionListener(e -> doLogin());
        toRegBtn.addActionListener(e -> {
            clearLoginFields();
            cardLayout.show(mainPanel, "register");
        });
        loginPassword.addActionListener(e -> doLogin());
        return panel;
    }

    private JPanel createRegisterPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(5, 5, 5, 5);
        gbc.fill = GridBagConstraints.HORIZONTAL;

        JLabel title = new JLabel("Register", SwingConstants.CENTER);
        title.setFont(new Font("SansSerif", Font.BOLD, 18));
        gbc.gridx = 0; gbc.gridy = 0; gbc.gridwidth = 2;
        panel.add(title, gbc);

        gbc.gridwidth = 1;
        gbc.gridx = 0; gbc.gridy = 1; panel.add(new JLabel("Login Name:"), gbc);
        regUsername = new JTextField(15);
        gbc.gridx = 1; panel.add(regUsername, gbc);
        gbc.gridx = 0; gbc.gridy = 2; panel.add(new JLabel("Password:"), gbc);
        regPassword = new JPasswordField(15);
        gbc.gridx = 1; panel.add(regPassword, gbc);
        gbc.gridx = 0; gbc.gridy = 3; panel.add(new JLabel("Confirm Password:"), gbc);
        regConfirmPassword = new JPasswordField(15);
        gbc.gridx = 1; panel.add(regConfirmPassword, gbc);

        JPanel buttons = new JPanel(new FlowLayout());
        JButton regBtn = new JButton("Register");
        JButton cancelBtn = new JButton("Cancel");
        buttons.add(regBtn); buttons.add(cancelBtn);
        gbc.gridx = 0; gbc.gridy = 4; gbc.gridwidth = 2;
        panel.add(buttons, gbc);

        regBtn.addActionListener(e -> doRegister());
        cancelBtn.addActionListener(e -> {
            clearRegisterFields();
            cardLayout.show(mainPanel, "login");
        });
        regConfirmPassword.addActionListener(e -> doRegister());
        return panel;
    }

    // ================= Game panel =================

    private JPanel createGamePanel() {
        JPanel panel = new JPanel(new BorderLayout());
        tabs = new JTabbedPane();
        tabs.addTab("User Profile", createProfileTab());
        tabs.addTab("Play Game", createPlayTab());
        tabs.addTab("Leader Board", createLeaderboardTab());
        tabs.addChangeListener(e -> {
            int i = tabs.getSelectedIndex();
            if (i == 0) refreshProfile();
            else if (i == 2) refreshLeaderboard();
        });
        panel.add(tabs, BorderLayout.CENTER);

        JButton logoutBtn = new JButton("Logout");
        logoutBtn.addActionListener(e -> doLogout());
        JPanel south = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        south.add(logoutBtn);
        panel.add(south, BorderLayout.SOUTH);
        return panel;
    }

    private JPanel createProfileTab() {
        JPanel p = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(8, 15, 8, 15);
        gbc.anchor = GridBagConstraints.WEST;

        profileName = new JLabel("Player");
        profileName.setFont(new Font("SansSerif", Font.BOLD, 20));
        gbc.gridx = 0; gbc.gridy = 0; gbc.gridwidth = 2; p.add(profileName, gbc);
        gbc.gridwidth = 1;
        gbc.gridy = 1; p.add(new JLabel("Games won:"), gbc);
        profileWins = new JLabel("0"); gbc.gridx = 1; p.add(profileWins, gbc);
        gbc.gridx = 0; gbc.gridy = 2; p.add(new JLabel("Games played:"), gbc);
        profilePlayed = new JLabel("0"); gbc.gridx = 1; p.add(profilePlayed, gbc);
        gbc.gridx = 0; gbc.gridy = 3; p.add(new JLabel("Average winning time:"), gbc);
        profileAvg = new JLabel("-"); gbc.gridx = 1; p.add(profileAvg, gbc);
        gbc.gridx = 0; gbc.gridy = 4; p.add(new JLabel("Rank:"), gbc);
        profileRank = new JLabel("-"); gbc.gridx = 1; p.add(profileRank, gbc);
        return p;
    }

    private JPanel cardRow;
    private JPanel playerListPanel;
    private JLabel liveEvalLabel;
    private static final char[] SUITS = {'\u2660','\u2665','\u2666','\u2663'};

    private JPanel createPlayTab() {
        playCardLayout = new CardLayout();
        playPanel = new JPanel(playCardLayout);

        // idle
        JPanel idle = new JPanel(new GridBagLayout());
        newGameBtn = new JButton("New game");
        newGameBtn.addActionListener(e -> joinGame());
        idle.add(newGameBtn);
        playPanel.add(idle, "idle");

        // waiting
        JPanel waiting = new JPanel(new GridBagLayout());
        waitingLabel = new JLabel("Waiting for players...", SwingConstants.CENTER);
        waitingLabel.setFont(new Font("SansSerif", Font.PLAIN, 16));
        waiting.add(waitingLabel);
        playPanel.add(waiting, "waiting");

        // playing
        JPanel playing = new JPanel(new BorderLayout(10, 10));
        cardRow = new JPanel(new FlowLayout(FlowLayout.CENTER, 12, 12));
        playing.add(cardRow, BorderLayout.CENTER);

        playerListPanel = new JPanel();
        playerListPanel.setLayout(new BoxLayout(playerListPanel, BoxLayout.Y_AXIS));
        playerListPanel.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
        JScrollPane playerScroll = new JScrollPane(playerListPanel);
        playerScroll.setPreferredSize(new Dimension(180, 0));
        playerScroll.setBorder(BorderFactory.createEmptyBorder());
        playing.add(playerScroll, BorderLayout.EAST);

        JPanel inputRow = new JPanel(new BorderLayout(6, 6));
        answerField = new JTextField();
        submitBtn = new JButton("Submit");
        liveEvalLabel = new JLabel("= ?");
        liveEvalLabel.setFont(new Font("SansSerif", Font.BOLD, 14));
        liveEvalLabel.setBorder(BorderFactory.createEmptyBorder(0, 8, 0, 8));
        inputRow.add(answerField, BorderLayout.CENTER);
        inputRow.add(liveEvalLabel, BorderLayout.EAST);
        JPanel inputRow2 = new JPanel(new BorderLayout(6, 6));
        inputRow2.add(inputRow, BorderLayout.CENTER);
        inputRow2.add(submitBtn, BorderLayout.EAST);
        playing.add(inputRow2, BorderLayout.SOUTH);

        submitBtn.addActionListener(e -> submitAnswer());
        answerField.addActionListener(e -> submitAnswer());
        answerField.getDocument().addDocumentListener(new javax.swing.event.DocumentListener() {
            public void insertUpdate(javax.swing.event.DocumentEvent e) { updateLiveEval(); }
            public void removeUpdate(javax.swing.event.DocumentEvent e) { updateLiveEval(); }
            public void changedUpdate(javax.swing.event.DocumentEvent e) { updateLiveEval(); }
        });

        statusLabel = new JLabel(" ", SwingConstants.CENTER);
        playing.add(statusLabel, BorderLayout.NORTH);
        playPanel.add(playing, "playing");

        // over
        JPanel over = new JPanel(new BorderLayout(10, 10));
        resultLabel = new JLabel(" ", SwingConstants.CENTER);
        resultLabel.setFont(new Font("SansSerif", Font.PLAIN, 18));
        over.add(resultLabel, BorderLayout.CENTER);
        nextGameBtn = new JButton("Next game");
        nextGameBtn.addActionListener(e -> joinGame());
        JPanel overSouth = new JPanel(new FlowLayout(FlowLayout.CENTER));
        overSouth.add(nextGameBtn);
        over.add(overSouth, BorderLayout.SOUTH);
        playPanel.add(over, "over");

        playCardLayout.show(playPanel, "idle");
        return playPanel;
    }

    private void updateLiveEval() {
        String text = answerField.getText().trim();
        if (text.isEmpty()) { liveEvalLabel.setText("= ?"); return; }
        Expr24.Rational r = Expr24.evaluate(text);
        liveEvalLabel.setText(r == null ? "= ?" : "= " + r);
    }

    private JComponent buildCardFace(int rank, char suit) {
        boolean red = (suit == '\u2665' || suit == '\u2666');
        JLabel label = new JLabel("<html><center>" + Expr24.rankToLabel(rank) +
                                  "<br>" + suit + "</center></html>", SwingConstants.CENTER);
        label.setFont(new Font("SansSerif", Font.BOLD, 36));
        label.setForeground(red ? new Color(180, 0, 0) : Color.BLACK);
        label.setOpaque(true);
        label.setBackground(Color.WHITE);
        label.setBorder(BorderFactory.createLineBorder(Color.GRAY, 2, true));
        label.setPreferredSize(new Dimension(80, 110));
        return label;
    }

    private void renderCards(int[] cards) {
        cardRow.removeAll();
        java.util.Random r = new java.util.Random(java.util.Arrays.hashCode(cards));
        for (int c : cards) {
            cardRow.add(buildCardFace(c, SUITS[r.nextInt(SUITS.length)]));
        }
        cardRow.revalidate(); cardRow.repaint();
    }

    private void renderPlayerList(java.util.List<String> players) {
        playerListPanel.removeAll();
        for (String p : players) {
            JPanel row = new JPanel();
            row.setLayout(new BoxLayout(row, BoxLayout.Y_AXIS));
            row.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(Color.GRAY, 1, true),
                BorderFactory.createEmptyBorder(4, 6, 4, 6)));
            JLabel name = new JLabel(p);
            name.setFont(new Font("SansSerif", Font.BOLD, 13));
            JLabel stat = new JLabel("Win: -/- avg: -");
            stat.setFont(new Font("SansSerif", Font.PLAIN, 11));
            row.add(name); row.add(stat);
            playerListPanel.add(row);
            playerListPanel.add(Box.createVerticalStrut(6));
            try {
                PlayerStats s = service.getStats(p);
                String avg = s.wins > 0 ? formatMs(s.avgWinTimeMs) : "0.0s";
                stat.setText("Win: " + s.wins + "/" + s.played + " avg: " + avg);
            } catch (Exception ex) {}
        }
        playerListPanel.revalidate(); playerListPanel.repaint();
    }

    private JPanel createLeaderboardTab() {
        JPanel panel = new JPanel(new BorderLayout());
        leaderboardModel = new DefaultTableModel(
            new String[]{"Rank", "Player", "Wins", "Played", "Avg Win Time"}, 0) {
            public boolean isCellEditable(int r, int c) { return false; }
        };
        leaderboardTable = new JTable(leaderboardModel);
        panel.add(new JScrollPane(leaderboardTable), BorderLayout.CENTER);
        JButton refresh = new JButton("Refresh");
        refresh.addActionListener(e -> refreshLeaderboard());
        JPanel south = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        south.add(refresh);
        panel.add(south, BorderLayout.SOUTH);
        return panel;
    }

    // ================= Actions =================

    private void doLogin() {
        String u = loginUsername.getText().trim();
        String p = new String(loginPassword.getPassword()).trim();
        if (u.isEmpty() || p.isEmpty()) { showError("Login name and password cannot be empty."); return; }
        try {
            String r = service.login(u, p);
            if (r.equals("success")) { onLoggedIn(u); }
            else showError(r);
        } catch (Exception ex) { showError("Connection error"); }
    }

    private void doRegister() {
        String u = regUsername.getText().trim();
        String p = new String(regPassword.getPassword()).trim();
        String c = new String(regConfirmPassword.getPassword()).trim();
        if (u.isEmpty() || p.isEmpty()) { showError("Login name and password cannot be empty."); return; }
        if (!p.equals(c)) { showError("Passwords do not match."); return; }
        try {
            String r = service.register(u, p);
            if (r.equals("success")) { onLoggedIn(u); }
            else showError(r);
        } catch (Exception ex) { showError("Connection error"); }
    }

    private void onLoggedIn(String username) {
        currentUser = username;
        profileName.setText(username);
        clearLoginFields(); clearRegisterFields();
        refreshProfile();
        refreshLeaderboard();
        cardLayout.show(mainPanel, "game");
        playCardLayout.show(playPanel, "idle");
    }

    private void doLogout() {
        try { service.logout(currentUser); } catch (Exception ex) {}
        currentUser = null;
        currentGameId = null;
        clearLoginFields(); clearRegisterFields();
        cardLayout.show(mainPanel, "login");
    }

    private void joinGame() {
        if (currentUser == null) return;
        try {
            ObjectMessage m = jmsSession.createObjectMessage(GameMessage.join(currentUser));
            queueProducer.send(m);
            waitingLabel.setText("Joined. Waiting for other players...");
            playCardLayout.show(playPanel, "waiting");
            tabs.setSelectedIndex(1);
        } catch (JMSException e) {
            showError("Send failed: " + e.getMessage());
        }
    }

    private void submitAnswer() {
        if (currentGameId == null) return;
        String expr = answerField.getText().trim();
        if (expr.isEmpty()) return;
        try {
            ObjectMessage m = jmsSession.createObjectMessage(
                GameMessage.answer(currentGameId, currentUser, expr));
            queueProducer.send(m);
            statusLabel.setText("Submitted: " + expr + " (waiting for result...)");
        } catch (JMSException e) {
            showError("Send failed: " + e.getMessage());
        }
    }

    private void refreshProfile() {
        if (currentUser == null) return;
        try {
            PlayerStats s = service.getStats(currentUser);
            profileWins.setText(String.valueOf(s.wins));
            profilePlayed.setText(String.valueOf(s.played));
            profileAvg.setText(s.wins > 0 ? formatMs(s.avgWinTimeMs) : "-");
            profileRank.setText(s.rank > 0 ? "#" + s.rank : "-");
        } catch (Exception e) {
            System.err.println("refreshProfile: " + e);
        }
    }

    private void refreshLeaderboard() {
        try {
            List<PlayerStats> list = service.getLeaderboard();
            leaderboardModel.setRowCount(0);
            for (PlayerStats s : list) {
                leaderboardModel.addRow(new Object[]{
                    s.rank > 0 ? String.valueOf(s.rank) : "-",
                    s.username, s.wins, s.played,
                    s.wins > 0 ? formatMs(s.avgWinTimeMs) : "-"
                });
            }
        } catch (Exception e) {
            System.err.println("refreshLeaderboard: " + e);
        }
    }

    private static String formatMs(long ms) { return String.format("%.2fs", ms / 1000.0); }

    // ================= JMS listener (topic) =================

    public void onMessage(Message m) {
        try {
            if (!(m instanceof ObjectMessage)) return;
            Object body = ((ObjectMessage) m).getObject();
            if (!(body instanceof GameMessage)) return;
            GameMessage gm = (GameMessage) body;
            SwingUtilities.invokeLater(() -> handleTopic(gm));
        } catch (Exception e) { System.err.println("onMessage: " + e); }
    }

    private void handleTopic(GameMessage gm) {
        switch (gm.type) {
            case PLAYER_JOINED:
                if (currentGameId == null && currentUser != null && gm.players != null &&
                    gm.players.contains(currentUser)) {
                    waitingLabel.setText("Players joined (" + gm.players.size() + "): " +
                                         String.join(", ", gm.players));
                    playCardLayout.show(playPanel, "waiting");
                }
                break;
            case GAME_START:
                if (gm.players != null && currentUser != null && gm.players.contains(currentUser)) {
                    currentGameId = gm.gameId;
                    gameStartTime = gm.timestamp;
                    renderCards(gm.cards);
                    renderPlayerList(gm.players);
                    statusLabel.setText("Players: " + String.join(", ", gm.players));
                    answerField.setText(""); answerField.setEnabled(true);
                    submitBtn.setEnabled(true);
                    liveEvalLabel.setText("= ?");
                    playCardLayout.show(playPanel, "playing");
                    tabs.setSelectedIndex(1);
                    answerField.requestFocus();
                }
                break;
            case GAME_RESULT:
                if (currentGameId != null && currentGameId.equals(gm.gameId)) {
                    String msg;
                    if (gm.winner == null) {
                        msg = "<html><center>Game over: no winner.<br/>Click Next Game to play again.</center></html>";
                    } else if (gm.winner.equals(currentUser)) {
                        msg = "<html><center>You win!<br/>" + gm.winningExpr + " = 24<br/>Time: " +
                              formatMs(gm.winTimeMs) + "</center></html>";
                    } else {
                        msg = "<html><center>" + gm.winner + " wins!<br/>" + gm.winningExpr +
                              " = 24<br/>Time: " + formatMs(gm.winTimeMs) + "</center></html>";
                    }
                    resultLabel.setText(msg);
                    answerField.setEnabled(false); submitBtn.setEnabled(false);
                    currentGameId = null;
                    playCardLayout.show(playPanel, "over");
                    refreshProfile();
                    refreshLeaderboard();
                }
                break;
            default: break;
        }
    }

    // ================= Helpers =================
    private void showError(String msg) {
        JOptionPane.showMessageDialog(this, msg, "Error", JOptionPane.ERROR_MESSAGE);
    }
    private void clearLoginFields() { loginUsername.setText(""); loginPassword.setText(""); }
    private void clearRegisterFields() {
        regUsername.setText(""); regPassword.setText(""); regConfirmPassword.setText("");
    }

    public static void main(String[] args) {
        String host = args.length > 0 ? args[0] : "localhost";
        SwingUtilities.invokeLater(() -> new JPoker24Game(host));
    }
}
