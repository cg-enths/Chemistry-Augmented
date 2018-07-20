#!/usr/bin/env python

import numpy as np
import cv2
import os

USAGE = '''
USAGE: calibrate.py [--save <filename>] [--debug <output path>] [--square_size] [<image mask>]
'''

if __name__ == '__main__':
    import sys, getopt
    from glob import glob

    args, img_mask = getopt.getopt(sys.argv[1:], '', ['save=', 'debug=', 'square_size='])
    args = dict(args)

    try:
        img_mask = img_mask[0]
    except:
        img_mask = 'chessboard/*.jpg'

    img_names = glob(img_mask)
    debug_dir = args.get('--debug')

    # Indicamos el tamaño de los cuadrados
    square_size = float(args.get('--square_size', 1.0))

    # Indicamos cuantas esquinas interiores tiene el tablero
    pattern_size = (8, 6)
    # Crea una matriz de dos dimensiones (12*9 filas) y 3 columnas (tres coordenadas)
    pattern_points = np.zeros( (np.prod(pattern_size), 3), np.float32 )
    # Esto esta explicado en el otro programa.
    pattern_points[:,:2] = np.indices(pattern_size).T.reshape(-1, 2)
    pattern_points *= square_size

    obj_points = []
    img_points = []
    h, w = 0, 0
    # Recorrer las imagenes
    for fn in img_names:
        print ('Processing %s...' % fn)

        # Leemos la imagen
        imgb = cv2.imread(fn, 0)
        # Cambiamos el tamaño porque mis fotos son enormes
        img = cv2.resize(imgb, (640,480))
        # Sacamos la altura y la anchura
        h, w = img.shape[:2]
        # En corners guardamos las esquinas
        found, corners = cv2.findChessboardCorners(img, pattern_size)

        # Si las encuentra...
        if found:
            term = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_COUNT, 30, 0.1)
            # Afina la ubicación de las esquinas
            cv2.cornerSubPix(img, corners, (5, 5), (-1, -1), term)

        # Para debug, pasando...
        if debug_dir:
            vis = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
            cv2.drawChessboardCorners(vis, pattern_size, corners, found)
            path, name, ext = splitfn(fn)
            cv2.imwrite('%s/%s_chess.bmp' % (debug_dir, name), vis)

        if not found:
            print ('Chessboard not found')
            continue

        img_points.append(corners.reshape(-1, 2))
        obj_points.append(pattern_points)

        print ('Ok')

    # Devuelve todos los valores de la calibracion
    rms, camera_matrix, dist_coefs, rvecs, tvecs = cv2.calibrateCamera(obj_points, img_points, (w, h), None, None)

    print ("RMS:", rms)
    print ("Camera matrix:\n", camera_matrix)
    print ("Distortion coefficients: ", dist_coefs.ravel())

    cv2.destroyAllWindows()
