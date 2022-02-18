using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;

public class Move : MonoBehaviour
{
    public GameObject qianmen;

    public float arrive_dis;

    private NavMeshAgent navMeshAgent;
    public Queue destQueue;
    private MovePlan plan;
    private enum State {ToStartNode,ToWindowNode, EATING};
    private State state;

    private Animator animator;
    // Start is called before the first frame update
    void Start()
    {
        navMeshAgent = gameObject.GetComponent<NavMeshAgent>();
        animator = gameObject.GetComponent<Animator>();
        destQueue = new Queue();
        state = State.ToStartNode;
        //navMeshAgent.SetDestination(new Vector3(9, 0.5f, -14));
        plan = new MovePlan(qianmen.transform.position);
        destQueue.Enqueue(plan);
        doPlan();
    }

    // Update is called once per frame
    void Update()
    {
        doPlan();
        // 检测结束
        bool flag = false;
        if (plan.state ==  MovePlan.State.STAY) 
        {
            plan.time -= Time.deltaTime;
            if (plan.time <= 0)
            {
                destQueue.Dequeue();
                flag = true;
            }
        } 
        else if (plan.state == MovePlan.State.MOVE)
        {
            if ((transform.position - plan.dest).magnitude < arrive_dis)
            {
                destQueue.Dequeue();
                flag = true;
            }
        }
        if (flag)//发生的时机是当前plan结束，准备更新下一个plan
        {
            // Debug.Log(plan.state + "," + plan.dest + "," + plan.time);
            //如果plan的doSomeThing非空，则执行它
            if (plan.doSomeThing != null)
            {
                plan.doSomeThing();
            }

            if (destQueue.Count == 0)
            {
                // 没方案去干嘛
                plan = new MovePlan(0.1f); // 原地等一下
                //Debug.Log("没方案，原地等");
                destQueue.Enqueue(plan);
            }
            plan = (MovePlan)destQueue.Peek();
            // Debug.Log(plan.state + "," + plan.dest + "," + plan.time);
        }
    }
    private void doPlan()
    {
         
        //throw new NotImplementedException();
        if (plan.state == MovePlan.State.STAY)
        {
            animator.SetInteger("state", 1);
            navMeshAgent.ResetPath(); // 停止运动
        }
        else if (plan.state == MovePlan.State.MOVE)
        {
            animator.SetInteger("state", 0);
            navMeshAgent.SetDestination(plan.dest);
        }
    }
    private void OnTriggerEnter(Collider other)
    {
        if (other.tag == "startNode" && state == State.ToStartNode)
        {
            destQueue.Enqueue(other.gameObject.GetComponent<StartNode>().GetMovePlan());
            state = State.ToWindowNode;      
        }
        else if (other.tag == "winNode" && state == State.ToWindowNode) 
        {
            List<MovePlan> plans = other.gameObject.GetComponent<WinNode>().GetMovePlan();
            foreach (var movePlan in plans)
            {
                destQueue.Enqueue(movePlan);
            }
            state = State.EATING;
        }
    }

    public void AddPlan(MovePlan newPlan)
    {
        destQueue.Enqueue(newPlan); 
    }
}
