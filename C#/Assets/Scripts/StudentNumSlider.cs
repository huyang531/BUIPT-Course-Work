using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.Serialization;
using UnityEngine.UI;

public class StudentNumSlider : MonoBehaviour
{
    [SerializeField] private GameObject canvas;
    void Start()
    {
        canvas.GetComponent<Controller>().numOfStudents = (int)gameObject.GetComponent<Slider>().value;
        gameObject.GetComponent<Slider>().onValueChanged.AddListener((v) =>
        {
            canvas.GetComponent<Controller>().numOfStudents = (int)v;
        });
    }
}
