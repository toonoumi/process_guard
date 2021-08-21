# Process Guard

This is a small daemon tool to keep a process running. If the guarded process is killed, the guard will fork and start a new one. The pguard has two proess watching each other, it won't be killed unless 2 of them are killed together. 

## Usage

./pguard \[-f\] \<COMMAND\> \[\<ARGV\> \]

## Notice

pguad does not work well with commands that takes stdin as its input, since killing process breaks the pipe.
