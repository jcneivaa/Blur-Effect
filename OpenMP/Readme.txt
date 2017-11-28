Para compilar y ejecutar se debe tener instalado la libreria opencv

compilar con g++ -fopenmp bluromp.c -o blur-effect `pkg-config opencv --cflags --libs` -pthread

Para ejecutar por script se deben otorgar permisos al script con "chmod +x nombre del script",
A su vez es necesario contar con 3 imagenes preferiblemente de resoluciones diferentes (720p, 1080p, 4k)
Las imagenes deben tener el nombre 720.jpg, 1080.jpg, 4k.jpg, en caso contrario modificar el script cambiando este nombre por el nombre de la imagen.

Para ejecutar por consola se deben pasar los siguientes parametros: "nombre imagen.extension" "numero de hilos a lanzar" "tamano del kernel".