using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class MainMenu : MonoBehaviour
{
    [SerializeField] private GameObject canvas;
    public void QuitGame()
    {
        Application.Quit();
    }

    public void PlayGame()
    {
        canvas.GetComponent<Controller>().begin();
    }
}
