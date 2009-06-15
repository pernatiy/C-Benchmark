import java.util.Date;

public class test {
    public static void main(String[] args) {
        test t = new test();
        t.test();
    }

    public void test() {
        long res = 0;
        int sign = -1;
        long startTime = new Date().getTime();

        for (int i = 1; i <= 10000000; i++) {
            res += testFunc(i % 5 + 1000) * sign;
            sign *= -1;
            res++;
        }

        long endTime = new Date().getTime();

        System.out.println("Execution time : " + (endTime - startTime) + " ms.");
        System.out.println("Result : " + res);
   }

   private long testFunc(long c) {
       return c * c * c;
   }
}
