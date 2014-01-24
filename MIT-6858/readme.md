MIT-6858  OS Security
=====

###lab1

make a httpd server, and a vm image could be start by kvm. In my x200.

the start cmd:

    kvm -m 512 -net nic -net user,hostfwd=tcp:127.0.0.1:2222-:22,hostfwd=tcp:127.0.0.1:8080-:8080 vm-6858.vmdk  


