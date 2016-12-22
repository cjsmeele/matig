#!/usr/bin/env matig

;; This is a comment.
(print "Hello, world!")
(print (+ 1 2 3 (print 4)))

(print '(hoi één twee . drie))

(print "Some Fibonacci numbers:")
(let ((fib-nummers '(1 0)))
  (let ((fib
          (lambda ()
            (set 'fib-nummers
                 (cons (+ (car fib-nummers)
                          (car (cdr fib-nummers)))
                       fib-nummers)))))
    (fib) (fib) (fib) (fib)
    (fib) (fib) (fib) (fib)
    (fib) (fib) (fib) (fib)
    (fib) (fib) (fib) (fib))
  (print fib-nummers))

;; vim: ft=lisp
