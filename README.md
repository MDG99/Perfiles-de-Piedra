# Perfiles de Piedra
## Librerias requeridas
- PySerial (``pip install pyserial`)
- Agregar la librería para arduino *Arduino-PID-Library-master.zip* en el IDE

## Instrucciones de Uso
- Conectar el arduino a la PC.
- Conectar el arduino a la aplicación mediante la interfaz de usuario (debe de mostrar una ventana emergente indicando que se ha realizado con éxito).
- Colocar la velocidad al máximo del PWM (Presionar el botón mostrar para verificar la velocidad actual del PWM)
- Manipular el programa según las necesidades del usuario.

## Posibles mejoras
- Detener el programa cuando se detecte un paro manual del motor (obstrucción).
- Agregar más funcionalidades de control.

## Nota:
- Verificar la verlocidad del PWM (Botón mostrar)
- Se tiene que utilizar una fuente mayor a 5 V con el Monster Shield para que el motor sea capaz de mover el perfil de piedra sin trabarse.
