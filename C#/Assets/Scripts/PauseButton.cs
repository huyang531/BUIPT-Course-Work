using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.Serialization;
using UnityEngine.UI;

public class PauseButton : MonoBehaviour
{
    [SerializeField] private Slider speedSlider;
    private float oldTimeScale;
    public bool isRunning = true;

    private void OnEnable()
    {
        gameObject.GetComponent<Button>().transform.GetChild(0).GetComponent<TextMeshProUGUI>().text = "暂停";
        isRunning = true;
    }

    public void OnClick()
    {
        if (isRunning)
        {
            isRunning = false;
            PauseGame();
        }
        else
        {
            isRunning = true;
            ResumeGame();
        }
    }
    
    private void PauseGame()
    {
        speedSlider.gameObject.SetActive(false);
        gameObject.GetComponent<Button>().transform.GetChild(0).GetComponent<TextMeshProUGUI>().text = "继续";
        oldTimeScale = Time.timeScale;
        Time.timeScale = 0;
    }

    private void ResumeGame()
    {
        speedSlider.gameObject.SetActive(true);
        gameObject.GetComponent<Button>().transform.GetChild(0).GetComponent<TextMeshProUGUI>().text = "暂停";
        Time.timeScale = oldTimeScale;
    }
}
