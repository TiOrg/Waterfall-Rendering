# Waterfall-Rendering
使用OpenGL实现的半环状瀑布动态场景，支持视角的移动。基于Mac OSX & Xcode & C++实现。



### 需要的库
GLFW: **G**raphics **L**ibrary **F**rame**w**ork--创建并管理窗口和 OpenGL 上下文

GLEW:  Open**GL** **E**xtension **W**rangler Library--基于OpenGL图形接口的跨平台C++扩展库

glm: Open**GL** **M**athematics--OpenGL数学计算扩展库

assimp: Open **As**set **Imp**ort Library--导入模型文件的扩展库

注：Mac OSX均可以直接用brew安装上述各库



### 实现效果

#### 包含元素

飞流而下的瀑布、环形山脉、起伏的水面、可移动的船、天空盒

####效果图

水平面-仰视视角

![bottom-view](/Users/simon/Desktop/waterfall-project/Waterfall-Rendering/material/readme-pics/bottom-view.png)



瀑布顶端-俯视视角

![up-view](/Users/simon/Desktop/waterfall-project/Waterfall-Rendering/material/readme-pics/up-view.png)



### TODOs

```
船随水面起伏的运动
水雾
山模型的修正
camera的固定
调参（颜色，位置，速度）
```

