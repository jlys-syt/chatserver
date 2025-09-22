# 定义一个变量包含了当前目录下所有的源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件
add_executable(ChatClient ${SRC_LIST})

# 指定可执行文件链接时需要的依赖的库文件muduo_net muduo_base pthread
target_link_libraries(ChatClient pthread)