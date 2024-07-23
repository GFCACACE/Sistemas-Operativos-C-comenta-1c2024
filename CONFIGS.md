# Configuraciones para las Pruebas

Cada módulo contiene una carpeta `./configs` con la configuración específica para cada prueba.
Para poder modificar un atributo de las configs se puede ejecutar desde la carpeta del tp:
 `./config_update.sh IP_MEMORIA nueva_ip`

---

## Prueba: PLANI

### Descripción
Configuraciones para la prueba de planificación.

### Configuración

| **Componente**   | **Ejecución**               |
|------------------|-----------------------------|
| **Memoria**      | `./exec PLANI`              |
| **Cpu**          | `./exec PLANI`              |
| **Kernel**       |                             |
| 1. Primera vez   | `./exec PLANI`              |
| 2. Segunda vez   | `./exec PLANI_RR`           |
| 3. Tercera vez   | `./exec PLANI_VRR`          |
| **Entrada/Salida** | `./exec SLP1`             |

---

## Prueba: DEADLOCK

### Descripción
Configuraciones para la prueba de deadlock.

### Configuración

| **Componente**   | **Ejecución**               |
|------------------|-----------------------------|
| **Memoria**      | `./exec DEADLOCK`           |
| **Cpu**          | `./exec DEADLOCK`           |
| **Kernel**       | `./exec DEADLOCK`           |
| **Entrada/Salida** | `./exec ESPERA`           |

---

## Prueba: MEMORIA_TLB

### Descripción
Configuraciones para la prueba de memoria y TLB.

### Configuración

| **Componente**   | **Ejecución**               |
|------------------|-----------------------------|
| **Memoria**      | `./exec TLB`                |
| **Cpu**          |                             |
| 1. Primera vez   | `./exec TLB`                |
| 2. Segunda vez   | `./exec TLB_LRU`            |
| **Kernel**       | `./exec TLB`                |
| **Entrada/Salida** | `./exec IO_GEN_SLEEP`     |

---

## Prueba: IO

### Descripción
Configuraciones para la prueba de entrada/salida.

### Configuración

| **Componente**   | **Ejecución**               |
|------------------|-----------------------------|
| **Memoria**      | `./exec IO`                 |
| **Cpu**          | `./exec IO`                 |
| **Kernel**       | `./exec IO`                 |
| **Entrada/Salida** |                           |
| -                | `./exec GENERICA`           |
| -                | `./exec TECLADO`            |
| -                | `./exec MONITOR`            |

---

## Prueba: FS

### Descripción
Configuraciones para la prueba del sistema de archivos.

### Configuración

| **Componente**   | **Ejecución**               |
|------------------|-----------------------------|
| **Memoria**      | `./exec FS`                 |
| **Cpu**          | `./exec FS`                 |
| **Kernel**       | `./exec FS`                 |
| **Entrada/Salida** |                           |
| -                | `./exec FS`                 |
| -                | `./exec TECLADO`            |
| -                | `./exec MONITOR`            |

---

## Prueba: Salvation's Edge

### Descripción
Configuraciones para la prueba de Salvation's Edge.

### Configuración

| **Componente**   | **Ejecución**               |
|------------------|-----------------------------|
| **Memoria**      | `./exec SE`                 |
| **Cpu**          | `./exec SE`                 |
| **Kernel**       | `./exec SE`                 |
| **Entrada/Salida** |                           |
| -                | `./exec GENERICA`           |
| -                | `./exec TECLADO TECLADO_SE` |
| -                | `./exec MONITOR`            |
| -                | `./exec ESPERA`             |
| -                | `./exec SLP1`               |
