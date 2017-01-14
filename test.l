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

(let ((Σ +))
  (print (Σ 1 2 3 4)))

(set 'bark-if-zero
     (lambda (n)
       (when (zero? n)
         (print "Woof, woof!"))))

(bark-if-zero 5)
(bark-if-zero 0)
(bark-if-zero 4)
(bark-if-zero 3)
(bark-if-zero 0)

(set 'find-zeroes
     (lambda (n)
       (if (zero? n)
         (print "Zero GET.")
         (print "No zeroes here."))))

(find-zeroes 4)
(find-zeroes 3)
(find-zeroes 0)
(find-zeroes 3)

;; vim: ft=lisp
