using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class RecordContent : MonoBehaviour
{
    public GameObject canvas;
    public GameObject RowPrefab;

    // 当列表被激活后，加载内容，因为列表在RecordCanvas激活后激活，所以此时数据已准备好
    void OnEnable()
    {
        // 删除旧数据
        foreach (Transform child in gameObject.transform)
        {
            GameObject.Destroy(child.gameObject);
            
        }
        
        var infoList = canvas.GetComponent<RecordCanvas>().InfoList;
       for (int i = 0; i < infoList.Count; i++) // DEBUG MARKER
        {
            GameObject obj = Instantiate(RowPrefab);
            obj.transform.Find("SeqText").GetComponent<TextMeshProUGUI>().text = (i + 1).ToString();
            obj.transform.Find("TimeText").GetComponent<TextMeshProUGUI>().text = infoList[i]["time"].ToString();
            obj.transform.Find("FromText").GetComponent<TextMeshProUGUI>().text = infoList[i]["from"].ToString();
            obj.transform.Find("ToText").GetComponent<TextMeshProUGUI>().text = infoList[i]["to"].ToString();

            obj.transform.SetParent(gameObject.transform, false);
        }
    }
}
