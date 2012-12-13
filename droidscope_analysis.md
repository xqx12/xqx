# DroidScope软件分析

## 1 DroidScope概述
DroidScope提供了三层API，包括硬件层、操作系统层和Dalvik虚拟机层。基于DroidScope开发了一系列的分析工具，可以收集native和Dalvik虚拟机的指令trace信息，分析API层的行为，并使用污点分析的方法可以分析信息泄漏等问题，包括Java组件和native组件。不同于其他桌面恶意软件分析平台，DroidScope无缝的重建了操作系统层和java层的语义。

1、描述了Linux系统层和dalvik虚拟机层的重建，在dalvik虚拟机层，实现了动态关闭just-in-time编译的功能；
 
2、描述了DroidScope，一个新的基于模拟器的Android恶意软件分析引擎。可以分析Anroid应用程序的java和native组件。并提供了基于事件的三层API接口， 可抽象为硬件层、 系统层、Dalvik层。

3、基于开发了4个分析工具：本地指令跟踪器、dalvik指令跟踪器、API跟踪器、和污点跟踪器。本地指令跟踪器、dalvik指令跟踪器用于记录分析样本程序的指令等细节，API跟踪器提供一个更高的视图，可以查看分析样本程序与系统其他部分的交互；污点检测器通过对本地指令的污点分析，但可以跟踪来自java objects的污点通过重构的dalvik视图。
	


## 2 DroidScope设计与实现

### 2.1 DroidScope系统架构

#### 2.1.1 总体架构
DroidScope架构可见文中图1。    
![Test](droidscope_analysis.files/droidscope-overview.png?raw=true)     
图 DroidScope架构    
 
DroidScope运行于QEMU模拟器之上，整个分析过程分析完全在Guest系统之外。
DroidScope修改Android模拟器通过AndroidSDK，而保持Guest系统（Android）不变，且不同的设备驱动可以被正常加载。
API跟踪器可以监视恶意软件再API层的行为，即恶意软件如何与运行时的Android环境进行交互。包括监视恶意软件的Java组件如何与Java Framework进行交互，本地组件如何与Linux系统进行交互，java组件如何通过JNI接口与本地组件交互。
	
本地指令跟踪器、dalvik指令跟踪器深入观测恶意软件行为通过记录指令细节。
dalvik指令跟踪器记录恶意软件java组件的Dalvik字节码指令，
本地指令跟踪器记录本地组件（如果存在）的机器指令。
	
污点跟踪器可以检测恶意软件如何通过污点分析获取并泄漏敏感信息（GPS位置、IMEI、IMSI？？等）。如果只有机器级的污点分析，Droidscope无法实现敏感信息泄漏检测，只有重构并统一了java层和OS层的语义，才能实现敏感信息泄漏检测。
	

#### 2.1.2 Linux视图重构

这一节主要介绍进程信息、线程信息等在运行时的重新构建。
操作系统层的视图构建是分析本地组件的基础，它也为ｊａｖａ分析组件分析ｊａｖａ层视图提供基础。
首先我们使得Ａｎｒｏｉｄ模拟器可以进行基本的指令插桩。这里我们先回顾一下ｑｅｍｕ的基础知识：
ｑｅｍｕ是一个基于二进制动态翻译技术的ＣＰＵ仿真器。其正常的执行流程包括：
	1. 将一个Ｇｕｅｓｔ指令块翻译成ＴＣＧ中间表示。
	2. 将ＴＣＧ指令块编译生成ｈｏｓｔ指令，并存入代码ｃａｃｈｅ中；
	3. 控制指令跳转到翻译过的代码，从而使得Ｇｕｅｓｔ中的指令被成功执行。
接下来，如果下次执行同一代码块，将直接跳到代码ｃａｃｈｅ中执行。
为了进行分析，ｄｒｏｉｄｓｃｏｐｅ在翻译后的指令中插入代码块，从而达到插桩的目的。具体的说，就是在翻译的过程中，在ＴＣＧ指令中插入额外的分析代码，这些分析代码在指令执行过程中将被调用。

通过基本的指令插桩，可以实现操作系统层面的语义抽取，如系统调用、运行进程、线程以及内存映射等等。

 * 系统调用
用户层的进程通过系统调用来访问系统资源，因此我们可以通过应用程序调用的系统调用来理解程序的行为。
在ＡＲＭ架构上，0号指令（ｓｖｃ #0） 被用来作为系统调用的接口，其调用号ｎｕｍ为寄存器ｒ7中的值。这与ｘ86架构中的ｉｎｔ 80类似。
为了获取系统调用信息，我们在这些特殊的指令处进行插桩，比如插入一些额外的ＴＣＧ指令，可以调用一个回调函数实现从内存中获取更多的信息。对于一些重要的系统调用，（比如ｏｐｅｎ，ｃｌｏｓｅ，ｒｅａｄ，ｗｒｉｔｅ等），我们将其进行记录，并获取其调用参数和返回值等。通过这些监视，我们可以大致知道一个用户程序如何访问文件系统、网络数据传送以及与其他进程通信情况等。

 * 进程和线程
主要通过维护一个影子任务链表来实现。链表节点还包括一些所需的额外信息。为了区分进程和线程信息，节点中还记录了线程组标识ｔｇｉｄ，除此之外，还包括ｕｉｄ，ｇｉｄ，ｅｕｉｄ，ｅｇｉｄ，父进程ｐｉｄ等。

根据Ｌｉｎｕｘ内核的设计，task_struct结构的地址可以很容易定位。通过当前thread_info结构可以直接找到task_struct，而thread_info可以是呀stack point & 0x1FFF来找到。这样我们可以通过遍历ｔａｓｋ_struct来枚举所有的进程，并记录到影子任务链表中。同时，通过监视sys_fork,sys_execve,sys_clone,sys_prtcl等函数来更新影子任务链表。

 * 内存映射
主要通过task_struct找到mm_struct，从而可以获取内存布局等信息，并通过监视sys_mmap2来更新影子内存的映射。




#### 2.1.3 Dalvik视图构建


通过操作系统层的视图和Dalvik虚拟机（DVM）的内部运行机制，可以重构Dalvik或Java层的视图，包括Dalvik指令，当前机器状态以及java对象等。

 * Dalvik指令
DVM的主要任务就是执行dalvik字节码指令。在Ginger-bread以及其后的版本中，主要有两种方式来执行：解释器解释执行和JIT即时编译后执行。

DVM中的解释器叫mterp，使用偏移地址的方法来映射Dalvik字节码到机器代码。每个opcode都有64字节的空间来存储相应的模拟代码，如果模拟代码超过64字节将使用额外的区域。其布局方式如下图所示。

![Test](droidscope_analysis.files/dalvik_opcode_layout.png?raw=true)     
图 DroidScope架构    

这种方式也使得从本地指令到dalvik指令的逆向也变得简单。当程序计数器的值指向这些代码，那么可以确定DVM正在解释一个字节码指令。

JIT技术的引入提高了DVM的i性能，尤其当有许多热代码块存在的时候。但JIT的引入使得细粒度的插桩变得更加困难，因为JIT的性能优化使得多个Dalvik代码块进行了融合，从而模糊了每条Dalvik指令的边界。

为了可以对Dalvik指令进行分析，一种比较简单的方法就是完全的禁用JIT。但是这样会导致系统性能的严重下降，更重要的是这将导致需要对虚拟设备进行修改。事实上，我们只需要对一部分Dalvik指令进行监视和分析，因此我们可以选择性的关闭JIT。当分析我们指定的代码区域时关闭JIT，而其他时候则不需要关闭。下图是DVM调用mterp和JIT执行代码的基本流程。

![Test](droidscope_analysis.files/mterp_jit.png?raw=true)     
图 DroidScope架构    

DVM通过都dvmGetCodeAddr函数来判断代码是否已经被翻译，如果是则直接执行，如果不是则再判断代码的“热”的程度，如果属于热代码则使用JIT执行，如果不属于则使用mterp解释执行。我们通过插桩dvmGetCodeAddr函数来实现选择性的JIT。即对于我们需要监控分析的代码，我们直接在dvmGetCodeAddr中返回NULL，这样也基本确保了不会对系统产生不良的副作用。


 * DVM状态
在ARM架构下，DVM在使用mterp解释执行指令的时候，其维护的虚拟机状态有以下特征。寄存去R4到R8存储了当前DVM执行的上下文环境。其中，R4是Dalvik程序计数器，指向当前执行的Dalvik指令。R5是Dalvik栈指针，指向当前栈帧的开始位置；R6指向interState数据结构glue。R7包含当前dalvik指令的两个字节的opcode。R8则存放了mterp模拟代码的基地址。
如果在x86架构下，edx是Dalvik程序计数器，esi是Dalvik栈指针，ebx包含当前dalvik指令的两个字节的opcode。edi则存放了mterp模拟代码的基地址。指向interState数据结构glue可以在堆栈中的某个固定位置找到。
Dalvik虚拟寄存器为32位，且按逆序存放在栈中。V0存放在栈顶，通过R5可以找到，V1则在V0之上，等等。其他的Dalvik状态信息可以通过glue数据结构获取，比如返回值、线程信息等。

通过这些寄存器和相关数据结构，便可以获取当前DVM的基本状态，如程序计数器、栈帧指针和虚拟寄存器等。






### 2.2 DroidScope的接口分析

所有的API分为三个层次：Native层、Linux层、Dalvik层。

每个层次上的API分为两类：事件相关的API及获取、处理该层语义信息的API。


##### Native API
* Event
  * instruction begin/end  指令开始执行和执行结束时触发事件
  * register read/write 寄存器读写时触发事件
  * memory read/write 内存读写时触发事件
  * block begin/end  块开始和结束时触发事件
* Query&Set
  * memory read/write 内存读写
  * memory read/write with pgd
  * register read/write 寄存器读写
  * taint set/check  污点设置和检查

##### Linux API
* Event
  * context switch  环境切换
  * system call  系统调用
  * task begin/end 任务开始结束
  * task updated  任务更新
  * memory map updated 
* Query&Set
  * query symbol database 查询符号表
  * get current context  获取当前环境
  * get task list 获取任务列表

##### Dalvik API
* Event
  * Dalvik instruction begin  Dalvik指令开始
  * method begin  方法开始
* Query&Set
  * query symbol database  查询符号表
  * interpret Java object  
  * get/set DVM state 获取、设置DVM状态
  * taint set/check objects 污点设置和检查
  * disable JIT  关闭ＪＩＴ

##### 重要数据结构及机制

* 回调函数

```
    #define CALLBACK_LIST_SIZE 10

    typedef struct _CallbackStruct
    {
      uint32_t condition;
      void* func;
    } CallbackStruct;

    typedef struct _CallbackList
    {
      size_t len;
      CallbackStruct arr[CALLBACK_LIST_SIZE];
    } CallbackList;
```

每一个CallbackList包含最多CALLBACK_LIST_SIZE个回调函数，每个回调函数用CallbackStruct结构封装，包含函数指针func和condition。

下面是对CallbackList进行操作的三个函数：

    int CallbackList_init(CallbackList* pList)
    int CallbackList_add(CallbackList* pList, void** pFunc, uint32_t condition)
    int CallbackList_remove(CallbackList* pList, void** pFunc, uint32_t condition)

一般以事件来命名CallbackList结构：eventCallbacks

关于condition，对于Process时间来说都是表示pid，但是ProcessStarted和ProcessUpdated的事件在注册的时候一个需要pid传一个不需要，但是在Call的时候好像没有区别。不知道为何不搞成统一的形式。

##### 事件注册机制

假设事件名称为Event，则使用registerEventCallback函数来注册事件相关的回调函数。

系统会在响应的事件点上使用CallEventCallback函数来调用所有已经注册的回调函数。

可以通过设置Name/PID/PGD来选择监控的进程。Name对应进程中的strName或strComm。

* set_by_name - 存储在watchName
* set_by_pid - 存储在watchPID
* set_by_pgd - 存储在watchPGD

要注意的一点是，如果使用set_by_name，ProcessBegin的事件中无法得到Process的strName，因此注册的ProcessBegin的回调方法无法正确的找到watchName所代表的进程。

int CallbackList_remove(CallbackList* pList, void** pFunc, uint32_t condition)

##### SystemCalls
注意到，TEMU_Init中默认是开启的。但是跟ProcessWatcher模块相关，只有ProcessWatcher中设置了监控的进程，才会输出系统调用。



#### 2.2.1 Natvie API

Natvie API主要是对本地指令进行操作的函数接口。主要分为Ｅｖｅｎｔ类型和可直接调用类型。下面将主要对Ｅｖｅｎｔ类型的回调函数类型进行介绍。


##### Event类型   

在Natvie API层，包括的回调函数类型及注册回调函数的方法如下：

回调函数类型：    

```      
typedef void (*insn_begin_callback_func)(CPUState* env, uint32_t eip, uint32_t insn);
typedef void (*insn_reg_callback_func)(CPUState* env, uint32_t regid, uint32_t val);
typedef void (*insn_mem_callback_func)(CPUState* env, uint32_t addr, uint32_t val);
typedef void (*insn_end_callback_func)(CPUState* env);
typedef void (*context_switch_callback_func)(CPUState* env, int reg, uint32_t oldval, uint32_t val);
typedef void (*block_begin_callback_func)(CPUState* env, TranslationBlock* tb);
typedef void (*block_end_callback_func)(CPUState* env, TranslationBlock* tb, uint32_t pc);
typedef void (*syscall_callback_func)(CPUState* env, uint32_t eip, uint32_t r7);

```    

主要回调函数接口：
    
```
/**
 * 注册一个函数，该函数在程序块开始事件触发QEMU时调用
 * @param func Pointer to the function
 * @return 0 if successful, failure otherwise
 */
int registerInsnBeginCallback(insn_begin_callback_func func);

```

```
/**
 * 注册一个函数，该函数在寄存器内容被作为操作数访问时调用
 * @param func Pointer to the function. regid is the register number. val is the value
 * @param bWrite TRUE means called when a write operation occurs, FALSE for read
 * @return 0 if successful, failure otherwise
 */
int registerInsnRegOperCallback(insn_reg_callback_func func, int bWrite);
```   
```
/**
 * 注册一个函数，该函数在内存操作数被访问的时候调用
 * @param func Pointer to the function. addr is the memory address, val is the value
 * @param bWrite TRUE for write, FALSE for read
 * @return 0 if successful, otherwise failure
 */
int registerInsnMemOperCallback(insn_mem_callback_func func, int bWrite);
```   
```
/**
 * 注册一个函数，该函数在当前指令仿真成功的时候调用
 * @param func Pointer to the function
 * @return 0 if successful, otherwise failure
 */
int registerInsnEndCallback(insn_end_callback_func func);
```   
```
/**
 * 注册一个函数，该函数在当前指令仿真失败的时候调用
 * @param func Pointer to the function
 * @return 0 if successful, otherwise failure
 */
int registerInsnEndErrorCallback(insn_end_callback_func func);
```   
```
/**
 * 注册一个函数，该函数在基本程序块开始的时候被调用
 * @param func Pointer to the function
 * @param addr Invoke callback only when the block's address begins at addr. INV_ADDR means always invoke this callback
 * @return 0 if successful, otherwise failure
 */
int registerBlockBeginCallback(block_begin_callback_func func, uint32_t addr);
```   
```
/**
 * 注册一个函数，该函数在基本程序块结束的时候被调用
 * @param func Pointer the function
 * @param addr NOT USED - pass INV_ADDR
 */
int registerBlockEndCallback(block_end_callback_func func, uint32_t addr);
```   
```
/**
 * 注册一个函数，该函数在上下文切换将要发生的时候被调用
 * @param func Pointer to the function
 * @return 0 if successful, otherwise failure
 */
int registerContextSwitchCallback(context_switch_callback_func func);
```   
```
/**
 * 注册一个函数，该函数在发生系统调用的时候被调用
 * @param func Pointer to the function, env is the current CPUState and r7 is the syscall number
 * @return 0 if successful, otherwise failure
 */
int registerSyscallCallback(syscall_callback_func func);
```   
```
/**
 * Register a new optimized block begin callback condition. A block begin callback condition is only generated
 * when the current address (e.g. the address of the beginning of the block) matches one of the registered conditions.
 * This only enables the callbacks - it is up to the receiver to determine if it is really interested in the block begin
 * For example:
 *   A calls registerNewOptimizedBlockBeginCallbackCondition(0x12345678, OCB_CONST)
 *   B calls registerNewOptimizedBlockBeginCallbackCondition(0xDEADBEEF, OCB_PAGE)
 *   BOTH A and B's registered callbacks will be invoked for a block that begins at 0x12345678,
 *   0xDEADBEE0, 0xDEADB000 and etc.(为什么???)
 *   A and B's callback functions should keep this in mind during implementation.
 *   This is especially important for OCB_ALL.
 * As implemented now, registering OCB_ALL would increment a counter. Removing OCB_ALL would decrement a counter - to 0.
 *   This is done to ensure that if A and B both register for OCB_ALL, and then A removes its condition, B will continue
 *   to get its requested callbacks.
 * @param addr The condition
 * @param type The condition type
 *
 */
int registerNewOptimizedBlockBeginCallbackCondition(uint32_t addr, OCB_t type);
```   
```
/**
 *移除一个条件
 * Removes a condition
 */
int removeOptimizedBlockBeginCallbackCondition(uint32_t addr, OCB_t type);
```   
```
/**
 * Register a new optimized block end callback. A callback will be generated when the current basic block
 * begins at the same page as FROM and the target address is on the same page as TO. Either of these can
 * be INV_ADDR which equates to a don't care. If both are INV_ADDR then it is the same as enabling
 * ALL block end conditions. Similar to the block begin callback conditions, a counter is used for the ALL
 * condition
 * @param from The from page
 * @param to The to page
 */
int registerNewOptimizedBlockEndCallbackCondition(uint32_t from, uint32_t to );
```   



#### 2.2.2 Linux API

[chy]需要加上对Linux API的描述，说明本节要做的事情

#####  Event类型   

[chy]下面的内容是要说明啥？

回调函数类型：  
```
typedef void (*process_begin_callback_func)(CPUState* env, int pid);
typedef void (*process_ended_callback_func)(CPUState* env, int pid);
typedef void (*process_updated_callback_func)(CPUState* env, int pid);
typedef void (*process_modules_updated_callback_func)(CPUState* env, int pid, ModuleNode* node);
```
```
/**
 * 注册一个新的回调函数-在进程开始时调用
 * @param func The function to call when a process begins
 * @return 0 if successful
 */
int registerProcessBeginCallback(process_begin_callback_func func);
```
```
/**
 * 注册一个新的回调函数-在进程结束时调用
 * @param func The function to call when a process ends
 * @return 0 if successful
 */
int registerProcessEndedCallback(process_ended_callback_func func);
```
```
/**
 * 注册一个新的回调函数-在进程信息变化时调用
 * @param func The function to call when a process's information changes
 * @param pid PID of the process for which the callback should be called. INV_ADDR means called when any process is updated
 * @return 0 if successful
 */
int registerProcessUpdatedCallback(process_begin_callback_func func, int pid);
```
```
/**
 * 注册一个新的回调函数，进程的pid和指向这个moduleNode的指针变化将被呈现，现在这部分还没有被实现 - 因此
 *  指针可能是NULL
 * @param func The function to call when a process's memory map changes
 * @param pid PID of the process for which the callback should be called. INV_ADDR means called when any process is updated
 * @return 0 if successful
 */
int registerProcessModulesUpdatedCallback(process_modules_updated_callback_func func, int pid);
```

#####   Control类型   
```
/**
 * 更新一个进程的module list
 * @param pid The process's PID
 */
void updateProcessModuleList(CPUState* env, int pid);
```
```
/**
 * 更新进程列表
 * @param pgd - PGD is the current PGD, this is used for determining if the command name (which is in the userland) is available
 */
void updateProcessList(CPUState* env, uint32_t pgd);
```
```
/**
 * 返回正在执行任务的PID
 * @return PID of the currently executing task - according to the shadow list
 */
int getCurrentPID();
```
```
/**
 * 返回正在执行的任务的PGD - 依据shadow list
 * @return PGD
 */
uint32_t getCurrentPGD();
```
```
/**
 * 返回名字 - 或者返回一个指向正在执行的进程的名字的指针常量
 * @return Pointer to the name
 */
const char* getCurrentName();
```
```
/**
 * 更新shadow process list中的进程的内容，只有那些指针不是INV_ADDR和NULL的被更新，其他的不用管。
 * @param pid
 * @param parentPid
 * @param tgid
 * @param glpid
 * @param uid
 * @param gid
 * @param euid
 * @param egid
 * @param pgd
 * @param strName
 * @param strComm
 * @return
 */
int updateProcess(int pid, int parentPid, int tgid, int glpid, int uid, int gid, int euid, int egid, uint32_t pgd, const char* strName, const char* strComm);
```
```
/**
 * 从列表中依据PID移除一个进程 - 这需要包含TGID，否则的话这将是一个BUG
 * @param pid
 * @return
 */
int removeProcess(int pid);
```
```
/**
 * 增加一个新进程，如果这个进程已经存在于列表之中，那么这个进程将会被重写
 * @param pid
 * @param parentPid
 * @param tgid
 * @param glpid
 * @param uid
 * @param gid
 * @param euid
 * @param egid
 * @param pgd
 * @param strName
 * @param strComm
 * @return
 */
int addProcess(int pid, int parentPid, int tgid, int glpid, int uid, int gid, int euid, int egid, uint32_t pgd, const char* strName, const char* strComm);

int addThread(int pid, int tid, gva_t threadInfo);

int removeThread(int pid, int tid);

int clearThreads(int pid);
```
```
/**
 * 打印出进程列表在FP指向的文件中，使用TEMU_fprintf。
 * @param fp
 * @return
 */
int printProcessList(FILE* fp);

int printThreadsList(FILE* fp);
```
```
/**
 * 销毁进程列表
 */
void destroyProcessList();
```
```
/**
 * 向一个进程增加一个新的模块，TODO：需要保证pid和tgid都是正在使用的
 * @param pid
 * @param startAddr
 * @param endAddr
 * @param flags
 * @param strName
 * @return
 */
int addModule(int pid, uint32_t startAddr, uint32_t endAddr, uint32_t flags, const char* strName);
```
```
/**
 * 通过名字移除一个module
 * @param pid
 * @param strName
 * @return
 */
int removeModuleByName(int pid, const char* strName);
```
```
/**
 * 将一个进程的module list打印在屏幕上
 * @param fp
 * @param pid
 */
void printModuleList(FILE* fp, int pid);
```
```
/**
 * 要求Shadow Task List在下一次上下文切换的时候被更新
 * @param env
 */
void requestProcessUpdate();
```

#####  Access类型   
```
/**
 *  判断一个符号是否和一个地址关联
 * @param pid
 * @param address
 * @return 1 If a symbol exists. 0 Otherwise
 */
int symbolExists(int pid, gva_t address);
```
```
/**
 * 取回symbol在地址为address，pid为pid并且用最大长度len把它拷贝进symbol
 * @param symbol The buffer where the symbol should be copied to
 * @param len Maximum characters to copy
 * @param pid The process' PID
 * @param address The address
 * @return 0 If successful. Error codes otherwise.
 */
int getSymbol(char* symbol, size_t len, int pid, gva_t address);
```
```
/**
 * 给定一个symbol，取回它在模块中的地址，对于pid为pid的进程
 * @param pid The process' PID
 * @param strModule The module name
 * @param strSymbol The symbol name
 * @return The address if its available or INV_ADDR if its not
 */
gva_t getSymbolAddress(int pid, const char* strModule, const char* strSymbol);
```
```
/**
 * 获取最近的和address相关联的symbol，
 * The search returns the symbol who's address
 * is closest to <= address. The idea is that if functions are packed together, this function
 * will tell you what function (i.e. symbol) this instruction (e.g. address) belongs to
 * @param symbol The buffer where the symbol should be copied into
 * @param len Maximum number of characters to copy
 * @param pid The process' pID
 * @param address The address
 * @return 0 If successful, error codes otherwise.
 */
int getNearestSymbol(char* symbol, size_t len, int pid, gva_t address);
```
```
/**
 * 利用pid为参数返回一个指向ProcessInfo的指针
 * @param pid The PID
 * @return Pointer, otherwise NULL if not found
 */
ProcessInfo* findProcessByPID(int pid);
```
```
/**
 * 利用pgd返回一个指向ProcessInfo结构的指针
 * @param pgd The PGD to look for
 * @return Pointer to the structure otherwise NULL if not found
 */
ProcessInfo* findProcessByPGD(uint32_t pgd);
```
```
/**
 * 用name做参数返回一个指向ProcessInfo的指针
 * @param strName The name
 * @return Pointer to the structure otherwise NULL if not found
 */
ProcessInfo* findProcessByName(const char* strName);
```
```
/**
 * 判断进程pid是否在shadow list之中
 * @param pid
 * @return
 */
int processExist(int pid);
```
```
/**
 * 判断进程pgd是否在shadow list之中
 * @param pgd
 * @return
 */
int processExistByPGD(uint32_t pgd);
```
```
/**
 * 判断进程strName是否存在
 * @param strName
 * @return
 */
int processExistByName(const char* strName);
```
```
/**
 * 对于给定的地址，返回模块的名字
 * @param pid The process's pid
 * @param str Where the name should go
 * @param len The maximum characters to copy
 * @param addr The address
 * @return 0 if successful. Error code if not.
 */
int getModuleName(int pid, char* str, size_t len, gva_t addr);
```
```
/**
 * 对于给定地址addr，返回所有的和这个地址相关联的模块的信息
 * @param pid THe process' pid
 * @param str Where the name goes
 * @param len The maximum number of characters
 * @param pStartAddr Where the start address should go
 * @param pEndAddr Where the end address should go
 * @param addr The address
 * @return
 */
int getModuleInfo(int pid, char* str, size_t len, gva_t* pStartAddr, gva_t* pEndAddr, gva_t addr);
```
```
/**
 * 返回模块的信息，通过首先用模块的名字来寻找这个模块
 * @param pid The process' pid
 * @param pStartAddr Where the start address should go
 * @param pEndAddr Where the end address should go
 * @param strName The module's name
 * @return
 */
int getModuleInfoByName(int pid, gva_t* pStartAddr, gva_t* pEndAddr, const char* strName);
```

#### 2.2.3 Dalvik API

##### Event类型  

回调函数类型：
```
typedef void (*dalvik_insn_begin_callback_func)(CPUState* env, uint32_t opcode);
typedef void (*dalvik_method_begin_callback_func)(CPUState* env);
```
```
/**
* ？？？待查证
*/
int registerDalvikInsnBeginCallback(dalvik_insn_begin_callback_func func);
int registerDalvikMethodBeginCallbck(dalvik_method_begin_callback_func func);
```

##### Control类型  

```
void disableJITInit(gva_t getCodeAddr);
/**
 * 对于pid为PID的进程添加一个范围来禁用JIT
 * TODO: 支持更多的pid而不只是一个
 * @param pid The process's PID
 * @param startAddr The starting address of the range
 * @param endAddr The ending address - non inclusive
 * @return
 */
int addDisableJITRange(int pid, gva_t startAddr, gva_t endAddr);
int removeDisableJITRange(int pid, gva_t startAddr, gva_t endAddr);
void disableJIT_close();

void dalvikMterpInit(CPUState* env, gva_t ibase);
int addMterpOpcodesRange(int pid, gva_t startAddr, gva_t endAddr);
int removeMterpOpcodesRange(int pid, gva_t startAddr, gva_t endAddr);
void dalvikMterp_close();
```

##### Access类型

```
/**
 * 从MemoryBuilder的内存中在地址为addr处获取Dalvik Java Object
 * @param addr The address of the Object, (Object *)
 * @param pObj Reference to an Object*. If successful, pObj will point to a NEW Object. NEEDS TO BE FREED!
 * @param pClazz Reference to a ClassObject*. If successful, pClazz will point to a new ClassObject that needs to be FREED.
 *   This is the contents of the ClassObject that the Object points to.
 * @returns 0 If successful
 */
int getObjectAt(gva_t addr, Object** pObj, ClassObject** pClazz);
```
```
/**
 * 把一个Dalvik Java String 转化成一个String
 * Dalvik Java Strings 使用的是UTF-16 编码, 所以我们把它变回到 CString.
 * 这个函数式基于CString to UTF16 函数在UtfString.c 和 dexGetUtf16FromUtf8 在 libdex/DexFile.h
 * @param pSO pointer to the StringObject to convert.
 * @param str Reference to a char* that will be malloced. NEEDS TO BE FREED!!!!
 * @return 0 If successful
 */
int convertJavaStringToString(StringObject* pSO, char** str);
```
```
/**
 * A wrapper that uses both getObjectAt and convertJavaStringToString. Don't forget to delete the string after use.
 */
int getCStringFromJStringAt(gva_t addr, char** str);
```
```
/**
 * 打印出fp指向的文件的java对象的字段
 * @param fp The output file
 * @param addr The address of the java object
 * @param pMap A pointer to the address-to-string map to use for getting object names and signatures.
 */
int printJavaObjectAt(FILE* fp, gva_t addr, struct U32StrMap* pMap);

int printJavaStringAt(FILE* fp, gva_t addr);
```
```
/**
 * 给定Glue structure的地址，获取线程的id
 */
uint32_t getDalvikThreadID(int pid, gva_t pGlue);

```
```
/**
 * 返回dvmASMInstructionStart的地址，也就是指令的基址
 * @param pid The process' pid - not used yet
 */
static inline gva_t getIBase(int pid)
{
  return(getSymbolAddress(pid, "/lib/libdvm.so", "<dvmAsmInstructionStart>:"));
}
```
```
/**
 * 返回dvmJitGetCodeAddr的地址
 */
static inline uint32_t getGetCodeAddrAddress(int pid)
{
  return (getSymbolAddress(pid, "/lib/libdvm.so", "<dvmJitGetCodeAddr>:"));
}
>>>>>>> e6e396be2a36d5e600c69fb1875319f2364e2a46

```

### 2.3 DroidScope 插件设计与实现

[chy]需要加上对DroidScope 插件的描述，说明本节要做的事情

#### 2.3.1 example

在TEMU/plugins目录中创建一个文件simple.c，代码如下：

    #include "TEMU/utils/Output.h"
    void simple_init()
    {
      TEMU_printf("Hello World\n");
    }

Output这个工具用来输出消息，这里调用了TEMU_printf函数来输出"Hello World"。输出到日志文件还是监视器取决于配置情况。优先级顺序是是日志文件、监视器、标准输出。

然后在TEMU/TEMU_Init.c中相应的地方添加如下代码：

    extern void simple_init();
    void temu_qemu_init()
    {
      ...
      simple_init(); //this should be at the end of the function
    }

DroidScope是事件驱动的，但这里并没有涉及任何事件，仅仅在qemu启动的时候调用simple_init函数。

最后在Makefile.android中讲新的插件simple.c包含进来：

    LOCAL_SRC_FILES += \
                   ...
                   TEMU/plugins/simple.c

重新编译、运行就会在标准输出中看到Hello World消息。

##### 写插件时需要重点关注的文件如下：

* TEMU/TEMU_Init.c - 插件的初始化函数都在这里调用
* Makefile.android - 新插件的源代码必须包含在这个Makefile中
* target-arm/translate.c - 所有的代码插桩都包含在这个文件中，包括一些配置选项，可以搜索CUSTOM_TRACE来找到所有DroidScope添加的代码。这个文件中最重要的配置选项是CHECK_IS_USER，设为1代表对所有内核和用户空间代码添加回调函数，默认或者设为0代表只对用户空间代码添加回调函数。


#### 2.3.2 tracer

[chy]需要加上对tracer的描述，说明本节要做的事情

##### native instruction tracer
通过注册ARM或x86指令回调函数，从而获取每条指令的信息，包括原始指令信息、操作数信息（寄存器和内存）、以及对应的值。该插件可将记录的指令保存到指定文件，默认为arm.trace，可以通过trace_reader_arm工具读取。

##### Dalvik instruction tracer
以dexdump格式将解码过的指令记录到指定文件，包括操作数及相应的值，以及可用的符号信息，比如类、域（field）、方法名。


##### API tracer
API tracer可以监视一个程序（包括java程序和本地组件）与系统和其他系统库调用的交互情况。首先通过注册system call events来记录所有的应用程序的系统调用，然后构造一个虚拟设备的本地库或java库的白名单，对所有程序加载，且不在白名单内的库进行分析。我们插桩dalvik字节码中的invoke*和execute*函数，判断并记录调用的方法。记录的信息包括当前执行的java线程，调用地址，被调用的方法，以及其输入的参数。在记录java string之前，先将其转换为本地字符串。之后我们插桩move-result*字节码指令，从而在系统方法返回时获取返回值。

##### taint tracker
使用动态污点分析API来分析Android应用程序的信息泄漏。首先指定敏感信息源（如IMEI, IMSI或联系人信息等）作为污点，并进行指令级别的污点传播，直到指定的结束点（比如sys_write,sys_send等）。通过操作系统视图和dalvik试图的构建，我们将创建一张可视化的图，来展现敏感信息是如何泄漏的。要构造这个图，就需要确定函数的边界。每当进行了污点传播，我们将在图上增加一个函数节点，同时标记被传播的内存。对于java对象的操作，我们进行了识别，从而创建一个对象节点来标记，而不是简单的内存定位。现只能对方法的输入参数和当前的对象进行污点检测。

## 3 droidscope执行过程分析

droidscope整个工程源代码分为android和qemu两部分，droidscope的主要实现在qemu文件夹下的temu下面，而temu是bitblaze的一个组件，因此droidscope针对linux层的实现主要借鉴了temu的内容。   
droidscope的通过在qemu中添加函数调用接口、回调事件等接口，利用插件的方式实现其主要功能。   
与其他程序一样，droidscope首先要做的事情就是初始化，其实现代码主要在TEMU_init.c中的temu_qemu_init，该函数在vl-android.c中的main函数中被调用。   

ARMInstructionTrace_init为例：    

```
void ARMInstructionTrace_init(const char* filename)
{
  ARMInstructionTraceInit(filename); //打开记录文件，并初始化一些计数变量。
  processWatcherInit();  //设置需要监控的进程。

  //设置回调函数，分别响应监控进程启动和结束事件。
  registerWatchProcessBeginCallback(&ARMInstructionTrace_process_begin_callback);
  registerWatchProcessEndedCallback(&ARMInstructionTrace_process_ended_callback);

  /** TEST Used to analyze ratc **
  registerSyscallCallback(&test_syscall_callback);
  /** END TEST **/
  
//
  registerInsnBeginCallback(&ARMInstructionTrace_insn_begin_callback);
  registerInsnEndCallback(&ARMInstructionTrace_insn_end_callback);
  registerInsnEndErrorCallback(&ARMInstructionTrace_insn_end_error_callback);
  registerInsnRegOperCallback(&ARMInstructionTrace_reg_read_callback, 0);
  registerInsnRegOperCallback(&ARMInstructionTrace_reg_write_callback, 1);
  registerInsnMemOperCallback(&ARMInstructionTrace_mem_read_callback, 0);
  registerInsnMemOperCallback(&ARMInstructionTrace_mem_write_callback, 1);
}
```

传入参数为记录的文件名称。   

设置b变量，表示需要记录指令；   
对每条指令翻译前将其记录到内存中，并进行计数，翻译完成后进行检查，如果达到MAX_CUSTOM_ENTRIES，则将记录的指令一次dump到文件中去。   
（可画一个流程图进行表示）   
其中 MAX_CUSTOM_ENTRIES = (16*1024*4096/sizeof(InsnEntry))；   
除此之外，还注册了以下几个回调，   
```
  registerInsnRegOperCallback(&ARMInstructionTrace_reg_read_callback, 0);
  registerInsnRegOperCallback(&ARMInstructionTrace_reg_write_callback, 1);
  registerInsnMemOperCallback(&ARMInstructionTrace_mem_read_callback, 0);
  registerInsnMemOperCallback(&ARMInstructionTrace_mem_write_callback, 1);
  
```
在有寄存器读写、内存读写的时候，分别将对应的值记录下来。   

NativeAPI_init

这个好像是所有插件的基础插件？
所有的回调函数链表在次进行初始化；   
```
CallbackList insnBeginCallbacks;
CallbackList insnRegReadCallbacks;
CallbackList insnRegWriteCallbacks;
CallbackList insnMemReadCallbacks;
CallbackList insnMemWriteCallbacks;
CallbackList insnEndCallbacks;
CallbackList insnEndErrorCallbacks;
CallbackList blockBeginCallbacks;
CallbackList blockEndCallbacks;
CallbackList contextSwitchCallbacks;
CallbackList syscallCallbacks;
```
创建了各种hash表，如： 
  
```
  pOBBTable = Hashtable_new();
  pOBBPageTable = Hashtable_new();
  pOBBTableNot = Hashtable_new();
  pOBBPageTableNot = Hashtable_new();

  pOBEFromPageTable = Hashtable_new();
  pOBEToPageTable = Hashtable_new();
  pOBEPageMap = Hashmap_new();
  
```

在NativeAPI.c中，提供了与qemu的接口，包括：   
```
//Here are the functions for interfacing with QMEU
void temu_insn_begin(CPUState* env, uint32_t eip, uint32_t insn)
void temu_insn_end(CPUState* env)
void temu_insn_end_error(CPUState* env)
void temu_reg_read(CPUState* env, uint32_t regid, uint32_t val)
void temu_reg_write(CPUState* env, uint32_t regid, uint32_t val)
void temu_mem_read(CPUState* env, uint32_t addr, uint32_t val)
void temu_mem_write(CPUState* env, uint32_t addr, uint32_t val)
void temu_pgd_write(CPUState* env, int reg, uint32_t oldval, uint32_t val)
void temu_block_begin(CPUState* env, TranslationBlock *tb)
void temu_block_end(CPUState* env, TranslationBlock* tb, uint32_t pc)
void temu_syscall(CPUState* env, uint32_t eip, uint32_t r7)
inline void do_flush_tb(Monitor* mon)
inline void do_flush_tlb(Monitor* mon)
```

NativeAPI为LinuxAPI插件做好了更底层准备，也就是将linux和arm指令进行了连接，提供了之间的接口。    



## 4 DroidScope使用方法

### 4.1  DroidScope编译安装过程   

我们使用的系统环境以Ubuntu 12.04为例，主要是步骤是在本机上安装一些Droidscope编译和使用时必要的软件，并且将Droidscope从镜像中移植到本机上来。       

#### 在本机上安装必要的软件

首先是在本机上安装一些必要的软件，如g++，libsdl-dev，binutils-multiarch等，需要在终端输入的命令如下：      

    sudo apt-get install g++ libsdl-dev binutils-multiarch    
    sudo apt-get install curl bison flex gperf git   
    
由于Gingerbread需要Java 1.6，因此我们需要在系统上进行安装，由于一些安全的问题，所以最好是忽略sun-java-plugin。需要如下的命令进行安装：   

    sudo add-apt-get repository "deb http://us.archive.ubuntu/ubuntu/hardy multiverse"   
    sudo apt-get update    
    sudo apt-get install sun-java-jdk         

然后，还可以根据自己的需要来选择安装Eclipse作为编辑DroidScope的IDE。  

#### 将Droidscope从镜像中移植到本机上   

在这个部分，首先是要将DroidScope的映像挂载到本机上，需要的命令如下：         

    sudo modprobe nbd    
    sudo qemu-nbd -c /dev/nbd0 DroidScope.qcow2    
    sudo mount /dev/nbd0p1    

对于上面第二行的命令，可以使用Droidscope的绝对路径。当然最后不要忘记卸载映像，需要如下命令：   

    sudo umount mnt    
    qemu-nbd -d /dev/nbd0   
    sudo modprobe -r nbd   

我们需要的文件就在虚拟镜像的桌面的四个文件夹中，因此我们需要将这四个文件夹拷贝到本机桌面上。   

#### 库文件的拷贝   

由于镜像中的库和本机上的库的差异，因此我们需要将镜像中的库文件移植到本机上，具体就是将镜像中/usr/include/下的文件拷贝到本机/usr/local/include/目录下。   

#### Droidscope的编译    

在终端下打开拷贝过来的qemu文件夹，输入make命令即可。   

在终端下打开qemu/objs文件夹，然后在终端输入如下的指令：    

    ./emulator -sysdir ~/Desktop/android-image \   
     -kernel ~/Desktop/android-kernel/zImage \   
     -qemu -monitor stdio    

如果出现android模拟器，则说明编译成功。    

### 4.2  DroidScope使用方法    

#### 第一次运行    

在终端下打开拷贝过来的qemu文件夹，输入make命令即可。   

在终端下打开qemu/objs文件夹，然后在终端输入如下的指令：    

    ./emulator -sysdir ~/Desktop/android-image \   
     -kernel ~/Desktop/android-kernel/zImage \   
     -qemu -monitor stdio    
     
在上述命令中,-sysdir是用来指定Android虚拟镜像的,-kernel用来指定将要被使用的内核,-qemu用来指定一些其他的参数.   

如果成功,可以看到一个安卓模拟器,并且可以再终端中使用help命令来获取一系列的系统命令.例如,你可以使用命令列表中的ps命令来获取进程的信息列表.      

#### 插件机制的使用   

以打印系统进程启动的信息为例来分析Droidscope中插件的使用.  

首先在插件目录plugins下创建一个新的文件,可命名为simple.c.文件中的代码如下:     

    #include "TEMU/LinuxAPI.h"    
    #include "TEMU/utils/Output.h"    
    
    void simple_process_bigin_callback(CPUState* env, int pid)    
    {       
       TEMU.printf("Process [%d] started\n", pid);
    }   
    
    void simple_init()    
    {     
        registerProcessBeginCallback(&simple_process_begin_callback);   
    }      
    
为了能够让Droidscope调用我们写的插件,我们需要编辑TEMU目录下的TEMU_init.c文件，其代码如下：

    extern void simple_init();
    void temu_qemu_init()
    {
     ...
      simple_init(); //this should be at the end of the function   
    }    
    
最后就是要编辑qemu文件目录下的Makefile.android文件,主要是要指定要调用的插件的位置.修改好之后，再终端下输入make命令就可以看到编写的插件的输出信息了.   


### 4.3  DroidScope测试分析软件实例

<DroidScope分析某恶意软件或软件漏洞的过程描述>
尝试使用DroidScope对以下几种恶意代码进行分析，分别描述如下：

 * Trojan/Android.Raden.a  （Raden木马伪装成iCalendar，中文名称兔兔日历）
 * DroidKungfu
 * zergRush

#### 4.3.1 Raden木马分析

##### 基本信息
病毒名称：Trojan/Android.Raden.a[SMS]

病毒类型：短信扣费木马

样本MD5：ACBCAD45094DE7E877B656DB1C28ADA2

样本长度： 782,964 字节

发现时间：2011.05.12

感染系统：Android 1.5及以上

Raden木马伪装成名为iCalendar如下图。    
![Test](droidscope_analysis.files/Trojan_android_raden_icalendar.png?raw=true)     
图 iCalendar界面    


Raden木马出现在Android手机的Google官方市场之上，它伪装成名为iCalendar（中文名“兔兔日历”）的一款日历软件，实际上发送扣费短信。具体而言，当用户浏览至5月的日历时，软件将向号码1066185829发送内容为“921X1”的短信，该号码为收费服务号码。此外，Raden拦截包括10086、10010、10000、1066185829等号码发来的短信，因此，用户无法发觉手机被恶意扣费。   



##### 特征及行为分析 


 * 特征


1. 敏感权限
android.permission.RECEIVE_SMS 接收短信
android.permission.SEND_SMS 发送短信
2. 入口点和恶意模块
活动（Activity）com.mj.iCalendar. iCalendar，是程序的主活动，其中包含向指定号码发送扣费短信的代码。
接收器（Receiver）com.mj.iCalendar.SmsReceiver，由android.provider.Telephony.SMS_RECEIVED事件触发，用于拦截指定号码发来的短信。
3. 敏感字符串
拦截的号码：”10086”、”10000”、”10010”、”1066185829”、”1066133”、”106601412004”
发送目标的号码及内容：”1066185829”、” 921X1”
4. 开启服务
无
5. 联网特征
无

 * 行为

1. 拦截信息

当用户手机在接收到短信时，接收器com.mj.iCalendar.SmsReceiver启动，对当前发送来的信息的号码进行验证拦截，如果所发信息的号码来源于10086、10000、10010、1066185829、1066133、106601412004中的任何一个，则自动调用abortBroadcast()事件进行拦截，这样会使用户不能得到验证码或其相关的信息，影响其相关功能及业务的应用。具体恶意代码如下：   


![Test](droidscope_analysis.files/Trojan_android_raden_lanjie.png?raw=true)     
图 关键拦截代码    


2. 向指定号码发送信息
当用户启动“兔兔日历”软件时，在点击屏幕5次（也就是第一次切换到5月的日历详情下）时，此时启动恶意代码，向指定的号码“1066185829”发送信息“921X1”，同时通过save()将当前系统中的State的值更新为“Y”，确保只执行一次发送。 


触发调用图片（月份）切换事件，代码如下：


![Test](droidscope_analysis.files/Trojan_android_raden_code1.png?raw=true)     


触发调用发送信息的事件，代码如下：


![Test](droidscope_analysis.files/Trojan_android_raden_code2.png?raw=true)     


触发发送信息事件及触发调用更新state的值的事件，代码如下：


![Test](droidscope_analysis.files/Trojan_android_raden_code3.png?raw=true)     



##### 使用DroidScope分析

在DroidScope中打开Dalvik instruction tracer插件和API tracer插件，首先启动droidscope， 启动命令如下：
```
./emulator -sysdir ~/xqx/droidscope/android-image -kernel ~/xqx/droidscope/android-kernel/zImage -qemu -monitor stdio
```

通过adb将样本安装到android中。安装成功后会增加一个兔兔日历的图标，如下图：

![Test](droidscope_analysis.files/iCalendar_installed.png?raw=true)     


设置其进程为需要监控的进程，可以通过进程名称指定，也可以通过查看进程PID指定，命令如下：
```
set_by_name com.tutusw.phonespeedup
```
通过ps命令可查看进程id如下：

![Test](droidscope_analysis.files/process_names.png?raw=true)     


点击运行恶意样本，DroidScope将记录指定进程调用的dalvik指令和linux API函数，这里我们做了一点小的改动，将分别记录的指令信息都定向到同一个文件。

根据恶意代码特征，我们对iCalendar程序进行操作，在点击屏幕5次之后，可以触发恶意代码的行为，从而进行分析。

通过DroidScope记录的指令信息，我们可以看到程序在记录界面点击次数等于5时，将调用sendsms函数发送信息的行为，如下图所示：


![Test](droidscope_analysis.files/dalvik_ins.png?raw=true)     




## 参考文献

 1. [实用符号执行](http://www.doc88.com/p-979464595223.html)
 
