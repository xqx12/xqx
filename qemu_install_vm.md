## qemu安装虚拟机
copy from http://blog.chinaunix.net/uid-22249770-id-395026.html

'''
1、创建img文件
qemu-img create -f qcow disk.img 12000M
#create是该命令的操作类型，即创建新的磁盘映像
#qcow是用于qemu的映像格式

2、加载ISO文件
qemu -kernel-kqemu -m 512 -L ./ -hda disk.img -cdrom D:/os1.iso -boot d
#-had指定硬盘映像为 disk.img
#-cdrom选项指定cdrom中的iso的文件
#-boot选项指定引导方式，参数d表示从cd-rom中引导，若是a表示从软盘，c是默认情况，表示从硬盘，n表示从网络
#-L选项指定bios.bin的位置，./表示当前目录
#该命令执行后，出现一个表示仿真机器的新的qemu窗口

3、切换ISO文件
#这种情况针对的是安装的iso文件有多个的情况。
#ctrl+alt+2可以从安装界面切换到qemu命令行，
#输入info block查看cdrom的扇区位置，若为ide1-cd0,执行下面的命令
change ide1-cd0 D:/os2.iso
#可以换到第二张盘
#按键ctrl+alt+1切回安装节界面
#。。。。。。。。以相同方式继续安装直到结束

4、安装好后的启动命令
qemu  -kernel-kqemu -m 512 -L ./ -hda disk.img -net nic -net tap,ifname=vpntap
# -net nic直到网络接口卡，
# -net tap,ifname=vpntap若主机和虚拟机通信通过vpn实现，需要该选项指定虚拟机的虚拟网卡，这种方式一般桥接

5、虚拟机网络配置一般命令
modprobe ne2k-pci && ifconfig eth0 10.20.20.2 && route add default gw 10.20.20.3 eth0 && echo $?
#add default gw的ip一般是主机的ip即可

'''
