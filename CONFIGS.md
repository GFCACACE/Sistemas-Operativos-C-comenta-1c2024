# CONFIGURACIONES PARA LAS PRUEBAS

Cada módulo contiene una carpeta ./configs con la configuración específica para cada prueba.


# PRUEBA_PLANI #

    **Memoria**:            ./exec PLANI
    **Cpu**:                ./exec PLANI
    **Kernel**: 
        Primera vez:        ./exec PLANI
        Segunda vez:        ./exec PLANI_RR
        Tercera vez:        ./exec PLANI_VRR
    **Entrada_Salida**:     ./exec SLP1 


# PRUEBA_DEADLOCK #

    **Memoria**:               ./exec DEADLOCK
    **Cpu**:                   ./exec DEADLOCK
    **Kernel**:                ./exec DEADLOCK      
    **Entrada_Salida**:        ./exec ESPERA 

# PRUEBA_MEMORIA_TLB #

    **Memoria**:               ./exec TLB
    **Cpu**:                 
        Primera vez:           ./exec TLB
        Segunda vez:           ./exec TLB_LRU
    **Kernel**:                ./exec TLB      
    **Entrada_Salida**:        ./exec IO_GEN_SLEEP 


# PRUEBA_IO #

    **Memoria**:               ./exec IO
    **Cpu**:                   ./exec IO
    **Kernel**:                ./exec IO      
    **Entrada_Salida**:        
                                ./exec GENERICA
                                ./exec TECLADO
                                ./exec MONITOR

# PRUEBA_FS #

    **Memoria**:               ./exec FS
    **Cpu**:                   ./exec FS
    **Kernel**:                ./exec FS      
    **Entrada_Salida**:        
                                ./exec FS
                                ./exec TECLADO
                                ./exec MONITOR


# PRUEBA_Salvation's Edge #

    **Memoria**:               ./exec SE
    **Cpu**:                   ./exec SE
    **Kernel**:                ./exec SE      
    **Entrada_Salida**:        
                                ./exec GENERICA
                                ./exec TECLADO TECLADO_SE
                                ./exec MONITOR
                                ./exec ESPERA
                                ./exec SLP1


Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante se guardará en la carpeta `bin` del módulo.

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Checkpoint

Para cada checkpoint de control obligatorio, se debe crear un tag en el
repositorio con el siguiente formato:

```
checkpoint-{número}
```

Donde `{número}` es el número del checkpoint.

Para crear un tag y subirlo al repositorio, podemos utilizar los siguientes
comandos:

```bash
git tag -a checkpoint-{número} -m "Checkpoint {número}"
git push origin checkpoint-{número}
```

Asegúrense de que el código compila y cumple con los requisitos del checkpoint
antes de subir el tag.

## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=entradasalida "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

Ante cualquier duda, podés consultar la documentación en el repositorio de
[so-deploy], o utilizar el comando `./deploy.sh -h`.

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy
