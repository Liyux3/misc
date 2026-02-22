import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * 24-game expression evaluator using exact rational arithmetic.
 * Grammar: Expr = Term (('+'|'-') Term)*; Term = Factor (('*'|'/') Factor)*;
 * Factor = number | '(' Expr ')'
 */
public class Expr24 {

    public static final class Result {
        public final boolean ok;
        public final String error;
        public Result(boolean ok, String error) { this.ok = ok; this.error = error; }
    }

    private final String s;
    private int pos;
    private final List<Integer> numbers = new ArrayList<>();

    private Expr24(String s) { this.s = s; }

    public static Result validate(int[] cards, String expression) {
        if (expression == null || expression.trim().isEmpty()) {
            return new Result(false, "Empty expression");
        }
        expression = expression.replace('\u00D7', '*').replace('\u00F7', '/');
        try {
            Expr24 p = new Expr24(expression);
            Rational r = p.parseExpr();
            p.skipWs();
            if (p.pos != p.s.length()) {
                return new Result(false, "Unexpected character at position " + p.pos);
            }
            int[] used = p.numbers.stream().mapToInt(Integer::intValue).sorted().toArray();
            int[] expected = cards.clone();
            Arrays.sort(expected);
            if (!Arrays.equals(used, expected)) {
                return new Result(false, "Expression must use exactly the 4 given cards");
            }
            if (!r.equals24()) {
                return new Result(false, "Expression evaluates to " + r + ", not 24");
            }
            return new Result(true, null);
        } catch (ArithmeticException e) {
            return new Result(false, e.getMessage());
        } catch (RuntimeException e) {
            return new Result(false, "Parse error: " + e.getMessage());
        }
    }

    private void skipWs() {
        while (pos < s.length() && Character.isWhitespace(s.charAt(pos))) pos++;
    }

    private Rational parseExpr() {
        Rational v = parseTerm();
        while (true) {
            skipWs();
            if (pos >= s.length()) break;
            char c = s.charAt(pos);
            if (c == '+' || c == '-') {
                pos++;
                Rational r = parseTerm();
                v = (c == '+') ? v.add(r) : v.sub(r);
            } else break;
        }
        return v;
    }

    private Rational parseTerm() {
        Rational v = parseFactor();
        while (true) {
            skipWs();
            if (pos >= s.length()) break;
            char c = s.charAt(pos);
            if (c == '*' || c == '/') {
                pos++;
                Rational r = parseFactor();
                v = (c == '*') ? v.mul(r) : v.div(r);
            } else break;
        }
        return v;
    }

    private Rational parseFactor() {
        skipWs();
        if (pos >= s.length()) throw new RuntimeException("Unexpected end");
        char c = s.charAt(pos);
        if (c == '(') {
            pos++;
            Rational v = parseExpr();
            skipWs();
            if (pos >= s.length() || s.charAt(pos) != ')')
                throw new RuntimeException("Missing ')'");
            pos++;
            return v;
        }
        // letter cards: A=1, J=11, Q=12, K=13 (case-insensitive)
        int letter = letterCardValue(c);
        if (letter > 0) {
            pos++;
            numbers.add(letter);
            return new Rational(letter, 1);
        }
        if (Character.isDigit(c)) {
            int start = pos;
            while (pos < s.length() && Character.isDigit(s.charAt(pos))) pos++;
            int n = Integer.parseInt(s.substring(start, pos));
            numbers.add(n);
            return new Rational(n, 1);
        }
        throw new RuntimeException("Unexpected character '" + c + "' at position " + pos);
    }

    private static int letterCardValue(char c) {
        switch (c) {
            case 'A': case 'a': return 1;
            case 'J': case 'j': return 11;
            case 'Q': case 'q': return 12;
            case 'K': case 'k': return 13;
            default: return 0;
        }
    }

    /** Evaluate expression without card-set/result validation. Returns null on parse error. */
    public static Rational evaluate(String expression) {
        if (expression == null || expression.trim().isEmpty()) return null;
        expression = expression.replace('\u00D7', '*').replace('\u00F7', '/');
        try {
            Expr24 p = new Expr24(expression);
            Rational r = p.parseExpr();
            p.skipWs();
            if (p.pos != p.s.length()) return null;
            return r;
        } catch (RuntimeException e) {
            return null;
        }
    }

    /** Check if a set of 4 cards has any valid 24 solution. */
    public static boolean hasSolution(int[] cards) {
        String[] ops = {"+", "-", "*", "/"};
        int[] perm = cards.clone();
        Arrays.sort(perm);
        do {
            for (String a : ops) for (String b : ops) for (String c : ops) {
                String[] patterns = {
                    "((%d%s%d)%s%d)%s%d", "(%d%s(%d%s%d))%s%d",
                    "(%d%s%d)%s(%d%s%d)", "%d%s((%d%s%d)%s%d)",
                    "%d%s(%d%s(%d%s%d))"
                };
                for (String f : patterns) {
                    String e = String.format(f, perm[0], a, perm[1], b, perm[2], c, perm[3]);
                    if (validate(cards, e).ok) return true;
                }
            }
        } while (nextPermutation(perm));
        return false;
    }

    private static boolean nextPermutation(int[] a) {
        int i = a.length - 2;
        while (i >= 0 && a[i] >= a[i + 1]) i--;
        if (i < 0) return false;
        int j = a.length - 1;
        while (a[j] <= a[i]) j--;
        int t = a[i]; a[i] = a[j]; a[j] = t;
        for (int l = i + 1, r = a.length - 1; l < r; l++, r--) {
            t = a[l]; a[l] = a[r]; a[r] = t;
        }
        return true;
    }

    public static String rankToLabel(int rank) {
        switch (rank) {
            case 1: return "A";
            case 11: return "J";
            case 12: return "Q";
            case 13: return "K";
            default: return String.valueOf(rank);
        }
    }

    /** Exact rational with long numerator/denominator. Sufficient for 24 puzzle. */
    public static final class Rational {
        final long n, d;
        Rational(long n, long d) {
            if (d == 0) throw new ArithmeticException("Division by zero");
            if (d < 0) { n = -n; d = -d; }
            long g = gcd(Math.abs(n), d);
            this.n = n / g;
            this.d = d / g;
        }
        Rational add(Rational o) { return new Rational(n * o.d + o.n * d, d * o.d); }
        Rational sub(Rational o) { return new Rational(n * o.d - o.n * d, d * o.d); }
        Rational mul(Rational o) { return new Rational(n * o.n, d * o.d); }
        Rational div(Rational o) {
            if (o.n == 0) throw new ArithmeticException("Division by zero");
            return new Rational(n * o.d, d * o.n);
        }
        boolean equals24() { return n == 24 * d; }
        private static long gcd(long a, long b) { return b == 0 ? a : gcd(b, a % b); }
        @Override public String toString() { return d == 1 ? String.valueOf(n) : n + "/" + d; }
    }
}
