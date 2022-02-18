using UnityEngine;
public class Chair : MonoBehaviour {
    private bool booked;

    private void Start()
    {
        booked = false;
    }

    public bool IsBooked(){
        return booked;
    }

    public void setBooked(bool isBooked){
        booked = isBooked;
    }
}
