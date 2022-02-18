using System;
using UnityEngine;
using UnityEngine.AI;

public class HellNode  : MonoBehaviour {
    public GameObject UI;

    private void OnTriggerEnter(Collider other){
        if (other.tag == "Player")
        {
            Transform CameraTransfom = UI.GetComponent<Controller>().myCamera.transform;
            if (CameraTransfom.IsChildOf(other.transform))
            {
                CameraTransfom.gameObject.GetComponent<Camera>().aerialView();
            }
            
            other.transform.position = new Vector3(-10, -10, -10);
            //UI.GetComponent<Controller>().people.Remove(other.gameObject);
            // other.gameObject.GetComponent<NavMeshAgent>().enabled = false;
            // //GameObject.Destroy(other.gameObject);
            //
            // other.gameObject.GetComponent<Move>().enabled = false;
            // other.gameObject.GetComponent<Animator>().enabled = false;
            // other.gameObject.GetComponent<Spread>().enabled = false;
            other.gameObject.SetActive(false);
        }
    }
}
