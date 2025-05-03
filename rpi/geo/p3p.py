import cv2
import numpy as np
from scipy.spatial.transform import Rotation as R

def rotation_matrix_to_euler_and_quaternion(rot):
    quat = rot.as_quat()  # [x, y, z, w]

    return quat

def rotation_matrix_to_heading_tilt_twist(R_mat: np.ndarray) -> dict:
    """
    Convert a rotation matrix into custom Heading-Tilt-Twist angles (in degrees).

    Returns:
        dict: {
            'heading': float,
            'tilt': float,
            'twist': float
        }
    """
    align = R.from_euler('y', -90, degrees=True).as_matrix()
    R_fixed = R_mat.T @ align

    # Now extract intrinsic ZYX (heading, tilt, twist)
    rot = R.from_matrix(R_fixed)

    # Important: we want intrinsic rotations (rotate axes after each step)
    # The sequence is intrinsic ZYX, which is equivalent to extrinsic XYZ
    heading, tilt, twist = rot.as_euler('ZYX', degrees=True)

    return {
        'heading': heading,
        'tilt': tilt,
        'twist': twist,
    }

def build_camera_matrix(fov_deg: float, width: int, height: int) -> np.ndarray:
    """
    Construct the camera intrinsic matrix from horizontal field of view and image size.

    Args:
        fov_deg (float): Horizontal field of view in degrees.
        width (int): Image width in pixels.
        height (int): Image height in pixels.

    Returns:
        np.ndarray: 3x3 camera intrinsic matrix.
    """
    # Convert FOV to radians
    fov_rad = np.deg2rad(fov_deg)
    # Focal length assuming square pixels
    fx = width / (2.0 * np.tan(fov_rad / 2.0))
    fy = fx
    # Principal point at image center
    cx = width / 2.0
    cy = height / 2.0

    return np.array([
        [fx, 0.0, cx],
        [0.0, fy, cy],
        [0.0, 0.0, 1.0]
    ], dtype=np.float64)


def solve_pnp(image_points: np.ndarray,
              object_points: np.ndarray,
              fov_deg: float,
              width: int,
              height: int,
              dist_coeffs: np.ndarray = None) -> dict:
    """
    Solve the PnP problem given N >= 4 3D-2D correspondences.

    Args:
        image_points (np.ndarray): Nx2 array of 2D image points.
        object_points (np.ndarray): Nx3 array of corresponding 3D points.
        fov_deg (float): Horizontal field of view in degrees.
        width (int): Image width in pixels.
        height (int): Image height in pixels.
        dist_coeffs (np.ndarray, optional): Distortion coefficients. Defaults to zeros.

    Returns:
        dict: {
            'rvec': np.ndarray,
            'tvec': np.ndarray,
            'R': np.ndarray,
            'reprojection_error': float
        }
    """
    camera_matrix = build_camera_matrix(fov_deg, width, height)

    if dist_coeffs is None:
        dist_coeffs = np.zeros((4, 1), dtype=np.float64)

    image_points = image_points.reshape(-1, 2).astype(np.float64)
    object_points = object_points.reshape(-1, 3).astype(np.float64)

    assert image_points.shape[0] >= 4 and object_points.shape[0] == image_points.shape[0], "Need at least 4 point correspondences"

    success, rvec, tvec = cv2.solvePnP(object_points,
                                       image_points,
                                       camera_matrix,
                                       dist_coeffs,
                                       flags=cv2.SOLVEPNP_ITERATIVE)

    if not success:
        raise RuntimeError("solvePnP failed")

    projected, _ = cv2.projectPoints(object_points, rvec, tvec,
                                     camera_matrix, dist_coeffs)
    projected = projected.reshape(-1, 2)

    print("Projected points:", projected)

    err = np.sqrt(np.mean(np.sum((projected - image_points)**2, axis=1)))

    R_mat, _ = cv2.Rodrigues(rvec)

    return {
        'rvec': rvec,
        'tvec': tvec,
        'R': R_mat,
        'reprojection_error': err
    }


if __name__ == '__main__':

    object_pts = np.array([
        [0, 300, 0],
        [100, 300, 0],
        [62.5, 197.5, 0],
        [37.5, 197.5, 0],
        [0, 256.845299462, 0],
        [43.1547005384, 300, 0],
    ], dtype=np.float64)

    image_pts = np.array([
        [1946, 969],
        [4080, 1008],
        [3338 , 1756],
        [2058, 1678],
        [1590, 1124],
        [2787, 982],
    ], dtype=np.float64)

    width = 4608
    height = 2592
    fov_deg = 67.87

    # No lens distortion
    dist = np.zeros((4, 1), dtype=np.float64)

    result = solve_pnp(image_pts, object_pts, fov_deg, width, height, dist)

    R_wc = result['R'].T  # Camera-to-world rotation
    t_wc = -R_wc @ result['tvec']  # Camera position in world coordinates

    # Adjust for Blender (camera looks along -Z, up is Y+ or Z+ depending on setup)
    blender_R = R.from_matrix(R_wc)  # this is the OpenCV-to-Blender "raw"

    # Option 1: Use camera pointing in -Z with Y up (Blender default)
    blender_rot = blender_R * R.from_euler('xyz', [180, 0, 0], degrees=True)
    print("Position:", t_wc)
    print("RPY: ", rotation_matrix_to_heading_tilt_twist(result['R']))
    print("quat: ", rotation_matrix_to_euler_and_quaternion(blender_rot))
    print(result)
