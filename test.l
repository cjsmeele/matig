#!/usr/bin/env matig

;; This is a comment.
(print "Hello, world!")
(print (+ 1 2 3 (print 4)))

(print '(hoi één twee . drie))

(let ((foo 1))
  (print "outer foo:")
  (print foo)
  (let ((foo (+ foo 1)))
    (print "inner foo:")
    (print foo))
  (print "outer foo again:")
  (print foo))

;; vim: ft=lisp
