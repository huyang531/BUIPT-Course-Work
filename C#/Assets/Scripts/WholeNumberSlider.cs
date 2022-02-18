using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class WholeNumberSlider : MonoBehaviour
{
    [SerializeField] private Slider _slider;
    [SerializeField] private TextMeshProUGUI _text;

    void Start()
    {
        _text.text = _slider.value.ToString("0");
        _slider.onValueChanged.AddListener((v) =>
        {
            _text.text = v.ToString("0");
        });
    }
}
