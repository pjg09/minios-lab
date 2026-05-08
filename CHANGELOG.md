# Changelog

# [1.4.0](https://github.com/pjg09/minios-lab/compare/v1.3.0...v1.4.0) (2026-05-08)


### Features

* implement scheduler_sigchld (SIGCHLD handler, zombie reaping, next process dispatch) ([f2f57c9](https://github.com/pjg09/minios-lab/commit/f2f57c978659ff0adc31d1db0d3a8516ecea37c1))

# [1.3.0](https://github.com/pjg09/minios-lab/compare/v1.2.0...v1.3.0) (2026-05-08)


### Features

* implement scheduler_tick (SIGALRM handler, round-robin context switch) ([57f3c2e](https://github.com/pjg09/minios-lab/commit/57f3c2e82f1ea3d25c8e115efd2d0ac83aaa5397))

# [1.2.0](https://github.com/pjg09/minios-lab/compare/v1.1.0...v1.2.0) (2026-05-08)


### Features

* implement scheduler_start (dequeue first process, SIGCONT, timer init) ([6a36685](https://github.com/pjg09/minios-lab/commit/6a366850c3ed49e2512f6c4564c5405bac34b45b))

# [1.1.0](https://github.com/pjg09/minios-lab/compare/v1.0.0...v1.1.0) (2026-05-08)


### Features

* implement scheduler_create_process (fork + exec + PCB init + SIGSTOP) and build check with -Werror and unit tests for ready_queue and pcb ([fab8142](https://github.com/pjg09/minios-lab/commit/fab81422334968e3b7d2eabf511c713624e92c1e))

# 1.0.0 (2026-05-08)


### Features

* laboratorio inicial de scheduler y shell para estudiantes ([7cc1cbd](https://github.com/pjg09/minios-lab/commit/7cc1cbd417e1fb7d17904e342b0b04461776be18))
