using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class InfectionRateSlider : MonoBehaviour
{
    [SerializeField] private GameObject canvas;

    // Start is called before the first frame update
    void Start()
    {
        gameObject.GetComponent<Slider>().onValueChanged.AddListener((v) =>
        {
            canvas.GetComponent<Controller>().infectionRate = v;
        });


    }
}
