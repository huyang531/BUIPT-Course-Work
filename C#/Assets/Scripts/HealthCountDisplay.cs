using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class HealthCountDisplay : MonoBehaviour
{
    [SerializeField] private GameObject canvas;
    [SerializeField] private TextMeshProUGUI healthValueText;
    [SerializeField] private TextMeshProUGUI infectedValueText;
    [SerializeField] private Slider slider;
    [SerializeField] private Toggle wearMasksToggle;

    // Start is called before the first frame update
    void Start()
    {
        UpdateText();
        InvokeRepeating(nameof(UpdateText), 0, 1.0f);
    }

    private void OnEnable()
    {
        Time.timeScale = 1;
        slider.value = 1;
        slider.transform.Find("ValueText").GetComponent<TextMeshProUGUI>().text = "1.00";
        slider.gameObject.SetActive(true);
        wearMasksToggle.isOn = canvas.GetComponent<Controller>().wearMasks;
        canvas.GetComponent<Controller>().aerialView();
    }

    private void OnDisable()
    {
        Time.timeScale = 0;
    }

    private void UpdateText()
    {
        canvas.GetComponent<Controller>().GetPop();
        int healthCount = canvas.GetComponent<Controller>().healthCount;
        healthValueText.text = healthCount.ToString();
        infectedValueText.text = (canvas.GetComponent<Controller>().numOfCreatedStudents - healthCount).ToString();
    }
    
    // 俯视角
    public void AerialView()
    {
        canvas.GetComponent<Controller>().aerialView();
    }
    
    // 跟随学生
    public void FollowStudent()
    {
        canvas.GetComponent<Controller>().followStudent();
    }
}
