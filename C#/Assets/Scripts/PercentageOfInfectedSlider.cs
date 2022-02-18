using UnityEngine;
using UnityEngine.UI;

public class PercentageOfInfectedSlider : MonoBehaviour
{
    [SerializeField] private GameObject canvas;
    // Start is called before the first frame update
    void Start()
    {
        gameObject.GetComponent<Slider>().onValueChanged.AddListener((v) =>
        {
            canvas.GetComponent<Controller>().percentageOfInfected = v;
        });

    }

}
