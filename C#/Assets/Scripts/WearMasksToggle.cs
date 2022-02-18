using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class WearMasksToggle : MonoBehaviour
{
    [SerializeField] private GameObject canvas;

    // Start is called before the first frame update
    void Start()
    {
        gameObject.GetComponent<Toggle>().onValueChanged.AddListener((v) =>
        {
            canvas.GetComponent<Controller>().wearMasks = v;
        });

    }
}
