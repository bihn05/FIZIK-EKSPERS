# 编译器设置
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# 项目名称
TARGET = string
# 源文件
SRCS = main.cpp
# 目标文件
OBJS = $(SRCS:.cpp=.o)
# 依赖文件
DEPS = $(SRCS:.cpp=.d)

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 编译源文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# 包含依赖文件
-include $(DEPS)

# 清理编译生成的文件
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

# 重新编译
rebuild: clean all

# 运行程序
run: $(TARGET)
	./$(TARGET)

# 调试模式（包含调试信息）
debug: CXXFLAGS += -g -O0
debug: rebuild

.PHONY: all clean rebuild run