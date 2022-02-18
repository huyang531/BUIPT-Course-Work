using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LeftVirus : MonoBehaviour
{
    // Start is called before the first frame update
    public float liveTime = 100;
    public GameObject owner;
    DateTime now;
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    IEnumerator wait_to_destory()
    {
        Debug.Log("Start");
        yield return new WaitForSeconds(liveTime);
        Debug.Log("End");
        Destroy(this.gameObject);
    }
}
