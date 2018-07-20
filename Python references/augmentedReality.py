import cv2
import numpy as np
import glob

# Matrices que devuelve el programa de calibración, estos son los datos de la webcam
mtx = np.array([ [848.93000659,            0, 358.05664273]
               , [           0, 852.12810392, 209.05793718]
               , [           0,            0,            1] ])

dist = np.array([0.27516314
              , -0.79606145
              , -0.01335555
              , 0.00363432
              , 2.19621258])

# Accedemos a la cámara
cap = cv2.VideoCapture(0)

if not (cap.isOpened()):
    cap.open();

# Funcion que dibuja un cubo
def drawCube(img, corners, imgpts):
    imgpts = np.int32(imgpts).reshape(-1,2)

    # dibujar la base
    cv2.drawContours(img, [imgpts[:4]], -1, (0,0,255), 3)

    # dibujar los pilares
    for i,j in zip(range(4),range(4,8)):
        cv2.line(img, tuple(imgpts[i]), tuple(imgpts[j]), (0,0,255), 3)

    # dibujar la tapa
    cv2.drawContours(img, [imgpts[4:]], -1, (0,0,255), 3)

    return img

# Condición de parada de un algoritmo iterativo
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

# Crea una matriz de dos dimensiones con 12*9 filas y 3 columnas (las tres coordendas)
objp = np.zeros((12*9,3), np.float32)

# Inicializa el grid combinado entre 0-11 y 0-8: (0,0,0) (0,1,0) (0,2,0) .. (11,8,0)
objp[:,:2] = np.mgrid[0:12,0:9].T.reshape(-1, 2)

coord = np.float32([[4,3,0], [4,6,0], [7,6,0], [7,3,0],
                    [4,3,-3],[4,6,-3],[7,6,-3],[7,3,-3]])

while (True):
    # Leemos de la cámara
    status, img = cap.read()

    # Si ha leido algo de la cámara
    if (status):
        # A blanco y negro
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        # En corners guardamos las esquinas
        ret, corners = cv2.findChessboardCorners(gray, (12, 9))

        # Si encuentra las esquinas
        if (ret):
            # Afina un poco más
            cv2.cornerSubPix(gray, corners, (5, 5), (-1, -1), criteria)

            # Calcula la rotacion y la translacion del plano
            _, rvecs, tvecs, inliers = cv2.solvePnPRansac(objp, corners, mtx, dist)

            # Proyecta las coordenadas 3D del objeto al plano de la pantalla 
            imgpts, jac = cv2.projectPoints(coord, rvecs, tvecs, mtx, dist)
            # Dibujar el objeto
            img = drawCube(img, corners, imgpts)
   
        cv2.imshow('Augmentating', img)

    # Para detener el bucle
    k = cv2.waitKey(10)
    if k == -1:
        break
    
cap.release()
cv2.destroyAllWindows()
