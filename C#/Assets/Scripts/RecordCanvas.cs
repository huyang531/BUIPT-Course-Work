using System.Collections.Generic;
using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;


public class RecordCanvas : MonoBehaviour
{
    [SerializeField] private Button button;
    [SerializeField] private TextMeshProUGUI timeText;
    [SerializeField] private TextMeshProUGUI healthCountText;
    [SerializeField] private TextMeshProUGUI totalCountText;
    [SerializeField] private TextMeshProUGUI infectedCountText;
    [SerializeField] private TextMeshProUGUI infectionRateText;
    [SerializeField] private GameObject content;

    
    public List<Dictionary<string, object>> InfoList;
    public HashSet<Dictionary<string, object>> InfoSet;

    // 每次进入查看记录界面，都从保存按钮更新一遍信息
    void OnEnable()
    {
        InfoList = new List<Dictionary<string, object>>();
        InfoSet = new HashSet<Dictionary<string, object>>();

        // 从保存按钮获得信息
        var info = button.GetComponent<SaveButton>();

        DateTime temp = DateTime.MinValue; // 初始日期0001年
        if (DateTime.Compare(info.time, temp) == 0) // 如果按钮还没被点过
        {
            // 弹出提示窗口
            return;
        }
        
        // 初始化数据
        timeText.text = info.time.ToString();
        healthCountText.text = info.healthCount.ToString();
        totalCountText.text = info.numOfStudents.ToString();
        infectedCountText.text = (info.numOfStudents - info.healthCount).ToString();
        infectionRateText.text = info.infectionRate.ToString("0.00");

        // 把传染信息字典化
        foreach (Tuple<int, List<Tuple<DateTime, int>>> i in info.fromInfo)
        {
            foreach (Tuple<DateTime, int> j in i.Item2)
            {
                InfoSet.Add(new Dictionary<string, object>(){
                {"from", j.Item2 }, // int
                {"to", i.Item1 }, // int
                {"time", j.Item1 }, // datetime
            });
            }
        }

        // 给感染信息排序
        foreach(var item in InfoSet)
        {
            InfoList.Add(item);
        }
        InfoList.Sort((x, y) => DateTime.Compare((DateTime)x["time"], (DateTime)y["time"]));
        
        // 激活列表
        content.SetActive(true);
    }

    private void OnDisable()
    {
        // 灭活列表
        content.SetActive(false);
    }
}
