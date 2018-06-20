# 多模式匹配python的C++扩展库

基于AC自动机算法

之后将会对AC自动机的实现使用双数组Trie进行优化

# 环境要求

C++11

python3.5+

# 用法

修改CPP文件中 Python.h 的路径，可以参照现有的

运行

<pre>
python setup.py build
</pre>

即可编译库源文件

生成的可调用库为 *.so* 结尾，在build目录中

可以直接在使用时将build目录加入到系统路径中：
<pre>
import sys
sys.path.append('./build/lib.linux-x86_64-3.6/')
</pre>

或者直接将生成的 *.so* 文件放入同级目录，即可直接调用

可以参见 *test.py* 文件