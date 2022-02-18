using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WinNode : MonoBehaviour
{
    public GameObject UI;
    private Transform chairTransform;

    public List<MovePlan> GetMovePlan(){
        //UI.GetComponent<Controller>().addWaiting(gameObject.transform);

        List<MovePlan> plans = new List<MovePlan>();
        plans.Add(new MovePlan(Random.Range(10, 20), //�Ŷӵ�ȡ��
            () => UI.GetComponent<Controller>().reduceWaiting(gameObject.transform))); //ȡ�ͺ��Ŷ�����-1

        chairTransform = UI.GetComponent<Controller>().getSelectedChair();
        plans.Add(new MovePlan(chairTransform.position)); //ȥ��λ

        plans.Add(new MovePlan(Random.Range(200, 300), //�Է�
            leavingFromChair)); //���극���뿪��λ��Ȼ�����λ��Ϊ����

        plans.Add(new MovePlan(UI.GetComponent<Controller>().exit.transform.position)); //ȥ����

        return plans;
    }

    public void leavingFromChair()
    {
        chairTransform.gameObject.GetComponent<Chair>().setBooked(false);
    }
}
