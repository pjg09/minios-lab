# miniOS — Simulador de Scheduler Round-Robin

Trabajo final de Sistemas Operativos. Implementación de un scheduler round-robin en user-space que gestiona procesos reales del sistema operativo usando mecanismos reales del kernel: `fork`, `exec`, `SIGSTOP`, `SIGCONT`, `SIGALRM`, `ptrace` y sockets AF_UNIX.

## Arquitectura

```
./minios (C) ──AF_UNIX──► bridge/index.js (Node) ──WebSocket──► dashboard/ (React)
```

El scheduler controla procesos hijo reales. Cada context switch detiene al proceso actual con `SIGSTOP`, captura sus registros con `ptrace`, reanuda al siguiente con `SIGCONT`, y emite un evento JSON al dashboard vía el bridge.

## Lo que se implementó

Las 8 funciones del núcleo del sistema, repartidas en dos archivos:

**`src/scheduler.c`**
| Función | Descripción |
|---|---|
| `scheduler_create_process` | `fork` + `execl` + captura de registros con ptrace + `SIGSTOP` + inicialización del PCB |
| `scheduler_start` | Desencola el primer proceso, le envía `SIGCONT` e instala el timer con `setitimer` |
| `scheduler_tick` | Handler de `SIGALRM` — realiza el context switch round-robin completo |
| `scheduler_sigchld` | Handler de `SIGCHLD` — recolecta zombies con `waitpid(WNOHANG)` y despacha el siguiente proceso |

**`src/shell.c`**
| Función | Descripción |
|---|---|
| `cmd_run` | Valida el path, crea el proceso y arranca el scheduler si es el primero |
| `cmd_ps` | Muestra la process table con PID, estado, CPU acumulado, espera y switches |
| `cmd_kill_proc` | Envía `SIGKILL` y deja que `scheduler_sigchld` limpie el estado |
| `cmd_stats` | Muestra métricas agregadas: CPU total, switches, promedios de CPU y espera |

## Requisitos

- Linux (WSL2 o nativo) con `build-essential`
- Node.js 18+

## Instalación

```bash
git clone <url-del-fork>
cd minios-lab
make
cd bridge && npm install && cd ..
cd dashboard && npm install && cd ..
```

## Ejecución

Abrir 3 terminales:

```bash
# Terminal 1 — bridge
cd bridge && WS_PORT=8081 node index.js

# Terminal 2 — dashboard (abrir http://localhost:5173)
cd dashboard && npm run dev

# Terminal 3 — scheduler
./minios
miniOS> log on
miniOS> run programs/bin/countdown 60
miniOS> run programs/bin/primos
miniOS> run programs/bin/fibonacci 40
miniOS> run programs/bin/busy_loop
miniOS> ps
miniOS> slice 200
miniOS> stats
miniOS> exit
```

## Comandos del shell

| Comando | Descripción |
|---|---|
| `run <binario> [arg]` | Lanza un proceso nuevo |
| `ps` | Muestra la process table y la ready queue |
| `kill <pid>` | Termina un proceso por PID |
| `slice <ms>` | Cambia el time slice en caliente (50–5000 ms) |
| `inspect <pid>` | Muestra los registros del CPU capturados por ptrace |
| `stats` | Métricas agregadas del scheduler |
| `log on\|off` | Activa o desactiva la emisión de eventos al dashboard |
| `runpair <nombre>` | Lanza un par de procesos comunicantes (`ping_pong`, `productor_consumidor`) |
| `exit` | Termina todos los procesos y sale limpiamente |

## Programas de ejemplo

```bash
make programs   # compila todo en programs/bin/
```

| Binario | Tipo | Descripción |
|---|---|---|
| `countdown <n>` | IO-bound | Cuenta regresiva imprimiendo cada segundo |
| `primos` | CPU-bound | Busca primos secuencialmente |
| `fibonacci <n>` | CPU-bound | Fibonacci recursivo |
| `busy_loop` | CPU-bound | Loop de cómputo puro ~20 segundos |
| `loteria` | CPU-bound | Busca 3 dígitos iguales consecutivos al azar |
| `bitacora` | IO-bound | Escribe líneas con timestamp a un archivo |
| `memory_hog` | Memoria | Reserva memoria incrementalmente |
| `ping_pong_server/client` | IPC | Pasan un contador por socket AF_UNIX |
| `productor/consumidor` | IPC | Productor-consumidor por socket AF_UNIX |

## Escenarios predefinidos

```bash
bash scenarios/cpu_contention.sh   # 3 procesos CPU-bound compitiendo
bash scenarios/io_mix.sh           # mezcla CPU-bound e IO-bound
bash scenarios/carga_variable.sh   # procesos con tiempos distintos
```

## Tests

```bash
make test   # unit tests de ready_queue y pcb (56 assertions)
```

El CI corre `make` con `-Werror` y `make test` en cada push a `main`.

## Decisiones de implementación destacadas

- **`wait_time_ms`** se acumula con un array estático `ready_since[MAX_PROCESSES]` en `scheduler.c` sin modificar `pcb.h`. Cada vez que un proceso entra a READY se registra el timestamp; cuando sale a RUNNING se calcula el delta.

- **`cmd_kill_proc` no llama `waitpid`** — delega el reaping completamente a `scheduler_sigchld`. Esto evita la race condition donde `cmd_kill_proc` roba el zombie antes de que el handler pueda resetear `current_running`, lo que dejaba el proceso muerto con estado `RUNNING` en la tabla.

- **La abstracción de plataforma es compile-time** — `platform_uses_ptrace()` determina en tiempo de ejecución si usar ptrace (Linux) o no (macOS/SIP). Todo el código de ptrace está aislado en `platform_linux.c`.

## Estructura del repositorio

```
minios-lab/
├── src/                  # Scheduler en C (única zona modificada)
│   ├── scheduler.c/h     # Core round-robin + handlers de señal
│   ├── shell.c/h         # Shell interactivo miniOS>
│   ├── pcb.c/h           # Process Control Block y process table
│   ├── ready_queue.c/h   # Cola circular FIFO
│   ├── timer.c/h         # setitimer + SIGALRM
│   ├── monitor.c/h       # Emisor de eventos JSON
│   └── platform/         # Abstracción Linux/macOS
├── programs/             # Procesos de ejemplo (C)
├── scenarios/            # Scripts de carga predefinidos
├── bridge/               # Relay Node.js (AF_UNIX → WebSocket)
├── dashboard/            # Visualización React + Vite
└── tests/                # Unit tests (ready_queue, pcb)
```
