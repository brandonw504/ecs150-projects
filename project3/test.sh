#!/usr/bin/bash
cmd1='curl -v http://localhost:8080/bootstrap.html'
cmd2='curl -v -X POST http://localhost:8080/hello_world.html'
$cmd1
$cmd2