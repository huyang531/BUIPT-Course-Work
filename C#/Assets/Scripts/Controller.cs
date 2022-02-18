using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using System;
using UnityEngine.Serialization;
using Random = UnityEngine.Random;

public class Controller : MonoBehaviour
{

    /// <summary>
    /// 使用窗口的Transform来索引在这个窗口排队的人数。
    /// </summary>
    public Dictionary<Transform, int> waitingInWindow;

    public GameObject myCamera;

    public GameObject caugh_prefab;
    public GameObject leaf_virus_prefab;

    public GameObject human;
    
    // 健康学生数量，可由此获得感染学生数量
    public int healthCount;

    public GameObject qianmen;

    public int numOfStudents;

    private GameObject[] tables;

    private GameObject[] windows;

    //保存了所有人的list
    public List<GameObject> students;

    //食堂出口
    public GameObject exit;

    public GameObject allWindows;

    public GameObject allTables;

    /* huyang */


    // 在食堂里面的学生人数
    [FormerlySerializedAs("numOfStudentsInCanteen")] public int numOfCreatedStudents;

    // 感染者占比
    public double percentageOfInfected;

    // 感染率
    public double infectionRate;

    // 是否佩戴口罩
    public bool wearMasks;

    // 模拟开始时间
    public DateTime simuStartTime;

    
    public void GetPop()
    {
        healthCount = 0;
        numOfCreatedStudents = 0;
        foreach (var stu in students)
        {
                numOfCreatedStudents++;
                if (stu.GetComponent<Spread>().health == Spread.HealthState.Healthy)
                {
                    healthCount++;
                }
        }
    }
    /* huyang end */

    void Start()
    {
        simuStartTime = DateTime.Now;
        students = new List<GameObject>();
        //获得所有打饭窗口
        windows = new GameObject[allWindows.transform.childCount];
        for (int i = 0; i < windows.Length; i++)
        {
            windows[i] = allWindows.transform.GetChild(i).GetChild(0).gameObject;
        }

        //获得所有桌子
        tables = new GameObject[allTables.transform.childCount];
        for (int i = 0; i < tables.Length; i++)
        {
            tables[i] = allTables.transform.GetChild(i).gameObject;
        }

        //初始化map
        waitingInWindow = new Dictionary<Transform, int>();
        foreach (var window in windows)
        {
            waitingInWindow.Add(window.transform, 0);
        }

        enabled = false;
        //MaxPersonNum = 100;
    }

    public void begin()
    {
        foreach (var person in students) {
            GameObject.Destroy(person);
        }
        
        foreach (Chair chair in allTables.GetComponentsInChildren<Chair>()) {
            chair.setBooked(false);
        }

        students = new List<GameObject>();
        Start();
        InvokeRepeating("creatPerson", 0, 0.3f);
        enabled = true;
    }
    
    
    public void followStudent(){
        for (int i = 0; i < numOfStudents; i++) {
            int index = Random.Range(students.Count - 40 >= 0 ? students.Count - 40 : 0, students.Count);
            if (students[index].activeSelf) {
                myCamera.GetComponent<Camera>().SendMessage("watchStudent", students[index]);
                break;
            }
        }
    }

    public void aerialView()
    {
        myCamera.GetComponent<Camera>().aerialView();
    }

    
    
    public void reEating(){
        CancelInvoke("creatPerson");
        foreach (var person in students) {
            person.SetActive(false);
        }
        InvokeRepeating("reborn", 0, 0.5f);
    }

    public void reborn(){
        if (0.2 > Random.value) {
            var person = students.First();
            students.Remove(person);
            Vector3 newPosition = qianmen.transform.position;
            newPosition.z += 10;
            person.transform.position = newPosition;
            person.GetComponent<Move>().AddPlan(new MovePlan(qianmen.transform.position));
            person.SetActive(true);
        }
    }
 
    
    public void creatPerson()
    {
        if (0.3 > Random.value)
        {
            GameObject newhuman = Instantiate(human);
            newhuman.GetComponent<Move>().qianmen = this.qianmen;
            newhuman.GetComponent<Spread>().caugh_prefab = this.caugh_prefab;
            newhuman.GetComponent<Spread>().leaf_virus_prefab = this.leaf_virus_prefab;
            newhuman.GetComponent<Spread>().canvas = this.gameObject;

            // 初始感染者
            if (Random.value < percentageOfInfected)
            {
                newhuman.GetComponent<Spread>().health = Spread.HealthState.Infected;
                newhuman.transform.GetComponentInChildren<SkinnedMeshRenderer>().material =
                    newhuman.GetComponent<Spread>().material;
                newhuman.transform.GetComponentInChildren<MeshRenderer>().material =
                    newhuman.GetComponent<Spread>().material;
            }

            students.Add(newhuman);
        }

        if (students.Count > numOfStudents)
        {
            CancelInvoke("creatPerson");
        }
    }

    /// <summary>
    /// 获得目标窗口的Transform。目标大概率是排队人数较少的窗口，但不是绝对的。
    /// </summary>
    public Transform getSelectedWindow()
    {
        //根据窗口排队人数，对窗口进行排序
        //结果是一个按排队人数升序排序的键值对数组
        var Ordered = waitingInWindow
            .OrderBy(entry => entry.Value)
            .ToList();

        //随机生成一个乱序序列，模拟用餐人员对窗口的偏好
        var random = new System.Random();
        var newList = new List<int>();
        for (int i = 0; i < waitingInWindow.Count; i++)
        {
            newList.Insert(random.Next(newList.Count), i);
        }

        //根据乱序序列更新窗口排序，返回最终结果
        int k = 0;
        return Ordered.ToDictionary(pair => pair, pair => k + 2*newList[k++])
            .OrderBy(orderedPair => orderedPair.Value)
            .First().Key.Key;
    }

    /// <summary>
    /// 通过椅子的chair脚本的isBooked变量来判断椅子是否坐了人
    /// </summary>
    /// <returns>获得目标椅子的Transform</returns>
    public Transform getSelectedChair(){
        var tempList = tables.ToList();
        tables = tempList.OrderBy(value => Random.value > 0.5? 1:-1).ToArray();
        
        //先找没有人坐的桌子，找到之后在这张桌子的随机一个位置坐下
        foreach (var table in tables) {
            bool isBookedTable = false;
            var chairs = table.transform.GetComponentsInChildren<Chair>();
            foreach (var chair in chairs) {
                isBookedTable |= chair.IsBooked();
            }
            if (!isBookedTable)
            {
                var temp = Random.Range(0, chairs.Length);
                chairs[temp].setBooked(true);
                return chairs[temp].gameObject.transform;
            }
        }

        //如果每一个桌子都有人坐了，那就哪里有空位坐哪里
        foreach (var table in tables) {
            foreach (var chair in table.transform.GetComponentsInChildren<Chair>()) {
                if (!chair.IsBooked()) {
                    chair.setBooked(true);
                    return chair.gameObject.transform;
                }
            }
        }
        
        //如果每一个椅子都有人坐了，那就去出口
        return exit.transform;
    }

    /// <summary>
    /// 增加在某窗口排队的人数
    /// </summary>
    /// <param name="index">窗口的name</param>
    public void addWaiting(Transform index)
    {
        waitingInWindow[index]++;
        // Debug.Log("adding");
    }

    /// <summary>
    /// 减少在某窗口排队的人数
    /// </summary>
    /// <param name="index">窗口的name</param>
    public void reduceWaiting(Transform index)
    {
        waitingInWindow[index]--;
    }

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.K))
        {
            followStudent();
        }
        if (Input.GetKeyDown(KeyCode.J))
        {
            aerialView();
        }
    }
}

