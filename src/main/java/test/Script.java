package test;

public final class Script {
    private Script() {}

    public static native void sayHelloCPP(String name);

    public static void sayHelloJava(String name) {
        System.out.printf("Hello from Java, %s!%n", name);
    }

    public static void sayHello(String name) {
        sayHelloJava(name);
        sayHelloCPP(name);
    }
}