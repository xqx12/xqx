# Symdrive分析

## 1 Symdrive概述

Device-driver开发和测试是非常复杂且易错的工作。比如，测试错误的处理代码需要从device模拟错误的输入；或者一个driver可以支持很多个devices，开发者没法遍历每一个device。结果，很多Linux driver补丁包含下列说明“compile tested only”。  
SymDrive是一个不通过真实的devices来测试Linux和FreeBSD的drivers的系统。Symdrive使用符号化执行来移除对硬件的需求，并且扩展了以往类似工具三方面新的特点。首先，Symdrive使用静态分析和source-to-source的转换来很大程度的减少测试新driver的工作量。其次，Symdrive checkers是c代码并且在内核中执行，可以获得所有的权限。最后，Symdrive提供一个跟踪执行工具来确定一个patch如何改变I/O到device和比较device-drive的实现。应用Symdrive到21个Linux drivers和5个FreeBSD drivers共找到39个bug。

### 1.1 Symdrive介绍

静态分析工具虽然可以快速的找到很多bug，但是却忽视了driver的行为。
Symdrive使用静态执行和动态执行结合的来测试device drivers，从而减少了符号化执行的运行时间以及避免了静态执行的一些缺点。  
设计Symdrive有三个目的。  

1. 	一个driver开发者可以使用SymDrive通过执行修改的代码影响的所有路径来测试driver补丁。  
2. 	开发者可以使用Symdrive作为debugging工具来比较functioning driver和non-functioning driver的行为。  
3. 	SymDriver可以作为一个通用的发现bug的工具，几乎不需要什么输入就可以运行适用于很多drivers的一般测试。  

Symdrive使用了S2E来进行符号执行，使device对driver的输入符号化，因此可以移除对硬件的依赖并遍历所有可能的输入。S2E也使得Symdrive可以通过标记其他的对driver的输入为符号化，来加强代码的覆盖率，例如从应用或kernel传来的数据。当检测到失败，比如无效的操作或者明确的检测，Symdrive会报告失败的位置和导致失败的输入。  
Symdrive扩展了S2E，包含以下三个主要的组成部分：  

1. 	Symdrive使用SymGen，一个静态分析和代码转换工具，在测试driver之前分析其代码。SymGen自动的运行了几乎所有之前系统留给开发者的工作，例如确定driver/kernel接口；同时SymGen提供给S2E结果来加速测试。所以，将Symdrive应用到其他的driver，driver classes和buses上工作量很小。作者将Symdrive应用于2个操作系统的5个buses上的11个drivers的classes上。  
2. 	Symdrive提供了一个测试框架，这个测试框架允许checkers(用c代码写成的用来确认driver行为)运行于kernel。这些checkers可以得到kernel state，driver和内核之间调用的参数和返回值。checker可以使前置、后置条件断言位于driver行为之上，并且当driver运行不正常时报错。借助于bugs、内核编程的相关知识，作者完成了49个checkers，564行代码进行通用的检测，例如从entry points开始的两两匹配的alloc/free调用、没有内存泄露、以及内核API的正确使用。  
3. 	Symdrive提供跟踪执行机制来记录driver的执行路径，包括每条IO操作的指令指针和栈的跟踪。这些traces可以被用作来比较不同driver再访问和实现的执行的不同点。例如，开发者可以debug一个driver和之前的driver在行为上的分支不同处。  

在实际测试使用中，Symdrive有代码覆盖率高，应用范围广的优点。  

### 1.2 Symdrive依赖于S2E的符号执行

Symdrive必须有如下性质：实用性、简洁性和效率性。具体来说，首先，Symdrive必须能够找到使用别的机制难以找到的bugs，例如普遍的测试或静态分析工具。其次，Symdrive需要很少的工作就可以测试新driver，因此可以支持很多device classes、buses和os。最后，Symdrive必须可以足够快的应用于各patch。  
Symdrive使用符号执行来运行device-driver代码，从而不依赖于真实的device。符号执行允许程序的输入被符号值替换（这个符号值代表所有该输入可能取到的值）。当程序比较一个符号值，就会分支产生多路径执行，每条路径都在不同的限定下执行，以此类推。当需要一个确定值的时候，比如print，符号值会在满足所有限定的条件下被具体化。  
符号执行也可以通过非法操作来检测bugs，例如空指针的解引用；或者通过对行为明确的断言，并可以展现失败的执行路径的状态。  

S2E符号执行  
Symdrive建立在S2E符号执行框架的一个版本上。S2E运行了一个完整的VM用来测试程序。因此，符号值可以被用在os的任何地方，包括drivers和apps。S2E是一个记录符号值在VM中使用的VMM。这个VMM记录了VM中的每一条执行路径，以及在不同路径之间调度cpu。每条路径被当作一个thread，调度器选择执行哪条路径以及何时切换到令一条路径。  
S2E支持plugins，plugins是加载进VMM的模块，可以引用plugins来记录信息或改变执行。Symdrive使用plugins来实现符号硬件，路径调度，以及代码覆盖率检测。  

符号执行的优点  
符号执行通常被使用通过测试所有可能的输入来达到高的代码覆盖率。对于device drivers来说，符号执行提供了一个附加的好处：不用真实的device。和大多数代码不同，driver代码需要device才可以加载和执行。更进一步，使device产生特定的输入也很困难，从而使得测试driver的错误处理变得困难。  
符号执行可以将device的输入符号化，从而移除了对硬件的依赖。还有一种方法是编写device的软件模型，但是需要更多准确的测试和大量的工作。而符号执行可以通过符号数据来模拟device行为，从而直接使用driver。  
符号执行可能会提供正确的functioning devices不会提供的输入。可是，由于硬件可能产生不期望或者错误的driver输入，所以符号执行模拟的device行为是合理的，即使device产生了一个错误的值，driver也不应该简单的挂掉。  
和静态分析工具比较，符号执行提供了很多优点。首先，直接使用kernel代码作为kernel行为的模型，而不是需要一个编程实现的模拟模型。其次，由于driver和kernel代码都被真实执行，kernel debugging工具可以被重用，例如死锁检测、已经存在的测试等，因此很多bugs不需要对driver行为的明确描述就可以找到。第三，符号执行可以产生一系列driver entry points，从而使得符号执行可以找到span invocations的bugs，例如资源泄露。而大多数静态分析工具只致力于一个单独的entry point中发生的bugs。  

符号执行的缺点  
1. 效率：符号执行会引发路径爆炸。路径爆炸可以通过使用“区别并优先执行成功完成的路径”的策略所减少。这一策略导致在driver更深的执行，如果driver初始化失败，os可能不会产生绝大多数driver entry points。S2E和DDT需要复杂的，人工写的注释来提供这一信息。这些注释依赖于kernel函数名字以及详细的行为，这些都是程序员难以提供的。例如，这些注释经常用来检查kernel函数的参数，修改基于这些参数的当前路径下的内存。DDT和S2E的路径调度策略更倾向于探索新代码，而可能在某一路径下执行的不够深入，导致不能测试所有的函数。  
2. 简洁：已存在的符号测试工具需要大量开发工作来测试一个drivers class，再加上额外的工作来测试每一个单独的driver。例如，在S2E中支持windows NDIS drivers需要1000+行代码。例如，S2E为windows函数NdisReadConfiguration包装了代码  

 * 读取调用的所有参数（因为代码运行在kernel外，所以这样的操作很重要）  
 * 为了不同的可能的符号返回代码分支额外的路径，  
 * 通过走这些额外的路径避开对该函数的调用，  
 * 注册一个单独的相比较复杂的包装函数，当调用返回的时候执行。  

因为开发者需要为很多其他的函数在driver/kernel接口中实现类似的复杂代码，在这些系统中测试drivers变得不切实际。因此，这些工具仅仅应用来测试很少的driver classes和drivers。测试更多的drivers需要新的技术使测试工作更加自动化。  
3. 分类：最后，符号执行不能提供对正确行为的分类：一个“hello world”driver没有错误，也没有正确，例如注册一个device。在已有的工具中，测试必须编码成扩展的debugger的样子，调用read/write remote address，而不是普遍的测试代码。允许开发者在熟悉的kernel环境中写测试会简化对正确行为的分类。  
因此，Symdrive致力与改进使用符号执行测试的简化性，扩展符号执行的适用性，使其能应用于几乎任何driver，任何class，任何bus。  

## 2 Symdrive设计与实现

### 2.1 Symdrive设计目的

Symdrive致力于通过测试drivers保证代码不会错误的使用kernel/driver接口，宕机或挂起。作者在driver代码有效的测试情况下，使用综合符号执行，静态代码分析以及转换的代码和一个kernel中可扩展的测试框架来简化测试。  

### 2.2 Symdrive总体结构

Symdrive设计如下图。os kernel、测试的driver以及用户模式下的测试程序都运行在VM中。符号执行为driver提供符号devices。Symdrive提供在每次调用进入和离开driver产生checkers的stubs。测试框架记录了执行状态，传递信息给plugins，加速测试以及增加测试覆盖率。  

![Fig1](./symdrive_files/symdrive_fig1.png?raw=true)  

在开发过程中，作者考虑过将符号执行限定在driver代码中。在这个模型中，不可能多路径探索kernel；通过回调而不是一个kernel行为的模型来允许在多分支上的执行。在实现这个设计的一个原型之后，作者得出结论：更推荐采用全系统的符号执行，因为可以通过使用真实的kernel代码而不是一个kernel模型来大量减少测试drivers的工作。  
Symdrive可以用于Linux和FreeBSD，这些kernel也提供了很多用来测试的driver。只有跑在kernel上的测试框架代码是和os相关的。作者对kernel做了少量改动来打印失败和栈的跟踪结果给S2E log，并在S2E的测试下注册了这个模块。Symdrive代码分解如下。  

![Table1](./symdrive_files/symdrive_table1.png?raw=true)

### 2.3 Symdrive组成及详细设计思路

Symdrive包含5个组成部分：  

1. 	一个修改过的S2E，由Symdrive插件+一些对S2E的修改组成。  
2. 	符号device，为driver提供符号硬件输入。  
3. 	运行于kernel中的测试框架，用来指导符号执行。
4. 	SymGen静态分析和代码转换工具，用来分析和准备测试的driver。
5. 	一系列os相关的checker集合，插入在driver/kernel接口中用来核实确认driver的行为。

#### 2.3.1 VM

Symdrive使用S2E 1.1版本来进行符号执行（S2E基于QEMU和KLEE）。S2E提供了运行环境，路径分支和符号执行必须的约束求解能力。所有的driver和kernel代码，包括测试框架，都在S2E VM中运行。对S2E的改动分为以下两种：  

 * 提供符号硬件支持，  
 * S2E的plugin——Symdrive路径选择机制。  

Symdrive使用无效的x86指令来在VMM和S2E plugin间传递信息，从而提供对执行代码的额外控制。作者修改S2E，为测试框架使用新的指令来传递信息到扩展的部分中去。这些指令被SymGen插入到driver代码中，直接被测试框架引发。  
这些指令的目的是为了在源码级别传递信息给Symdrive plugin，plugin使用这些信息来指导driver的执行。这些指令  

 * 当为DMA映射数据时，控制内存区域是否为符号化的；  
 * 影响路径调度通过调整优先级、搜索策略、或者杀死其他路径；  
 * 支持跟踪记录的开启关闭，提供栈的信息。  


#### 2.3.2 符号化设备(Symbolic Devices)  

driver通过严格的、良好设计的接口和device做交互。对于PCI device drivers来说，这个接口由I/O load store访存，端口I/O指令，bus操作，DMA内存，和中断组成。driver使用别的bus，例如SPI、I2C，使用这些bus提供的函数进行相似的操作。  
Symdrive提供一个符号化的device来测试driver，同时也模拟了系统中的其他device。符号设备提供如下三个关键行为。首先，可以被发现，因此kernel可以加载合适的driver。其次，提供读写device的方法，并在读的时候返回符号值。第三，如果需要，支持中断和DMA。Symdrive目前在Linux上支持5个bus：PCI (and its variants), I2 C (including SMBus), Serial Pe-
ripheral Interface (SPI), General Purpose I/O (GPIO)和Platform，以及在FreeBSD上支持PCI bus。  

##### 2.3.2.1 发现设备

Symdrive在S2E VM中创建符号设备，并使得存在的bus代码发现新的设备然后加载合适的driver。对于一些bus来说，例如I2C，kernel或者另外的driver在初始化时通常创建一个静态的具体device对象。对于这样的device，作者创建了一个小的kernel模块，由715行代码组成，来生成需要的符号设备。  
Symdrive在加载driver之后通过测试框架向PCI bus函数返回符号数据来使device结构空间符号化。PCI device使用这一I/O内存区域来得到即插即用信息，例如芯片供应商和设备标示符。如果这个数据是符号化的，设备ID就是符号化的并且使driver为每个它所支持的device执行不同的路径。其他bus也有相似的结构数据，例如SPI bus上的“platform data”。开发者可以从kernel源码中复制这一数据，在创建device对象时提供它，或者符号化这一数据为了额外的代码覆盖率。  

##### 2.3.2.2 符号化IO

很多Linux和FreeBSD的driver对设定的I/O和DMA做了混合。Symdrive支持两种格式的设定的I/O。对于driver来说通过硬件指令来执行I/O，例如inb，或者通过内存映射的I/O，Symdrive使S2E忽略写操作，因为写操作并不返回能影响driver执行的值，同时使读操作返回符号数据。测试框架重写了bus I/O函数，例如使用在I2C driver的函数，从而起到类似的作用。

##### 2.3.2.3 符号化中断

在一个driver注册了一个中断处理之后，测试框架在每一个从driver到kernel的转变时引发中断。这一模型在实用和简易中提出了一个折衷方案：保证了中断处理被足够的调用使得driver成功执行，但是同时也会在driver不期望产生中断的时候产生不合理的中断。

##### 2.3.2.4 符号化DMA

当一个driver引发DMA映射函数，例如dma_alloc_coherent，测试框架使用一个新的S2E指令，该指令使内存表现的像一个内存映射I/O区域一样：每个read返回一个新的符号值，write不起作用。放弃对DMA内存写操作表明device有在任意时刻通过DMA写数据的能力。driver不能假设写的数据在接下来可以被使用。当driver取消内存的映射关系时，测试框架使S2E把这一内存区域回复到之前，此时写操作可以被后续的读操作使用。

#### 2.3.3 测试框架

测试框架是一个在VM中执行的kernel模块，有助于符号执行和执行checkers。Symdrive依赖于测试框架从以下三种方式来指导和监视符号执行：首先，测试框架实现了哪条路径应该更优先更深入的执行。其次，测试框架注入了额外的符号化数据来增加代码覆盖率。如之前提到的，它为一些device class实现了符号化IO接口。最后，它为执行跟踪提供给VMM一个栈的跟踪，提供了driver I/O操作的一个跟踪记录。  
测试框架支持一些加载时间的参数来控制它的行为。当使用insmod或FreeBSD的kldload加载测试框架时，开发者可以使用测试框架开启高代码覆盖模式、跟踪记录、或一个特定的符号device。开发者可以传送device的I/O capabilitis和名字作为参数来配置device。因此开发者可以创建符号化device来自动测试。  
Symdrive在测试driver中有两个相矛盾的目标：  

 * 尽可能深入的执行一条路径来完成初始化和展示driver其余的功能。  
 * 在每个函数中执行尽可能多的代码以达到完全执行。  


##### 2.3.3.1 深度探索(Reaching Deeply)

充分的测试driver的一个关键的挑战是符号执行庞大的分支代码，例如循环和探查硬件的初始化代码。Symdrive依赖于两个技术来限制路径爆炸：成功优先调度和循环省略。这两个技术使得开发者可以在一个driver中执行的更深入，测试初始化之后可用的功能。  
 * 成功优先调度  
执行过去的driver初始化非常困难，因为代码经常有很多条件来支持多种配置。例如，初始化一个声音driver，可能会在特定硬件的识别上执行多达1000个分支。每个分支创建了额外的探索路径。  
Symdrive使用了成功优先路径选择调度算法减轻了这个问题，这一算法将成功执行的路径优先化，使之成为探索优先级高的路径。测试框架在每次成功的函数返回都会减少当前路径的优先级，不论路径在driver中或是在driver/kernel接口中。高优先级可以使当前路径在切换到其他路径前更深入的被探索。由于通过函数的成功路径很短，这一策略在小函数中效果很好。  
在每一个函数退出时，测试框架通知S2E该函数是否成功完成，使得VMM可以把成功的路径优先化从而进行更深入的代码探索。测试框架判断成功的调教取决于函数的返回值。当函数返回整数，测试框架在返回值不为errno时判断为成功，errno为Linux和FreeBSD的错误值。如果判断为成功，测试框架会通知VMM优先化当前路径。如果开发者想要使用其他的启发式策略，可以添加注释优先化任意代码块。作者使用这一方法在一些网络driver中优先选择那些实现传输数据包的代码的路径。  
为了致力于driver上的符号执行，当控制权成功回到kernel时测试框架修剪了路径。它杀死了其他所有仍在driver中执行的路径，并使用了一个指令具体化VM中的所有数据，因此kernel会在具体值上执行同时不会产生分支路径。这样保证了kernel中只运行单一的路径，同时使开发者可以与系统做交互，运行用户模式的测试。  
 * 循环省略  
循环是符号执行的一个挑战，因为每次迭代都可能产生新的分支路径。S2E提供一个“EdgeKiller”plugin来中止复杂的循环，但是需要开发者确认每个循环在driver二进制码中的偏移量，因此需要很多人工的工作量。  
Symdrive通过优先化快速离开循环的路径来明确的定位循环。假设一个执行路径A进入了循环，执行了一次，在这个迭代中更多的路径被创建。如果路径A没有在一次迭代之后离开循环，Symdrive执行第二次迭代，除非提前退出，否则第二次迭代优先值会减少，因为它看起来陷在循环中。Symdrive会选择A分支出来的其他的路径B，然后执行B。Symdrive重复这样的流程直到某一条路径可以离开循环。如果没有路径快速的离开循环，Symdrive在每次随后的迭代中随机选择一些路径并优先化之，目的是希望可以离开循环。如果路径在20次迭代之后依然没有离开循环。Symdrive打印一个warning提示这是一条极端的路径，在没有人为注释的条件下没有明确的方法来有效率的执行循环。  
这种方法可以有效率、自动的执行硬件轮询的循环，同时当循环引起性能上的问题时警告开发者。可是，如果一个循环在uninstrumented kernel代码中出现时，这种方法可能会失败。这种方法在轮询循环超时的情况下也会导致很差的执行代码覆盖率。此外，产生值的循环，例如计算checksum的循环，在不停掉driver的情况下不会很早的退出。但是作者不认为这些问题很重要。  
Symdrive的方法从两个方向扩展了EdgeKiller plugin。首先，允许开发者注释driver源码而不是解析编译好的代码。其次，源码注释可以在driver修改中持续使用，而EdgeKiller plugin中使用的二进制偏移量在每次driver改动后都需要修改。  
人工注释代码可以改进执行的性能，也确实的减少了Symdriver在代码中发现bug的能力。注释需要被写在有问题的循环至少被执行一次的地方。例如，在checksum循环之后，我们添加了一行返回符号checksum值的代码，该值会被和正确的值进行比较。  

##### 2.3.3.2 增大覆盖率(Increasing Coverage)

Symdrive为测试特定的函数提供了一个高覆盖率模式，例如那些被patch修改的函数。这一模式改变了路径优先策略和kernel函数的行为。当开发者加载测试框架模块时，他可以指定任意的driver函数在这一模式下执行。  
当执行进入特定的函数时，测试框架通知S2E优先没有执行过的代码（默认策略）而不是优先成功路径。测试框架中止所有路径并返回kernel为了集中分析driver的执行。此外，当driver引发kernel函数时，测试框架会使返回值符号化。这一模式类似于S2E中的local一致性模型，但是不需要用户提供的注释和plugin，而且支持所有返回标准错误值的内核函数。例如，kmalloc返回一个符号值或者为NULL或者为一个有效的地址，用于测试driver的错误处理。  
很少的kernel函数会返回非标准的值，SymGen有一个例外表以及如何处理这些函数的返回值。完整的Linux例外表包括所有支持的driver中的100个函数。其中有64个是硬件相关的函数，例如inb和readl，这些函数总是返回符号值。14个是算术操作，例如div32.其余的22个函数成功返回时返回值为负数，或者当错误的使用时被编译器使用触发一个编译错误，例如_bad_percpu_size。  
Symdrive也通过引入额外的符号数据改进了代码覆盖率，引入这些符号数据是为了执行需要kernel或app特殊输入的代码。Symdrive可以自动的将一个Linux driver模块的参数符号化，使用所有可能的参数执行driver。Checkers也可以使传给driver的参数符号化，例如ioctl指令值。这可以使所有ioctl代码在driver的一个调用中被检测，因为每个不同的指令都会被分支执行。此外，S2E可以在VM的任何地方使用符号值，所以一个用户模式的测试也可以传给driver符号数据。  

##### 2.3.3.3 跟踪执行(Execution Tracing)

测试框架可以生成执行的跟踪记录，这对于比较同一driver的两个版本很有帮助。例如，当一个driver为新bug打上了补丁，这些跟踪记录可以用来和其之前版本的行为做比较。此外，开发者可以使用driver的其他实现（甚至可以从别的os得来）来找到表明与硬件的错误交互的矛盾。  
开发者可以通过命令行工具开启跟踪记录，该工具使用一个用户指令来通知Symdrive开启记录。在这个模式，S2E plugin记录每个driver的IO操作，包括读/写端口，MMIO，和DMA内存，和driver栈操作。测试框架在每个函数调用都会传递给S2E当前栈。  
这些跟踪记录存储成前缀树来简洁的重现代码的多路径，并能使用diff来比较。Symdrive使用driver调用栈在IO操作注释了每个跟踪记录的入口。这有助于分析特定函数，和从函数级比较driver。由于跟踪记录受限于时间变化和线程交叉切换，所以这种方式很有用。  

#### 2.3.4 SymGen

与代码交互的测试框架的所有特点，例如成功优先调度、循环优化、和使kernel返回符号值都是通过静态分析和代码生成来自动处理。SymGen工具分析driver代码来确认用来测试的相关代码，例如函数范围和循环，以及调用测试框架和checker的代码。SymGen基于CIL创建的。  

 * 插桩（Stubs）  
SymDrive对所有进入和离开driver的调用做了插桩，调用了测试框架和checkers。对于driver中的每个函数，SymGen做了两个插桩处理：一是在函数顶端做的前序插桩，二是在函数尾部做的后序插桩。生成的代码传递函数的参数和返回值通过插桩传递给checkers使用。对于每个driver输入的kernel函数，SymGen生成一个把这个kernel函数包装起来的有同样特征的插桩函数。  
为了支持前序和后序条件断言，当kernel调用进入driver或者driver调用进入kernel时会通过插桩引发checkers。在checkers中与特定的函数function_x相联系的函数名为function_x_check。在对插桩第一次执行时，测试框架会在kernel符号表中寻找一个相应的checker。如果函数存在，插桩会记录地址以便于以后的调用。当目标是kernel接口中的函数时，这一机制可以引发对任意driver函数的checkers。  
插桩使用了从driver传给kernel的一个函数指针来对相应的checker二次查找，例如PCI probe函数。kernel插桩在被传递给一个函数指针时，在一个表中记录了这个函数指针和它的目的。例如，Linux的pci_register_driver函数将pci_driver参数中的每个函数的地址和函数包括的结构名及内容对应起来。因此，对一个pci_driver结构的probe方法的插桩就被命名为pci_driver_probe_check。FreeBSD drivers也使用了相似的技术。  
插桩通过跟踪调用栈的深度来检测执行进入了driver。driver的第一个函数在入口处通知测试框架driver执行开始，在结束时通知测试框架控制权已经回到kernel。插桩也将这个信息传给VMM，使得可以根据函数返回值进行路径调度。  
 * 检测  
SymGen的潜在的检测原则是为了在VMM执行driver时通知VMM代码级的信息，使其可以更好的决定执行哪条路径。SymGen通过调用插桩来对每个driver函数的开始和结束检测。SymGen也重写了函数使其只有一个退出点。它也为Linux和FreeBSD的kernel/driver接口中常用的内联函数生成了同样的检测。  
SymGen也调用了执行SymDrive特定指令的短函数检测了开始、结束和循环体。这些指令直接使VMM优先化快速离开循环的路径以及反之。这个检测代替了需要S2E定位每个driver循环的大量工作，以及S2E需要为每个driver/kernel接口函数写一个一致性模型的工作。SymGen同样在driver中插入了循环指令，来通知S2E哪条路径离开循环，应该将其优先化。  

![Fig2](./symdrive_files/symdrive_fig2.png?raw=true)  

对于测试缓慢的复杂代码来说，SymGen支持程序员提供的注释来简化代码或临时的使代码不起作用。短的循环和那写不生成新状态的循环不需要这些人工的工作。只有那些必须执行多次且每次迭代都会产生新路径的循环才需要人工注释，作者通过C的#ifdef宏来实现。例如，E1000网卡driver将校验和与EEPROM做校对，作者修改使其能接受任意校验和。总的来说，这样的情况是非常少见的。

#### 2.3.5 局限性(Limitations)

Symdrive还不是很完善。错误的测试结果主要来自于以下两点：首先，符号执行很慢，会使kernel打印时间警告以及在错误的时间触发driver 定时器。其次，一开始的checkers不精确而且不允许kernel认为合法的行为。作者已经修改了checker，之后没有看到checker生成错误的测试结果。  
尽管没有观测到checker生成错误的结果，Symdrive还是不能检测所有类型的bug。在11个常见的安全漏洞中，Symdrive不能检测整数溢出和线程间的数据竞争，尽管由于使用VMM解释执行而不是直接执行，所以在原则上可以检测溢出。此外，Symdrive不能达到对所有driver的全路径覆盖率，因为Symdrive对路径的修剪可能会中止引起bug的路径。Symdrive也会忽视竞争条件，例如那些通过特殊的方式的竞争条件：需要中断处理和另外的线程交替执行。  

## 3 Checkers

Symdrive使用checkers检测driver/kernel接口的违规使用。driver/kernel接口是在driver和kernel控制权转换是插入的函数来判定driver的行为并使其合法化。每个driver/kernel接口中的函数都可以（不必须）有自己的checker。driver通过插桩引发driver/kernel接口函数的单独的checker。由于checker在VM中和符号执行driver一起运行，checker可以验证每条测试路径的运行时间属性。  
checker使用了support library（支持库）通过提供很多功能来简化开发。库提供了state变量来跟踪driver和当前线程的状态，例如是否注册成功，是否可以被调度。库也提供了一个tracker对象来记录现在driver中使用kernel对象。这个tracker对象提供了一个简单的机制来记录锁是否被初始化，从而发现内存泄露。最后，库提供了一般的kernel对象的class提供了普遍的checker，例如锁和allocator。这些普遍的checker对这些对象进行了编码，因此完成了大量的工作。例如，互斥锁和自选锁使用同样的checker，因为它们共享语义。  
写一个checker需要在一个call-out函数中实现check。我们使用库的API实现了564行的49个checker，针对于很多普遍的device-driver bug。  
Fig3中的Test #1就是一个pci_register_driver的call-out例子。  

![Fig3](./symdrive_files/symdrive_fig3.png?raw=true)  

driver函数中的插桩用参数和kernel函数的返回值引发了checker函数，并设置了一个precondition标志来确认checker被调用是在函数之前还是之后。此外，库提供了全局的state变量，checker可以使用这个变量记录关于driver活动的信息。从例子中可以看到，一个checker可以通过precondition检验state是否正确，并可以基于调用的结果更新state。checker可以获得driver的运行时间state，并可以存储任意的数据，所以可以发现生成driver多次调用的，过程间的，特定指针的bug。  
不是所有的行为都需要一个checker。符号执行支持很多check（包括作为kernel debug option的），比如内存错误和锁的check。绝大多数check在从driver中调用的函数里运行，因此引发了多路径。此外，引发kernel crash和panic的任何bug都会被os检测因此不需要checker。  
下面会介绍作者实现的49个checker中的一小部分。  

### 3.1 执行上下文(Execution Context)  

Linux不允许调用，由于正在执行中断处理或者hold一个自选锁的原因所阻塞，这样的函数。execution-context checker保证传给内存alloc函数（如kmalloc）的标志符在当前的执行上下文中是有效的。支持库提供了一个状态机并使用栈来记录driver当前的上下文。当进入driver时，库基于进入点更新栈。库也提供锁和中断的管理。比如当driver请求或者释放一个自选锁时，库会push或者pop记录锁的栈。  

### 3.2 内核API使用不当(Kernel API Misuse)

kernel需要driver正确使用kernel的API，错误使用会导致无功能的driver或者资源泄露。支持库的state变量提供为这一bug测试的环境。例如，checker可以记录重要的driver入口点的成功和失败。比如说init_module和PCI probe函数，并确保driver是否在初始化时被注册，以及是否在关闭是取消注册。Fig3中的Test #1展示了对state的使用来确保driver只引发一次pci_register_driver。  

### 3.3 (Collateral Evolutions)

当kernel接口中的一个小改动迫使许多driver一齐改动时，collateral evolution就发生了。开发者可以使用Symdrive通过确保打过补丁的driver不会回归到任意测试来确认collateral evolution被正确的应用。  
Symdrive可以确保补丁想要达到的效果在driver执行中被反映出来。例如，现在的kernel不再需要网卡driver在start_xmit函数中更新net_device->trans_start变量。作者写了一个checker来验证在start_xmit调用中trans_start保持不变。  

### 3.4 内存泄露(Memory Leaks)

泄露的checker使用支持库的tracker对象来保存一个alloc的地址和长度。作者实现了checker来验证19对alloc和free函数，来确保一个对象的alloc和free使用匹配的例程。  
库的API将为额外的alloc写checker的工作量简化到很少的代码量。Fig3中的Test #2展示了当检测kmalloc时，库使用的generic_allocator调用，这一调用记录了kmalloc alloc的返回的内存。一个相应的为kfree的checker验证了kmalloc alloc的是释放过的地址。

## 4 Symdrive评估

评估的目的是为了验证Symdrive是否达到了它的目标：（i)实用性，（2）简洁性，（3）效率性。  

### 4.1 方法(Methodology)

![Table2](./symdrive_files/symdrive_table2.png?raw=true)  

作者在Linux kernel的不同版本中测试了11个class里的26个driver（2.6.29中13个，3.1.1中4个，Android手机中的4个）以及FreeBSD 9中的5个driver。  
在26个driver中，作者挑选了19个作为特定的buss或者class的样例，因为针对这些driver会经常有补丁发布，因此希望能在其中找到bug。  
运行环境是Ubuntu 10.10，4核Intel 2.50 GHz Q9300 CPU，8GB内存。测试结果是在单线程模式下运行Symdrive所得到，由于Symdrive目前不能在S2E的多线程模式下工作。  
为了测试每个driver，作者执行了如下操作：  

1.	在driver上运行SymGen并编译出结果。  
2.	使用需要的参数定义一个虚拟硬件device，启动Symdrive虚拟机。  
3.	用insmod加载driver，等待初始化成功完成。完成这一步必然会执行至少一条成功路径并返回成功，尽管可能其他失败的路径也会运行并在之后被忽视。  
4.	执行一个workload（可选）。作者确保所有的网卡driver尝试传输，声音driver尝试播放声音。  
5.	卸载driver。  

如果Symdrive报告由于复杂循环造成的多路径警告，作者会注释driver的代码并重复之前的操作。对于大多数driver来说，只需要在driver上运行SymGen就可以了。对于与一个库有finegrained（很多？）交互的driver，需要在库和driver上都运行SymGen。作者在Symdrive指定的位置为每个driver添加了注释，并用49个checker测试了每个Linux driver的一系列常见bug。对于FreeBSD driver，作者只使用了os内置的测试功能。  

### 4.2 寻找bug(Bug Finding)

在Table 2中列举的26个driver中，找到了39个明显的bug，描述见Table 3。

![Table3](./symdrive_files/symdrive_table3.png?raw=true)  

在这些bug中，S2E通过kernel警告或kernel crash检测出17个，checkers检测出剩下的22个。尽管这些bug不是必然会引起driver crash，它们表现出以下问题，需要定位以及如果对driver/kernel接口不可见就很难找到。  
这一结果表明了符号执行的价值所在。39个bug中，56%生成了多个driver调用。例如，akm8975在准备好处理中断之前就使用了driver调用request_irq。如果一个中断正好发生在这个调用之后，driver就会crash，因为中断处理会解引用一个未初始化的指针。此外，41%的bug发生在driver的唯一路径上，而不是那些返回成功的路径。54%包含指针，而指针的属性很难被静态分析检测出来。

 * bug确认  

发现的39个bug中，至少17个在2.6.29到3.1.1间被修复，这表明这些bug足够重要被定位。其中的7个由于driver的变动无法确定正确的状态。作者提交了Linux主要的driver中5个没修复的bug，并已经被kernel开发者确认为真正的bug。其余的bug不在Linux主kernel中，所以并没有提交。  

### 4.3 开发者工作

Symdrive的一个目的是为了减少测试一个driver的工作量。测试的工作量主要来自于三方面：（i)为了准备测试driver的注释，（ii)测试时间，（iii)kernel接口改变时需要的代码更新。  
为了测量应用Symdrive到一个新的driver的工作量，我们测试了phantom driver。尽管对这个driver没有经验也没有硬件，完成测试的总时间是1小时45分。这个时间包括，配置符号硬件，写用户测试程序把符号数据传到driver的入口点，在不同的配置下执行driver4次。运行SymGen的时间小于1分钟，执行用了38分钟。尽管不是一个很大的driver，这个测试从开发者的角度证明了Symdrive的可用性。  

 * 注释  
Symdrive中每个driver都需要编码的地方是减缓测试的循环的注释以及优先特定路径的注释。Table 2列出了每个driver需要注释的位置的数量。在26个driver中，只有6个需要超过2个注释，9个不需要注释。Symdrive在每个可以有助于测试的注释处打印了一个警告标志。  
 * 测试时间  
符号执行会比正常的执行更慢。因此，我们希望它在开发接近尾声的时候使用，如在提交一个补丁之前，或者使用在driver代码中周期性的扫描中。在Table 2中记录了加载、初始化、卸载driver（需要检测资源泄露）的时间。初始化时间是测试中所需最少的。  
总的来说，初始化driver的时间和driver的大小成比例。大多数driver在5分钟或更少的时间内就可以完成初始化，尽管ens1371声音driver需要27分钟，相应的FreeBSD的es137x driver需要58分钟。这两个结果是由于在初始化里和deveic做了大量的交互所造成的。排除这些结果，每个补丁的执行都足够快，对一组受到collateral evolution影响的每个driver也会足够快的执行。  
 * kernel演变  
开发接近尾声时，作者从Linux 2.6.29将Symdrive更新到Linux 3.1.1。如果Symdrive的很多代码都是特定针对于kernel接口的话，更新Symdrive会需要大量的工作。可是，Symdrive使用静态分析和代码生成，从而减少了由于kernel变化维护测试的工作量：需要的改动只有更新少量的相应的kernel函数改动的checker。系统其余的部分，包括SymGen和测试框架，都不需要改动。改动的代码行数少于100。
更进一步，将Symdrive应用于一个新的os也不会很难。作者将Symdrive（不包括checker）移植到FreeBSD 9。全部的过程花了3人周的工作量。FreeBSD的实现很大程度上共享了Linux版本的相同的代码，只有一小部分是os特定的。这一结果也证明了Symdrive使用的技术可以是os通用的。  

### 4.4 覆盖率

由于Symdrive主要使用符号执行来模拟device，好处还有相比于标准测试的高代码覆盖率。Table 4展示了driver中每个class的覆盖率结果，以及执行的函数的百分比（"Touched Funcs."），以及这些函数中的基本块的百分比（"Coverage"）。此外，这张表还提供了在一台机器上运行测试的CPU总时间（CPU），以及使用多机器运行的最长执行时间（Latency）。  

![Table4](./symdrive_files/symdrive_table4.png?raw=true)  

作者运行了多次并取了平均值。作者采用的策略是：一旦运行到了一个稳定的状态则中止运行，一旦覆盖率在每次运行间没有明显的增长则停止测试driver。  
总的来说，Symdrive在大多数drivers中执行了driver函数的绝大部分（80%），并在这些函数中有着很高的覆盖率（80%）。这一结果低于100%由于以下两个原因。首先，作者没有调用一些driver中的所有入口点。例如，econet需要用户模式的软件来触发额外的driver入口点，所以Symdrive不能调用这个入口点。在其他的情况下，作者仅仅是没有花足够的时间来理解如何调用driver的所有代码，因为一些功能需要driver在一个特定的难以理解的状态。其次，在Symdrive执行的函数中，kernel传来的额外的输入或符号数据需要测试所有路径。根据S2E relaxed一致性模型，将更多的kernel API符号化，会有助于提高覆盖率。  
作为比较，作者在一个真实的网卡上测试了8139too driver，使用gcov来衡量同样测试的覆盖率。加载、卸载driver，并保证传输、接受，执行所有ethtool函数。总的来说，这些测试执行了77%的driver函数，并覆盖了这些函数中的75%。而且Symdrive执行了93%的函数，并覆盖了其中的83%的代码。尽管由于不同的方法所一没有比较其他的覆盖率结果，这一结果可以表明相比于在真实的硬件上跑driver，Symdrive可以提供更好的覆盖率。  

### 4.5 补丁测试

Symdrive的第二个主要用法就是验证driver补丁。作者评估了Symdrive对补丁测试的支持，通过应用3.1.1到3.4-rc6的补丁到8139too(net)、ks8851(net)和lp5523(LED controller)driver，分别有4、2和6个补丁。其他的driver缺失近期的补丁而只有不重要的补丁，或者需要更新kernel，所以没有被考虑。  
为了测试补丁影响的函数，作者使用了成功优先调度来快速向前执行一个补丁过的函数，而且开启了高覆盖率模式。结果见Table 5。  

![Table5](./symdrive_files/symdrive_table5.png?raw=true)  

Table 5中的结果证明Symdrive可以允许开发者不使用任何硬件device来测试几乎所有改变的代码，从而快速的测试补丁。Symdrive可以执行3个driver里12个patch中的100%的函数，并在补丁修改过的函数中达到98%的代码覆盖率。此外，完成测试仅仅花了平均12分钟。  

 * 执行跟踪  
执行跟踪提供了一个备用的方法来通过比较应用补丁前后的行为来验证补丁。作者使用了跟踪记录验证了Symdrive可以区分改变了driver/device交互的补丁和那些没改变的，例如collateral evolution。作者测试了8139too网卡driver的5个补丁（重构代码，添加功能，或者改变driver与硬件的交互）。作者执行了原始的和打过补丁的driver，并记录了硬件的交互。通过对跟踪记录的比较，不同的IO操作可以明确的识别出添加功能和改变driver/device交互的补丁。重构代码的补丁没有什么区别。  
跟踪记录还被应用于比较不同os中的相同的device上的driver的行为。Linux的8139too driver和FreeBSD的rl driver的跟踪记录表明这些driver如何与同样的硬件进行不同的交互，从而引起错误的行为。在这个例子中，Linux的8139too driver错误的将一个1字节的寄存器当作4字节，另一方面，FreeBSD的rl driver为一个特殊支持的芯片使用了错误的寄存器偏移量。开发者在我们发现这一bug之后独立的修复Linux中的bug。FreeBSD的bug被FreeBSD kernel的开发者证实。在之前的结果中没有包含这些bug，由于这并不能使用Symdrive自动执行。  
这些bug证明通过比较独立的driver实现来找到硬件特定的bug的能力。尽管作者人工的比较了这一跟踪记录，这一过程可以被自动执行。  

### 4.6 和其他工具的比较

通过和其他driver测试/bug发现工具的比较来证明Symdrive的可用性，简洁性和效率性。  

 * S2E  
为了证明Symdrive对S2E扩展的价值所在，作者执行了8139too driver，只对driver源码添加注释来指导路径探索，而没有使用测试框架，或者SymGen来优先化有用的路径。在这一配置下，S2E使用strict一致性，其中符号化数据源码只有硬件，并使用MaxTbSearcher plugin最大化了覆盖率。如果开发者不写API特定的插件的话，这一模式是默认的；如果这些插件可用，结果会有明显的提高。作者运行S2E直到23分钟后存储路径的内存用尽。  
在测试中，driver中只有33%的函数被执行，平均覆盖率为69%。相比较，在2.5小时中，Symdrive执行了93%函数，平均覆盖率为83%。只是用S2E，driver没有完成初始化，而且没有开始尝试传输包。此外，不能在driver源码上写S2E注释，而只能在二进制代码上完成。因此，每次driver编译时都需要重新生成注释。  
添加更多的RAM，运行driver更长的时间可能会使得driver完成执行初始化程序。可是，由于S2E没有自动的方式来修建路径，所以会存在许多我们不感兴趣的路径。因此，由于S2E会继续执行失败的执行路径，对开发者来说，验证driver其他的入口点会有可以想得到的困难。  
为了S2E达到高覆盖率，需要一个实现relexed一致性模型的plugin。可是，8139too driver（v3.1.1）调用了73个kernel函数，所以需要开发者在插件中编写相应的函数。  
 * 静态分析工具  
静态分析工具可以找到很多driver的bug，但是需要很多工作来实现os行为的模型。例如，Microsoft的Static Driver Verifier（SDV）需要39170行C代码来实现一个os模型。Symdrive仅需要IO bus实现的模型，5个bus的代码量总计715行。Symdrive用了491行针对特定os的代码完成了对FreeBSD的支持（主要是测试框架），并且可以使用os中已有的debug功能测试driver。  
此外，SDV通过简化分析来达到高速，因而它的checker不能重现任意的状态。因此，对于复杂的属性的检测很困难，例如从不同入口点的一个变量是否有匹配的alloc/free调用。  
 * kernel debug支持
大多数kernel为了帮助kernel开发者而提供debugging，例如检测死锁、监视内存泄露、或者发现内存错误的工具。一些测试框架的checker和Linux中的debug功能很相近。相比与Linux泄露checker，kmemleak，测试框架允许测试一个单独driver的泄露，而使用Linux的工具，单独driver的泄露会被淹没在整个kernel的泄露列表中。更进一步，为Symdrive写checker更简单：Linux 3.1.1的kmemleak模块有1113行，而测试框架的tracker对象，包括了一个完整的哈希表的实现，只有772并提供了更多精确的结果。  

## 5 结论

Symdrive结合测试框架和静态分析来使用符号执行在没有相应device的情况下测试Linux和FreeBSD driver代码。结果表明Symdrive可以在成熟的driver代码中找到很多类型的bug，并且允许开发者更深入的测试driver补丁。Symdrive也减少了测试的障碍，从而使得更多的开发者来为driver代码打补丁。未来，作者计划实现一个针对于增加人工代码review的补丁的自动的测试服务，同时研究将Symdrive的技术应用到其他kernel子系统中。

## 6 symdriver执行过程分析

//采用类似gdb调试的方式，分析symdriver的执行过程，如基于源码和gdb调试，说明它如何一步一步完成了对一个漏洞分析的执行过程。

### 6.1 符号化设备

通过s2e config文件添加，如下lp5523的config文件，符号化了名为“lp5523f”的硬件：

```
pluginsConfig.SymbolicHardware = {
     pcntpci5f = {
        id="lp5523f",
        type="pci",
        vid=0x9892,
        pid=0x9893,
        classCode=0,
        revisionId=0x0,
        interruptPin=1,
        resources={
           -- isIo = true means port I/O
           -- isIo = false means I/O memory
           r0 = { isIo=false, size=0x100000, isPrefetchable=false},
           r1 = { isIo=false, size=0x100000, isPrefetchable=false},
           r2 = { isIo=false, size=0x100000, isPrefetchable=false},
        }
    }
}

pluginsConfig.RawMonitor = {
   kernelStart = 0xC000000,

   test_pci = {
      name = "lp5523_stub",
      size = 0,
      start = 0,
      nativebase = 0,
      delay = false,        -- delay load?
      kernelmode = true,
      primaryModule = true
   },

   test_framework_lp5523 = {
      name = "test_framework_lp5523",
      size = 0,
      start = 0,
      nativebase = 0,
      delay = false,        -- delay load?
      kernelmode = true,
      primaryModule = false
   }
}

pluginsConfig.SymDriveSearcher = {
   test_pci = {
      moduleName = "lp5523_stub",
      moduleDir = "/home/fwl/s2e/symdrive/test/lp5523"
   },

   test_framework = {
      moduleName = "test_framework_lp5523",
      moduleDir = "/home/fwl/s2e/symdrive/test/test_framework_lp5523"
   }
}
```

### 6.2 测试框架

内核模块，用于优先选择路径，增加代码覆盖率及跟踪执行。有助于符号执行和checkers。编译之后生成test_framework_DRIVERNAME.ko(如test_framework_lp5523.ko)

### 6.3 SymGen

SymGen改写了静态分析工具cil，在需要测试的driver程序中进行重写并插桩，如下例lp5523中，driver的原函数：

```
static int __init lp5523_init(void)
{
        int ret;

        ret = i2c_add_driver(&lp5523_driver);

        if (ret < 0)
                printk(KERN_ALERT "Adding lp5523 driver failed\n");

        return ret;
}
```

使用SymGen之后，变为：

```
__inline static int prefn_lp5523_init(void) ;
__inline static int postfn_lp5523_init(int retval_valid , int __attribute__((__cold__))  *retval ) ;
//#line  1038 "/root/test/lp5523/leds-lp5523.c"
static int __attribute__((__cold__))  lp5523_init(void)
{
  int ret ;
  int __attribute__((__cold__))  __retres2 ;
  int _call_kernel_fn_ ;

  {
  _call_kernel_fn_ = prefn_lp5523_init();
  if (_call_kernel_fn_ != 0) {
    postfn_lp5523_init(0, & __retres2);
    return (__retres2);
  }
  {
//#line  1042
  ret = i2c_add_driver(& lp5523_driver);
  }
//#line  1044
  if (ret < 0) {
    {
//#line  1045
    printk("<1>Adding lp5523 driver failed\n");
    }
  }
//#line  1047
  __retres2 = (int __attribute__((__cold__))  )ret;
  _call_kernel_fn_ = postfn_lp5523_init(1, & __retres2);
//#line  1038
  return (__retres2);
}
}
```

编译之后生成DRIVERNAME.ko文件（如lp5523-stub.ko)。
另外在dirver/kernel转换时，通过插桩来调用checkers（可选）进行bug的检测。
测试框架也是通过SymGen插桩来调用的。

### 6.4 SymDrive-S2E

首先加载测试框架，进行一些初始化的设置。  
然后加载SymGen处理后的driver，在加载过程中会启动符号执行，通过插桩调用checkers和测试框架来与符号化硬件和S2E plugin交互，并输出log信息。  
之后可以通过不同的entry points来测试driver，同样也会启动符号执行和插桩调用。  
最后卸载driver，使用符号执行和插桩调用检测remove过程中是否有bug。

## 7 Symdrive使用方法

### 7.1 Symdrive编译安装过程（以下步骤如没有明确指出在chroot jail下执行，则均在Ubuntu下执行）

#### 7.1.1 目录结构

```
~/s2e：根目录
     /symdrive/cil：SymGen源码及编译后的结果。
     /symdrive/debian32：debian32 chroot jail。用于编译Linux内核及在driver上运行SymGen。
     /symdrive/gtf：Generalized test framework（包括支持库和checkers）
     /symdrive/qemu：用于启动s2e及debian32 chroot。
	        /backup_qemu.sh
	        /debian32.sh：用于进入debian32 chroot。
	        /qemu.sh：用于简化s2e的使用。
	        /sendkey.py
	        /telnet.sh
	        /i386：存放VM镜像的地方
		  /s2e_disk_linux.qcow2：使用的镜像文件。
		  /s2e_disk_linux.qcow2.backup：用于恢复的镜像文件。
     /symdrive/test：存放想要测试的driver。
     /s2e：s2e源码（在某一版本上使用SymDrive patch得到的源码）。
     /build：S2E/symDrive编译生成的文件。
```

#### 7.1.2 安装S2E

1. 在~/s2e目录下执行`git clone https://dslabgit.epfl.ch/git/s2e/s2e.git`。
2. 然后在~/s2e/s2e下执行`git checkout -b SymDrive b5cfd33051c3be3ab254a5d3e9f4376861188c77`。
3. 下载symdrive.patch并执行`git apply symdrive.patch`。
4. 其余步骤同安装S2E的步骤。

#### 7.1.3 chroot jail

1. `sudo apt-get install debootstrap`
2. `mkdir ~/s2e/symdrive && mkdir ~/s2e/symdrive/debian32`
3. 在symdrive目录下` sudo debootstrap --arch i386 squeeze debian32/ http://mirror.switch.ch/ftp/mirror/debian/`
4. 将cil、gtf、test、qemu压缩文件在~/s2e/symdrive下解压缩，执行~/s2e/symdrive/qemu下的debian32.sh进入debian32 chroot。
5. 在jail中，执行`cd root`，然后`mkdir cil test gtf`。
6. 在jail中，执行`apt-get install build-essential kernel-package locales libncurses-dev`

#### 7.1.4 编译Linux 3.1.1 kernel

1. 进入chroot jail，创建/root/kernel目录。
2. 在该目录下解压kernel_files.tbz压缩文件。
3. 在该目录下解压linux-3.1.1.tar.bz2压缩文件。
4. 在该目录下执行`patch -p1 > ./linux_311.patch`。（补丁主要用于向S2E的log发送信息，在kernel中修改更容易）
5. 进入/root/kernel/linux-3.1.1目录并执行`chmod +x *.sh`，然后执行`./1_menuconfig.sh`。
6. 执行`./2_make.sh`。

#### 7.1.5 安装SymGen

1. 在chroot jail中执行`apt-get install ocaml`以及`apt-get install autoconf`。
2. 进入jail中/root/cil目录，执行`make configure`、`./configure`、`make`。
3. 在jail中，将/root/cil/bin添加到PATH中，使用`cilly --help`查看是否有`--dodrivers Enable device-driver analysis`这一行帮助信息出现，如果出现，则证明SymGen被正确安装。

#### 7.1.6 处理sample driver lp5523

1. 在chroot jail中/root/test下执行`./make.sh lp5523`
2. 查看/root/test/lp5523/output.txt文件尾部如果出现`All done`证明已经成功处理。

### 7.2 Symdrive使用方法

#### 7.2.1 镜像的使用

1. 在~/s2e/symdrive/qemu下执行`./qemu.sh 14159265358979323846`用于生成backup文件。
2. 执行`./qemu.sh 4982`来使用backup文件覆盖用到的镜像文件。

#### 7.2.2 执行S2E/SymDrive

1. 在~/s2e/symdrive/qemu下执行`./qemu.sh 4982 drivernumber 10`，其中lp5523为44。
2. 打开一个新终端，在同目录下执行`./debian32.sh`来启动chroot jail。
3. 在QEMU启动到登陆界面后，再新开一个终端，在同目录下执行`./qemu 1000`，将需要的文件复制进去，同时保存快照。
4. 保存好快照之后，在同目录下执行`./qemu.sh drivernumber 11`，进入到S2E-QEMU中。
5. 在S2E-QEMU中执行

```
1. insmod ./spi-bitbang.ko # This could be compiled into the kernel
2. insmod ./test_framework.ko g_i2c_enable=1 g_i2c_chip_addr=0x30 g_i2c_names=lp5523
3. insmod ./lp5523-stub.ko
```

### 7.3 Symdrive测试分析实例bug的log信息

//TODO 正在和作者联系如何分析出lp5523的bug