(require 'compile)

(setq init-script-path (convert-standard-filename (file-name-directory load-file-name)))
(setq project-root-path init-script-path)
(setq build-output-path (concat project-root-path "builds"))
(setq source-path (concat project-root-path "source"))
(setq debug-executable-path (concat build-output-path "\\" "hello_debug.exe"))
(setq release-executable-path (concat build-output-path "\\" "hello_release.exe"))
(setq build-script-path (concat project-root-path "build.bat"))
(setq build-command (concat build-script-path " " source-path " " build-output-path))
(setq debug-build-command (concat build-command " " "debug"))
(setq release-build-command (concat build-command " " "release"))
(setq execution-root-path (concat project-root-path "builds"))

;; F9 saves everything and recompiles debug build
(global-set-key (kbd "<f1>")
                (lambda ()
                  (interactive)
                  (setq compile-command debug-build-command)
                  (save-some-buffers 1) ; save all buffers with changes
                  (recompile)))

;; F10 saves everything and recompiles debug build
(global-set-key (kbd "<f2>")
                (lambda ()
                  (interactive)
                  (setq compile-command release-build-command)
                  (save-some-buffers 1) ; save all buffers with changes
                  (recompile)))

;; F5 executes the debug build
(global-set-key (kbd "<f5>")
		(lambda ()
		  (interactive)
		  (shell-command (concat "cd " execution-root-path " && "
                                 debug-executable-path "&"))))

;; F6 executes the release build
(global-set-key (kbd "<f6>")
		(lambda ()
		  (interactive)
		  (shell-command (concat "cd " execution-root-path " && "
                                 release-executable-path "&"))))
