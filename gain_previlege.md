**Linux内核提权漏洞**
=
上一篇中介绍了linux内核的一个整数溢出漏洞，本文介绍其他两个Linux内核的漏洞，这两个漏洞被用于提权，笔者亲自测试了exploit代码，工作正常。下面将针对漏洞的信息和测试exploit的过程进行简要的介绍。

**一.Linux Kernel vmsplice**
--
存在此漏洞的内核版本是2.6.17- 2.6.22.17，2.6.23-2.6.23.15 和 2.6.24-2.6.24.1。  
漏洞的机理其实很简单，在fs/splice.c的do_vmsplice()中，连续定义了两个数组：  
	`struct page *pages[PIPE_BUFFERS];`    
	`struct partial_page partial[PIPE_BUFFERS];`  
大小都是PIPE_BUFFERS，这个值确定为16。在后面的引用过程中，没有对两个数组访问的下标进行严格的检查。而糟糕的是，用户可以编写程序控制对这两个数组的修改，一旦对数组的修改超出了他本身的大小，就会覆盖其他数据。另外，由于这两个数组是连续定义的，那么如果对后定义的数组partial进行超出容量的修改，也就是访问越界，就会覆写第一个数组pages，事实上exploit就是利用了这一点，然后结合pages的特征，采用mmap()调用等手段，欺骗内核执行修改当前用户uid和gid的code，从而实现提权。关于这个漏洞的详细介绍可以参考http://www.chinaunix.net/old_jh/29/1083369.html。  
为了亲自测试该漏洞的存在，测试exploit能否工作，笔者查看了各个linux发行版本的内核信息，最后决定在centos5上进行测试。虽然可以直接编译相应的内核，安装在某发行版上，但是为了避免期间可能出现的各种问题，没有采取这种办法。值得一提的是，最开始下载了一个centos5.4的镜像，启动之后，使用uname -r查看内核版本是2.6.18-164.elf5，符合上面列举的范围，但是exploit却不能正常工作，运行exploit之后在最后一行提示vmsplice:bad address，而不是理想中的root，看来是开发者给该版本的的内核加了补丁。尝试centos5.0，exploit工作正常。下面把测试exploit的过程记录如下：

**1.qemu安装centos5.0**  
	首先下载centos5.0的ISO文件，使用qemu安装centos5.0，在qemu虚拟机中安装gcc，拷贝exploit代码，编译。这个过程中，除了安装gcc，其他的和上一篇安装redhat9的过程一样，就不再赘述(请参考intof_ip_options_get.md)。Exploit代码可以在当前code目录下找到。  
	这里把安装gcc的过程描述如下:  
1) 安装好系统之后，重新启动。挂载iso文件(我们要用里面的安装包)  
$qemu-system-i386 -m 1024 -hda centos.qcow2 -cdrom xxx.iso -boot c  
2) 在虚拟机中进入挂载的iso中的CentOS目录下，依次安装如下rpm包：  
  rpm -ivh cpp-4.1.2-42.el5.i386.rpm  
  rpm -ihv kernel-headers-2.6.18-92.el5.i386.rpm  
  rpm -ivh glibc-headers-2.5-24.i386.rpm  
  rpm -ivh glibc-devel-2.5-24.i386.rpm  
  rpm -ivh libgomp-4.1.2-42.el5.i386.rpm  
  rpm -ivh gcc-4.1.2-42.el5.i386.rpm  
3) 运行gcc编译exploit吧	

**2.测试exploit的代码**  
	这里默认你已经准确执行完第一步了，也就是在qemu虚拟机中已经有可以执行的exploit程序。启动虚拟机，运行exploit，效果如下：  
![](./images/gp-1.bmp)  
哈哈，可见我们已经进入root权限啦。  

**二.Linux kernel sock_diag_handlers(CVE-2013-1763)**
--
存在此漏洞的版本是linux kernel 3.3 - 3.8。这是一个最近新发现的漏洞，已经被CVE收录，编号2013-1763(http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2013-1763)。  
漏洞的机理和前一个非常类似，net/core/sock_diag.c中的 __sock_diag_rcv_msg函数允许未授权用户发送netlink消息，越界访问sock_diag_handlers[]数组，导致权限提升。  
网上已经给出了exploit代码http://www.exploit-db.com/exploits/24746/，测试环境是Ubuntu12.10 64-bits。下面把测试过程中的细节描述如下：  
**1. qemu安装ubuntu12.10 64-bits**  
这里唯一需要注意的就是要使用qemu-system-x86_64去安装和启动，而不是qemu-system-i386.  
直接sudo apt-get install gcc就可以安装gcc。  
**2. 运行exploit**  
	启动qemu虚拟机，编译exploit，运行效果如下：  
![](./images/gp-2.png)  
这和前面一个不太一样，只有一个**#**接受命令，很山寨，我们可以测试到底是不是root权限。运行apt-get install xxx,显示如下：  
![](./images/gp-3.png)  
可见，有权限执行，只不过xxx包找不到而已。再切回普通用户执行该命令，显示如下:  
![](./images/gp-4.png)  
可见，普通用户是没有权限执行的，所以我们确实是通过exploit进入了root权限。如果你还持怀疑态度，那么你可以事先以root身份建立一个文件，看看在exploit之后能否编辑保存该文件，笔者亲自测试过，是可以编辑保存的。  
如果读者在重现时遇到问题，可以email联系(whq).