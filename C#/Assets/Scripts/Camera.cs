using UnityEngine;
public class Camera : MonoBehaviour {
    
    // public Transform mFollower;
    //
    // //相机的高度
    // public float offsetY;
    //
    // //摄像机移动处理一般放在LateUpdate
    // void LateUpdate ()
    // {
    //     if (mFollower != null) {
    //         Vector3 targetPoint = mFollower.position;
    //         targetPoint.y += offsetY;
    //         targetPoint.z -= 7f;
    //         transform.position = Vector3.Lerp (transform.position, targetPoint, Time.deltaTime * 5f);
    //     }
    // }

    public void watchStudent(GameObject student){
        transform.SetParent(student.transform);
        transform.localPosition = new Vector3(0, 2.5f, -3);
        transform.localEulerAngles = new Vector3(14, 0, 0);

    }

    public void aerialView(){
        var mainSense = transform.parent;
        if (mainSense != null)
        {
            mainSense = mainSense.parent;
        }
        transform.SetParent(mainSense);
        
        transform.SetPositionAndRotation(new Vector3(-1, 50, -12), new Quaternion());
        transform.Rotate(90, 90, 0);//Destroy(other.gameObject);
    }
}
