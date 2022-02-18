using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Cough :  MonoBehaviour
{
    // Start is called before the first frame update
    public float liveTime = 1; // 咳嗽判定点的存活时间
    public GameObject owner; // 咳嗽的发起者，在发起者创建时设置这个

    DateTime now; // 打算存当前时间，暂时不知道干什么
    void Start()
    {
        now = DateTime.Now;
        StartCoroutine(wait_to_destory());

    }

    // Update is called once per frame
    void Update()
    {
        
    }

    // 等待一段时间之后，销毁自己
    IEnumerator wait_to_destory()
    {
        // Debug.Log("Start");
        yield return new WaitForSeconds(liveTime);
        // Debug.Log("End");
        Destroy(this.gameObject);
    }
}
