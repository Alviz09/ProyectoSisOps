<h1 align="center">📚 Sistema de Préstamo de Libros</h1>

<p align="center">
  <em>Proyecto académico del curso <strong>Sistemas Operativos (SIU4085)</strong> - Pontificia Universidad Javeriana</em><br>
  <strong>Procesos y Hilos POSIX · Comunicación IPC · Sincronización · C</strong>
</p>

---

## ✒️ Autores

- Carlos Santiago Pinzón  
- Jorge Enrique Olaya  
- Samuel Jerónimo Gantiva  
- Daniel Hoyos  
- Juan Sebastián Álvarez  

**Profesor:** John Corredor  
📅 **Entrega:** 27 de mayo de 2025  

---

## 🎯 Objetivos

✔️ Manejar solicitudes de préstamo, renovación, devolución y salida de usuarios.  
✔️ Utilizar **procesos** y **hilos POSIX** con **sincronización segura**.  
✔️ Comunicar procesos usando **tuberías nombradas (FIFO)**.  
✔️ Actualizar una base de datos en **archivo de texto**.  
✔️ Ejecutar comandos de consola para monitoreo y finalización del sistema.

---

## 🛠️ Arquitectura del Sistema

```mermaid
graph TD
  PS[Proceso Solicitante] -->|Solicitud| PIPE[pipeReceptor]
  PIPE --> RP[Receptor de Peticiones]
  RP --> Hilo1[Hilo Auxiliar 1: Renovación/Devolución]
  RP --> Hilo2[Hilo Auxiliar 2: Comandos Consola]
  RP -->|Respuesta| PS
```

### 🧩 Componentes

- **Solicitantes (PS):** Ejecutados desde consola para enviar solicitudes.
- **Receptor (RP):** Proceso principal que recibe, delega y responde.
- **Hilos:**
  - Hilo Principal → Procesa préstamos.
  - Auxiliar 1 → Renueva y devuelve libros.
  - Auxiliar 2 → Maneja comandos `s` y `x`.

---

## 🧪 Pruebas

| Tipo de Prueba | Descripción |
|----------------|-------------|
| ✅ Unitarias   | Validación de PS, RP y hilos por separado |
| ✅ Integración | Comunicación y coordinación entre procesos e hilos |
| ✅ Carga       | Solicitudes múltiples simultáneas |
| ✅ Estrés      | Peticiones inválidas y casos extremos |

---

## ⚙️ Ejecución

### Iniciar Receptor
```bash
./receptor -p pipeReceptor -f archivoDatos [-v] [-s archivoSalida]
```

### Ejecutar Solicitante
```bash
./solicitante [-i archivo] -p pipeReceptor
```

📌 **Formato de solicitudes:**
```
<OPERACIÓN>,<NOMBRE_LIBRO>,<ISBN>
```

🔁 Operaciones soportadas:
- `P`: Préstamo  
- `R`: Renovación  
- `D`: Devolución  
- `Q`: Salida del solicitante  

---

## 📁 Estructura de Archivos

| Archivo/Tubería        | Descripción |
|------------------------|-------------|
| `pipeReceptor`         | FIFO principal de entrada para RP |
| `canal_respuesta_<PID>`| FIFO temporal por cada solicitante |
| `archivoDatos`         | Base de datos de libros (texto plano) |

---

## 🚧 Desafíos Técnicos

- 🔒 Sincronización con mutex y condiciones (`vacio` / `lleno`).
- 📡 Comunicación robusta mediante tuberías con `select()` y `mkfifo`.
- ✍️ Escritura concurrente segura en archivos de texto.
- ✅ Validación de operaciones y gestión de errores extremos.

---

## 🚀 Mejoras Futuras

- 🖥️ Interfaz gráfica con Qt o web app.
- 🗄️ Uso de base de datos relacional (SQLite).
- 🏢 Soporte multi-biblioteca.
- 🔔 Notificaciones automáticas por correo o SMS.

---

## ✅ Conclusión

Este proyecto permitió aplicar conceptos clave de sistemas operativos como concurrencia, comunicación inter-procesos y sincronización. La solución construida es robusta, modular y extensible, adecuada para simulaciones realistas de bibliotecas digitales.

> _“Un buen sistema operativo no solo ejecuta tareas, las ejecuta a tiempo y de forma segura.”_
