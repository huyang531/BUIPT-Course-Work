using System.Collections.Generic;
using System;
using UnityEngine;

public class SaveButton : MonoBehaviour
{
    [SerializeField] private GameObject canvas;

    // 模拟信息
    // List<Tuple<我, List<Tuple<时间, 别人>>>>
    public List<Tuple<int, List<Tuple<DateTime, int>>>> fromInfo;
    public List<Tuple<int, List<Tuple<DateTime, int>>>> toInfo;
    public int healthCount;
    public int numOfStudents;
    public double infectionRate;
    public DateTime time;

    void Start()
    {
        time = DateTime.MinValue;
        fromInfo = new List<Tuple<int, List<Tuple<DateTime, int>>>>();
        toInfo = new List<Tuple<int, List<Tuple<DateTime, int>>>>();
    }

    // 保存当前模拟信息
    public void OnClick()
    {
        fromInfo.Clear();
        toInfo.Clear();
        // deep copy information from Controller
        var ctrlr = canvas.GetComponent<Controller>();
        var students = ctrlr.students;
        int stuNo;
        foreach (var stu in students)
        {
            stuNo = ctrlr.students.IndexOf(stu);
            fromInfo.Add(new Tuple<int, List<Tuple<DateTime, int>>>(stuNo, stu.GetComponent<Spread>().GetFromInfo()));
            toInfo.Add(new Tuple<int, List<Tuple<DateTime, int>>>(stuNo, stu.GetComponent<Spread>().GetToInfo()));
        }
        ctrlr.GetPop();
        healthCount = ctrlr.healthCount;
        numOfStudents = ctrlr.numOfCreatedStudents;
        infectionRate = ctrlr.infectionRate;
        time = ctrlr.simuStartTime;
    }
}
