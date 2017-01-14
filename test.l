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

(set 'foo
     (lambda
       (x (y 5) &rest z)
       (print "-- in foo --")
       (print x)
       (print y)
       (print z)
       (+ x y)))

(foo)                 ; This is an error.
(foo 1)               ; => 1 5 nil
(foo 1 2)             ; => 1 2 nil
(foo 1 2 3)           ; => 1 2 (3)
(foo 1 2 3 4)         ; => 1 2 (3 4)
(foo 1 2 3 (foo 9) 4) ; => 1 2 (3 14 4)

;; vim: ft=lisp
