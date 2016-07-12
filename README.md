# Remotus-Hardware
El proyecto consiste en el desarrollo de un __sistema embebido__ que permite:
+ Capturar información sobre el ambiente ofrecido por los sensores. (Temperatura, humedad, viento, lluvia, luz, etc…)
+ Realizar acciones desencadenadas por el usuario.
+ Enviar los datos recogidos a un servidor MQTT de forma que este reparta la información por los distintos usuarios

La aplicación está desarrollada con tecnologías que permiten ser usada con cualquier dispositivo, bien sea un ordenador personal, 
tableta o móvil. 
La aplicación consta de una parte que se ejecuta en el sistema empotrado y de otra parte que se ejecuta en un servidor MQTT. 
+ Los sensores de bajo consumo, que se han usado junto con la placa mbed NXP LPC1768 realizarán las mediciones, tomaran las deciciones 
marcadas y enviarán la respectiva información hacia el servidor MQTT. 
+ Servidor MQTT: Es el encargado de gestionar la red y transmitir los mensajes entre usuarios, sistema embedido y sensores, para la 
iteracción entre estos. 

Los diferentes sensores que pueda haber instalados se comunican directamente con la placa mbed NXP LPC1768 y esta a su vez se 
comunica con el servidor MQTT para publicar en los tópicos correspondientes la información proporcionada por lo sensores. Los 
sensores se diferenciarán en el servidor por el tópico en él que se publica su información, para que luego el servidor MQTT se 
encargue de notificar a todos los usuarios subscritos en estos tópicos sobre los datos de los sensores. De este modo podríamos 
tener varios sensores funcionando al mismo tiempo y poder ser recibida su información en varias estaciones distintas. Se usaría 
el mismo proceso para comunicar alguna estación con la placa mbed y actuar con un sensor. 
Para la comunicación entre la placa mbed y el servidor es necesario que haya un punto de acceso que permita la conexión al 
servidor remoto MQTT o servidor en red local.


## Microcontrolador

El microcontrolador usado es: __mbed NXP LPC1768__.

La microcontrolador mbed consta de un procesador ARM Cortex-M3 (96Mhz) de 32 bits con un apreciable número de periféricos y una interfaz de programación y comunicación USB. Viene en un factor de forma DIP de 40 pines y un tamaño de 54x26mm, la cual podemos conectar directamente al prontboard y poder experimentar con mayor facilidad. Soporta muchas interfaces que incluyen USB, SPI, I2C CAN, ethernet, serial y contiene una memoria FLASH de 512KB y memoria SRAM de 64KB

## Recursos usados

+ Sensor de luz (LDR)
+ Sensor de humedad/temperatura
+ Sensor de humedad de tierra
+ Sensor de viento (velocidad/dirección)
+ Sensor de lluvia
+ Sensor de distancia (Sonar)

## Descripción de las tareas 

El cometido principal de la aplicación consiste en la lectura de datos de los sensores y enviar dichos datos a un servidor 
MQTT remoto, ademas de poder recibir datos de este servidor y procesarlos posteriormente. 

Para este echo, extraemos la información de un fichero JSON para poder leer, configurar y enviar los datos de los sensores al 
servidor MQTT. Este fichero se encuentra almacenado en la memoria FLASH del microcontrolador, desde la cual obtenemos los datos 
de conexión al servidor MQTT y la configuración de los distintos sensores. 

De este modo ofrecemos flexibilidad sin tener que recompilar el fichero binario con una nueva configuración de pines, dandole así al usuario final una simplificada opción de estructurar sus sensores de forma personal y sin demasiada complejidad.

### Read and parse file 

1. Lo primero es leer el fichero JSON de la memoria del procesador el cual es almacenado en una variable. Esto se realiza llamando a la function ReadFile().
2. Despues realizamos el parsing de la variable quedando almacenado en un array ademas de contar cuantos sensores se han configurado.
3. Lo siguiente es crear los vectores de tamaño al número de sensores leídos anteriormente.
  
  a. objectsSensors: almacena objetos tipo generico (object) 
  
  b. topicsMQTT: almacena tópico en el cual publica 
  
  c. typesSensors: almacena tipo de sensor

4. Estos tres vectores comparten posiciones en el cual la posición “x” en los tres vectores corresponde al objeto del sensor, al tópico donde se publica los datos de dicho sensor y el tipo de sensor respectivamente (este ultimo para saber que tipo de sensor es). 

5. Por último se recorre el array creado anteriormente para inicializar los tres vectores con sus respectivas informaciones de configuración además de obetener dirección y puerto del servidor MQTT. 

### Conectar servidor MQTT

1. Una vez realizada la tarea anterior, realizamos la configuración Ethernet de nuestro microcontrolador mediante MQTTEthernet().
2. Despues conectamos el microcontrolador a el servidor MQTT, con la información recogida en la tarea anterior. Si la conexión con el servidor fallase este espera 1 segundo y vuelve a intentar conectarse.
3. Seguidamente se crea una variable mensaje utilizada para publicar la información en el servidor MQTT además de crear un timer de 3 segundos para actualizar datos de los sensores.

## Loop

Las siguientes acciones se realizan de forma permanente.

1. El timeout creado en la tarea anterior llama a una función la cual coloca una variable semáforo global a “1” cada tres segundos.
2. Luego en línea de ejecución del programa comprueba la variable semáforo la cual tiene dos opciones
  
  a. Igual a “1” : realiza una llamada a la función updateSensorData() que actualiza los datos de los sensores enviandolos al servidor MQTT además de volver a colocar la variable a “0”
  
  b. Igual a “0” : sigue con la línea de ejecución del programa.
  
3. Posteriormente se ejecuta la función yield(), que tiene dos objetivos:

  a. Mantener activa la conexión con el servidor
  
  b. Comprobar mensajes recibidos sobre los tópicos subscritos en el servidor.
  
4. En el caso de recibir mensajes desde el servidor, ejecuta la función messageArrived() que actualiza los datos de respectivos sensores para volver a enviar al servidor MQTT.
