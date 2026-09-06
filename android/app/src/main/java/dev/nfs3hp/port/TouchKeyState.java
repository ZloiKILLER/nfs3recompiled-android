package dev.nfs3hp.port;
import java.util.HashMap;
import java.util.Map;

/** One finger must not release a key still held by another control. */
final class TouchKeyState {
    interface Sink { void send(int key, boolean down); }
    private final Sink sink;
    private final Map<Integer,Integer> held=new HashMap<>();
    TouchKeyState(Sink sink) { this.sink=sink; }
    void press(int key) {
        if(key==0) return;
        int count=held.getOrDefault(key,0);held.put(key,count+1);
        if(count==0) sink.send(key,true);
    }
    void release(int key) {
        int count=held.getOrDefault(key,0);
        if(count==0) return;
        if(count==1) { held.remove(key);sink.send(key,false); } else held.put(key,count-1);
    }
    void releaseAll() { for(int key:held.keySet()) sink.send(key,false);held.clear(); }
}
