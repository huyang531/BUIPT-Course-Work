using UnityEngine;

public class MovePlan
{
    public enum State { MOVE, STAY };
    public delegate void Delegate(); //新建一个无参数、无返回值的委托类

    public State state;
    public float time = 0;
    public Vector3 dest;
    public Delegate doSomeThing = null;

    public MovePlan(float time)
    {
        state = State.STAY;
        this.time = time;
    }
    public MovePlan(Vector3 dest)
    {
        state = State.MOVE;
        this.dest = dest;
    }
    public MovePlan(float time, Delegate doSomeThing)
    {
        state = State.STAY;
        this.time = time;
        this.doSomeThing = doSomeThing;
    }

    public MovePlan(Vector3 dest, Delegate doSomeThing)
    {
        // Debug.Log("do");
        state = State.MOVE;
        this.dest = dest;
        this.doSomeThing = doSomeThing;
    }
}