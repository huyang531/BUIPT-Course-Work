using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Random = UnityEngine.Random;

/**
 * 传播组件，挂在人上
 * 存储传播信息，创建传播组件
 * 通过碰撞检测修改传播
 */
public class Spread : MonoBehaviour
{
    public GameObject tx;
    public Material material;
    public GameObject caugh_prefab;
    public GameObject leaf_virus_prefab;
    public float radius; // 咳嗽范围
    public float radius_virus; // 残留病毒范围
     
    public enum HealthState { Healthy, Infected};
    public HealthState health;

    public List<Tuple<DateTime, GameObject>> from, to;

    private List<GameObject> virus;
    /* huyang */
    public GameObject canvas; // 获得controller
    // 返回传染信息深拷贝
    public List<Tuple<DateTime, int>> GetFromInfo()
    {
        if (from == null) return new List<Tuple<DateTime, int>>();
        var ctrlr = canvas.GetComponent<Controller>();
        List<Tuple<DateTime, int>> ret = new List<Tuple<DateTime, int>>();
        foreach (var info in from)
        {
            ret.Add(new Tuple<DateTime, int>(info.Item1, ctrlr.students.IndexOf(info.Item2)));
        }
        return ret;
    }

    public List<Tuple<DateTime, int>> GetToInfo()
    {
        if (to == null) return new List<Tuple<DateTime, int>>();
        var ctrlr = canvas.GetComponent<Controller>();
        List<Tuple<DateTime, int>> ret = new List<Tuple<DateTime, int>>();
        foreach (var info in to)
        {
            ret.Add(new Tuple<DateTime, int>(info.Item1, ctrlr.students.IndexOf(info.Item2)));
        }
        return ret;
    }
    /* huyang end */

    // Start is called before the first frame update
    void Start()
    {
        from = new List<Tuple<DateTime, GameObject>>();
        virus = new List<GameObject>();
        if (from == null)
        {
            Debug.Log("ERROR in 56");
        }
        to = new List<Tuple<DateTime, GameObject>>();
        //health = HealthState.Healthy;
        InvokeRepeating(nameof(RandomToCreateVirus), 0, 10.0f);
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.C) && this.health == HealthState.Infected)
        {
            CreateCough();// 暂时用单击创建
        }
        // TODO:概率创建咳嗽
        
    }

    private void OnTriggerEnter(Collider other)
    {
        if ((other.tag == "Cough" || other.tag == "LeftVirus")
            && health == HealthState.Healthy)
        {
            if (Random.value > 
                (canvas.GetComponent<Controller>().wearMasks? 
                    0.1 * canvas.GetComponent<Controller>().infectionRate
                    :canvas.GetComponent<Controller>().infectionRate))
            {
                return;
            }
            health = HealthState.Infected;

            transform.GetComponentInChildren<SkinnedMeshRenderer>().material = material;
            transform.GetComponentInChildren<MeshRenderer>().material = material;
            
            GameObject prefabInstance = Instantiate(tx);
            prefabInstance.transform.position = new Vector3(transform.position.x, transform.position.y+1.5f, transform.position.z);

            if (other.GetComponent<Cough>().owner == this.gameObject)
            {
                Debug.Log("zen me hui1");
            }
            getInfected(other.GetComponent<Cough>().owner, other.tag);
        }
    }

    void CreateCough()
    {
        GameObject it = Instantiate(caugh_prefab);
        //it.transform.SetParent(this.transform);
        it.transform.localPosition = transform.position;
        it.GetComponent<Cough>().owner = this.gameObject;
        it.transform.localScale = new Vector3(radius, 0.5f, radius);
        virus.Add(it);
    }

    void CreatLeafVirus()
    {
        GameObject it = Instantiate(leaf_virus_prefab);
        //it.transform.SetParent(this.transform);
        it.transform.localPosition = transform.position;
        it.GetComponent<LeftVirus>().owner = this.gameObject;
        it.transform.localScale = new Vector3(radius_virus, 0.5f, radius_virus);
        virus.Add(it);
    }

    void getInfected( GameObject infectFrom,string type)
    {
        // Debug.Log(this.name + "被" + infectFrom.name + "感染");
        from ??= new List<Tuple<DateTime, GameObject>>();
        health = HealthState.Infected;
        from.Add(new Tuple<DateTime, GameObject>(DateTime.Now, infectFrom));
        infectFrom.gameObject.GetComponent<Spread>().to.Add(
            new Tuple<DateTime, GameObject>(DateTime.Now, this.gameObject));
        // Debug.Log("this:" + this.from[0]);
        // Debug.Log("infect from" + infectFrom.GetComponent<Spread>().to[0]);
    }

    void RandomToCreateVirus()
    {
        if (health == HealthState.Healthy)
        {
            return;
        }
        double rate = canvas.GetComponent<Controller>().infectionRate;
        
        if (canvas.GetComponent<Controller>().wearMasks)
        {
            rate /= 10;
        } 
        if (Random.value < rate)
        {
            CreateCough();
        }
        // if (Random.value < 0.1 * rate) 
        // {
        //    CreatLeafVirus();
        // }
    }

    private void OnDestroy()
    {
        foreach (var VARIABLE in virus)
        {
            GameObject.Destroy(VARIABLE.gameObject);
        }
    }
}
