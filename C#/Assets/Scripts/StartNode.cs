using System;
using System.Collections;
using System.Collections.Generic;
using System.Numerics;
using UnityEngine;

public class StartNode : MonoBehaviour {
    public GameObject UI;

    public MovePlan GetMovePlan()
    {
        Transform window = UI.GetComponent<Controller>().getSelectedWindow();
        return new MovePlan(window.position,
            () => UI.GetComponent<Controller>().addWaiting(window)); //doSomeThing在这个plan执行结束的时候执行
    }
}
